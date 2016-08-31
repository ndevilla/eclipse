/*----------------------------------------------------------------------------*/
/**
   @file    spjload.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.8 $
   @brief   Spectroscopic jitter data loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjload.c,v 1.8 2003/04/22 08:01:44 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/22 08:01:44 $
	$Revision: 1.8 $
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

#include "spjtypes.h"
#include "spjconfig.h"
#include "spjini.h"

/*-----------------------------------------------------------------------------
  							Private functions
 -----------------------------------------------------------------------------*/

static int spjitter_load_data(spjitter_config_t * spjc);
static int spjitter_generic_load(spjitter_config_t * spjc);
static int spjitter_chop_load(spjitter_config_t * spjc);
static cube_t * spjitter_chop_load_strings(framelist * flist, instrument_t);
static int spjitter_nochop_load(spjitter_config_t * spjc);
static int spjitter_loadoffsets(spjitter_config_t * spjc) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Load the data
  @param    ininame     Name of the input ini file
  @return   a spectroscopic jitter configuration object 
  This function creates a spjitter configuration object that contains the data 
  loaded into it.
 */
/*----------------------------------------------------------------------------*/
spjitter_config_t * spjitter_load(char * ininame)
{
    spjitter_config_t   *   spjc ;
    char                *   first_frame ;

    /* Create blank config */
    spjc = spjitter_config_new();

    /* Load ini file into it */
    e_comment(1, "parsing ini file...");
    if (spjitter_ini_parse(ininame, spjc)!=0) {
        spjitter_config_del(spjc);
        return NULL ;
    }
    
    /* Identify the data type */
    first_frame = framelist_firstname(spjc->in_name) ;
    spjc->data_type = pfits_identify_ins(first_frame);
    
    /* Load data into it */
    e_comment(1, "loading data...");
    if (spjitter_load_data(spjc)!=0) {
        spjitter_config_del(spjc);
        return NULL ;
    }
    spjc->status_load = OK ;
    return spjc ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Data loader
  @param    spjc    Spectroscopic jitter configuration object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_load_data(spjitter_config_t * spjc)
{
    int                 status ;

    /* Test inputs */
    if (spjc==NULL) return -1 ;
    if (file_exists(spjc->in_name)!=1) {
        e_error("cannot find file: %s", spjc->in_name);
        return -1 ;
    }

    /* Input FITS file has to be written in an ASCII list */
    if (is_fits_file(spjc->in_name)) {
        e_error("Write your FITS file name in an ASCII file") ;
        return -1 ;
    }
    
    /* Input file has to be an ASCII list */
    if (!is_ascii_list(spjc->in_name)) {
        e_error("spjitter expects an ASCII list of frame(s)") ;
        return -1 ;
    }
    
    /* Check the instrument and call the loader accordingly */
    switch ((spjc->data_type).ins) {
        case instrument_isaac:
            switch ((spjc->data_type).mode) {
                case insmode_nochop:
                    e_comment(0, "Non-chopped data");
                    status = spjitter_nochop_load(spjc) ;
                    break ;
                case insmode_chop:
                    e_comment(0, "Chopped data");
                    status = spjitter_chop_load(spjc) ;
                    break ;
                default:
                    e_warning("Mode not recognized - use algorithm mode");
                    switch ((spjc->algo).mode) {
                        case insmode_nochop:
                            e_comment(0, "Non-chopped data");
                            status = spjitter_nochop_load(spjc) ;
                            break ;
                        case insmode_chop:
                            e_comment(0, "Chopped data");
                            status = spjitter_chop_load(spjc) ;
                            break ;
                        default:
                            e_warning("Mode not recognized - use gen. loader");
                            status = spjitter_generic_load(spjc) ;
                            break ;
                    }
                    break ;
            }
            break ;
        default:
            e_warning("Instrument not recognized - use generic loader");
            status = spjitter_generic_load(spjc) ;
            break ;
    }
    if (status == -1) {
        e_error("cannot load cube from frame list: %s", spjc->in_name);
        return -1 ;
    }

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for unidentified instrument 
  @param    spjc    spectroscopic spjitter_config object
  @return   0 if ok, -1 otherwise 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_generic_load(spjitter_config_t * spjc)
{
    return -1 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for no-chopping mode images
  @param    spjc  spjitter_config object
  @return   0 if ok, -1 otherwise
  One frame per file expected. 
  Valid frame : DPR.TYPE = OBJECT or STD
 */
/*----------------------------------------------------------------------------*/
static int spjitter_nochop_load(spjitter_config_t * spjc)
{
    cube_t      *   loaded ;
    char        *   value ;
    framelist   *   flist ;
    int             i ;

    /* Frame list in input */
    if ((flist = framelist_load(spjc->in_name)) == NULL) {
        e_error("cannot load frame list: %s", spjc->in_name);
        return -1 ;
    }

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return -1 ;
    }

    /* Store the frames in the spjitter_config object */
    spjc->nframes = spjc->nobjframes = loaded->np ;
    spjc->frame = calloc(spjc->nframes, sizeof(spjitter_frame_t));
    for (i=0 ; i<spjc->nframes ; i++) {
        strcpy(spjc->frame[i].name, flist->name[i]);
        spjc->frame[i].pnum = 0 ;
        spjc->frame[i].xtnum = 0 ;
        if (flist->type != NULL) { 
            if (flist->type[i] != NULL) 
                spjc->frame[i].docatg = strdup(flist->type[i]);
            else spjc->frame[i].docatg = NULL ; 
        } else spjc->frame[i].docatg = NULL ;
        spjc->frame[i].image = loaded->plane[i];
        loaded->plane[i] = NULL ;
    }
    spjc->lx = loaded->lx ;
    spjc->ly = loaded->ly ;
    spjc->total_pixin = (long)loaded->lx * (long)loaded->ly * (long)loaded->np ;
    framelist_del(flist) ;
    cube_del_shallow(loaded);

    /* Reject non-science frames */
    for (i=0 ; i<spjc->nframes ; i++) {
        value = pfits_get(spjc->data_type, spjc->frame[i].name, "dpr_type") ;
        if (value == NULL) {
            e_warning("cannot read DPR TYPE") ;
            spjc->frame[i].type = type_rej ;
            spjc->nobjframes -- ;
        } else {
            if ((strcmp(value, "OBJECT")) && (strcmp(value, "STD"))) {
               spjc->frame[i].type = type_rej ;
               spjc->nobjframes -- ;
            }
        }
    }

    /* Load the frame containing the sky lines (used for wl and distortion) */
    for (i=0 ; i<spjc->nframes ; i++) {
        if (spjc->frame[i].type == type_obj) {
            spjc->sky_lines = image_load(spjc->frame[i].name) ;
            break ;
        }
    }
    
    /* Load x-correlation offsets */
    if (spjitter_loadoffsets(spjc) != 0) return -1 ;

    /* Return */
    return 0  ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Load data set for chopping mode images
  @param    spjc  spjitter_config object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_chop_load(spjitter_config_t * spjc)
{
    cube_t      *   loaded ;
    char        *   value ;
    framelist   *   flist ;
    int             i ;

    /* Frame list in input */
    if ((flist = framelist_load(spjc->in_name)) == NULL) {
        e_error("cannot load frame list: %s", spjc->in_name);
        return -1 ;
    }

    /* Load input images in a cube */
    if ((loaded = spjitter_chop_load_strings(flist, spjc->data_type)) == NULL) {
        e_error("cannot load the cube") ;
        return -1 ;
    }

    /* Store the frames in the spjitter_config object */
    spjc->nframes = spjc->nobjframes = loaded->np ;
    spjc->frame = calloc(spjc->nframes, sizeof(spjitter_frame_t));
    for (i=0 ; i<spjc->nframes ; i++) {
        strcpy(spjc->frame[i].name, flist->name[i]);
        spjc->frame[i].pnum = 0 ;
        spjc->frame[i].xtnum = 0 ;
        if (flist->type != NULL) { 
            if (flist->type[i] != NULL) 
                spjc->frame[i].docatg = strdup(flist->type[i]);
            else spjc->frame[i].docatg = NULL ; 
        } else spjc->frame[i].docatg = NULL ;
        spjc->frame[i].image = loaded->plane[i];
        loaded->plane[i] = NULL ;
    }
    spjc->lx = loaded->lx ;
    spjc->ly = loaded->ly ;
    spjc->total_pixin = (long)loaded->lx * (long)loaded->ly * (long)loaded->np ;
    framelist_del(flist) ;
    cube_del_shallow(loaded);

    /* Reject non-science frames */
    for (i=0 ; i<spjc->nframes ; i++) {
        value = pfits_get(spjc->data_type, spjc->frame[i].name, "dpr_type") ;
        if (value == NULL) {
            e_warning("cannot read DPR TYPE") ;
            spjc->frame[i].type = type_rej ;
            spjc->nobjframes -- ;
        } else {
            if ((strcmp(value, "OBJECT")) && (strcmp(value, "STD"))) {
               spjc->frame[i].type = type_rej ;
               spjc->nobjframes -- ;
            }
        }
    }

    /* Reject half-cycle frames */
    for (i=0 ; i<spjc->nframes ; i++) {
        value = pfits_get(spjc->data_type, spjc->frame[i].name, 
                "detector_frame_type") ;
        if (value == NULL) {
            e_warning("cannot read FRAME TYPE") ;
            spjc->frame[i].type = type_hc ;
            spjc->nobjframes -- ;
        } else {
            if ((strcmp(value, "INT")) && (strcmp(value, "CUBE1"))) {
               spjc->frame[i].type = type_hc ;
               spjc->nobjframes -- ;
            }
        }
    }

    /* Load the frame containing the sky lines (used for wl and distortion) */
    /* No sky lines in LW */
    spjc->sky_lines = NULL ;

    /* Load x-correlation offsets */
    if (spjitter_loadoffsets(spjc) != 0) return -1 ;

    /* Return */
    return 0  ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Load a list of files into a cube.
  @param    flist   Name of framelist to load
  @param    type    Data type to use with pfits_get()
  @return   1 pointer to newly allocated cube.
  This function hides the fact that ISAAC LW data may come as list of 
  single-frame or double-frame (NAXIS3=2) files. If the input list designates 
  single-frame files, they are all loaded into a cube. If the list designates 
  double-frame files, each pair of frame is loaded, frame 2 subtracted from 
  frame 1 and the result stored into the returned cube.
 */
/*----------------------------------------------------------------------------*/
static cube_t * spjitter_chop_load_strings(
        framelist   *   flist,
        instrument_t    type)
{
    cube_t      *   loaded ;
    cube_t      *   new_cube ;
    int         *   types ;
    char        *   value ;
    int             i, j ;

    /* Check input parameter */
    if (flist==NULL) return NULL ;

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return NULL ;
    }

    /* Allocate types array */
    types = calloc(flist->n, sizeof(int)) ;

    /* Update types array : 0 unknown, 1 for INT, 2 for CUBE1 */
    for (i=0 ; i<flist->n ; i++) {
        if ((value = pfits_get(type, flist->name[i],
                        "detector_frame_type")) == NULL) {
            e_error("cannot read DET FRAME TYPE") ;
            free(types) ;
            cube_del(loaded) ;
            return NULL ;
        } else {
            if (!strcmp(qfits_pretty_string(value), "INT"))        types[i] = 1;
            else if (!strcmp(qfits_pretty_string(value), "CUBE1")) types[i] = 2;
            else {
                e_warning("Expected frame types are INT or CUBE1") ;
                types[i] = 0 ;
            }
        }
    }

    /* Create the new cube with differences for CUBE1 type */
    new_cube = cube_new(loaded->lx, loaded->ly, flist->n) ;
    j = 0 ;
    for (i=0 ; i<flist->n ; i++) {
        if (types[i] == 0) {
            new_cube->plane[i] = loaded->plane[j] ;
            loaded->plane[j] = NULL ;
            j++ ;
        } else if (types[i] == 1) {
            new_cube->plane[i] = loaded->plane[j] ;
            loaded->plane[j] = NULL ;
            j++ ;
        } else if (types[i] == 2) {
            image_sub_local(loaded->plane[j], loaded->plane[j+1]) ;
            image_del(loaded->plane[j+1]) ;
            loaded->plane[j+1] = NULL ;
            new_cube->plane[i] = loaded->plane[j] ;
            loaded->plane[j] = NULL ;
            j += 2 ;
        }
    }

    /* Free and return */
    free(types) ;
    cube_del_shallow(loaded) ;
    return new_cube ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Offset loading
  @param    spjc    Spjitter configuration object
  @return   0 if ok -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_loadoffsets(spjitter_config_t * spjc)
{
    char            *   val ;
    double3         *   offs ;
    int                 i ;

    switch (spjc->offsets_source) {
        case offsets_header:
            /* Get offsets */
            for (i=0 ; i<spjc->nframes ; i++) {
                val=pfits_get(spjc->data_type,spjc->frame[i].name,"cumoffsety");
                if (val==NULL) {
                    e_error("cannot get offset info for frame %s\n"
                            "try changing one of the following:\n"
                            "- Instrument (currently [%s])\n"
                            "- Offset source (e.g. file)",
                            spjc->frame[i].name,
                            spjconv_ins(spjc->data_type));
                    return -1 ;
                }
                /* Set offset info in config */
                spjc->frame[i].offset = (double)atof(val);
            }
            break ;

        case offsets_file:
            /* Load offsets from text file */
            offs = load_offsets_from_txtfile(spjc->offsets_file);
            if (offs==NULL) {
                e_error("cannot load offsets: aborting");
                return -1 ;
            }
            if (offs->n != spjc->nframes) {
                e_error("inconsistency: got %d planes from %s\n"
                        "               got %d offsets from %s",
                        spjc->nframes,
                        spjc->in_name,
                        offs->n,
                        spjc->offsets_file);
                return -1 ;
            }
            for (i=0 ; i<spjc->nframes ; i++) {
                spjc->frame[i].offset = offs->x[i];
            }
            double3_del(offs);
            break ;

        case offsets_blind:
            /* Do nothing */
            break ;

        default:
            e_error("Unrecognized Offsets source") ;
            return -1 ;
    }
    return 0 ;
}


