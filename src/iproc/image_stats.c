/*----------------------------------------------------------------------------*/
/**
   @file	image_stats.c
   @author	Nicolas Devillard
   @date	Aug 22, 1995
   @version	$Revision: 1.83 $
   @brief	statistics computation for images
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: image_stats.c,v 1.83 2003/11/28 09:33:11 llundin Exp $
	$Author: llundin $
	$Date: 2003/11/28 09:33:11 $
	$Revision: 1.83 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include "image_stats.h"
#include "photometry.h"
#include "dstats.h"
#include "pi.h"
#include "random.h"
#include "generate.h"
#include "detect.h"
#include "detector.h"
#include "histogram.h"
#include "function_1d.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

/* Determined empiracally by C. Lidman for Strehl error computation */
#define STREHL_ERROR_COEFFICIENT    M_PI * 0.007 / 0.0271 

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute various statistics of an image.
  @param	image_in	Input image.
  @return	1 newly allocated image_stats structure.

  Compute various images statistics. Results are all stored into a returned
  structure, than must be deallocated using free(). See the structure details 
  in local_types.h.
 */
/*----------------------------------------------------------------------------*/
image_stats * image_getstats(image_t *image_in)
{
    int		      	i ;
    pixelvalue  	max_pix,
					min_pix;
	pixelvalue		median_pix ;
    pixelvalue  *	curr_pix ;
    double      	pix_sum,
					sqr_sum,
    				abs_sum ;
    int		     	min_pos,
					max_pos ;
    image_stats *	ret_stats ;
	int				npix ;

	if (image_in==NULL) return NULL ;
    ret_stats = malloc(sizeof(image_stats)) ;

    min_pix = (pixelvalue)image_in->data[0];
    max_pix = (pixelvalue)image_in->data[0];
    min_pos = max_pos = 0L ;

    pix_sum = sqr_sum = abs_sum = 0.0 ;
    curr_pix = image_in->data ;
	npix = image_in->lx * image_in->ly ;
    for (i=0 ; i<npix ; i++) {
        if (*curr_pix < min_pix) {
            min_pix = *curr_pix ;
            min_pos = i ;
        } else {
            if (*curr_pix > max_pix) {
                max_pix = *curr_pix ;
                max_pos = i ;
            }
        }

        pix_sum += (double)*curr_pix ;
        abs_sum += (double)fabs(*curr_pix) ;
        sqr_sum += ((double)*curr_pix) * ((double)*curr_pix) ; 
        curr_pix++ ;
    }

    ret_stats->flux = pix_sum ;
    ret_stats->absflux = abs_sum ;
    ret_stats->energy = sqr_sum ;

    ret_stats->min_pix = min_pix ;
    ret_stats->min_x = (int)(min_pos % (int)image_in->lx) ;
    ret_stats->min_y = (int)(min_pos / (int)image_in->lx) ;

    ret_stats->max_pix = max_pix ;
    ret_stats->max_x = (int)(max_pos % (int)image_in->lx) ;
    ret_stats->max_y = (int)(max_pos / (int)image_in->lx) ;
    
    ret_stats->avg_pix = pix_sum/(double)npix ;
    /* Rounding errors can cause the variance to be negative */
    ret_stats->stdev = (sqr_sum-((pix_sum*pix_sum)/(double)npix))
                     / ((double)npix-1.0);
    ret_stats->stdev = ret_stats->stdev > 0 ? sqrt(ret_stats->stdev) : 0;

	ret_stats->npix = npix ;

	/* Compute median pixel */
	median_pix = (double)image_getmedian(image_in);
	ret_stats->median_pix = median_pix ;

    return ret_stats ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute various statistics of an image.
  @param	image_in	Input image.
  @param	map			(optional) map of valid pixels
  @param	val_range	(optional) pixel bounds.
  @param	zone		(optional) zone definition for stat computation
  @return	1 newly allocated image_stats structure.

  Compute various images statistics. Results are all stored into a returned
  structure, than must be deallocated using free(). See the structure details 
  in local_types.h.

  This function allows to compute statistics only on pixels satisfying some 
  criteria. Possible inputs are:

  - A bad pixel map, which must have the same image size. All pixels set to
    1 are taken into account, pixels set to zero are rejected.
  - A pixel value range. Only pixels with values falling in this interval
    are considered. The given parameter must be a pointer to an array of
    two pixelvalue elements, giving respectively the low and high bound for
    pixels to be considered.
  - A zone defined as {xmin,xmax,ymin,ymax}. Only pixels within this image
    zone will be considered (borders are inclusive). Zone definition is
    given using the FITS convention (from 1 to lx and from 1 to ly).

  It is Ok to provide NULL for all optional parameters.
 */
/*----------------------------------------------------------------------------*/
image_stats * image_getstats_opts(
		image_t		*	in,
		pixelmap	*	map,
		pixelvalue	*	val_range,
		int			*	zone)
{
	int				i, j, pos ;
	int				valid ;
	int				npix ;
	int				bufsz ;
	pixelvalue	*	pixbuf ;
	pixelvalue		curpix ;
	int				xmin, xmax, ymin, ymax ;
    int		     	min_flag,
					max_flag ;
    image_stats *	ret_stats ;
	double			sum,
					sqsum,
					asum ;

	if (in==NULL) return NULL ;

	/* Stupid check */
	if (map==NULL && val_range==NULL && zone==NULL) {
		return image_getstats(in);
	}

	/* Validate pixel map size */
	if (map!=NULL) {
		if (in->lx!=map->lx || in->ly!=map->ly) {
			e_error("pixel map size and image size do not match");
			return NULL ;
		}
	}

	/* Set interest zone */
	if (zone!=NULL) {
		/* Check zone validity */
		if (zone[0]<0 || zone[1]<0 || zone[2]<0 || zone[3]<0) {
			e_error("bound error for zone: xrange[%d %d] yrange[%d %d]",
					zone[0],
					zone[1],
					zone[2],
					zone[3]);
			return NULL ;
		}
		xmin = (zone[0]<1) ? 1 : (zone[0]>in->lx) ? in->lx : zone[0] ;
		xmax = (zone[1]<1) ? 1 : (zone[1]>in->lx) ? in->lx : zone[1] ;
		ymin = (zone[2]<1) ? 1 : (zone[2]>in->ly) ? in->ly : zone[2] ;
		ymax = (zone[3]<1) ? 1 : (zone[3]>in->ly) ? in->ly : zone[3] ;
		/* Check bound order */
		if (xmin>xmax) {
			e_error("bound error for zone: xmin=%d xmax=%d", xmin, xmax);
			return NULL ;
		}
		if (ymin>ymax) {
			e_error("bound error for zone: ymin=%d ymax=%d", ymin, ymax);
			return NULL ;
		}
		/* Switch lower bounds to C notation */
		xmin-- ;
		ymin-- ;
		/* Higher bounds are not switched to keep a C-like notation */
	} else {
		xmin = 0;
		xmax = in->lx ;
		ymin = 0 ;
		ymax = in->ly ;
	}

	/*
	 * This first pass gathers various statistics about the valid zone,
	 * taking into account only valid pixels. The number of valid pixels is
	 * also counted for a second pass meant to find the median.
	 */

	ret_stats = calloc(1, sizeof(image_stats));

    min_flag=0 ;
	max_flag=0 ;
	sum = 0.0 ;
	sqsum = 0.0 ;
	asum = 0.0 ;
	bufsz=0 ;
	pos=0 ;
	for (j=ymin ; j<ymax ; j++) {
		for (i=xmin ; i<xmax ; i++) {
			pos = i + (j*in->lx) ;
			valid=1 ;
			if (map!=NULL) {
				if (map->data[pos]==PIXELMAP_0)
					valid = 0 ;
			}
			if (val_range!=NULL) {
				if (in->data[pos]<val_range[0])
					valid = 0 ;
				if (in->data[pos]>val_range[1])
					valid = 0 ;
			}
			if (valid) {
				curpix = in->data[pos] ;
				/* Min */
				if (min_flag==0) {
					ret_stats->min_pix = curpix ;
					min_flag = 1 ;
				} else if (curpix < ret_stats->min_pix) {
					ret_stats->min_pix = curpix ;
					ret_stats->min_x = i ;
					ret_stats->min_y = j ;
				}
				/* Max */
				if (max_flag==0) {
					ret_stats->max_pix = curpix ;
					max_flag = 1 ;
				} else if (curpix > ret_stats->max_pix) {
					ret_stats->max_pix = curpix ;
					ret_stats->max_x = i ;
					ret_stats->max_y = j ;
				}
				/* Sum */
				sum += (double)curpix ;
				/* Square sum */
				sqsum += (double)curpix * (double)curpix ;
				/* Absolute sum */
				asum += fabs((double)curpix);
				bufsz ++ ;
			}
		}
	}

	/* Test that there is at least one valid pixel */
	if (bufsz<1) {
		e_warning("no valid pixel value found for stats");
		free(ret_stats);
		return NULL ;
	}

	/* Finish computation of average, rms, energy, flux and aflux */
    ret_stats->flux 	= sum ;
    ret_stats->absflux 	= asum ;
    ret_stats->energy 	= sqsum ;

    ret_stats->avg_pix = sum/(double)bufsz ;
	if (bufsz==1) {
		ret_stats->stdev = 0 ;
	} else {
        /* Rounding errors can cause the variance to be negative */
        ret_stats->stdev =(sqsum-((sum*sum)/(double)bufsz))/((double)bufsz-1.0);
        ret_stats->stdev =ret_stats->stdev > 0 ? sqrt(ret_stats->stdev) : 0;
	}
	ret_stats->npix = bufsz ;

	/* Now compute median */
	/* Allocate a new array of pixels with only valid pixels */
	pixbuf = malloc(bufsz * sizeof(pixelvalue));

	/* Store valid pixels in pixbuf array */
	npix=0 ;
	for (j=ymin ; j<ymax ; j++) {
		for (i=xmin ; i<xmax ; i++) {
			pos=i+j*in->lx ;
			valid=1 ;
			if (map!=NULL) {
				if (map->data[pos]==PIXELMAP_0)
					valid = 0 ;
			}
			if (val_range!=NULL) {
				if (in->data[pos]<val_range[0])
					valid = 0 ;
				if (in->data[pos]>val_range[1])
					valid = 0 ;
			}
			if (valid) {
				pixbuf[npix] = in->data[pos] ;
				npix ++ ;
			}
		}
	}
	ret_stats->median_pix = (double)median_pixelvalue(pixbuf, bufsz);
	free(pixbuf);
	return ret_stats ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes mean pixel value over an image.
  @param	image_in	Input image.
  @return	1 double.
  Computes the mean pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
double image_getmean(image_t * image_in)
{
    int		 i ;
    double   m ;
    register pixelvalue  * pt ;

	if (image_in==NULL) return 0 ;
    m = (double)0.0 ;
    pt = image_in->data ;
    for (i=0 ; i<(image_in->lx * image_in->ly) ; i++) {
        m += (double)(*pt++) ;
    }

    m /= (double)(image_in->lx * image_in->ly) ;
    return m ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the mean pixel value inside a vignette.
  @param	in		Input image.
  @param	xmin	x min (included in vig)
  @param	xmax	x max (included in vig)
  @param	ymin	y min (included in vig)
  @param	ymax	y max (included in vig)
  @return	1 double.

  Compute the mean pixel value inside a vignette in the image. The vignette
  is defined by its lower-left and upper-right corners, given in the FITS
  convention (x from 1 to lx and from left to right, y from 1 to ly and
  from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
double image_getmean_vig(
		image_t    *	in,
		int				xmin,
		int				xmax,
		int				ymin,
		int				ymax)
{
    int		      	i, j ;
	int				npix ;
    double      	m ;

	if (in==NULL) return 0 ;
    m = (double)0.0 ;

	/* Do some clipping over the boundaries */
	if (xmin<1) xmin=1 ;
	if (ymin<1) ymin=1 ;
	if (xmax>in->lx) xmax=in->lx ;
	if (ymax>in->ly) ymax=in->ly ;
	if ((xmin>xmax) || (ymin>ymax)) return 0;

	/* Switch from FITS to C notation */
	xmin -- ; xmax -- ;
	ymin -- ; ymax -- ;

	npix=0 ;
	for (j=ymin ; j<=ymax ; j++) {
		for (i=xmin ; i<=xmax ; i++) {
			m += (double)in->data[i+j*in->lx];
			npix++ ;
		}
	}
    m /= (double)npix ;
    return m ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes minimum pixel value over an image.
  @param	image_in	Input image.
  @return	1 pixelvalue.
  Finds the minimum pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmin(image_t * image_in)
{
    int			i ;
    pixelvalue  min_pix ;
    register
    pixelvalue  *pt ;

	if (image_in==NULL) return 0.00 ;
    min_pix = image_in->data[0] ; ;
    pt = image_in->data ;
    for (i=0 ; i<(image_in->lx * image_in->ly) ; i++) {
        if (*pt < min_pix)
            min_pix = *pt ;
        pt++ ;
    }
    return min_pix ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes maximum pixel value over an image.
  @param	image_in	Input image.
  @return	1 pixelvalue.
  Finds the maximum pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmax(image_t * in)
{
    int			i ;
    pixelvalue  max_pix ;
    register
    pixelvalue  *pt ;

	if (in==NULL) return 0.00 ;
    max_pix = in->data[0] ; ;
    pt = in->data ;
    for (i=0 ; i<(in->lx * in->ly) ; i++) {
        if (*pt > max_pix)
            max_pix = *pt ;
        pt++ ;
    }
    return max_pix ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the max pixel value inside a vignette.
  @param	in		Input image.
  @param	xmin	x min (included in vig)
  @param	xmax	x max (included in vig)
  @param	ymin	y min (included in vig)
  @param	ymax	y max (included in vig)
  @return	1 pixelvalue.

  Compute the max pixel value inside a vignette in the image. The vignette
  is defined by its lower-left and upper-right corners, given in the FITS
  convention (x from 1 to lx and from left to right, y from 1 to ly and
  from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmax_vig(
		image_t    *	in,
		int				xmin,
		int				xmax,
		int				ymin,
		int				ymax)
{
    double      	m ;
    int		      	i, j ;

	if (in==NULL) return 0.00 ;

	/* Do some clipping over the boundaries */
	if (xmin<1) xmin=1 ;
	if (ymin<1) ymin=1 ;
	if (xmax>in->lx) xmax=in->lx ;
	if (ymax>in->ly) ymax=in->ly ;
	if ((xmin>xmax) || (ymin>ymax)) return 0.00 ;
    
	/* Switch from FITS to C notation */
	xmin -- ; xmax -- ;
	ymin -- ; ymax -- ;

    m = in->data[xmin + ymin * in->lx] ;
	for (j=ymin ; j<=ymax ; j++) {
		for (i=xmin ; i<=xmax ; i++) {
			if ((double)in->data[i+j*in->lx] > m) 
                m = (double)in->data[i+j*in->lx] ;
		}
	}
    return m ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes maximum pixel value and position over an image.
  @param    image_in    Input image.
  @param	px			ptr to the x coordinate of the max position
  @param	py			ptr to the y coordinate of the max position
  @return   1 pixelvalue.
  Finds the maximum pixel value and its position in the image.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmaxpos(
        image_t		*   in,
        int         *   px,
        int         *   py)
{
    pixelvalue      max_value ;
    int             i,
                    j ;

    /* Initialize */
    max_value = 0.0 ;

    for (i=0 ; i<in->lx ; i++) {
        for (j=0 ; j<in->ly ; j++) {
            if (in->data[i+j*in->lx] > max_value) {
                max_value = in->data[i+j*in->lx] ;
                *px = i ;
                *py = j ;
            }
        }
    }
    return max_value ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes median pixel value over an image.
  @param	image_in	Input image.
  @return	1 pixelvalue.
  Finds the median pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmedian(image_t * in)
{
	pixelvalue	*	copybuf ;
	int				npix ;
	pixelvalue		median ;

	if (in==NULL) return 0 ;

	npix = in->lx * in->ly ;
	copybuf = malloc(npix * sizeof(pixelvalue));
	memcpy(copybuf, in->data, npix * sizeof(pixelvalue));
	median = median_pixelvalue(copybuf, npix);
	free(copybuf);
	return median ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the median pixel value inside a vignette.
  @param	image_in	Input image.
  @param	llx			x pos of lower left corner.
  @param	lly			y pos of lower left corner.
  @param	urx			x pos of upper right corner.
  @param	ury			y pos of upper right corner.
  @return	1 pixelvalue.

  Compute the median pixel value inside a vignette in the image. The
  vignette is defined by its lower-left and upper-right corners, given in
  the FITS convention (x from 1 to lx and from left to right, y from 1 to
  ly and from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmedian_vig(
		image_t * 	in,
		int			llx,
		int			lly,
		int			urx,
		int			ury)
{
	image_t	*	clone ;
	pixelvalue		median ;

	clone = image_getvig(in, llx, lly, urx, ury) ;
	if (clone == NULL) {
		e_error("cannot clone image: aborting median search") ;
		return 0.00 ;
	}
	median = median_pixelvalue(clone->data, clone->lx * clone->ly) ;
	image_del(clone) ;
	return median ;
}


/*----------------------------------------------------------------------------*/
/**
    @brief  Computes a moving median on a column within an image
            using a verical window of size window_size.
    @param  in	gray image
    @param  x	the coolumn at which the moving miedian is to be computed
    @param  window_size the size of the moving window,
    @return 1 newly allocated array of ly pixelvalues.

    Median_pixelvalue is destructive, so pixels MUST be copied into
    windowline.  The window_size/2 first and last elements are constant.

    The returned array must be deallocated using free().
*/
/*----------------------------------------------------------------------------*/
pixelvalue * image_getmedian_mov_vert(
        image_t		*   in,
        int             x,
        int             window_size)
{
    pixelvalue  *   local_med  = NULL,
                *   windowline = NULL ;
    int             w2 ;
    int             wdiff ;
    int             i,
                    j,
                    k,
                    l ;

    /* Allocate pixelvalues arrays */
    local_med = calloc(in->ly, sizeof(pixelvalue)) ;
    windowline = calloc(window_size, sizeof(pixelvalue)) ;

    /* Initialize */
    i = x ;
    w2 = window_size / 2 ;

    for (j=0 ; j<in->ly ; j++) {
        l = i ;
        wdiff = w2 - j ;
        if (wdiff > 0) {
            for (k=0 ; k<window_size-wdiff ; k++) {
                windowline[k] = in->data[l] ;
                l += in->lx ;
            }
        } else {
            wdiff = j - in->ly + w2 + 1 ;
            if (wdiff < 0) wdiff = 0 ;
            for (k=0 ; k<window_size-wdiff ; k++) {
                windowline[k] = in->data[l-w2*in->lx] ;
                l += in->lx ;
            }
        }
        local_med[j] = median_pixelvalue(windowline, window_size-wdiff) ;

        i += in->lx ;
    }

    /* Free and return   */
    free(windowline) ;
    return local_med;
}


/*----------------------------------------------------------------------------*/
/*
    @brief  Computes a moving median on a line within an image
    @param  in  gray image
    @param  y   the Y coord. of the line at which the mov. median is computed
    @param  window_size the size of the moving window,
    @return an array of in-lx pixelvalues to be freed by free()
    
	Computes a moving median on a line within an image using a horizontal 
    window of size window_size. The window_size/2 first and last elements are 
    constant. Median_pixelvalue is destructive, so pixels MUST be copied into 
    windowline.
*/
/*----------------------------------------------------------------------------*/
pixelvalue * image_getmedian_mov_horz(
        image_t    	*   in,
        int             y,
        int             window_size)
{
    pixelvalue  *   local_med  = NULL,
                *   windowline = NULL ;
    int         l,
                wdiff,
                w2 ;
    int         i,
                j = 0 ;

    /* Allocate arrays of pixelvalues */
    local_med = calloc(in->lx, sizeof(pixelvalue)) ;
    windowline = calloc(window_size, sizeof(pixelvalue)) ;

    /* Initialize */
    i = y * in->lx ;
    w2 = window_size / 2 ;

    for (j=0 ; j<in->lx ; j++) {
        l = i ;
        wdiff = w2 - j ;
        if (wdiff > 0) {
            memcpy(windowline,
                    &in->data[0],
                    (window_size-wdiff)*sizeof(pixelvalue)) ;
        } else {
            wdiff = j - in->lx + w2 + 1 ;
            if (wdiff < 0) wdiff = 0 ;
            memcpy(windowline,
                    &in->data[i-w2],
                    (window_size-wdiff)*sizeof(pixelvalue)) ;
        }
        local_med[j] = median_pixelvalue(windowline, window_size-wdiff) ;
        i++ ;
    }

    /* Free and return */
    free(windowline) ;
    return local_med ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes kth smallest pixel value over an image.
  @param	image_in	Input image.
  @param	k			Rank of the value to find.
  @return	1 pixelvalue.

  Finds the kth smallest pixel value in the image. k=1 is the minimum,
  k=npix is the maximum, k=npix/2 is the median.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getpercentile(
		image_t 	*	in, 
		int 			k)
{
	image_t	*	clone ;
	pixelvalue		kth ;

	clone = image_copy(in) ;
	if (clone == NULL) {
		return 0.00 ;
	}
	kth = kth_smallest(clone->data, clone->lx * clone->ly, k);
	image_del(clone) ;
	return kth ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the sum of pixel values over an image.
  @param	image_in	Input image.
  @return	1 double.
  Computes the sum of all pixel values in an image.
 */
/*----------------------------------------------------------------------------*/
double image_getsumpix(image_t * image_in)
{
    int			i ;
    double      pix_sum;
    register
    pixelvalue  *pt ;

	if (image_in==NULL) return (pixelvalue)0;
    pix_sum = 0.00 ;
    pt = image_in->data ;
    for (i=0 ; i<(image_in->lx * image_in->ly) ; i++) {
        pix_sum += (double)(*pt) ;
        pt++ ;
    }
    return pix_sum ;
}
 

/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the sum of pixel values inside a vignette.
  @param	image_in	Input image.
  @param	llx			x pos of lower left corner.
  @param	lly			y pos of lower left corner.
  @param	urx			x pos of upper right corner.
  @param	ury			y pos of upper right corner.
  @return	1 double.

  Compute the sum of all pixel values inside a vignette in the image. The
  vignette is defined by its lower-left and upper-right corners, given in
  the FITS convention (x from 1 to lx and from left to right, y from 1 to
  ly and from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
double image_getsumpix_vig(
		image_t		*	inimage,
		int				llx,
		int				lly,
		int				urx,
		int				ury)
{
	int	 			i, j ;
	double			pix_sum ;
	register
	pixelvalue	*	pt ;
	
	if (inimage == NULL) return 0.00 ;

	/* Bullet proof the rectangle coordinates */
	if ((llx<1) || (llx>inimage->lx) ||
		(urx<1) || (urx>inimage->lx) ||
		(lly<1) || (lly>inimage->ly) ||
		(ury<1) || (ury>inimage->ly) ||
		(llx>urx) || (lly>ury)) {
			e_error("invalid rectangle coordinates:\n"
					"lower left is [%d %d] upper right is [%d %d]",
					llx, lly, urx, ury) ;
			return 0.00 ;
	}
	
	pix_sum = 0.00 ;
	pt = inimage->data ;
	
	/* Shift from FITS coordinates to C coordinates */
	llx -- ; lly -- ;
	urx -- ; ury -- ;

	/* Double loop in the rectangle */
	for (j=lly ; j<=ury ; j++) {
		for (i=llx ; i<=urx ; i++) {
			pix_sum += (double)pt[i+j*inimage->lx] ;		
		}
	}
	return pix_sum ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the pixel standard deviation in an image.
  @param	image_in	Input image.
  @return	1 double.
  Finds the stdev. of the pixel value distribution in the input image.
 */
/*----------------------------------------------------------------------------*/
double image_getstdev(image_t * image_in)
{
    double      pix_sum ;
    double      sqr_sum ;
    double      var ;
    int			i ;
    register
    pixelvalue  *pt ;
	int			npix ;

	if (image_in==NULL) return 0.00 ;
	npix = image_in->lx * image_in->ly ;
    pix_sum = sqr_sum = 0.0 ;
    pt = image_in->data ;
    for (i=0 ; i<npix ; i++) {
        pix_sum += (double)(*pt) ;
        sqr_sum += (double)(*pt) * (double)(*pt) ;
        pt++ ;
    }

    /* Rounding errors can cause the variance to be negative */
    var = (sqr_sum-((pix_sum*pix_sum)/(double)npix))/((double)npix-1.0);
    return var > 0 ? sqrt(var) : 0;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes the pixel standard deviation in a vignette.
  @param	in			Input image.
  @param	xmin		X min (included)
  @param	ymin		X max (included)
  @param	xmax		Y min (included)
  @param	ymax		Y max (included)
  @return	1 double.

  Finds the standard deviation of the pixel value distribution
  in a vignette in the input image.
 */
/*----------------------------------------------------------------------------*/
double image_getstdev_vig(
		image_t	*	in,
		int		  	xmin,
		int		  	xmax,
		int		  	ymin,
		int		  	ymax)
{
	double		p ;
    double      pix_sum ;
    double      sqr_sum ;
    int			i, j ;
	int			npix ;

	if (in==NULL) return 0.00 ;

	/* Do some clipping over the boundaries */
	if (xmin<1) xmin=1 ;
	if (ymin<1) ymin=1 ;
	if (xmax>in->lx) xmax=in->lx ;
	if (ymax>in->ly) ymax=in->ly ;
	if ((xmin>xmax) || (ymin>ymax)) return 0;

	/* Switch from FITS to C notation */
	xmin -- ; xmax -- ;
	ymin -- ; ymax -- ;

	npix = 0 ;
    pix_sum = sqr_sum = 0.0 ;
	for (j=ymin ; j<=ymax ; j++) {
		for (i=xmin ; i<=xmax ; i++) {
			p = (double)in->data[i+j*in->lx];
			pix_sum += p ;
			sqr_sum += p * p ;
			npix ++ ;
		}
    }
	if (npix<1) {
		return 0.0 ;
	}
	return
		sqrt((sqr_sum-((pix_sum*pix_sum)/(double)npix))/((double)npix-1.0));
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the energy within a disk.
  @param	image_in	Input image.
  @param	cx			x pos of the disk center.
  @param	cy			y pos of the disk center.
  @param	radius		Disk radius.
  @return	1 double.

  Compute the pixel energy within a circle (the sum of squared pixel values). 
  Provide (-1,-1) as center coordinates if you want to use the center of the 
  image.

  The coordinates of the center are given in the C convention: x from 0 to lx-1
  and y from 0 to ly-1.
 */
/*----------------------------------------------------------------------------*/
double image_get_radenergy(
    	image_t		*	image_in,
    	int    	     	cx, 
    	int    	     	cy,
    	int         	radius)
{
    int      i, j ;
    int      dist ;
    double   energy ;
    double   cur ;

	if (image_in==NULL) return 0.00 ;
    energy = 0.00 ;

    if (cx == -1)
        cx = image_in->lx /2 ;
    if (cy == -1)
        cy = image_in->ly /2 ;
    radius *= radius ;

    for (j=0 ; j<image_in->ly ; j++) {
        for (i=0 ; i<image_in->lx ; i++) {
            /* compute distance from the point to (cx,cy)   */
            dist =  (i-(int)cx)*(i-(int)cx) +
                    (j-(int)cy)*(j-(int)cy) ;

            /* Compare square distance to square radius */
            if (dist<=(int)radius) {
                cur = (double)image_in->data[i+j*image_in->lx] ;
                energy += cur * cur ; 
            }       
        }
    }
    return energy ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Find background noise level in a 1d array around a peak.
  @param	array		1d array of pixel values.
  @param	array_size	Number of pixelvalues in the array.
  @param	max_pos		Position of the peak in the array.
  @return	1 pixelvalue.

  The input signal is assumed to be mostly flat with a peak somewhere. You must
  provide the precise position of the peak as an integer rank in the array. The
  pixelvalues around the peak are used to determine the background level.
 */
/*----------------------------------------------------------------------------*/
pixelvalue find_noise_level_around_peak(
    	pixelvalue	*   array, 
    	int             array_size, 
    	int             max_pos)
{
	double			noise_left,
					noise_right ;
    int          	i;
    pixelvalue   *	smooth_array ;

	if (array==NULL || array_size<1 || max_pos<1) return (pixelvalue)0;
	
    /* Smooth out the array to be less sensitive to noise */
	smooth_array = function1d_filter_lowpass(array,
											 array_size,
											 LOW_PASS_LINEAR,
											 1);

    /* Find noise level on the left side of the peak. */
	i = max_pos ;
	while (i > 0) {
		if (smooth_array[i]>smooth_array[i-1]) i-- ;
		else break;
	}
	noise_left = (double)smooth_array[i] ;

    /* Find noise level on the right side of the peak */
	i = max_pos ;
	while (i < array_size-1) {
		if (smooth_array[i]>smooth_array[i+1]) i++ ;
		else break ;
	}
	noise_right = (double)smooth_array[i] ;

    /* Compute the average of the noise on the right and left peak sides */
	free(smooth_array) ;
	return (pixelvalue)((noise_left + noise_right) / 2.0) ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Interpolate linearly the x pos between two points.
  @param	x1	Known first position.
  @param	y1	Known first position.
  @param	x1	Known second position.
  @param	y1	Known second position.
  @param	y	Known y of the searched position.
  @param	x	Output interpolated x position.
  @return	int 0 if Ok, -1 else.
  Interpolate linearly between two pts to find the x pos for a given y value.
 */
/*----------------------------------------------------------------------------*/
int imstat_x_for_y_between_2_points(
		int         	x1, 
     	pixelvalue 		y1, 
     	int        		x2, 
     	pixelvalue 		y2, 
     	pixelvalue 		y,
     	double		*	x)
{
	double	d[5] ;

	d[0] = (double)x1 ;
	d[1] = (double)x2 ;
	d[2] = (double)y1 ;
	d[3] = (double)y2 ;
	d[4] = (double)y ;

    if( fabs(d[3]-d[2]) < 1e-8 ) {
        return(-1) ;
    } else {
		/* Linear interpolation */
		*x = d[0] + (d[1]-d[0]) * (d[4]-d[2]) / (d[3]-d[2]) ;
    }
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute full width at Y with linear method.
  @param	array		Array of pixelvalues.
  @param	array_size	Size of the array.
  @param	max_pos		Position of the peak in the array.
  @param	Y			Height for width computation.
  @return	1 double

  The expected array of pixelvalues is assumed to be flat, with a peak
  somewhere. The position of the peak is given by max_pos. The width of the
  peak will be computed at height Y. If Y=peak/2, this function computes
  the full width at half maximum (FWHM).
 */
/*----------------------------------------------------------------------------*/
double get_fullwidth_on_y_linear(
    	pixelvalue *    array, 
   	 	int             array_size, 
    	int             max_pos, 
    	double          Y)
{
	double		x_left, x_right ;
    int         x1, x2 ; 
    pixelvalue  y1, y2 ;
    int         i ;

	/* Find first value lower than Y on the left of the maximum */
	i = max_pos ;
	while ((i>0) && (array[i]>Y) ) i-- ;
	if (i==0) {
		return -1.0 ;
	}
	x1 = i ;	y1 = array[x1] ;
	x2 = i+1 ;	y2 = array[x2] ;
    if (imstat_x_for_y_between_2_points(x1,y1,x2,y2,Y,&x_left) != 0){
		return -1.0 ;
	}
	if (x_left<i){
		return -1.0;
	}
	/* Find first value lower than Y on the right of the maximum */
	i = max_pos ;
	while ((i<array_size-1) && (array[i]>Y) ) i++ ;
	if (i==(array_size-1)) {
		return -1.0 ;
	}
	x1 = i-1 ;	y1 = array[x1] ;
	x2 = i ;	y2 = array[x2] ;
    if (imstat_x_for_y_between_2_points(x1,y1,x2,y2,Y,&x_right) != 0){ 
		return -1.0 ;
	}
	if (x_right>=i){
		return -1.0;
	}

	return x_right - x_left ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the FWHM in an image at a given position.
  @param	image_in		Input image.
  @param	thres_flag		Threshold flag.
  @param	thres_value		Threshold value.
  @param	x_expect		x pos of the expected star.
  @param	y_expect		y pos of the expected star.
  @param	half_size_x		X Half size of the search domain.
  @param	half_size_y		Y Half size of the search domain.
  @return	Pointer to a newly allocated array of 2 doubles.

  This function expects an image and the position of a star-like object in this
  image. It will search around the provided position for a maximum, and will 
  compute an FWHM in x and y on this peak.
  The returned pointer contains 2 doubles. It must be deallocated using free().
 */
/*----------------------------------------------------------------------------*/
double * image_getfwhm(
    	image_t    *	image_in,
    	int		     	thres_flag,
    	pixelvalue  	thres_value,
    	int         	x_expect,
    	int         	y_expect,
    	int         	half_size_x,
		int				half_size_y)
{
    int         	x_min,
					y_min,
					x_max,
					y_max ;
    image_t	*	sub_image ;
    image_stats *	sub_ima_stats ;
    pixelvalue  *	Column ;
    pixelvalue  *	Row ;
    pixelvalue  	half_max ;
    double       	fwhm_x,
					fwhm_y ;
    double       *	ret ;

	if (image_in==NULL) return NULL ;

    /* Check that the peak position estimate is in the frame */
    if( (x_expect < 1) ||
        (x_expect > image_in->lx) ||
        (y_expect < 1) ||
        (y_expect > image_in->ly)){
        e_error("peak position estimate out of frame: [%d %d]",
				x_expect, y_expect) ;
        return NULL ;
    }

    x_min = x_expect - half_size_x ;
    y_min = y_expect - half_size_y ;
    x_max = x_expect + half_size_x ;
    y_max = y_expect + half_size_y ;

    if(x_min < 1) x_min = 1 ;
    if(y_min < 1) y_min = 1 ;
    if(x_max > image_in->lx) x_max = image_in->lx ;
    if(y_max > image_in->ly) y_max = image_in->ly ;

    sub_image = image_getvig(image_in,x_min,y_min,x_max,y_max) ;
    sub_ima_stats = image_getstats(sub_image) ;
    image_del(sub_image) ;

    x_expect = sub_ima_stats->max_x + x_min ;
    y_expect = sub_ima_stats->max_y + y_min ;
    free(sub_ima_stats) ;

    x_min = x_expect - half_size_x ;
    y_min = y_expect - half_size_y ;
    x_max = x_expect + half_size_x ;
    y_max = y_expect + half_size_y ;

    if(x_min < 1) x_min = 1 ;
    if(y_min < 1) y_min = 1 ;
    if(x_max > image_in->lx) x_max = image_in->lx ;
    if(y_max > image_in->ly) y_max = image_in->ly ;

    sub_image = image_getvig(image_in,x_min,y_min,x_max,y_max) ;
    sub_ima_stats = image_getstats(sub_image) ;
    image_del(sub_image) ;

	/* extract two arrays centered on the maximum */
    Row    = image_getrow(	image_in, sub_ima_stats->max_y + y_min -1) ;
    Column = image_getcol(	image_in, sub_ima_stats->max_x + x_min -1) ;

    if (!thres_flag ){                                            
        thres_value = find_noise_level_around_peak(Row, image_in->lx,
                                sub_ima_stats->max_y + y_min -1) ;
        thres_value +=  find_noise_level_around_peak(Column, image_in->ly,
                                  sub_ima_stats->max_x + x_min-1) ;
        thres_value = thres_value / 2 ;
    }
    half_max = ( sub_ima_stats->max_pix + thres_value ) / 2 ;
	if (half_max>sub_ima_stats->max_pix){
		if (verbose_active())
			e_warning("Object(%d,%d):  Max(%4d,%4d)=%8.2f hm=%8.2f", 
				x_expect,y_expect,
				(int)sub_ima_stats->max_x + x_min-1,
				(int)sub_ima_stats->max_y + y_min-1, sub_ima_stats->max_pix, 
				half_max);  
		fwhm_x = -1.0;
		fwhm_y = -1.0;
	} else {
    	fwhm_x = get_fullwidth_on_y_linear(Row, image_in->lx,
                             sub_ima_stats->max_x + x_min-1,
                             half_max);
    	fwhm_y = get_fullwidth_on_y_linear(Column, image_in->ly,
						     sub_ima_stats->max_y + y_min-1,
                             half_max);
	}
    free(sub_ima_stats) ;
    free(Row) ;
    free(Column) ;

    ret = malloc(2 * sizeof(double)) ;
    ret[0] = fwhm_x ;
    ret[1] = fwhm_y ;

    return ret ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Computes first and second order image statistics using median.
  @param	in		Input image.
  @param	sigma	Output computed sigma value.
  @return	1 double.

  This function takes an image in input. It tries to estimate the average and 
  standard deviation of the image by approximating them by resp. the median and
  the average absolute distance to the median.

  The median is the returned double. The average absolute distance to the median
  is written into sigma.
 */
/*----------------------------------------------------------------------------*/
double image_median_stat(
		image_t		*	in, 
		double 		*	sigma)
{
	double 	median_val;
	int		i;
	int		npix ;

	npix = in->lx * in->ly ;
	*sigma=0.0;
	median_val = (double)image_getmedian(in) ;
	for (i=0 ; i<npix ; i++) 
		*sigma+= fabs((double)(in->data[i]-median_val)) ;
	*sigma/= (double)npix ; 
	return median_val;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Refine the location of a local maximum in a window.
  @param	img			Reference image containing the window
  @param	px			X position to refine
  @param	py			Y position to refine
  @param	search_hx	Half-size of search zone in x
  @param	search_hy	Half-size of search zone in y
  @param	refpos		output (refined) coordinates
  @return	int 0 if Ok, -1 if error occurred.

  This function is taking care of finding out where a maximum in a window truly
  is. Provide the image you want to search, an approximate position of the 
  center you are looking for, and a search zone definition, through a half-size
  in X and Y.

  Provide also refpos as a pointer to at least two integers, in which the 
  results will be stored.

  The algorithm is the following:
  
  - Extract the sub-window as a vignette.
  - Filter the vignette with a median filter.
  - Locate the maximum pixel in the filtered vignette.
  - Return coordinates of this local maximum.

  This function does not provide sub-pixel precision, it is only meant to 
  locate a local maximum in a sub-window. It is clear that if there is a 
  brighter object in the neighborhood of the object of interest, it will 
  produce a false detection.
 */
/*----------------------------------------------------------------------------*/
int image_locate_peak(
		image_t    	*   img,
    	int             px,
    	int             py,
    	int             search_hx,
    	int             search_hy,
    	int         *   refpos)
{
    image_t     *   ref_vig ;
    image_t     *   filt_vig ;
    image_stats *   stats ;

    /* check entries */
    if (img==NULL || refpos==NULL) return -1 ;
	if (px<0 || px>=img->lx || py<0 || py>=img->ly) return -1 ;
	if (search_hx<1 || search_hy<1) return -1 ;
    if (px<search_hx || px>(img->lx-search_hx) ||
        py<search_hy || py>(img->ly-search_hy)) return -1 ;

    refpos[0] = px ;
    refpos[1] = py ;
    ref_vig = image_getvig(img,
						   px - search_hx, py - search_hy,
						   px + search_hx, py + search_hy) ;
    if (ref_vig==NULL)
		return -1 ;
    filt_vig = image_filter_median(ref_vig);
    image_del(ref_vig) ;
    if (filt_vig==NULL)
		return -1 ;
    stats = image_getstats(filt_vig) ;
    image_del(filt_vig) ;
    if (stats==NULL)
		return -1 ;

    refpos[0] = stats->max_x + px - search_hx ;
    refpos[1] = stats->max_y + py - search_hy ;
    free(stats);

    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the strehl ratio in an image	
  @param	image_in	Input image.
  @param    spar        structure containing the parameters for strehl comp.
  @return   0 if ok, -1 if not	
 */
/*----------------------------------------------------------------------------*/
int image_compute_strehl(
        image_t		* in,
		strehl_parm * spar)
{
    int		    star_x, 
                star_y ;
	double	    psf_flux ;
	double	    psf_peak ;
	double3 *   position ;
	image_t	*   psf ;
	image_t *   extracted ;
    int         star_radius ;
	int         llx, 
                lly, 
                urx, 
                ury ;
    int         ring[4] ;
    double      noise ;
	int		    i ;

    /* Test inputs */
	if (in==NULL || spar==NULL) return -1 ;

    /* Computing a Strehl ratio is a story between an ideal PSF */
    /* and a candidate image supposed to approximate this ideal PSF. */

	/* Generate first appropriate PSF to find max peak */
    psf_flux = 0 ;
	if ((psf = image_gen_psf(spar->m1,
						spar->m2,
						spar->l0,
						spar->dl,
						spar->pscale,
						spar->size)) == NULL) {
		e_error("generating PSF: aborting strehl computation") ;
		return -1 ;
	}
    
	/* Save PSF if requested */
	if (spar->psf_save) {
		e_comment(0, "saving PSF file %s",
				spar->psf_filename ? spar->psf_filename : "psf1.fits");
		image_save_fits(psf,
            spar->psf_filename ? spar->psf_filename : "psf1.fits", BPP_DEFAULT);
	}

	/* Compute flux in PSF and find max peak */
	psf_peak = (double)psf->data[0] ;
	for (i=0 ; i<psf->lx * psf->ly ; i++) {
		psf_flux += (double)psf->data[i] ;
		if ((double)psf->data[i]>psf_peak) psf_peak = (double)psf->data[i] ;
	}
	image_del(psf);
	
	/* Store these results into strehl_parm structure */
	spar->psf_flux = psf_flux ;
	spar->psf_peak = psf_peak ;
			 
	/* Identify a candidate for Strehl computation in the input image */
	star_x = spar->pos_x ;
	star_y = spar->pos_y ;
	if (star_x<0 || star_y<0) {
        /* No candidate was provided, find the brightest */
	  position = detected_ks_brightest_stars(in, 1, 5.0) ;
      if (position==NULL) {
          e_error("no star detected in image");
          return -1 ;
      }
      if (position->n<1) {
          e_error("no star detected in image");
          return -1 ;
      }
	  star_x = (int)(position->x[0]) ;
	  star_y = (int)(position->y[0]) ;
	  double3_del(position) ;
    }

    /* Measure the background in the candidate image if requested */
	if (spar->estim_bg) {
        if ((spar->bg_radius1 < 0.0) || (spar->bg_radius2 < 0.0)) {
            /* Define the zone to extract */
            llx = star_x - (int)((spar->size)/2) ;
            lly = star_y - (int)((spar->size)/2) ;
            urx = llx + spar->size ;
            ury = lly + spar->size ;

            /* Extract a zone */
            extracted = image_getvig(in, llx, lly, urx, ury) ;
        
            spar->star_bg = image_estimate_background(extracted, 0.1, 50);

            if (debug_active()>1) {
                /* Write out the extracted image  */
                image_save_fits(extracted, "extract.fits", BPP_DEFAULT) ;
            }
            image_del(extracted) ;
        } else {
            spar->star_bg = image_get_disk_background(in,
                                star_x,
                                star_y,
                                (int)((spar->bg_radius1)/(spar->pscale)),
                                (int)((spar->bg_radius2)/(spar->pscale)),
                                BG_METHOD_AVER_REJ) ;
        }
    }

    /* Compute star_radius in pixels */
    star_radius = (int)(spar->star_radius / spar->pscale) ;
    
	/* Measure the flux on the candidate image */
	spar->star_flux = image_get_disk_flux(in,
										  star_x,
										  star_y,
										  star_radius,
										  spar->star_bg);
	
    /* Measure the peak value on the candidate image */
    spar->star_peak = image_getmax_vig(in,star_x-5,star_x+5,star_y-5,star_y+5);
	
    if (debug_active()>1) {
		printf("psf: flux=%g peak=%g\n", spar->psf_flux, spar->psf_peak);
		printf("sta: flux=%g peak=%g around %d %d\n", spar->star_flux, 
                spar->star_peak, star_x, star_y) ;
	}

	/* Compute Strehl */
	spar->strehl = (spar->star_peak / spar->star_flux) / 
        (spar->psf_peak / spar->psf_flux) ;
	
    /* Compute Strehl error */
    /* Compute bg_noise */
    ring[0] = star_x ;
    ring[1] = star_y ;
    ring[2] = (int)((spar->bg_radius1)/(spar->pscale)) ;
    ring[3] = (int)((spar->bg_radius2)/(spar->pscale)) ;
    if (image_ring_readout_noise(in, ring, &noise, NULL) == -1) {
        e_warning("cannot compute Strehl error");
        spar->bg_noise = -1.0 ;
        spar->strehl_err = -1.0 ;
    } else {
        spar->bg_noise = noise ;
       
        /* Deduce the error */
        spar->strehl_err = STREHL_ERROR_COEFFICIENT * spar->bg_noise * 
            spar->pscale * star_radius * star_radius / spar->star_flux ;
    }

	return 0;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the entropy of an image on 16 bits.
  @param	im		Input image
  @return	1 double representing the image entropy (on 16 bits)

  This image computes the image entropy on 16 bits. All pixels are matched to 
  the set of 16-bit numbers from -2^15 to 2^15, then a histogram is built to 
  compute probabilities for each pixel value. The entropy is the sum:

  \begin{verbatim}
  entropy = - sum(p_i*log2(p_i))
  \end{verbatim}

  A theoretical maximal entropy is reached for p_i=2^-15 for all i, which 
  yields a value of 16 (out of 16 bits, i.e. all bits are useful).
  A minimal entropy is found when all pixels have the same value.
 */
/*----------------------------------------------------------------------------*/
#define INVLOG2	1.44269504088896340737
double image_compute_entropy(image_t * im)
{
	histogram	*	hist ;
	double			entropy ;
	double			prob ;
	double			norm ;
	int				i ;

	hist = histogram_compute(im,
							(int)(1<<16),
							(pixelvalue)(-1<<15),
							(pixelvalue)( 1<<15));
	if (hist==NULL) return -1 ;

	norm = 1.0 / (double)(im->lx * im->ly);
	entropy=0 ;
	for (i=0 ; i<hist->nbin ; i++) {
		if (hist->array[i]) {
			prob = (double)hist->array[i] * norm ;
			entropy += prob * log(prob) * INVLOG2 ;
		}
	}
	histogram_del(hist);
	return -entropy ;
}
