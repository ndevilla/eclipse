/*----------------------------------------------------------------------------*/
/**
   @file	xcorrelation.c
   @author	N. Devillard & Y. Jung
   @date	November 2000
   @version	$Revision: 1.31 $
   @brief	Cross correlation fonctions
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: xcorrelation.c,v 1.31 2003/02/14 09:18:26 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/14 09:18:26 $
	$Revision: 1.31 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "xcorrelation.h"
#include "detect.h"
#include "dstats.h"

#include "fourier.h"
#include "image_intops.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define XCORR_MAX_POINTS	100
#define XCORR_MIN_POINTS	1

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

static double xcorr_apodisation(double, double, double);
static double xcorr_private(pixelvalue *, pixelvalue *, int, int, int,
        int, int, int, int, int, int, int, int, int, double * ,double *) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Find correlating regions in an image.
  @param    in_image            Image to search for correlating regions.
  @param    edge_x              Edge definition in X.
  @param    edge_y              Edge definition in Y.
  @param    sigma_threshold     Sigma threshold for kappa-sigma clipping.
  @param    min_points          Min # of points to detect.
  @param    max_points          Max # of points to detect.
  @return   Newly allocated list of points in the image.
 
  This function is responsible for searching into an image for
  interesting points for cross-correlation. A point is interesting for
  cross-correlation if it has a clear disymmetry in X and Y, which
  allows to locate it to subpixel precision. Point-like sources are
  okay.
 
  The detected objects are required to be at least edge_x and edge_y
  pixels from the edges of the image. This is done to avoid
  cross-correlating on the edges of an image, which is always tricky
  (loss of precision mostly, deformations of the detector in these
  regions, etc.).
 
  The detection method is currently trying to locate point-like
  sources. It should be enhanced in future versions to locate points
  with a more "cross-correlating" way of behaving.
 
  min_points and max_points can be given as -1, in which case default
  values are used (min=1, max=100).

  The returned list of points must be deallocated using double3_del().
 */
/*----------------------------------------------------------------------------*/
double3 * get_xcorrelation_points(
    	image_t		*   in_image,
    	int             edge_x,
    	int             edge_y,
    	double          sigma_threshold,
    	int             min_points,
		int             max_points)
{
    image_t     *   smeared ;
    double3     *   xcorr_peaks ;
    int				min_p, 
					max_p ;

    /* Error handling: test entries */
    if (in_image == NULL) return NULL ;
    if (edge_x<0 || edge_x>in_image->lx/2 || 
			edge_y<0 || edge_y>in_image->ly/2) {
        e_error("inconsistent edge requirements: cannot find xcorr objs");
        return NULL ;
    }
    if (sigma_threshold<=0.0) sigma_threshold = DEFAULT_SIGMA_THRESHOLD ;
    if (min_points<1) min_p = XCORR_MIN_POINTS ; else min_p = min_points ;
    if (max_points<1) max_p = XCORR_MAX_POINTS ; else max_p = max_points ;

	/* Strategy 1: Try the brightest objects */
    e_comment(1, "looking for xcorrelation centers...") ;
    e_comment(2, "trying bright objects... (1)") ;
	if ((xcorr_peaks = get_points_engine(in_image,
										sigma_threshold,
										edge_x,
										edge_y,
										min_p,
										max_p)) == NULL) {
		e_comment(2, "no suitable bright object found (1)") ;
	} else return xcorr_peaks ;
	
	/* Strategy 2: increase detectability in the image, by first */
	/* applying a low-pass filter on the input image */
    e_comment(2, "trying bright objects (2)...");
    smeared = image_filter5x5(in_image, image_filter_getkernel("mean5",0,0)) ;
	if ((xcorr_peaks = get_points_engine(smeared,
										sigma_threshold,
										edge_x,
										edge_y,
										min_p,
										max_p)) == NULL) {
		e_comment(2, "no suitable bright object found (2)") ;
	} else {
		image_del(smeared) ;
		return xcorr_peaks ;
	}
	
	/* Strategy 3: increase detectability by halving the sigma */
	/* threshold and use the smeared image */
    e_comment(2, "trying bright objects (3)...");
	if ((xcorr_peaks = get_points_engine(smeared,
										sigma_threshold / 2.0,
										edge_x,
										edge_y,
										min_p,
										max_p)) == NULL) {
		e_comment(2, "no suitable bright object found (3)") ;
	} else {
		image_del(smeared) ;
		return xcorr_peaks ;
	}
    
	/* Strategy 4: increase detectability by using 20% of the sigma */
	/* threshold, use the smeared image for detection */
    e_comment(2, "trying bright objects (4)...");
	if ((xcorr_peaks = get_points_engine(smeared,
										sigma_threshold / 5.0,
										edge_x,
										edge_y,
										min_p,
										max_p)) == NULL) {
		e_comment(2, "no suitable bright object found (4)") ;
	} else {
		image_del(smeared) ;
		return xcorr_peaks ;
	}

    image_del(smeared);

    /* Detection failed */
    return NULL ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get points to blindly estimate the offsets
  @param	inimage				input image
  @param	sigma_threshold		threshold
  @param	edge_x				X edge size to reject
  @param	edge_y				Y edge size to reject
  @param 	min_points			Minimum nb of points to detect
  @param	max_points			Maximum nb of points to detect
  @return	List of points in the image
  This is the engine of get_xcorrelation_points()
 */
/*----------------------------------------------------------------------------*/
double3 * get_points_engine(
		image_t		*	inimage,
		double			sigma_threshold,
		int				edge_x,
		int				edge_y,
		int				min_points,
		int				max_points)
{
	detected	*	det ;
	double3		*	peaks ;
	double3		*	xcorr_peaks ;
	int			*	valid_flags ;
	int				nvalid ;
	int				i, j ;

	/* Initialize */
	xcorr_peaks = NULL ;
	
	/* Detect objects */
	if ((det = detected_ks_engine(inimage, sigma_threshold, 0)) == NULL) {
		e_warning("cannot detect any object") ;
		return NULL ;
	}
    if (det->nbobj<1) {
        detected_del(det);
		e_warning("cannot detect any object") ;
		return NULL ;
    }
    peaks = detected2double3(det) ;
	detected_del(det) ;

	/* Only keep detected objects that are valid for Xcorrelation */
    valid_flags = malloc(peaks->n * sizeof(int)) ;
    localize_xcorr_centers(peaks, inimage->lx, inimage->ly, edge_x,
                            edge_y, &nvalid, valid_flags) ;
    if (nvalid < min_points) {
        e_error("Not enough valid points found : %d < %d", nvalid, min_points);
        double3_del(peaks) ;
        free(valid_flags) ;
        return NULL ;
    }
    if (nvalid > max_points) nvalid = max_points ;
    
    e_comment(2, "%d valid object(s) found", nvalid) ;
    xcorr_peaks = double3_new(nvalid) ;
    i = j = 0 ;
    while (j<nvalid) {
        if (valid_flags[i]) {
            xcorr_peaks->x[j] = peaks->x[i] ;
            xcorr_peaks->y[j] = peaks->y[i] ;
            xcorr_peaks->z[j] = peaks->z[i] ;
            j++ ;
        }
        i++ ;
    }

    /* Free and return */
    double3_del(peaks) ;
    free(valid_flags) ;
    return xcorr_peaks ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find out points within bounds in a given list.
  @param    peaks       List of input points to scrutinize.
  @param    lx          Image size in x.
  @param    ly          Image size in y.
  @param    edge_x      Edge condition on x.
  @param    edge_y      Edge condition on y.
  @param    nvalid      Number of valid points in the list.
  @param    valid_flags Flag array indicating which points are within bounds.
  @return   void
  This function sorts a list of points from the closest to the center to the 
  furthest. Only the points that are not too far fro the center are valid.
 */
/*----------------------------------------------------------------------------*/
void localize_xcorr_centers(
    	double3 *   peaks,
    	int         lx,
    	int         ly,
    	int         edge_x,
    	int         edge_y,
    	int     *   nvalid,
    	int     *   valid_flags)
{
    double      x, y ;
    int         cx, cy ;
    int         i, count ;

    cx = lx/2 ;
    cy = ly/2 ;
    for (i=0 ; i<peaks->n ; i++) {
        x = peaks->x[i] ;
        y = peaks->y[i] ;
        peaks->z[i] = (x-(double)cx)*(x-(double)cx) +
                      (y-(double)cy)*(y-(double)cy) ;
    }
    /* sort out peaks by increasing distance from the image center */
    double3_sort(peaks, 1);

    /* identify valid peaks */
    count = 0 ;
    for (i=0 ; i<peaks->n ; i++) {
        cx = (int)peaks->x[i] ;
        cy = (int)peaks->y[i] ;
        if ((cx >= edge_x) && (cx <= (lx - edge_x)) &&
            (cy >= edge_y) && (cy <= (ly - edge_y))) {
            valid_flags[i] = 1 ;
            count++ ;
        } else valid_flags[i] = 0 ;
    }
	*nvalid = count ;
    return ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Compare a reference plane to every plane in the cube.
  @param    to_compare      Cube to be compared plane by plane.
  @param    pattern         Reference image to compare with.
  @param    estimates       First-guess estimation of the offsets.
  @param    xcorr_p         List of anchor points.
  @param    search_width    Half-width of search area.
  @param    search_height   Half-height of search area.
  @param    hx              Half-width of measurement area.
  @param    hy              Half-height of measurement area.
  @return   A newly allocated offset measurement as a double3 object.
 
  This function expects in input a reference frame (pattern) against
  which every plane in the input cube (to_compare) will be matched.
  The matching is performed using a 2d cross-correlation, using a
  minimal squared differences criterion. One measurement is performed
  per input anchor point, and the median offset is returned together
  with a measure of similarity for each plane.
 
  The search is performed over 2*search_width+1 times
  2*search_height+1 and at every point 2*hx+1 times 2*hy+1
  measurements are taken.
 
  See other functions in this module to learn more about how the
  cross-correlation is computed.

  The returned object must be deallocated using double3_del().
 */
/*----------------------------------------------------------------------------*/
double3 * xcorr_with_objs(
		cube_t    	*	to_compare,
		image_t   	*	pattern,
		double3		*	estimates,
		double3		*	xcorr_p,
		int        		search_width,
		int        		search_height,
		int				hx,
		int				hy)
{
    double3		*	offsets ;
    double3		*	one_offset ;
    double3		*	one_estimate ;
	image_t 	*	med_pattern ;
	image_t 	*  	med_compare ;

    int        		i ;
    
	/* Error handling : test entries    */
	if (to_compare==NULL || pattern==NULL || xcorr_p==NULL) return NULL ;

	offsets = double3_new(to_compare->np);
	med_pattern = image_filter_median(pattern);

    for (i=0 ; i<to_compare->np ; i++) {
		compute_status("cross-correlating", i, to_compare->np, 1) ;

		/* Check if an estimate was given */
		if (estimates!=NULL) {
			one_estimate = double3_new(1);
			one_estimate->x[0] = estimates->x[i] ;
			one_estimate->y[0] = estimates->y[i] ;
			one_estimate->z[0] = estimates->z[i] ;
		} else {
			one_estimate = NULL ;
		}

		/* Median filter the input images */
		med_compare = image_filter_median(to_compare->plane[i]);
		/* Perform cross-correlation */
		one_offset = xcorr_get_median_offset( med_pattern,
											  med_compare,
											  one_estimate,
											  xcorr_p,
											  search_width,
											  search_height,
											  hx,
											  hy) ;
		image_del(med_compare);

		/* Deallocate temporary estimate */
		if (one_estimate!=NULL) double3_del(one_estimate);

		if (one_offset==NULL) {
			/* Nothing was found. */
			/* declare the offset as invalid: null offsets and dist=-1 */
			offsets->x[i] =  0.00 ;
			offsets->y[i] =  0.00 ;
			offsets->z[i] = -1.00 ;
		} else {
			/* Something was found. */
			/* One standard failure case is when the returned offset is */
			/* located on the border of the search zone. Identify such */
			/* cases and flag the frame as not registrable by setting */
			/* the offset vector to nil and the distance to -1. */
			if ((fabs(one_offset->x[0]-(double)search_width)<1e-2) ||
				(fabs(one_offset->y[0]-(double)search_height)<1e-2)) {
				e_warning("frame %d does not X-correlate: discard frame", i+1);
				offsets->x[i] =  0.00 ;
				offsets->y[i] =  0.00 ;
				offsets->z[i] = -1.00 ;
			} else {
				/* The returned offset is correct */
				offsets->x[i] = one_offset->x[0] ;
				offsets->y[i] = one_offset->y[0] ;
				offsets->z[i] = one_offset->z[0] ;
			}
			/* Deallocate temporary return value */
			double3_del(one_offset) ;
		}
    }
	image_del(med_pattern);
    return offsets ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the median offset between 2 images.
  @param    reference       Reference image.
  @param    compared        Compared image.
  @param    estimate        First-guess estimates of the offsets.
  @param    xcorr_p         List of anchor points.
  @param    search_width    Search width in pixels (+/-)
  @param    search_height   Search height in pixels (+/-)
  @param    hx              Measurement area half-width.
  @param    hy              Measurement area half-height.
  @return   Newly allocated list of offset measurements as a double3.
 
  This function compares two images using a 2d cross-correlation. The
  input images are expected to be more or less the same, up to a shift
  in x and y. The returned offset measures the translation of the
  'compared' image with respect to the 'reference' image.
 
  The cross-correlation is carried out for each anchor point given in
  the list. It is Ok to provide only one anchor point in xcorr_p.
 
  For each point, a cross-correlation criterion will be computed on a
  grid of 2hx+1 by 2hy+1, on the search area defined by search_width
  and search_height. The total search size is 2*search_width+1 by
  2*search_height+1. The total number of pixel operations is quite
  high: number of anchor points times number of search area pixels
  times number of measurement area pixels.
 
  The returned measurement is stored into a double3 object.
  This returned object must be freed using double3_del().
 */
/*----------------------------------------------------------------------------*/
double3 * xcorr_get_median_offset(
		image_t			*	reference,
		image_t			*	compared,
		double3			*	estimate,
		double3			*	xcorr_p,
		int					search_width,	
		int					search_height,	
		int					hx,
		int					hy)
{
	double3	*	delta ;
	double3	*	measure ;
	int			i, j ;
	double		init_dx,
				init_dy ;
	int			ix, iy ;
	double		median_dx,
				median_dy ;
	int			best_rank ;
	double		diff,
				min_diff ;
	int			at_x1, at_y1;
	int			valid_pts ;
	double		cdx, cdy ;

	if (reference==NULL || compared==NULL) return NULL ;
    if (search_width <= 0) search_width = CORR_DX_MAX ;
    if (search_height <= 0) search_height = CORR_DY_MAX ;
	if (estimate!=NULL) {
		init_dx = estimate->x[0] ; init_dy = estimate->y[0] ;
	} else {
		init_dx = 0.00 ; init_dy = 0.00 ;
	}
	ix = (int)init_dx;
	iy = (int)init_dy;

	/* Loop on all correlating points */
	delta = double3_new(xcorr_p->n);
	valid_pts = 0 ;
	for (i=0 ; i<xcorr_p->n ; i++) {
		at_x1 = (int)(xcorr_p->x[i]) ;
		at_y1 = (int)(xcorr_p->y[i]) ;
		if ((at_x1+ix < search_width+hx) ||
			(at_x1+ix >= (compared->lx-search_width-hx)) ||
			(at_y1+iy < search_height+hy) ||
			(at_y1+iy >= (compared->ly-search_height-hy))) {
			/* This point is declared invalid in the current image */
			delta->x[i] =  0.0 ;
			delta->y[i] =  0.0 ;
			delta->z[i] = -1.0 ;
		} else {
			delta->z[i] = xcorr_private(
								/* image buffers */
								reference->data,
								compared->data,
								/* buffer widths */
								reference->lx,
								reference->ly,
								compared->lx,
								compared->ly,
								/* where to search in each image */
								at_x1,
								at_y1,
								at_x1 + ix,
								at_y1 + iy,
								/* search area size */
								search_width,
								search_height,
								/* measure area size */
								hx,
								hy,
								/* returned computed offset */
								&cdx,
								&cdy);
			/* Save computed offset into delta array */
			delta->x[i] = cdx ;
			delta->y[i] = cdy ;
			if (delta->z[i] > -1e-16) valid_pts ++ ;
		}
	}

	/* Test if there are valid points */
	if (valid_pts < 1) {
		e_error("no valid point found for correlation") ;
		double3_del(delta);
		return NULL ;
	}

	/* If there was a single measurement point, return the offset */
	if (valid_pts == 1) {
		measure = double3_new(1);
		i=0 ;
		while (delta->z[i]<0.0) i++ ;
		measure->x[0] = (double)ix - delta->x[i] ;
		measure->y[0] = (double)iy - delta->y[i] ;
		measure->z[0] = delta->z[i] ;
		double3_del(delta);
		return measure ;
	}

	/* Squeeze the input list to keep only valid points */
	if (valid_pts<xcorr_p->n) {
		measure = double3_new(valid_pts);
		i=0 ; j=0 ;
		for (i=0 ; i<xcorr_p->n ; i++) {
			if (delta->z[i]>=0.0) {
				measure->x[j] = delta->x[i] ;
				measure->y[j] = delta->y[i] ;
				measure->z[j] = delta->z[i] ;
				j++ ;
			}
		}
		double3_del(delta);
		delta = measure ;
	}

	/* From all correlation measurement, compute a median offset */
	median_dx = double_median(delta->x, delta->n);
	median_dy = double_median(delta->y, delta->n);

	/* Find the offset measurement which is the closest to this median */
	best_rank = 0 ;
	min_diff = fabs(delta->x[0]-median_dx) + fabs(delta->y[0]-median_dy) ;

	for (i=0 ; i<delta->n ; i++) {
		diff = fabs(delta->x[i]-median_dx) + fabs(delta->y[i]-median_dy) ;
		if (diff<min_diff)
			best_rank=i ;
	}
	measure = double3_new(1);
	measure->x[0] = (double)ix - delta->x[best_rank] ;
	measure->y[0] = (double)iy - delta->y[best_rank] ;
	measure->z[0] = delta->z[best_rank] ;

	double3_del(delta);
	return measure ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Estimate the minimal squared difference between 2 image buffers.
  @param    buffer_in1      Reference pixel buffer.
  @param    buffer_in2      Compared pixel buffer.
  @param    lx1             X size of buffer1.
  @param	ly1				Y size of buffer1.
  @param    lx2             X size of buffer2.
  @param	ly2				Y size of buffer2.
  @param    at_x1           X-coordinate of search center in buffer1.
  @param    at_y1           Y-coordinate of search center in buffer1.
  @param    at_x2           X-coordinate of search center in buffer2.
  @param    at_y2           Y-coordinate of search center in buffer2.
  @param    dx_max          Search area half-width.
  @param    dy_max          Search area half-height.
  @param    hx              Measurement area half-width.
  @param    hy              Measurement area half-height.
  @param    dx              Returned apodized position in x.
  @param    dy              Returned apodized position in y.
  @return   A double measuring the minimal squared difference factor.
 
  This is the low-level function performing the 2d cross-correlation.
  It is very configurable and thus not easy to use! Users in need of a
  cross-correlation function should be using a higher-level function.
 
  Notice that the returned position is apodized to subpixel precision.
 
  The double returned by the function measures the lowest squared
  difference between the two input buffers (in the search area and
  over the measurement area as requested by the caller). It is often a
  good indicator of how well the cross-correlation performed.
 
  Be aware that this function is only a mathematical operator and does
  not try to apply any quality criterion over the results it is
  returning. It will find a place of best matching, but will not try
  to judge the quality of the match. This is left to the caller of
  this function...
 
  Good indicators of a match failure are usually an unusually high
  lowest squared difference factor, or a found displacement that is
  exactly on the border of the search area.
 */
/*----------------------------------------------------------------------------*/
static double xcorr_private(
		pixelvalue	*	buffer_in1,	/* Search is made on this buffer */
		pixelvalue	*	buffer_in2, /* The pattern we are looking for */
		int     		lx1,        /* nb pixels/row buffer 1 */
		int				ly1,		/* nb of rows in buffer1 */
		int     		lx2,        /* nb pixels/row buffer 2 */
		int				ly2, 		/* nb of rows in buffer2 */
		int     		at_x1,      /* Search center in buffer 1 */
		int     		at_y1,
		int     		at_x2,      /* Search center in buffer 2 */
		int     		at_y2,
		int			    dx_max,     /* Search area size */
		int     		dy_max,
		int     		hx,         /* Measure area size */
		int     		hy,
		double   	*	dx,         /* Returned apodized position in x */
		double   	*	dy)         /* Returned apodized position in y */
{
    double				*	distances ;
    int						inc1,
							inc2 ;
    double					inv_surface ;
    int						k_min,
							l_min ;
    double					somme_min ;
    register pixelvalue	*	reg1,
						*	reg2 ;
    register double			value,
							somme ;
    double					inc_x, 
							inc_y ;
    int						pos_min ;
    double					best_distance ;
	
	int						i,
							j,
							k,
							l ;
    /* Error handling: test entries */
    if (buffer_in1==NULL || buffer_in2==NULL) {
		*dx=0.0 ;
		*dy=0.0 ;
		return -1.0 ;
    }
    if ((at_x1<=0) || (at_x1>=lx1) || (at_x2<=0) || (at_x2>=lx2)) {
        e_error("value out of bounds for requested correlation center") ;
		*dx=0.0 ;
		*dy=0.0 ;
        return -1.0 ;
    }
	if ((at_x1 <= dx_max+hx) || (at_y1 <= dy_max+hy) ||
		(at_x1 >= lx1-(dx_max+hx)) || (at_y1 >= ly1-(dy_max+hy))) {
		e_error("value out of bounds for requested correlation center") ;
		*dx=0.0 ;
		*dy=0.0 ;
		return -1.0 ;
	}
	if ((at_x2 <= dx_max+hx) || (at_y2 <= dy_max+hy) ||
		(at_x2 >= lx2-(dx_max+hx)) || (at_y2 >= ly2-(dy_max+hy))) {
		e_error("value out of bounds for requested correlation center") ;
		*dx=0.0 ;
		*dy=0.0 ;
		return -1.0 ;
	}
	
    distances = malloc((2*dy_max+1)*(2*dx_max+1)*sizeof(double));
    somme_min = (double)MAX_PIX_VALUE*(double)MAX_PIX_VALUE*
                (double)((2*hy+1)*(2*hx+1)) ;
    k_min = l_min = 0 ;

    /* Move into the buffers, up to the requested searching place   */
    buffer_in1 += at_x1 + at_y1*lx1 ;
    buffer_in2 += at_x2 + at_y2*lx2 ;
    inc1 = lx1-hx-hx-1;
    inc2 = lx2-hx-hx-1;
 
    inv_surface = 1.0 / ((double)(2*hx+1)*(double)(2*hy+1)) ;
 
    for (l=-dy_max;l<=dy_max;l++)
		for (k=-dx_max;k<=dx_max;k++) {
            somme = 0;
			reg1 = buffer_in1+k-hx+(l-hy)*lx1;
			reg2 = buffer_in2-hx-hy*lx2;
			for (j=-hy;j<=hy;j++) {
                for (i=-hx;i<=hx;i++) {
					value = (double)(*reg1++)-(double)(*reg2++);
					value *= value;
                    somme += value;
                }
                reg1+=inc1;
                reg2+=inc2;
            }
            if (somme<somme_min) {
                l_min = l;
                k_min = k;
                somme_min = somme;
            }
			distances[dx_max+k+(2*dx_max+1)*(dy_max+l)] = somme * inv_surface ;
		}
	
	/* xcorr_apodisation : sub pixel correlation */
    pos_min = dx_max+k_min+(2*dx_max+1)*(dy_max+l_min) ;
    best_distance = distances[pos_min] ;
 
    /* Take care of edge effects in measure */
    if ((k_min == -dx_max)||(k_min == dx_max)) inc_x = 0.0 ;
    else inc_x = xcorr_apodisation(distances[pos_min-1],
                                distances[pos_min],
                                distances[pos_min+1]) ;
 
    if ((l_min == -dy_max)||(l_min == dy_max)) inc_y = 0.0 ;
    else inc_y = xcorr_apodisation(distances[pos_min-(2*dx_max+1)],
                                distances[pos_min],
                                distances[pos_min+(2*dx_max+1)]) ;
 
    *dx = (double)k_min + inc_x  ;
    *dy = (double)l_min + inc_y ;

    free(distances) ;
    return(best_distance) ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Subpixel accuracy in correlation.
  @param	d1	left squared diff.
  @param	d2	minimal distance found in correlation.
  @param	d3	right squared diff.
  @return	Apodized location of lowest squared difference.

  The subpixel precision is achieved by fitting a parabola over the
  minimal squared differences, and looking for a minimum. Absurd
  values (lower than -0.5 or greater than 0.5) are clipped to the
  closest half-integer.

 */
/*----------------------------------------------------------------------------*/
static double xcorr_apodisation(
		double   d1,
		double   d2,
		double   d3)
{
    double   value ;

	/* d2 is the minimal distance found in correlation */
    /* d1 and d3 are the closest distance values around d2 (1d signal) */

    /* Special cases */
	if (fabs(d1-d2)<1e-8) return -0.5 ;
	if (fabs(d2-d3)<1e-8) return  0.5 ;
	if (fabs(d1-d3)<1e-8) return  0.0 ;

	value = (0.5*((d1-d3)/(d1-(2.0*d2)+d3)));

    /* Clipping of absurd values */
    if (value>0.5) value = 0.5 ;
    if (value<-0.5) value = -0.5 ;
    return value ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Get a list of offset measurements from an ASCII file.
  @param    filename        Name of the ASCII file to read.
  @return   A newly allocated list of offset measurements as a double3.
 
  The input ASCII file is expected to contain three columns of
  numbers. The first column contains the plane number (integer), the
  second one contains the offset in X (floating-point or integer), the
  third one contains the offset in Y (floating-point or integer).
 
  The first line in the ASCII file indicates what kind of offsets are
  provided in the file. If the first pair of offsets is (0,0) the offset
  are considered as relative offsets since no object can be at that
  position, i.e. offsets are all relative to the first frame.
  Otherwise the offsets are considered absolute, i.e. they indicate
  the position of the same object in all frames.
 
  Here is an example of a valid offset file:
  \begin{verbatim}
  # Comment lines start with a hash
  1   0   0
  2  -38  45
  3  3.4  2.3
  \end{verbatim}
 
  Notice that plane numbers start at 1.

  The returned object is a double3. At this stage, only the x and y fields
  are used, the z field is left initialized to 0.00 because it has not
  been measured but read from a file. The returned object must be
  deallocated using double3_del().
 */
/*----------------------------------------------------------------------------*/
double3 * load_offsets_from_txtfile(char * filename)
{
    double3  *   offsets ;
    int          i ;

	offsets = double3_read(filename);
	if (offsets==NULL) {
		e_error("reading offset file [%s]", filename);
		return NULL ;
	}

	for (i=0 ; i<offsets->n ; i++) {
		offsets->x[i] = offsets->x[i] ;
		offsets->y[i] = offsets->y[i] ;
		offsets->z[i] = 0.00 ;
	}

    /* If given offsets are absolute, convert them to relative */
	if ((fabs(offsets->x[0])>1e-2) || (fabs(offsets->y[0])>1e-2)) {
		for (i=offsets->n-1 ; i>=0 ; i--) {
			offsets->x[i] -= offsets->x[0] ;
			offsets->y[i] -= offsets->y[0] ;
		}
    }
    return offsets;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Find offsets in an image sequence without a priori information.
  @param    cube_i      Input cube   
  @param    pattern     Pattern to compare all planes in the cube.
  @return   1 newly allocated double3 object containing offset estimates.

  This function applies the image processing textbook method to detect simple 
  shifts between frames. It is very sensitive to offset and gain variations 
  between images, so be careful.

  The input images can have any size > 256x256.
 
  Blind zone size must be a power of 2 and all images going through an FFT 
  must be square.
 */
/*----------------------------------------------------------------------------*/
#define BLIND_ZONE_INIT 512
double3 * cube_blindoffsets(
        cube_t  *   cube_i, 
        image_t *   pattern)
{
    image_t *   ext_pat ;
    image_t *   sub_pat ;
    image_t *   sub_pla ;
    image_t *   ext_pla ;

    cube_t  *   fft_plane ;
    cube_t  *   fft_pattern ;
    cube_t  *   xcorr_res ;
    int         p ;
    int         i ;
    int         npix ;
    char        fname[FILENAMESZ];
    double      x1, y1, x2, y2, x, y ;
    int         px, py ;
    double3 *   offs ;
    pixelvalue  mean ;
    int         xmin, xmax, ymin, ymax ;
    int         blind_zone ;

    /* Initialize */
    blind_zone = BLIND_ZONE_INIT ; 

    /* Bulletproof */
    if (cube_i==NULL || pattern==NULL) return NULL ;
    if (cube_i->lx != pattern->lx || cube_i->ly != pattern->ly) return NULL ;

    if ((pattern->lx < blind_zone) || (pattern->ly < blind_zone)) {
        e_warning("Correlation zone reduced to fit in inputs (%d->%d)",
                blind_zone, blind_zone/2);
        blind_zone /= 2 ;
        if ((pattern->lx < blind_zone) || (pattern->ly < blind_zone)) {
            e_warning("Correlation zone reduced to fit in inputs (%d->%d)",
                    blind_zone, blind_zone/2);
            blind_zone /= 2 ;
            if ((pattern->lx < blind_zone) || (pattern->ly < blind_zone)) {
                e_warning("Correlation zone reduced to fit in inputs (%d->%d)",
                        blind_zone, blind_zone/2);
                blind_zone /= 2 ;
                if ((pattern->lx < blind_zone) || (pattern->ly < blind_zone)) {
                    e_warning("Correlation zone > inputs:  %dx%d > %dx%d",
                        blind_zone, blind_zone, pattern->lx, pattern->ly) ;
                    return NULL ;
                }
            }
        }
    }

    /* Compute position of central image part */
    xmin = (pattern->lx / 2) - blind_zone/2 + 1 ;
    ymin = (pattern->ly / 2) - blind_zone/2 + 1 ;
    xmax = xmin + blind_zone - 1 ;
    ymax = ymin + blind_zone - 1 ;

    e_comment(2, "pre-processing pattern...");

    /* Extract central part of the image */
    ext_pat = image_getvig(pattern, xmin, ymin, xmax, ymax);
    /* Subsample by a factor 2 */
    sub_pat = image_subsample(ext_pat);
    image_del(ext_pat);
    npix = sub_pat->lx * sub_pat->ly ;

    /* Subtract central value */
    mean = (pixelvalue)image_getmean(sub_pat);
    for (i=0 ; i<npix ; i++) {
        sub_pat->data[i] -= mean ;
    }
    /* Compute FFT on input pattern */
    fft_pattern = image_fft(sub_pat, NULL, FFT_FORWARD);
    image_del(sub_pat);
    /* Pattern is now ready */

    /* Loop on all input planes */
    offs = double3_new(cube_i->np);
    for (p=0 ; p<cube_i->np ; p++) {
        compute_status("blind offsets...", p, cube_i->np, 2);

        /* Extract central part of the image */
        ext_pla = image_getvig(cube_i->plane[p], xmin, ymin, xmax, ymax);

        /* Subsample by a factor 2 */
        sub_pla = image_subsample(ext_pla);
        image_del(ext_pla);

        /* Subtract central value */
        mean = (pixelvalue)image_getmean(sub_pla);
        for (i=0 ; i<npix ; i++) {
            sub_pla->data[i] -= mean ;
        }

        /* Compute FFT for this plane */
        fft_plane = image_fft(sub_pla, NULL, FFT_FORWARD);
        image_del(sub_pla);
        if (fft_plane==NULL) {
            e_error("cannot compute FFT");
            cube_del(fft_pattern);
            double3_del(offs);
            return NULL ;
        }

        /*
         * Multiply patterns in Fourier space
         * Warning: this is a multiplication in C (complex)
         * of the pattern and the candidate (conjugate) so:
         * (x1 + i.y1) * (x2 + i.y2) = (x1.x2 - y1.y2) + i*(x1.y2 + x2.y1)
         */
        for (i=0 ; i<npix ; i++) {
            x1 =  (double)fft_pattern->plane[0]->data[i] ;
            y1 =  (double)fft_pattern->plane[1]->data[i] ;

            x2 =  (double)fft_plane->plane[0]->data[i] ;
            y2 = -(double)fft_plane->plane[1]->data[i] ;

            x = x1*x2 - y1*y2 ;
            y = x1*y2 + x2*y1 ;

            fft_plane->plane[0]->data[i] = (pixelvalue)x ;
            fft_plane->plane[1]->data[i] = (pixelvalue)y ;

        }
        /* Back to image space */
        xcorr_res = image_fft(fft_plane->plane[0],
                              fft_plane->plane[1],
                              FFT_INVERSE);
        cube_del(fft_plane);

        /* Delete imaginary part, contains only numerical noise */
        image_del(xcorr_res->plane[1]);
        xcorr_res->plane[1]=NULL ;

        /* Swap quadrants to put image back in center */
        image_swapquad(xcorr_res->plane[0]);

        /* Save xcorr image in debug mode */
        if (debug_active()>1) {
            sprintf(fname, "xcorr_%02d_x.fits", p+1);
            image_save_fits(xcorr_res->plane[0], fname, BPP_DEFAULT);
        }

        /* Get image maximum */
        image_getmaxpos(xcorr_res->plane[0], &px, &py);
        cube_del(xcorr_res);

        /*
         * Bring offsets back to initial referential. The factor 2 comes from 
         * the subsampling by 2. The additive factor comes from the position 
         * of the center of the subsampled frame.
         */
        offs->x[p] = - (double)(2.0 * (px-blind_zone/4) ) ;
        offs->y[p] = - (double)(2.0 * (py-blind_zone/4) ) ;
    }
    cube_del(fft_pattern);

    return offs ;
}

