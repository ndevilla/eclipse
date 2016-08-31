/*----------------------------------------------------------------------------*/
/**
   @file    jsky.c
   @author
   @date    March 2002
   @version	$Revision: 1.22 $
   @brief   Jitter sky estimation/subtraction
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jsky.c,v 1.22 2003/08/12 13:17:51 yjung Exp $
	$Author: yjung $
	$Date: 2003/08/12 13:17:51 $
	$Revision: 1.22 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"

#include "jtypes.h"
#include "jconfig.h"
#include "jsave.h"
#include "jload.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
   								Functions prototype
 -----------------------------------------------------------------------------*/

static void jitter_sky_output(jitter_config_t *) ;

/*-----------------------------------------------------------------------------
   								Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter sky estimation and correction
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 if error occurred.
 */
/*----------------------------------------------------------------------------*/
int jitter_sky(jitter_config_t * jc)
{
    cube_t      *   cube ;
    double      *   background ;
    int         *   selection ;
    image_t     *   sky ;
    double          bg_val ;
    int             status ;
    int             i ;
   
    /* Test if the sky correction is requested or not */
    if (jc->sky_active == 0) {
        jc->status_sky = ALGO_SKIPPED ;
        return 0 ;
    }

    /*
     * Check which sky filtering method has been requested
     */
    switch (jc->sky_method) {

        case skymethod_auto:
        /* In automatic method case, decide which method has to be used  */
        if (jc->nframes < jc->skyfilter_minframes || jc->sky_ispresent) {
            jc->sky_method_used = skymethod_medianframe ;
        } else {
            jc->sky_method_used = skymethod_combine_mc ;
        }
        break ;

        case skymethod_medianframe:
        /* Median frame filtering is always possible */
        jc->sky_method_used = skymethod_medianframe ;
        break ;

        case skymethod_combine:
        if (jc->nframes < jc->skyfilter_minframes) {
            e_error("not enough frames to use sky combination (%d<%d)",
                    jc->nframes,
                    jc->skyfilter_minframes);
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }
        if (jc->sky_ispresent) {
            e_error("cannot use sky combination if sky frames are present");
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }
        jc->sky_method_used = skymethod_combine ;
        break ;

        case skymethod_combine_mc:
        if (jc->nframes < jc->skyfilter_minframes) {
            e_error("not enough frames to use sky combination(mc) (%d<%d)",
                    jc->nframes,
                    jc->skyfilter_minframes);
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }
        if (jc->sky_ispresent) {
            e_error("cannot use sky combination(mc) if sky frames are present");
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }
        jc->sky_method_used = skymethod_combine_mc ;
        break ;

        default:
        e_error("internal: undefined sky method");
        jc->status_sky = ALGO_FAILED ;
        return -1 ;
    }
    
    /*
     * Switch on requested method
     */
    switch (jc->sky_method_used) {

        /*
         * Sky filtering with frame combination (running filter)
         */
        case skymethod_combine:
        e_comment(1, "sky filtering (combine)") ;
            
        /* Build a cube with all the planes */
        cube = jitter_cubeget(jc, NULL) ;
            
        /* Allocate a background values array */
        background = malloc(cube->np * sizeof(double)) ;
            
        /* Apply the running method by quadrants or not  */
        if (jc->skyfilter_sepquad) {
            status = cube_3dfilt_runminmax_by_quad(&cube,
                        jc->skyfilter_rejhw,
                        jc->skyfilter_rejmin,
                        jc->skyfilter_rejmax,
                        background) ;
        } else {
            status = cube_3dfilt_runminmax(&cube,
                        jc->skyfilter_rejhw,
                        jc->skyfilter_rejmin,
                        jc->skyfilter_rejmax,
                        background) ;
        }
        
        /* Copy plane pointers back into jitter_config */
        jitter_cubeput(jc, NULL, cube);
        /* Destroy the cube */
        cube_del_shallow(cube) ;

        /* Update the background field in the input frames */
        for (i=0 ; i<jc->nframes ; i++) jc->frame[i].skyval = background[i];
            
        /* Free memory */
        free(background) ;

        /* In failure case */
        if (status == -1) {
            e_error("combination method failed") ;
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }

        /* Update status field in jitter_config */
        jc->status_sky = ALGO_OK ;
        break ;

        /*
         * Sky filtering with frame combination minus central value
         */
        case skymethod_combine_mc:
        e_comment(1, "sky filtering (combine without central value)") ;
            
        /* Build a cube with all the planes */
        cube = jitter_cubeget(jc, NULL) ;
            
        /* Allocate a background values array */
        background = malloc(cube->np * sizeof(double)) ;
        status = cube_3dfilt_runminmax_central(
                    &cube,
                    jc->skyfilter_rejhw,
                    jc->skyfilter_rejmin,
                    jc->skyfilter_rejmax,
                    background) ;
        
        /* Copy plane pointers back into jitter_config */
        jitter_cubeput(jc, NULL, cube);
        /* Destroy the cube */
        cube_del_shallow(cube) ;

        /* Update the background field in the input frames */
        for (i=0 ; i<jc->nframes ; i++) jc->frame[i].skyval = background[i];
            
        /* Free memory */
        free(background) ;

        /* In failure case */
        if (status == -1) {
            e_error("combination method failed") ;
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }

        /* Update status field in jitter_config */
        jc->status_sky = ALGO_OK ;
        break ;

        /*
         * Sky filtering with median frame subtraction
         */
        case skymethod_medianframe:
        e_comment(1, "sky filtering (median frame)") ;
       
        /* Fill a flag array to identify the sky frames */
        selection = jitter_cubeselect(jc, type_sky) ;
        
        /* Build a cube with all the input frames */
        cube = jitter_cubeget(jc, NULL) ;

        /* Correct the obj frames with a median sky estimation */
        sky = cube_subtract_median_sky(cube, selection) ;

        /* No need of sky frames any more */
        for (i=0 ; i < cube->np ; i++) {
            if (selection[i] == 1) {
                image_del(cube->plane[i]) ;
                cube->plane[i] = NULL ;
            }
        }

        /* Copy plane pointers back into jitter_config */
        jitter_cubeput(jc, NULL, cube) ;
        
        /* Destroy the cube */
        cube_del_shallow(cube) ;
       
        /* In failure case */
        if (sky == NULL) {
            e_error("median method failed") ;
            jc->status_sky = ALGO_FAILED ;
            return -1 ;
        }
        
        /* Compute the background value */
        bg_val = image_getmedian(sky) ;
        
        /* Destroy the sky */
        image_del(sky) ;

        /* Fill the background value in the obj frames field */
        for (i=0 ; i<jc->nframes ; i++) {
            if (selection[i] == 0) jc->frame[i].skyval = bg_val ;
        }
        
        /* Destroy the selection array */
        free(selection) ;

        /* Update status field in jitter_config */
        jc->status_sky = ALGO_OK ;
        break ;

        
        default:
        e_warning("internal: not a sky filtering method");
        jc->status_sky = ALGO_FAILED ;
        return -1 ;
        break ;
    }

    /* Output the corrected planes if requested */
    if (jc->sky_outdiff) {
        e_comment(1, "saving sky-subtracted frames") ;
        jitter_sky_output(jc);
    }

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter sky output files 
  @param    jc  Current jitter config
  @return   void 
 */
/*----------------------------------------------------------------------------*/
static void jitter_sky_output(jitter_config_t * jc) 
{
    char                output_name[FILENAMESZ] ; 
    cube_t          *   cube ;
    int             *   sel ;
    qfits_header    *   fh ;
    procat              pro_catg ;
    char            *   val ;

    /* Extract object cube */
    sel = jitter_cubeselect(jc, type_obj);
    cube = jitter_cubeget(jc, sel);
    free(sel);
    
    /* Define the output file complete name */
    sprintf(output_name, "%s_dif.fits", jc->output_basename);

    /* Read the FITS header of the ref file    */
    fh = qfits_header_read(jc->frame[0].name) ;

    /* Update FITS header with PRO keywords */
    /* Default */
    pro_catg = procat_invalid ;
    /* Find out the pro catg keyword to write : depends on the used arm */
    if ((val = pfits_get(jc->data_type, jc->frame[0].name, "arm")) != NULL) { 
        if (toupper(val[0])=='S') pro_catg = procat_imag_sw_jitter_diff ;
        else if (toupper(val[0])=='L') pro_catg = procat_invalid ;
    }
    jitter_add_pro_keys(jc, fh, pro_catg) ;
    
    cube_save_fits_hdrdump(cube, output_name, fh) ;
    qfits_header_destroy(fh) ;
    cube_del_shallow(cube);

    e_comment(1, "difference produced: [%s]", output_name) ;
    return ;
}

