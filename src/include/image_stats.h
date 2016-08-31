/*----------------------------------------------------------------------------*/
/**
   @file    image_stats.h
   @author  Nicolas Devillard
   @date    Aug 22, 1995
   @version $Revision: 1.42 $
   @brief   statistics computation for images
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: image_stats.h,v 1.42 2002/10/18 14:52:59 yjung Exp $
    $Author: yjung $
    $Date: 2002/10/18 14:52:59 $
    $Revision: 1.42 $
*/

#ifndef _IMAGE_STATS_H_
#define _IMAGE_STATS_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "xmemory.h"
#include "image_handling.h"
#include "image_arith.h"
#include "dead_pixels.h"
#include "pixel_handling.h"
#include "median.h"

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @brief	Input/output parameters for a Strehl computation.

  This structure holds all necessary parameters to compute a Strehl ratio on an
  image. Since these parameters are likely to evolve, it is better to keep them
  inside such a structure to avoid modifying calling code.

  Beware of units! Mirror dimensions are given in meters, wavelengths in 
  microns. The returned strehl and strehl error are in [0..1].
 */
/*----------------------------------------------------------------------------*/
typedef struct _strehl_parm_ {
	/* Parameters for PSF generation */
	double		m1 ;		    /* M1 size in meters */
	double		m2 ;		    /* M2 size in meters */
	double		l0 ;		    /* Central wavelength in microns */
	double		dl ;		    /* Wavelength range in microns */
	double		pscale ;	    /* Pixel scale in arcsec per pixel */
	int			size ;		    /* Size of the image to generate */

	/* Properties of the PSF */
	double		psf_flux ;	    /* Total flux in PSF */
	double		psf_peak ; 	    /* Peak value in PSF */
	int			psf_save ;	    /* Flag to indicate PSF archiving */
	char	*	psf_filename ;  /* File name for output PSF */


	/* Parameters for flux computation */
	int			pos_x ;		    /* X Position of star in image */
	int			pos_y ;		    /* Y Position of star in image */

    double      star_radius;    /* Star radius in arcsec */
	double		star_flux ;	    /* Total flux on star */
	double		star_peak ;	    /* Peak value of candidate star */

	int			estim_bg;       /* Request background estimate? */
	double		star_bg ;	    /* Estimated background */
    double      bg_noise ;      /* RMS of bg to compute strehl errror */
    double      bg_radius1;     /* First radius for bg ring */
    double      bg_radius2;     /* Second radius for bg ring */

	/* Returned measurements */
	double		strehl ;	    /* Measured Strehl ratio */
	double		strehl_err ;    /* Estimated error bar on Strehl */
} strehl_parm ;


/*-----------------------------------------------------------------------------
   						Function ANSI C prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute various statistics of an image.
  @param    image_in    Input image.
  @return   1 newly allocated image_stats structure.

  Compute various images statistics. Results are all stored into a returned
  structure, than must be deallocated using free(). See the structure details 
  in local_types.h.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_stats * image_getstats(image_t *image_in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute various statistics of an image.
  @param    image_in    Input image.
  @param    map         (optional) map of valid pixels
  @param    val_range   (optional) pixel bounds.
  @param    zone        (optional) zone definition for stat computation
  @return   1 newly allocated image_stats structure.

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
/* <python> */
image_stats * image_getstats_opts(
        image_t     *   in,
        pixelmap    *   map,
        pixelvalue  *   val_range,
        int         *   zone) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes mean pixel value over an image.
  @param    image_in    Input image.
  @return   1 double.
  Computes the mean pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_getmean(image_t * image_in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the mean pixel value inside a vignette.
  @param    in      Input image.
  @param    xmin    x min (included in vig)
  @param    xmax    x max (included in vig)
  @param    ymin    y min (included in vig)
  @param    ymax    y max (included in vig)
  @return   1 double.

  Compute the mean pixel value inside a vignette in the image. The vignette
  is defined by its lower-left and upper-right corners, given in the FITS
  convention (x from 1 to lx and from left to right, y from 1 to ly and
  from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_getmean_vig(
        image_t    *    in,
        int             xmin,
        int             xmax,
        int             ymin,
        int             ymax) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes minimum pixel value over an image.
  @param    image_in    Input image.
  @return   1 pixelvalue.
  Finds the minimum pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue image_getmin(image_t * image_in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes maximum pixel value over an image.
  @param    image_in    Input image.
  @return   1 pixelvalue.
  Finds the maximum pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue image_getmax(image_t * in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the max pixel value inside a vignette.
  @param    in      Input image.
  @param    xmin    x min (included in vig)
  @param    xmax    x max (included in vig)
  @param    ymin    y min (included in vig)
  @param    ymax    y max (included in vig)
  @return   1 pixelvalue.

  Compute the max pixel value inside a vignette in the image. The vignette
  is defined by its lower-left and upper-right corners, given in the FITS
  convention (x from 1 to lx and from left to right, y from 1 to ly and
  from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
pixelvalue image_getmax_vig(
        image_t    *    in,
        int             xmin,
        int             xmax,
        int             ymin,
        int             ymax) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes maximum pixel value and position over an image.
  @param    image_in    Input image.
  @param    px          ptr to the x coordinate of the max position
  @param    py          ptr to the y coordinate of the max position
  @return   1 pixelvalue.
  Finds the maximum pixel value and its position in the image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue image_getmaxpos(
        image_t     *   in,
        int         *   px,
        int         *   py) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes median pixel value over an image.
  @param    image_in    Input image.
  @return   1 pixelvalue.
  Finds the median pixel value in the image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue image_getmedian(image_t * in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the median pixel value inside a vignette.
  @param    image_in    Input image.
  @param    llx         x pos of lower left corner.
  @param    lly         y pos of lower left corner.
  @param    urx         x pos of upper right corner.
  @param    ury         y pos of upper right corner.
  @return   1 pixelvalue.

  Compute the median pixel value inside a vignette in the image. The
  vignette is defined by its lower-left and upper-right corners, given in
  the FITS convention (x from 1 to lx and from left to right, y from 1 to
  ly and from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue image_getmedian_vig(
        image_t *   in,
        int         llx,
        int         lly,
        int         urx,
        int         ury) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
    @brief  Computes a moving median on a column within an image
            using a verical window of size window_size.
    @param  in  gray image
    @param  x   the coolumn at which the moving miedian is to be computed
    @param  window_size the size of the moving window,
    @return 1 newly allocated array of ly pixelvalues.

    Median_pixelvalue is destructive, so pixels MUST be copied into
    windowline.  The window_size/2 first and last elements are constant.

    The returned array must be deallocated using free().
*/
/*----------------------------------------------------------------------------*/
pixelvalue * image_getmedian_mov_vert(
        image_t     *   in,
        int             x,
        int             window_size) ;


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
        image_t     *   in,
        int             y,
        int             window_size) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes kth smallest pixel value over an image.
  @param    image_in    Input image.
  @param    k           Rank of the value to find.
  @return   1 pixelvalue.

  Finds the kth smallest pixel value in the image. k=1 is the minimum,
  k=npix is the maximum, k=npix/2 is the median.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue image_getpercentile(
        image_t     *   in,
        int             k) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the sum of pixel values over an image.
  @param    image_in    Input image.
  @return   1 double.
  Computes the sum of all pixel values in an image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_getsumpix(image_t * image_in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the sum of pixel values inside a vignette.
  @param    image_in    Input image.
  @param    llx         x pos of lower left corner.
  @param    lly         y pos of lower left corner.
  @param    urx         x pos of upper right corner.
  @param    ury         y pos of upper right corner.
  @return   1 double.

  Compute the sum of all pixel values inside a vignette in the image. The
  vignette is defined by its lower-left and upper-right corners, given in
  the FITS convention (x from 1 to lx and from left to right, y from 1 to
  ly and from bottom to top). Corners are included in the vignette.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_getsumpix_vig(
        image_t     *   inimage,
        int             llx,
        int             lly,
        int             urx,
        int             ury) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the pixel standard deviation in an image.
  @param    image_in    Input image.
  @return   1 double.
  Finds the stdev. of the pixel value distribution in the input image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_getstdev(image_t * image_in) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes the pixel standard deviation in a vignette.
  @param    in          Input image.
  @param    xmin        X min (included)
  @param    ymin        X max (included)
  @param    xmax        Y min (included)
  @param    ymax        Y max (included)
  @return   1 double.

  Finds the standard deviation of the pixel value distribution
  in a vignette in the input image.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_getstdev_vig(
        image_t *   in,
        int         xmin,
        int         xmax,
        int         ymin,
        int         ymax) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the energy within a disk.
  @param    image_in    Input image.
  @param    cx          x pos of the disk center.
  @param    cy          y pos of the disk center.
  @param    radius      Disk radius.
  @return   1 double.

  Compute the pixel energy within a circle (the sum of squared pixel values). 
  Provide (-1,-1) as center coordinates if you want to use the center of the 
  image.

  The coordinates of the center are given in the C convention: x from 0 to lx-1
  and y from 0 to ly-1.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_get_radenergy(
        image_t     *   image_in,
        int             cx,
        int             cy,
        int             radius) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Find background noise level in a 1d array around a peak.
  @param    array       1d array of pixel values.
  @param    array_size  Number of pixelvalues in the array.
  @param    max_pos     Position of the peak in the array.
  @return   1 pixelvalue.

  The input signal is assumed to be mostly flat with a peak somewhere. You must
  provide the precise position of the peak as an integer rank in the array. The
  pixelvalues around the peak are used to determine the background level.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelvalue find_noise_level_around_peak(
        pixelvalue  *   array,
        int             array_size,
        int             max_pos) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Interpolate linearly the x pos between two points.
  @param    x1  Known first position.
  @param    y1  Known first position.
  @param    x1  Known second position.
  @param    y1  Known second position.
  @param    y   Known y of the searched position.
  @param    x   Output interpolated x position.
  @return   int 0 if Ok, -1 else.
  Interpolate linearly between two pts to find the x pos for a given y value.
 */
/*----------------------------------------------------------------------------*/
int imstat_x_for_y_between_2_points(
        int             x1,
        pixelvalue      y1,
        int             x2,
        pixelvalue      y2,
        pixelvalue      y,
        double      *   x) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute full width at Y with linear method.
  @param    array       Array of pixelvalues.
  @param    array_size  Size of the array.
  @param    max_pos     Position of the peak in the array.
  @param    Y           Height for width computation.
  @return   1 double

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
        double          Y) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the FWHM in an image at a given position.
  @param    image_in        Input image.
  @param    thres_flag      Threshold flag.
  @param    thres_value     Threshold value.
  @param    x_expect        x pos of the expected star.
  @param    y_expect        y pos of the expected star.
  @param    half_size_x     X Half size of the search domain.
  @param    half_size_y     Y Half size of the search domain.
  @return   Pointer to a newly allocated array of 2 doubles.

  This function expects an image and the position of a star-like object in this
  image. It will search around the provided position for a maximum, and will 
  compute an FWHM in x and y on this peak.
  The returned pointer contains 2 doubles. It must be deallocated using free().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double * image_getfwhm(
        image_t    *    image_in,
        int             thres_flag,
        pixelvalue      thres_value,
        int             x_expect,
        int             y_expect,
        int             half_size_x,
        int             half_size_y) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes first and second order image statistics using median.
  @param    in      Input image.
  @param    sigma   Output computed sigma value.
  @return   1 double.

  This function takes an image in input. It tries to estimate the average and 
  standard deviation of the image by approximating them by resp. the median and
  the average absolute distance to the median.

  The median is the returned double. The average absolute distance to the median
  is written into sigma.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
double image_median_stat(
        image_t     *   in,
        double      *   sigma) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Refine the location of a local maximum in a window.
  @param    img         Reference image containing the window
  @param    px          X position to refine
  @param    py          Y position to refine
  @param    search_hx   Half-size of search zone in x
  @param    search_hy   Half-size of search zone in y
  @param    refpos      output (refined) coordinates
  @return   int 0 if Ok, -1 if error occurred.

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
        image_t     *   img,
        int             px,
        int             py,
        int             search_hx,
        int             search_hy,
        int         *   refpos) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the strehl ratio in an image    
  @param    image_in    Input image.
  @param    spar        structure containing the parameters for strehl comp.
  @return   0 if ok, -1 if not  
 */
/*----------------------------------------------------------------------------*/
int image_compute_strehl(
        image_t     * in,
        strehl_parm * spar) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the entropy of an image on 16 bits.
  @param    im      Input image
  @return   1 double representing the image entropy (on 16 bits)

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
double image_compute_entropy(image_t * im) ;

#endif
