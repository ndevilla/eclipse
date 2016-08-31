/*----------------------------------------------------------------------------*/
/**
   @file    image_filters.h
   @author  Nicolas Devillard
   @date    Aug 29, 1995
   @version $Revision: 1.20 $
   @brief   various image filters in spatial domain
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: image_filters.h,v 1.20 2003/10/24 08:40:14 yjung Exp $
    $Author: yjung $
    $Date: 2003/10/24 08:40:14 $
    $Revision: 1.20 $
*/

#ifndef _IMAGE_FILTERS_H_
#define _IMAGE_FILTERS_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image_handling.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define OEFILT_VERSION      "1.0 (08-Feb-2002)"

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Get a pre-defined filter kernel definition.
  @param    name    Name of the filter.
  @param    nval    Output number of values in the array.
  @param    morpho  Flag to set to 1 if required filter is morpho.
  @return   1 pointer to a list of doubles.

  This function takes as input the name of a filter, and returns
  the filter definition as an array of doubles. The array is
  statically allocated inside this module and should not be modified
  or freed by the caller of this function.

  The number of values in the filter is returned in 'nval'. If 'nval'
  is passed as a NULL pointer, it is not updated but the function
  still returns a valid pointer to an array (if one can be found).

  The 'morpho' flag is updated to contain 1 if the filter is
  morphological, 0 if not. If 'morpho' is set to NULL it is not
  updated.

  Valid filters are:

  - "mean3"     3x3 mean (flat)
  - "mean5"     5x5 mean (flat)
  - "dx"        3x3 derivative in x
  - "dy"        3x3 derivative in y
  - "d2x"       3x3 second derivative in x
  - "d2y"       3x3 second derivative in y
  - "contour1"  3x3 contour detector
  - "contour2"  3x3 contour detector
  - "contour3"  3x3 contour detector
  - "contrast1" 3x3 contrast enhancement
  - "min"       3x3 morphological min
  - "max"       3x3 morphological max
  - "median"    3x3 morphological median
  - "max-min"   3x3 morphological max - min

 */
/*----------------------------------------------------------------------------*/
double * image_filter_getkernel(char * name, int * nval, int * morpho);

/*----------------------------------------------------------------------------*/
/**
  @brief    Filter an image in spatial domain with a 3x3 kernel.
  @param    image_in    Image to filter.
  @param    filter      Filter definition.
  @return   1 newly allocated image.

  The input filter is defined by a 3x3 double matrix, given as an array
  of 9 doubles. If the matrix is:

  @verbatim
  f7 f8 f9
  f4 f5 f6
  f1 f2 f3
  @endverbatim

  Then the filter is given as an array: {f1, f2, ... f9}.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter3x3(image_t * image_in, double * filter) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Filter an image in spatial domain with a 3x1 kernel.
  @param    image_in    Image to filter.
  @param    filter      Filter definition.
  @return   1 newly allocated image.

  The input filter is defined by a 3x1 double matrix, given as an array
  of 3 doubles: {f1, f2, f3}.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter3x1(image_t * image_in, double * filter) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Filter an image in spatial domain with a 5x5 kernel.
  @param    image_in    Image to filter.
  @param    filter      Filter definition.
  @return   1 newly allocated image.

  The input filter is defined by a 5x5 double matrix, given as an array
  of 25 doubles. If the matrix is:

  @verbatim
  f21  f22  f23  f24  f25
  f16  f17  f18  f19  f20
  f11  f12  f13  f14  f15
  f6   f7   f8   f9   f10
  f1   f2   f3   f4   f5
  @endverbatim

  Then the filter is given as an array: {f1, f2, ... f25}.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter5x5(image_t * image_in, double * filter) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Filter an image in spatial domain with a 3x3 morpho kernel.
  @param    image_in    Image to filter.
  @param    filter      Filter definition.
  @return   1 newly allocated image.

  The input filter is defined by a 3x3 double matrix, given as an array
  of 9 doubles. The first array element will be applied to the min pixel
  in the 3x3 neighborhood, the second to the second-to-min, etc. and
  the last coefficient is applied to the max pixel.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_morpho(image_t * image_in, double * filter) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a spatial 3x3 median filter to an image.
  @param    in  Image to filter.
  @return   1 newly allocated image.

  Apply a spatial 3x3 median filter to an image, return a newly allocated
  image which must be freed using image_del(). This is an optimized
  version.
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_median(image_t * in) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a vertical median filter to an image.
  @param    in          Image to filter.
  @param    filtsize    Size of the median kernel.
  @return   1 newly allocated image.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_vertical_median(image_t * in, int filtsize) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a spatial median filter to an image, with rect kernel.
  @param    in          Image to filter
  @param    filtsizex   Size of the filter box in x.
  @param    filtsizey   Size of the filter box in y.
  @return   1 newly allocated image.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_large_median(image_t *in, int filtsizex, int filtsizey);


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply an horizontal median filter to an image.
  @param    in          Image to filter.
  @param    filtsize    Size of the filter to apply.
  @return   1 newly allocated image.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_horizontal_median(image_t * in, int filtsize) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Filter an image with a flat kernel of given size.
  @param    im      Image to filter.
  @param    ksize   Half-size of the filtering kernel.
  @return   1 newly allocated image.

  Apply a flat filter of given size. A flat filter is defined by a
  convolution matrix filled with 1's everywhere. The matrix is always
  odd-sized and square. The given parameter defines the half-size of the
  filter to apply.

  The filter is applied in one pass to avoid memory overflows. Applying it
  in two passes would be faster but requires twice the amount of memory.
  This is an issue, especially with WFI frames.

  Example: applying a 9x9 flat filter would be done by setting ksize to 4.
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_flat(image_t * im, int ksize) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Filter an image with a flat rectangular kernel of given size.
  @param    in      Image to filter.
  @param    hx      Horizontal half-size of the kernel
  @param    hy      Vertical half-size of the kernel
  @return   1 newly allocated image.
 */
/*--------------------------------------------------------------------------*/
image_t * image_rectangle_filter_flat(image_t * in, int hx, int hy) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Standard deviation filter
  @param    in      input image
  @param    hx      horizontal zone half size
  @param    hy      vertical zone half size
  @return   filtered image
  
  For each pixel, compute the standard deviation of a local zone.
  The image borders are set to 0.
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_stdev(image_t * in, int hx, int hy) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    50 Herz correction filter
  @param    in      input image
  @return   0 if ok, -1 otherwise   
  The input image is modified
 */
/*--------------------------------------------------------------------------*/
int image_remove_fiftyhertz(image_t * in) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Remove the odd-even effect for each quadrant separately.
  @param    im  Image to process
  @return   1 newly allocated image.

  This function tries to remove the odd-even effect from an image, applying
  image_de_oddeven() to each quadrant in sequence. The input image size must
  be a power of 2 and the image must be square.
 */
/*----------------------------------------------------------------------------*/
image_t * image_de_oddeven_byquad(image_t * im) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Remove the odd-even effect inside an image.
  @param    im  Image to process
  @return   1 newly allocated image.

  This function tries to remove the odd-even effect. The input image size
  must be a power of 2 and the image must be square.
 */
/*----------------------------------------------------------------------------*/
image_t * image_de_oddeven(image_t * im) ;

#endif
