/*----------------------------------------------------------------------------*/
/**
   @file    jcalib.c
   @author
   @date    March 2002
   @version	$Revision: 1.15 $
   @brief   Jitter calibration handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jcalib.c,v 1.15 2003/10/24 11:12:21 yjung Exp $
	$Author: yjung $
	$Date: 2003/10/24 11:12:21 $
	$Revision: 1.15 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "qfits.h"
#include "eclipse.h"

#include "jtypes.h"
#include "jconfig.h"

/*----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Apply calibrations to type_obj frames
  @param    jc  Jitter configuration object
  @return   0 if ok, -1 otherwise
  This function loads the dark flatfield and bad pixel map specified and apply
  the corrections to the object frames.
 */
/*----------------------------------------------------------------------------*/
int jitter_calibration(jitter_config_t * jc)
{
    cube_t      *   incube ;
    image_t     *   dark ;
    image_t     *   ff ;
    image_t     *   tmp_im ;
    pixelmap    *   badpix ;
    pixelmap    *   tmp_pm ;
    int             i ;

    /* Load the calibration data */
    /* Dark */
    if (jc->dark_sub) {
        dark = image_load(jc->dark_name) ;
        if (jc->zone.left || jc->zone.right || jc->zone.bottom || jc->zone.top){
            if ((tmp_im = image_getvig(dark, 
                                jc->zone.left + 1,
                                jc->zone.bottom + 1,
                                dark->lx - jc->zone.right,
                                dark->ly - jc->zone.top)) != NULL) {
                image_del(dark) ;
                dark = tmp_im ;
            }
        }
    } else dark = NULL ;
    
    /* Flatfield */
    if (jc->ff_div) {
        ff = image_load(jc->ff_name) ;
        if (jc->zone.left || jc->zone.right || jc->zone.bottom || jc->zone.top){
            if ((tmp_im = image_getvig(ff, 
                                jc->zone.left + 1,
                                jc->zone.bottom + 1,
                                ff->lx - jc->zone.right,
                                ff->ly - jc->zone.top)) != NULL) {
                image_del(ff) ;
                ff = tmp_im ;
            }
        }
    } else ff = NULL ;
    
    /* Badpixel */
    if (jc->badpix_rep) {
        badpix = pixelmap_load(jc->badpixmap) ;
        if (jc->zone.left || jc->zone.right || jc->zone.bottom || jc->zone.top){
            if ((tmp_pm = pixelmap_getvig(badpix, 
                                jc->zone.left + 1,
                                jc->zone.bottom + 1,
                                badpix->lx - jc->zone.right,
                                badpix->ly - jc->zone.top)) != NULL) {
                pixelmap_del(badpix) ;
                badpix = tmp_pm ;
            }
        }
    } else badpix = NULL ;
  
    /* Construct the cube with all jitter_config planes */
    incube = jitter_cubeget(jc, NULL) ;

    /* Apply the odd-even effect correction if requested */
    if (jc->preproc_active && jc->preproc_oddeven) {
        for (i=0 ; i<incube->np ; i++) {
            tmp_im = image_de_oddeven_byquad(incube->plane[i]) ;
            if (tmp_im == NULL) {
                if (dark != NULL)   image_del(dark) ;
                if (ff != NULL)     image_del(ff) ;
                if (badpix != NULL) pixelmap_del(badpix) ;
                cube_del_shallow(incube) ;
                jc->status_calib = ALGO_FAILED ;
                return -1 ;
            }
            image_del(incube->plane[i]) ;
            incube->plane[i] = tmp_im ;
        }
    }
    
    /* Apply the calibration corrections */
    cube_correct_ff_dark_badpix(incube, ff, dark, badpix) ;
  
    /* Copy plane pointers back into blackboard */
    jitter_cubeput(jc, NULL, incube);

    /* Free the cube (but the images) */
    cube_del_shallow(incube) ;
    
    /* Free the calibration data */
    if (dark != NULL)   image_del(dark) ;
    if (ff != NULL)     image_del(ff) ;
    if (badpix != NULL) pixelmap_del(badpix) ;
    
    /* Update the status in jitter_config */
    jc->status_calib = ALGO_OK ;
   
    /* Return */
    return 0 ;
}
