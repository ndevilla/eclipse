/*----------------------------------------------------------------------------*/
/**
   @file	detect_ks.c
   @author	N. Devillard
   @date	June 2001
   @version	$Revision: 1.10 $
   @brief	Object detection with kappa-sigma clipping
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: detect_ks.c,v 1.10 2003/02/20 13:34:01 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/20 13:34:01 $
	$Revision: 1.10 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "comm.h"
#include "image_handling.h"
#include "image_filters.h"
#include "pixelmaps.h"
#include "detect_ks.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Locate objects in an image according to a kappa-sigma clipping.
  @param    ref         Reference image.
  @param    kappa       Kappa for kappa-sigma clipping.
  @param    smear_flag  Request image smearing before applying detection.
  @return   the detected object
 */
/*----------------------------------------------------------------------------*/
detected * detected_ks_engine(
	    image_t  * ref,
	    double     kappa,
	    int        smear_flag)
{
	detected	*	det ;
    image_t  	*	detect_image ;
    double   	 	medval, abs_med ;
    int      	   	k ;
    int      	   	npix ;
    pixelmap 	*  	thresh ;
    intimage 	*  	lab ;
    int      	   	nobj ;

    /* Review input parameters */
    if (ref == NULL) return NULL ;

    /* Smear input image if requested */
    if (smear_flag) {
        detect_image = image_filter5x5(ref,
                       image_filter_getkernel("mean5", NULL, NULL));
        if (detect_image==NULL) {
            e_error("smearing image: aborting object detection");
            return NULL ;
        }
    } else {
        detect_image = ref ;
    }
	
    /* Assign default kappa if needed */
    kappa = (kappa<0) ? DETECTED_KAPPA : kappa ;
	
    /* Median estimation and threshold */
    medval = image_getmedian(detect_image);
    npix = detect_image->lx * detect_image->ly ;
    abs_med = 0 ;
    for (k=0 ; k<npix ; k++) {
        abs_med += fabs((double)detect_image->data[k] - medval);
    }
    abs_med /= (double)npix ;

    /* Threshold to binary map */
    thresh = image_threshold2pixelmap(detect_image,
                                      medval + kappa * abs_med,
                                      MAX_PIX_VALUE);
    if (detect_image != ref) image_del(detect_image);

    if (thresh == NULL) {
        e_error("thresholding image: aborting detection");
        return NULL ;
    }

    /* Morphology */
    if (pixelmap_morpho_closing(thresh) == -1) {
        e_error("closing binary map: aborting detection");
        return NULL ;
    }

    /* Labelize pixel map into intimage */
    lab = intimage_labelize_pixelmap(thresh, &nobj);
    pixelmap_del(thresh);
    if (lab == NULL) {
        e_error("assigning labels to binary map: aborting detection");
        return NULL ;
    }

	/* Create detected object */
	det = detected_compute_objstat(ref, lab, nobj);
    intimage_del(lab);
    return det ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the brigthest stars in an image with the ks method
  @param    image1  image
  @param	nbobjs	number of objects to detect
  @param    kappa   kappa for detection
  @return   the stars position
 */
/*----------------------------------------------------------------------------*/
double3 * detected_ks_brightest_stars(
        image_t *   image1,
		int			nbobjs,
        double      kappa)
{
    double3     *   list_pixpos ;
    double3     *   pos_tmp ;
    detected    *   det ;
    int             nb_objects ;
    int             i ;

    if (image1 == NULL) return NULL ;

    /* Find centers of all nonzero regions  */
    if ((det = detected_ks_withstats(image1, kappa)) == NULL) {
        e_error("cannot find any object") ;
        return NULL ;
    }

    /* The fine positioning has to be computed */
    if ((det->fine_x == NULL) || (det->fine_y == NULL)) {
        e_error("cannot find the fine positions") ;
        detected_del(det) ;
        return NULL ;
    }

    /* The flux has to be there  */
    if (det->obj_flux == NULL) {
        e_error("cannot find the flux") ;
        detected_del(det) ;
        return NULL ;
    }

    /* Convert detected obj in double3 and fill z field with the flux */
    pos_tmp = double3_new(det->nbobj) ;
    for (i=0 ; i<det->nbobj ; i++) {
        pos_tmp->x[i] = det->fine_x[i] ;
        pos_tmp->y[i] = det->fine_y[i] ;
        pos_tmp->z[i] = det->obj_flux[i] ;
    }
    detected_del(det) ;

    /* Sort the detected stars according to decreasing flux */
    double3_sort(pos_tmp, -1) ;

    /* Set the number of objects to return */
    if (det->nbobj <= nbobjs) nb_objects = det->nbobj ;
    else nb_objects = nbobjs ;

    /* Find brigthest stars among detected ones */
    list_pixpos = double3_new(nb_objects) ;
    for (i=0 ; i<nb_objects ; i++) {
        list_pixpos->x[i] = pos_tmp->x[i] ;
        list_pixpos->y[i] = pos_tmp->y[i] ;
        list_pixpos->z[i] = pos_tmp->z[i] ;
    }

    /* Free and return */
    double3_del(pos_tmp) ;
    return list_pixpos ;

}

