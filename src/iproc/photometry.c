/*-------------------------------------------------------------------------*/
/**
   @file	photometry.c
   @author	Nicolas Devillard
   @date	Jun 23, 1997
   @version	$Revision: 1.19 $
   @brief	photometry measurement routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: photometry.c,v 1.19 2005/09/15 12:14:35 yjung Exp $
	$Author: yjung $
	$Date: 2005/09/15 12:14:35 $
	$Revision: 1.19 $
*/

/*---------------------------------------------------------------------------
  								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

#include "photometry.h"
#include "histogram.h"

/*---------------------------------------------------------------------------
  								Defines
 ---------------------------------------------------------------------------*/

#define BG_MINIMUM_NB_OF_PIXELS			30
#define REJECT_LOW                      0.1
#define REJECT_HIGH                     0.1


/*---------------------------------------------------------------------------
 							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the flux in a disk in a given image.
  @param	in			Input image.
  @param	x_center	x center of the disk.
  @param	y_center	y center of the disk.
  @param	radius		Disk radius.
  @param	background	Optional background value.
  @return	1 double.

  Computes the energy (sum of pixels) in a disk in a given image. Provide
  the disk center coordinates in the C convention (x from 0 to lx-1 and y
  from 0 to ly-1, x from left to right and y from bottom to top) and the
  disk radius in pixels. If you provide a background value, it will be
  subtracted from each pixel before summation.

  Use image_get_disk_background() to get an accurate estimate of the
  background if you want to provide one.

  Returns -1.0 in case of error.
 */
/*--------------------------------------------------------------------------*/
double image_get_disk_flux(
		image_t		*	in,
		double			x_center,
		double			y_center,
		double			radius,
		pixelvalue		background) 
{
	double		phot ;
	double		dist ;
	double		sqr_radius ;
	int			i, j ;
	int			window[4] ; /* contains xmin xmax ymin ymax */

	if (in == NULL) return -1.0 ;
	if (radius <= 0.0) {
		e_error("negative radius: %g cannot compute photometry", radius);
		return -1.0 ;
	}

	/* Go on computing photometry now	*/
	window[0] = (int)(x_center - radius - 2.00) ;
	if (window[0]<0) window[0]=0 ;
	window[1] = (int)(x_center + radius + 2.00) ;
	if (window[1]>(in->lx-1)) window[1]=in->lx-1 ;
	window[2] = (int)(y_center - radius - 2.00) ;
	if (window[2]<0) window[2]=0 ;
	window[3] = (int)(y_center + radius + 2.00) ;
	if (window[3]>(in->ly-1)) window[3]=in->ly-1 ;
	sqr_radius = radius * radius ;
	phot = 0.00 ;
	for (j=window[2] ; j<window[3] ; j++) {
		for (i=window[0]; i<window[1] ; i++) {
			dist = (double)(
					(i - x_center) * (i - x_center) +
					(j - y_center) * (j - y_center)) ;
			if (dist <= sqr_radius) {
				phot += (double)(in->data[i+j*in->lx] - background) ;
			}
		}
	}
	return phot ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Computes the flux per pixel in a given ring in an image.
  @param	in			Input image.
  @param	x_center	x center of the ring.
  @param	y_center	y center of the ring.
  @param	rad_int		Internal radius of the ring.
  @param	rad_ext		External radius of the ring.
  @param	method		Computing method.
  @return	1 double

  This function tries to estimate the background in an image for
  photometric purposes. Usually, you provide a ring in which all pixels are
  taken into account for background estimation. The returned value is a
  measurement of the background per pixel.

  Possible methods are simply a linear average of all pixels in the ring
  (set method to BG_METHOD_AVERAGE), or the median value of all pixels in
  the ring (set method to BG_METHOD_MEDIAN).

  Returns 0.0 in case of error.
 */
/*--------------------------------------------------------------------------*/
double image_get_disk_background(
		image_t		*	in,
		double			x_center,
		double			y_center,
		double			rad_int,
		double			rad_ext,
		int				method)
{
	double	        flux ;
	int		        i, j, k ;
	double	        sqr_int,
			        sqr_ext,
			        dist ;
	int		        npix ;		
	int		        window[4] ;
    int             low_ind ;
    int             high_ind ;
	pixelvalue	*	pix_arr ;

	if (in == NULL) return 0.0 ;
	if ((rad_int <= 0.0) || (rad_ext <= 0.0)) {
		e_error("negative radius: [%g %g] cannot compute photometry", 
				rad_int, rad_ext);
		return 0.0 ;
	}
	if ((rad_ext - rad_int) < 1e-10) {
		e_error("wrong radii: range [%g %g] is illegal", rad_int, rad_ext) ;
		return 0.0 ;
	}
	
	/* Compute background level	*/

	window[0] = (int)(x_center - rad_ext - 2.00) ;
	if (window[0]<0) window[0]=0 ;
	window[1] = (int)(x_center + rad_ext + 2.00) ;
	if (window[1]>(in->lx-1)) window[1]=in->lx-1 ;
	window[2] = (int)(y_center - rad_ext - 2.00) ;
	if (window[2]<0) window[2]=0 ;
	window[3] = (int)(y_center + rad_ext + 2.00) ;
	if (window[3]>(in->ly-1)) window[3]=in->ly-1 ;

	sqr_int = rad_int * rad_int ;
	sqr_ext = rad_ext * rad_ext ;
	flux = 0.00 ;
	switch(method) {
		case BG_METHOD_AVERAGE:
        npix = 0 ; /* used to count pixels in the ring	*/
		for (j=window[2] ; j<window[3] ; j++) {
			for (i=window[0] ; i<window[1]; i++) {
				dist = (double)(
						(i - x_center) * (i - x_center) +
						(j - y_center) * (j - y_center)) ;
				if ((dist >= sqr_int) && (dist <= sqr_ext)) {
					flux += (double)in->data[i+j*in->lx] ;
					npix ++ ;
				}
			}
		}
		flux /= (double)npix ;
		break ;

		case BG_METHOD_MEDIAN:
        /* Count number of pixels in the ring */
        npix = 0 ; /* used to count pixels in the ring	*/
		for (j=window[2] ; j<window[3] ; j++) {
			for (i=window[0] ; i<window[1]; i++) {
				dist = (double)(
						(i - x_center) * (i - x_center) +
						(j - y_center) * (j - y_center)) ;
				if ((dist >= sqr_int) && (dist <= sqr_ext)) {
					npix ++ ;
				}
			}
		}
		if (npix < BG_MINIMUM_NB_OF_PIXELS) {
            return 0.0 ;
        }
        /* Allocate pixel array to hold values in the ring */
		pix_arr = (pixelvalue*)malloc(npix * sizeof(pixelvalue)) ;

		/* Retrieve all pixels which belong to the ring	*/
        k=0 ;
		for (j=window[2] ; j<window[3] ; j++) {
			for (i=window[0] ; i<window[1]; i++) {
				dist = (double)(
						(i - x_center) * (i - x_center) +
						(j - y_center) * (j - y_center)) ;
				if ((dist >= sqr_int) && (dist <= sqr_ext)) {
                    pix_arr[k] = in->data[i+j*in->lx] ;
					k++ ;
				}
			}
		}
		/* Compute median value for the array */
		flux = median_pixelvalue(pix_arr, npix) ;
		free(pix_arr) ;
		break ;

        case BG_METHOD_AVER_REJ:
        /* Count number of pixels in the ring */
        npix = 0 ; /* used to count pixels in the ring	*/
		for (j=window[2] ; j<window[3] ; j++) {
			for (i=window[0] ; i<window[1]; i++) {
				dist = (double)(
						(i - x_center) * (i - x_center) +
						(j - y_center) * (j - y_center)) ;
				if ((dist >= sqr_int) && (dist <= sqr_ext)) {
					npix ++ ;
				}
			}
		}
		if (npix < BG_MINIMUM_NB_OF_PIXELS) {
            return 0.0 ;
        }
        /* Allocate pixel array to hold values in the ring */
		pix_arr = (pixelvalue*)malloc(npix * sizeof(pixelvalue)) ;

		/* Retrieve all pixels which belong to the ring	*/
        k=0 ;
		for (j=window[2] ; j<window[3] ; j++) {
			for (i=window[0] ; i<window[1]; i++) {
				dist = (double)(
						(i - x_center) * (i - x_center) +
						(j - y_center) * (j - y_center)) ;
				if ((dist >= sqr_int) && (dist <= sqr_ext)) {
                    pix_arr[k] = in->data[i+j*in->lx] ;
					k++ ;
				}
			}
		}
		/* Sort the array */
        pixel_qsort(pix_arr, npix) ;
        low_ind = (int)(npix*REJECT_LOW) ;
        high_ind= (int)(npix*(1-REJECT_HIGH)) ;
        for (i=low_ind ; i<high_ind ; i++) flux += pix_arr[i] ;
        flux /= (high_ind - low_ind) ;
        free(pix_arr) ;
		break ;

		default:
		e_error("unknown background estimation method requested") ;
		flux = 0.0 ;
		break ;
	}
	return flux ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Estimate the background in an image.
  @param	in				Input image.
  @param	stop_thr		Threshold to stop procedure.
  @param	max_it			Maximum number of iterations to perform.
  @return	1 double.

  This function is a fairly simple background estimator. It assumes the
  input image to be made mainly of background pixels, with some high-flux
  signal in it (in minority). It computes iteratively the mean value of the
  image, rejecting at each pass all pixels above 3 sigmas. It stops when
  the improvement to the mean value between two iterations is lower than
  the provided stop_thr	 value.

  The iterations will either stop because the mean improvement is lower
  than the requested value, or because the maximal number of iterations has
  been reached.

  Returns 0.0 in case of error.
 */
/*--------------------------------------------------------------------------*/
#define ESTBG_REJ_THRESHOLD		3.0
double image_estimate_background(
   		image_t	*  	in,
    	double      stop_thr,
		int			max_it)
{
    image_stats *   stats ;
    int             pix ;
    double          central,
                    bias,
					dyn_thresh,
					central_previous ;
	double			diff, sum, sq_sum ;
	int				acc_npix ;
	int				iter ;
	histogram	*	hist ;

	if ((in==NULL) || (stop_thr<=0.0) || (max_it<1)) return 0.0;
	/*
	 * Initialize method with mean and stdev
	 */
	stats   = image_getstats(in) ;
	central = stats->avg_pix ;
	bias    = stats->stdev ;
	free(stats) ;
	iter    = 0 ;

    while (1) {
		central_previous = central ;
		dyn_thresh 	= bias * ESTBG_REJ_THRESHOLD ;
		/* dyn_thresh 	= bias ; */
		sum 		= 0.00 ;
		sq_sum 		= 0.00 ;
		acc_npix	= 0 ;

    	for (pix=0 ; pix<(in->lx * in->ly) ; pix++) {
			/*
			 * Compute how far this pixel is from the mean
			 */
			diff = fabs((double)in->data[pix] - central) ;
			if (diff <= dyn_thresh) {
				/* Pixel is within acceptable range */
				sum += (double)in->data[pix] ;
				sq_sum += (double)in->data[pix] * (double)in->data[pix] ;
				acc_npix ++ ;
			}
		}
        if (acc_npix<1) {
            /* Fail: no pixel satisfies the conditions */
            return 0.0 ;
        }
		sum    /= (double)acc_npix ;
		central = sum ;
		sq_sum /= (double)acc_npix ;
        /* Rounding errors can cause the variance to be negative */
        bias    = sq_sum - sum*sum ;
        bias    = bias > 0 ? sqrt(bias) : 0 ;
		iter ++ ;
		if ((iter >= max_it) ||
			(fabs(central - central_previous) < stop_thr)) {
			break ;
		}
    }

	/* A good estimate of the central value is now known */
	hist = histogram_compute(in, 256, central-bias, central+bias);
	if (hist!=NULL) {
		central = histogram_find_mode(hist);
		histogram_del(hist);
	}
    return central ;
}
#undef ESTBG_REJ_THRESHOLD

