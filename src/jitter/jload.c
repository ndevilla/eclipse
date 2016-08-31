/*----------------------------------------------------------------------------*/
/**
   @file    jload.c
   @author
   @date    March 2002
   @version	$Revision: 1.37 $
   @brief   Jitter data loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jload.c,v 1.37 2004/02/09 16:11:04 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:11:04 $
	$Revision: 1.37 $
*/

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define NEGLIG_OFF_DIFF     0.1
#define SQR(x) ((x)*(x))

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "qfits.h"
#include "xmemory.h"
#include "pfits.h"
#include "eclipse.h"

#include "jtypes.h"
#include "jconfig.h"
#include "jini.h"

/*-----------------------------------------------------------------------------
  							Private functions
 -----------------------------------------------------------------------------*/

static int jitter_load_data(jitter_config_t * jc);
static int jitter_generic_load(jitter_config_t * jc);
static int jitter_isaac_chop_load(jitter_config_t * jc);
static int jitter_isaac_nochop_load(jitter_config_t * jc);
static int jitter_naco_load(jitter_config_t * jc);
static int jitter_loadoffsets(jitter_config_t * jc);
static int jitter_loadxcorrp(jitter_config_t * jc);
static int jitter_abba_classification(jitter_config_t *, int **, int **) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Load the data
  @param    ininame     Name of the input ini file
  @return   a jitter configuration object 
  This function creates a jitter configuration object that contains the data 
  loaded into it.
 */
/*----------------------------------------------------------------------------*/
jitter_config_t * jitter_load(char * ininame)
{
    jitter_config_t *   jc ;
    char            *   first_frame ;

    /* Create blank config */
    jc = jitter_config_new();

    /* Load ini file into it */
    e_comment(1, "parsing ini file...");
    if (jitter_ini_parse(ininame, jc)!=0) {
        jitter_config_del(jc);
        return NULL ;
    }
    
    /* Identify the data type */
    first_frame = framelist_firstname(jc->in_name) ;
    jc->data_type = pfits_identify_ins(first_frame);
   
    /* Load data into it */
    e_comment(1, "loading data...");
    if (jitter_load_data(jc)!=0) {
        jitter_config_del(jc);
        return NULL ;
    }
    jc->status_load = ALGO_OK ;
    return jc ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Data loader
  @param    jc      Jitter configuration object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_load_data(jitter_config_t * jc)
{
    int                 status ;

    /* Test inputs */
    if (jc==NULL) return -1 ;
    if (file_exists(jc->in_name)!=1) {
        e_error("cannot find file: %s", jc->in_name);
        return -1 ;
    }

    /* Input FITS file has to be written in an ASCII list */
    if (is_fits_file(jc->in_name)) {
        e_error("Write your FITS file name in an ASCII file") ;
        return -1 ;
    }
    
    /* Input file has to be an ASCII list */
    if (!is_ascii_list(jc->in_name)) {
        e_error("Jitter expects an ASCII list of frame(s)") ;
        return -1 ;
    }
    
    /* Check the instrument and call the loader accordingly */
    switch ((jc->data_type).ins) {
        case instrument_isaac:
            switch ((jc->data_type).mode) {
                case insmode_nochop:
                    e_comment(0, "ISAAC non-chopped data");
                    status = jitter_isaac_nochop_load(jc) ;
                    break ;
                case insmode_chop:
                    e_comment(0, "ISAAC chopped data");
                    status = jitter_isaac_chop_load(jc) ;
                    break ;
                default:
                    e_warning("Mode not recognized - use generic loader");
                    status = jitter_generic_load(jc) ;
                    break ;
            }
            break ;
        case instrument_naco:
            e_comment(0, "NACO data");
            status = jitter_naco_load(jc) ;
            break ;
        default:
            e_warning("Instrument not recognized - use generic loader");
            status = jitter_generic_load(jc) ;
            break ;
    }
    if (status == -1) {
        e_error("cannot load cube from frame list: %s", jc->in_name);
        return -1 ;
    }

    /* Load x-correlation places if needed */
    if (jitter_loadxcorrp(jc)!=0) return -1 ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for unidentified instrument 
  @param    jc  jitter_config object
  @return   0 if ok, -1 otherwise 
   
  The instrument used is not known.
  Each frame of the input ASCII list is loaded as an object frame.
  If the second column contains the word sky, it is loaded as a sky frame.
  The border specified (if specified) in the INI file are rejected.
 */
/*----------------------------------------------------------------------------*/
static int jitter_generic_load(jitter_config_t * jc)
{
    cube_t      *   loaded ;
    cube_t      *   cube_tmp ;
    framelist   *   flist ;
    char        *   ftype ;
    int             i ;

    /* Frame list in input */
    if ((flist = framelist_load(jc->in_name)) == NULL) {
        e_error("cannot load frame list: %s", jc->in_name);
        return -1 ;
    }

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return -1 ;
    }

    /* There may be some borders rejections */
    if (jc->zone.bottom || jc->zone.top || jc->zone.left || jc->zone.right){
        if ((cube_tmp = cube_getvig(loaded, 
                            jc->zone.left + 1,
                            jc->zone.bottom + 1,
                            loaded->lx - jc->zone.right,
                            loaded->ly - jc->zone.top)) == NULL) {
            e_warning("cannot reject the specified regions") ;
        } else {
            cube_del(loaded) ;
            loaded = cube_tmp ;
        }
    }

    /* Store the frames in the jitter_config object */
    jc->nframes = loaded->np ;
    jc->frame = calloc(jc->nframes, sizeof(jitter_frame_t));
    for (i=0 ; i<jc->nframes ; i++) {
        strcpy(jc->frame[i].name, flist->name[i]);
        jc->frame[i].pnum = 0 ;
        jc->frame[i].xtnum = 0 ;
        jc->frame[i].image = loaded->plane[i];
        loaded->plane[i] = NULL ;
        jc->frame[i].docatg = NULL ;
    }
    jc->lx = loaded->lx ;
    jc->ly = loaded->ly ;
    jc->total_pixin = (long)loaded->lx *
                      (long)loaded->ly *
                      (long)loaded->np ;
    cube_del_shallow(loaded);

    /* Initialize default: no sky frames are present */
    jc->sky_ispresent=0 ;
   
    /* Identify frame types */
    if (flist->type!=NULL) {
        for (i=0 ; i<jc->nframes ; i++) {
            jc->frame[i].type = type_obj ;
            if (flist->type[i]!=NULL) {
                jc->frame[i].docatg = strdup(flist->type[i]);
                ftype = strlwc(flist->type[i]);
                if (strstr(ftype, "sky")) {
                    jc->frame[i].type = type_sky ;
                    jc->sky_ispresent=1 ;
                }
            }
        }
    }
    
    /* Load x-correlation offsets if needed */
    if (jitter_loadoffsets(jc)!=0) return -1 ;

    /* Free and return */
    framelist_del(flist) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for isaac instrument in no-chopping mode 
  @param    jc  jitter_config object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_isaac_nochop_load(jitter_config_t * jc)
{
    cube_t      *   loaded ;
    cube_t      *   cube_tmp ;
    char        *   value ;
    framelist   *   flist ;
    char        *   ftype ;

    int             i ;

    /* Frame list in input */
    if ((flist = framelist_load(jc->in_name)) == NULL) {
        e_error("cannot load frame list: %s", jc->in_name);
        return -1 ;
    }

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return -1 ;
    }
   
    /* There may be some borders rejections */
    if (jc->zone.bottom || jc->zone.top || jc->zone.left || jc->zone.right){
        if ((cube_tmp = cube_getvig(loaded, 
                            jc->zone.left + 1,
                            jc->zone.bottom + 1,
                            loaded->lx - jc->zone.right,
                            loaded->ly - jc->zone.top)) == NULL) {
            e_warning("cannot reject the specified regions") ;
        } else {
            cube_del(loaded) ;
            loaded = cube_tmp ;
        }
    }

    /* Store the frames in the jitter_config object */
    jc->nframes = loaded->np ;
    jc->frame = calloc(jc->nframes, sizeof(jitter_frame_t));
    for (i=0 ; i<jc->nframes ; i++) {
        strcpy(jc->frame[i].name, flist->name[i]);
        jc->frame[i].pnum = 0 ;
        jc->frame[i].xtnum = 0 ;
        jc->frame[i].docatg = NULL ;
        jc->frame[i].image = loaded->plane[i];
        loaded->plane[i] = NULL ;
    }
    jc->lx = loaded->lx ;
    jc->ly = loaded->ly ;
    jc->total_pixin = (long)loaded->lx * (long)loaded->ly * (long)loaded->np ;
    cube_del_shallow(loaded);

    /* Initialize default: no sky frames are present */
    jc->sky_ispresent=0 ;
   
    /* Identify frame types */
    if (flist->type!=NULL) {
        for (i=0 ; i<jc->nframes ; i++) {
            jc->frame[i].type = type_obj ;
            if (flist->type[i]!=NULL) {
                jc->frame[i].docatg = strdup(flist->type[i]);
                ftype = strlwc(flist->type[i]);
                if (strstr(ftype, "sky")) {
                    jc->frame[i].type = type_sky ;
                    jc->sky_ispresent=1 ;
                }
            }
        }
    }
    
    /* Check if data are of type INT. If not, reject it */
    for (i=0 ; i<flist->n ; i++) {
        if ((value = pfits_get(jc->data_type, flist->name[i], 
                        "detector_frame_type")) == NULL) {
            e_warning("cannot read DET FRAM TYPE") ;
            jc->frame[i].type = type_rej ;
        } else {
            if (strcmp(qfits_pretty_string(value), "INT")) {
                e_warning("Expected frame type is INT") ;
                jc->frame[i].type = type_rej ;
            }
        }
    }
    framelist_del(flist) ;

    /* Load x-correlation offsets if needed */
    if (jitter_loadoffsets(jc)!=0) return -1 ;

    /* Return */
    return 0  ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for isaac instrument in chopping mode
  @param    jc  jitter_config object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_isaac_chop_load(jitter_config_t * jc)
{
    cube_t      *   loaded ;
    cube_t      *   new_cube ;
    cube_t      *   cube_tmp ;
    char        *   value ;
    char        *   pvalue ;
    framelist   *   flist ;
    char        *   ftype ;
    int         *   chop_a ;
    int         *   chop_b ;
    int             nb_chop ;

    int             i, j ;

    /* Frame list in input */
    if ((flist = framelist_load(jc->in_name)) == NULL) {
        e_error("cannot load frame list: %s", jc->in_name);
        return -1 ;
    }

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return -1 ;
    }
 
    /* There may be some borders rejections */
    if (jc->zone.bottom || jc->zone.top || jc->zone.left || jc->zone.right){
        if ((cube_tmp = cube_getvig(loaded, 
                            jc->zone.left + 1,
                            jc->zone.bottom + 1,
                            loaded->lx - jc->zone.right,
                            loaded->ly - jc->zone.top)) == NULL) {
            e_warning("cannot reject the specified regions") ;
        } else {
            cube_del(loaded) ;
            loaded = cube_tmp ;
        }
    }
   
    /* Create the new cube with differences for CUBE1 type */
    new_cube = cube_new(loaded->lx, loaded->ly, flist->n) ;
    j = 0 ;
    for (i=0 ; i<flist->n ; i++) {
        if ((value = pfits_get(jc->data_type, flist->name[i], 
                        "detector_frame_type")) == NULL) {
            e_error("cannot read DET FRAME TYPE") ;
            cube_del(loaded) ;
            cube_del(new_cube) ;
            framelist_del(flist);
            return -1 ;
        } else {
            if (!strcmp(qfits_pretty_string(value), "CUBE1")) {
                /* cubes : frame1 = frame1-frame2, remove frame2 */
                image_sub_local(loaded->plane[j], loaded->plane[j+1]) ;
                image_del(loaded->plane[j+1]) ;
                loaded->plane[j+1] = NULL ;
                new_cube->plane[i] = loaded->plane[j] ;
                loaded->plane[j] = NULL ;
                j += 2 ;
            } else { 
                /* Just copy normal planes */
                new_cube->plane[i] = loaded->plane[j] ;
                loaded->plane[j] = NULL ;
                j++ ;
            }
        }
    }
    cube_del_shallow(loaded) ;

    /* Store the frames in the jitter_config object */
    jc->nframes = new_cube->np ;
    jc->frame = calloc(jc->nframes, sizeof(jitter_frame_t));
    for (i=0 ; i<jc->nframes ; i++) {
        strcpy(jc->frame[i].name, flist->name[i]);
        jc->frame[i].pnum = 0 ;
        jc->frame[i].xtnum = 0 ;
        jc->frame[i].image = new_cube->plane[i];
        new_cube->plane[i] = NULL ;
        jc->frame[i].docatg = NULL ;
    }
    jc->lx = new_cube->lx ;
    jc->ly = new_cube->ly ;
    jc->total_pixin = (long)new_cube->lx *
                      (long)new_cube->ly *
                      (long)new_cube->np ;
    cube_del_shallow(new_cube);

    /* Identify frame type sky */
    /* Initialize default: no sky frames are present */
    jc->sky_ispresent=0 ;
    if (flist->type!=NULL) {
        for (i=0 ; i<jc->nframes ; i++) {
            jc->frame[i].type = type_obj ;
            if (flist->type[i]!=NULL) {
                jc->frame[i].docatg = strdup(flist->type[i]);
                ftype = strlwc(flist->type[i]);
                if (strstr(ftype, "sky")) {
                    jc->frame[i].type = type_sky ;
                    jc->sky_ispresent=1 ;
                }
            }
        }
    }
    framelist_del(flist) ;

    /* First identify Half-Cycle frames and label them as such */
    for (i=0 ; i<jc->nframes ; i++) {
        value = pfits_get(jc->data_type, jc->frame[i].name, 
                "detector_frame_type");
        pvalue = qfits_pretty_string(value);
        if ((strcmp(pvalue, "INT")) && (strcmp(pvalue, "CUBE1"))) {
            jc->frame[i].type = type_hc ;
        }
    }

    /* Load x-correlation offsets if needed */
    if (jitter_loadoffsets(jc)!=0) return -1 ;

    /* Classify chop_a and chop_b */
    if ((nb_chop=jitter_abba_classification(jc, &chop_a, &chop_b)) == -1) {
        e_error("cannot classify chopped frames") ;
        return -1 ;
    }
    
    /* Compute subtractions (chop_a-chaop_b)/2 */
    for (i=0 ; i<nb_chop ; i++) {
        image_sub_local(jc->frame[chop_a[i]].image, jc->frame[chop_b[i]].image);
        image_cst_op_local(jc->frame[chop_a[i]].image, 2.0, '/') ;
        jc->frame[chop_b[i]].type = type_subtracted ;
    }
   
    /* Free */
    free(chop_a) ;
    free(chop_b) ;

    /* Return */
    return 0  ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for naco instrument
  @param    jc  jitter_config object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_naco_load(jitter_config_t * jc)
{
    cube_t      *   loaded ;
    cube_t      *   cube_tmp ;
    framelist   *   flist ;
    char        *   ftype ;
    int             i ;

    /* Frame list in input */
    if ((flist = framelist_load(jc->in_name)) == NULL) {
        e_error("cannot load frame list: %s", jc->in_name);
        return -1 ;
    }

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return -1 ;
    }

    /* There may be some borders rejections */
    if (jc->zone.bottom || jc->zone.top || jc->zone.left || jc->zone.right){
        if ((cube_tmp = cube_getvig(loaded, 
                            jc->zone.left + 1,
                            jc->zone.bottom + 1,
                            loaded->lx - jc->zone.right,
                            loaded->ly - jc->zone.top)) == NULL) {
            e_warning("cannot reject the specified regions") ;
        } else {
            cube_del(loaded) ;
            loaded = cube_tmp ;
        }
    }

    /* Store the frames in the jitter_config object */
    jc->nframes = loaded->np ;
    jc->frame = calloc(jc->nframes, sizeof(jitter_frame_t));
    for (i=0 ; i<jc->nframes ; i++) {
        strcpy(jc->frame[i].name, flist->name[i]);
        jc->frame[i].pnum = 0 ;
        jc->frame[i].xtnum = 0 ;
        jc->frame[i].image = loaded->plane[i];
        loaded->plane[i] = NULL ;
        jc->frame[i].docatg = NULL ;
    }
    jc->lx = loaded->lx ;
    jc->ly = loaded->ly ;
    jc->total_pixin = (long)loaded->lx *
                      (long)loaded->ly *
                      (long)loaded->np ;
    cube_del_shallow(loaded);

    /* Initialize default: no sky frames are present */
    jc->sky_ispresent=0 ;
   
    /* Identify frame types */
    if (flist->type!=NULL) {
        for (i=0 ; i<jc->nframes ; i++) {
            jc->frame[i].type = type_obj ;
            if (flist->type[i]!=NULL) {
                jc->frame[i].docatg = strdup(flist->type[i]);
                ftype = strlwc(flist->type[i]);
                if (strstr(ftype, "sky")) {
                    jc->frame[i].type = type_sky ;
                    jc->sky_ispresent=1 ;
                }
            }
        }
    }

    /* Load x-correlation offsets if needed */
    if (jitter_loadoffsets(jc)!=0) return -1 ;

    /* Return */
    framelist_del(flist) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Offset loading
  @param    jc      Jitter configuration object
  @return   0 if ok -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_loadoffsets(jitter_config_t * jc)
{
    int                 i ;
    char            *   xval ;
    char            *   yval ;
    double3         *   offs ;

    if (jc->saa_active!=1) return 0 ;

    switch (jc->saa_offsource) {
        
        case offsource_header:
            /* Get offsets */
            for (i=0 ; i<jc->nframes ; i++) {
                xval=pfits_get(jc->data_type, jc->frame[i].name, "cumoffsetx");
                yval=pfits_get(jc->data_type, jc->frame[i].name, "cumoffsety");
                /* Check errors */
                if (xval==NULL || yval==NULL) {
                    e_error("cannot get offset info for frame %s\n"
                            "try changing one of the following:\n"
                            "- Instrument (currently [%s])\n"
                            "- Offset source (e.g. file)",
                            jc->frame[i].name,
                            jconv_ins(jc->data_type));
                    return -1 ;
                }
                /* Set offset info in config */
                jc->frame[i].off_x = (double)atof(xval);
                jc->frame[i].off_y = (double)atof(yval);
            }
            break ;

        case offsource_file:
            /* Load offsets from text file */
            offs = load_offsets_from_txtfile(jc->saa_offfilename);
            if (offs==NULL) {
                e_error("cannot load offsets: aborting");
                return -1 ;
            }
            if (offs->n != jc->nframes) {
                e_error("inconsistency: got %d planes from %s\n"
                        "               got %d offsets from %s",
                        jc->nframes,
                        jc->in_name,
                        offs->n,
                        jc->saa_offfilename);
                return -1 ;
            }
            for (i=0 ; i<jc->nframes ; i++) {
                jc->frame[i].off_x = offs->x[i];
                jc->frame[i].off_y = offs->y[i];
            }
            double3_del(offs);
            break ;

        case offsource_blind:
            /* Do nothing */
            break ;

        default:
            e_error("Unrecognized Offsets source") ;
            return -1 ;
    }

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    x-correlation places loading
  @param    jc      Jitter configuration object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_loadxcorrp(jitter_config_t * jc)
{
    double3 *   xcorrp ;
    int         i ;

    /* Check if the list of objects has to be loaded */
    if (jc->saa_active!=1)
        return 0 ;
    if (jc->saa_objsource != objsource_file)
        return 0 ;

    /* Load objects from user file */
    e_comment(2, "loading user objects from file: %s", jc->saa_objfile);
    if (file_exists(jc->saa_objfile)!=1) {
        e_error("cannot read %s", jc->saa_objfile);
        return -1 ;
    }
    /* Read file in using double3_read() */
    xcorrp = double3_read(jc->saa_objfile);
    if (xcorrp==NULL) {
        e_error("reading list of x-correlation objects from file %s",
                jc->saa_objfile);
        return -1 ;
    }
    /* Allocate storage into config */
    jc->saa_xcorrp_n = xcorrp->n ;
    jc->saa_xcorrp_x = malloc(xcorrp->n*sizeof(double));
    jc->saa_xcorrp_y = malloc(xcorrp->n*sizeof(double));

    /* Copy data into blackboard */
    for (i=0 ; i<xcorrp->n ; i++) {
        jc->saa_xcorrp_x[i] = xcorrp->x[i] ;
        jc->saa_xcorrp_y[i] = xcorrp->y[i] ;
    }
    /* Deallocate temporary double3 */
    double3_del(xcorrp);
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Classification of frames in chopping mode
  @param    jc      Jitter configuration object
  @param    chop_a  index array giving the positions of chopA frames
  @param    chop_b  index array giving the positions of chopB frames
  @return   Number of chop a frames (= nb of chop b frames) 
 */
/*----------------------------------------------------------------------------*/
static int jitter_abba_classification(
        jitter_config_t *   jc,
        int             **   chop_a,
        int             **   chop_b)
{
    int         nb_obj ;
    double      throw_x, 
                throw_y ;
    double  *   dist ;
    int     *   dcount ;
    double      sel_dist ;
    int         max_count ;
    int         i_max,
                j_max ;
    int         classified ;
    int         i, j, k, l ;

    /* Find number of type_obj frames */
    nb_obj = 0 ;
    for (i=0 ; i<jc->nframes ; i++) if (jc->frame[i].type == type_obj) nb_obj++;
    
    /* If no object frame, exit */
    if (nb_obj == 0) { 
        e_error("cannot find object frames") ;
        return -1 ;
    }
    
    /* Odd number of frames ? */
    if (nb_obj%2) e_warning("odd number of frames in input [%d]", nb_obj) ;
    
    /* Find all distances between offsets and count them */
    dist    = calloc(nb_obj*nb_obj, sizeof(double)) ;
    dcount  = calloc(nb_obj*nb_obj, sizeof(int)) ;
    k = 0 ;
    for (i=0 ; i<jc->nframes ; i++) {
        l = 0 ;
        if (jc->frame[i].type == type_obj) {
             for (j=0 ; j<jc->nframes ; j++) {
                 if (jc->frame[j].type == type_obj) {
                     dist[k+l*nb_obj] = 
                         sqrt(SQR(jc->frame[i].off_x - jc->frame[j].off_x) +
                                 SQR(jc->frame[i].off_y - jc->frame[j].off_y)) ;
                     l++ ;
                 }
             }
             k++ ;
        }
    }

    for (i=0 ; i<nb_obj*nb_obj ; i++) {
        sel_dist = dist[i] ;
        for (j=0 ; j<nb_obj*nb_obj ; j++)
            if (fabs(sel_dist-dist[j]) <= NEGLIG_OFF_DIFF) dcount[i] ++ ;
    }
    
    /* Find Chop offsets as the nonzero distance with maximal occurrence */
    k = 0 ;
    max_count = 0 ;
    i_max     = 0 ;
    j_max     = 0 ;
    for (i=0 ; i<jc->nframes ; i++){
        l = 0 ;
        if (jc->frame[i].type == type_obj) {
            for (j=0 ; j<jc->nframes ; j++) {
                if (jc->frame[j].type == type_obj) {
                    if ((dcount[k+l*nb_obj]>max_count)&&(dist[k+l*nb_obj]>0.5)){
                        max_count = dcount[k+l*nb_obj] ;
                        i_max = i ;
                        j_max = j ;
                    }
                    l++ ;
                }
            }
            k++ ;
        }
    }

    /* Free */
    free(dist) ;
    free(dcount) ;

    /* Compute the throw */
    throw_x = jc->frame[j_max].off_x - jc->frame[i_max].off_x ;
    throw_y = jc->frame[j_max].off_y - jc->frame[i_max].off_y ;
    
    /* Test the throw */
    if ((throw_x == 0) && (throw_y == 0)) {
        e_error("Throw is equal to 0 - cannot classify") ;
        return -1 ;
    }

    /* Allocate maximal possible nb of chop frames (worst case: abab)*/
    *chop_a = calloc(nb_obj/2, sizeof(int)) ;
    *chop_b = calloc(nb_obj/2, sizeof(int)) ;

    k = l = 0 ;
    for (i=0 ; i<jc->nframes ; i++) {
        if (jc->frame[i].type == type_obj) {
            for (j=i+1 ; j<jc->nframes ; j++) {
                if (jc->frame[j].type == type_obj) {
                    /* Consider the successive pairs of frames */
                    /* AB ?  */
                    if ((fabs(jc->frame[i].off_x-jc->frame[j].off_x+throw_x) 
                                < NEGLIG_OFF_DIFF) &&
                        (fabs(jc->frame[i].off_y-jc->frame[j].off_y+throw_y) 
                                < NEGLIG_OFF_DIFF)) {
                        (*chop_a)[k++] = i ;
                        (*chop_b)[l++] = j ;
                    /* BA ? */
                    } else if ((fabs(jc->frame[i].off_x-jc->frame[j].off_x
                                    -throw_x) < NEGLIG_OFF_DIFF) &&
                            (fabs(jc->frame[i].off_y-jc->frame[j].off_y
                                  -throw_y) < NEGLIG_OFF_DIFF)) {
                        (*chop_b)[l++] = i ;
                        (*chop_a)[k++] = j ;
                    /* AA or BB ? -> do nothing */
                    } 
                    break ;
                }
            }
        }
    }

    /* Classify the frames not seen until here as rejected */
    for (i=0 ; i<jc->nframes ; i++) {
        if (jc->frame[i].type == type_obj) {
            classified = 0 ;
            /* For each frame, check if it has been classified */
            for (j=0 ; j<nb_obj ; j++) {
                if ((*chop_a)[j] == i || (*chop_b)[j] == i) classified = 1 ;
            }
            if (classified == 0) jc->frame[i].type = type_rej ;
        }
    }

    /* There should be as much chop a as chop b frames */
    if (k != l) {
        free(*chop_a) ;
        free(*chop_b) ;
        return -1 ;
    }
    return k ;
}


