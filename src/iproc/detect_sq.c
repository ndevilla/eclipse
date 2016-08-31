/*-------------------------------------------------------------------------*/
/**
   @file	detect_sq.c
   @author	Yves Jung
   @date	June 2001
   @version	$Revision: 1.7 $
   @brief	Object detection with square filter method
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: detect_sq.c,v 1.7 2001/12/12 16:18:33 yjung Exp $
	$Author: yjung $
	$Date: 2001/12/12 16:18:33 $
	$Revision: 1.7 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "detect.h"
#include "xmemory.h"
#include "comm.h"
#include "doubles.h"
#include "image_handling.h"
#include "image_filters.h"
#include "histogram.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define SQ_MAX_STARS	1024

/*---------------------------------------------------------------------------
					Functions private to this module
 ---------------------------------------------------------------------------*/

static pixelmap * detect_sq_binarize(image_t * in) ;

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Locate objects in an image according to the square method.
  @param    ref     Reference image
  @param    hx      Square half-size in X.
  @param    hy      Square half-size in Y.
  @return   the detected object
 */
/*--------------------------------------------------------------------------*/
detected * detected_sq_engine(
	    image_t     *   ref,
	    int             hx,
	    int             hy)
{
    image_t     *   mfilt ;
    image_t     *   squares ;
	pixelmap	*	mfilt_bin ;
	pixelmap	*	squares_bin ;
	intimage    * 	labels ;
	int				nobj ;
	detected	*	det ;
	int				hx_loc, hy_loc ;
	
    /* Initialize variables */
    if (hx < 1) hx_loc = DETECTED_SQHX ;
	else hx_loc = hx ;
    if (hy < 1) hy_loc = DETECTED_SQHY ;
	else hy_loc = hy ;

    /* Median filter to remove bad pixels */
    if ((mfilt = image_filter_median(ref)) == NULL) {
		e_error("filtering input image");
		return NULL ;
	}

    /* Standard deviation filter */
    if ((squares = image_filter_stdev(mfilt, hx_loc, hy_loc)) == NULL) {
        e_error("in stdev filtering") ;
        image_del(mfilt) ;
        return NULL ;
    }
	
	/* Binarize squares */
	if ((squares_bin = detect_sq_binarize(squares)) == NULL) {
		image_del(squares) ;
		image_del(mfilt) ;
		return NULL ;
	}
	image_del(squares) ;

    /* Binarize mfilt */
	if ((mfilt_bin = detect_sq_binarize(mfilt)) == NULL) {
		image_del(mfilt) ;
		pixelmap_del(squares_bin) ;
		return NULL ;
	}

	/* squares_bin AND mfilt_bin  */
	if (pixelmap_binary_AND(mfilt_bin, squares_bin) == -1) {
		e_error("cannot perform a AND between two pixelmaps") ;
		image_del(mfilt) ;
		pixelmap_del(squares_bin) ;
		pixelmap_del(mfilt_bin) ;
		return NULL ;
	}
	pixelmap_del(squares_bin) ;
	
	/* Morphology to clean residuals */
    if (pixelmap_morpho_closing(mfilt_bin) == -1) {
        e_error("closing binary map: aborting detection");
		image_del(mfilt) ;
		pixelmap_del(mfilt_bin) ;
        return NULL ;
    }

    /* Labelize pixel map into an intimage */
    labels = intimage_labelize_pixelmap(mfilt_bin, &nobj);
    if (labels == NULL) {
        e_error("assigning labels to binary map: aborting detection");
		image_del(mfilt) ;
		pixelmap_del(mfilt_bin) ;
        return NULL ;
    }
	pixelmap_del(mfilt_bin) ;

    /* Create detected object and compute obs stats */
    if ((det = detected_compute_objstat(mfilt, labels, nobj)) == NULL) {
		e_error("cannot create the detected structure") ;
		image_del(mfilt) ;
		intimage_del(labels) ;
		return NULL ;
	}

	/* Free and return */
	intimage_del(labels) ;
	image_del(mfilt) ;
    return det ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Detect the brigthest stars in an image with the sq method
  @param    image1  image
  @param	nbobjs	number of objects to detect
  @param	hx		half square size x
  @param	hy		half square size y
  @return   the stars position
 */
/*--------------------------------------------------------------------------*/
double3 * detected_sq_brightest_stars(
        image_t *   image1,
		int			nbobjs,
        int			hx,
		int			hy)
{
    double3     *   list_pixpos ;
    double3     *   pos_tmp ;
    detected    *   det ;
	int				nb_objects ;
    int             i ;

    if (image1 == NULL) return NULL ;

    /* Find centers of all nonzero regions  */
    if ((det = detected_sq_withstats(image1, hx, hy)) == NULL) {
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


/*-------------------------------------------------------------------------*/
/**
  @brief	binarize an image 
  @param	input image
  @return	the binarized image
  The threshold used is background + 2 * sigma
 */
/*--------------------------------------------------------------------------*/
static pixelmap * detect_sq_binarize(image_t * in)
{
	pixelmap	*	out ;
	histogram	*	h ;
	double			background ;
	double			sigma ;
	double			threshold ;
	int				i ;

    /* Find the threshold  using the histogram */
    if ((h = histogram_compute(in,
                                in->lx,
                                MIN_PIX_VALUE,
                                MAX_PIX_VALUE)) == NULL) {
        e_error("cannot compute histogram") ;
        return NULL ;
    }
    background = (double)histogram_find_mode(h) ;
    histogram_del(h) ;
    sigma = 0.0 ;
    for (i=0 ; i<(in->lx * in->ly) ; i++) {
        sigma += fabs((double)(in->data[i]-background)) ;
    }
    sigma /= (double)(in->lx * in->ly) ;
    threshold = background + 2.0 * sigma ;
	
	/* Binarize */
	out = image_threshold2pixelmap(in, threshold, MAX_PIX_VALUE) ;

	return out ;
}


