/*-------------------------------------------------------------------------*/
/**
   @file    cube_filters.h
   @author  Nicolas Devillard
   @date    Aug 29, 1995
   @version $Revision: 1.18 $
   @brief   various cube filters in image domain
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: cube_filters.h,v 1.18 2002/07/31 14:34:38 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/07/31 14:34:38 $
    $Revision: 1.18 $
*/

#ifndef _CUBE_FILTERS_H_
#define _CUBE_FILTERS_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "comm.h" 
#include "xmemory.h"
#include "cube_handling.h"
#include "image_filters.h"

/*---------------------------------------------------------------------------
  						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply an image filter to all planes of a cube.
  @param    cube_in     Cube to modify.
  @param    filter      Filter name.
  @param    filtval     (optional) filter values.
  @return   int 0 if Ok, -1 otherwise.

  Every plane in the input cube is modified by applying a filter on
  it. The possible filter names are:

  \begin{itemize}
  \item user-linear
  \item mean3
  \item dx
  \item dy
  \item d2x
  \item d2y
  \item contour1
  \item contour2
  \item contour3
  \item contrast1
  \item mean5
  \item min
  \item max
  \item median
  \item max-min
  \item user-morpho
  \item 3x1
  \item flat
  \end{itemize}

  Only 'user-linear', 'user-morpho' and '3x1' actually request filter
  values to be provided through the 'filtval' array of doubles.

  The 'flat' filter requires a single value, an integer representing the
  half-size of the kernel to use. This integer is actually passed as the
  first double in the filtval list. If the passed value is not truly an
  integer, it is rounded up to the closest integer.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter(
        cube_t  *   cube_in,
        char    *   filter,
        double  *   filtval) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a 3x3 filter to all planes of a cube.
  @param    cube1           Cube to modify.
  @param    filter_array    Array of 9 filter coefficients.
  @return   int 0 if Ok, -1 otherwise.

  Filter coefficients are spread over a 3x3 matrix. If the matrix is:

  \begin{verbatim}
  f0  f1  f2
  f3  f4  f5
  f6  f7  f8
  \end{verbatim}

  Then the filter is given as a pointer to 9 doubles as {f0,...,f8}.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter_3x3(
        cube_t  *   cube1,
        double  *   filter_array) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a 3x1 filter to all planes of a cube.
  @param    cube1           Cube to modify.
  @param    filter_array    Array of 3 filter coefficients.
  @return   int 0 if Ok, -1 otherwise.
 
  Filter coefficients are spread over a 3x1 matrix: {f1, f2, f3}.
 
  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter_3x1(
        cube_t  *   cube1,
        double  *   filter_array) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a 5x5 filter to all planes of a cube.
  @param    cube1           Cube to modify.
  @param    filter_array    Array of 25 filter coefficients.
  @return   int 0 if Ok, -1 otherwise.

  Filter coefficients are spread over a 5x5 matrix. If the matrix is:

  \begin{verbatim}
  f0  f1  f2  f3  f4
  f5  f6  f7  f8  f9
  f10 f11 f12 f13 f14
  f15 f16 f17 f18 f19
  f20 f21 f22 f23 f24
  \end{verbatim}

  Then the filter is given as a pointer to 25 doubles as {f0,...,f24}.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter_5x5(
        cube_t  *   cube1,
        double  *   filter_array) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a morpho filter to all planes of a cube.
  @param    cube1           Cube to modify
  @param    filter_array    Array of 9 filter coefficients.
  @return   int 0 if Ok, -1 otherwise.

  Filter coefficients are spread over a 3x3 matrix. If the matrix is:

  \begin{verbatim}
  f0  f1  f2
  f3  f4  f5
  f6  f7  f8
  \end{verbatim}

  Then the filter is given as a pointer to 9 doubles as {f0,...,f8}.

  The first filter element applies to the min value in the 3x3
  neighbourhood of the current pixel, the 9th element of the filter
  matrix applies to the maximum, and other elements are sorted
  according to their position.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter_morpho(
        cube_t  *   cube1,
        double  *   filter_array) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a median filter to all planes of a cube.
  @param    cube1           Cube to modify
  @return   int 0 if Ok, -1 otherwise.

  A 3x3 median filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter_median(cube_t * cube1) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a flat filter to all planes of a cube.
  @param    cube1           Cube to modify.
  @param    kern_hsize      Kernel half-size
  @return   int 0 if Ok, -1 otherwise.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_filter_flat(
        cube_t  *   cube1,
        int         kern_hsize) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    3d-filtering on a cube with minmax rejection.
  @param    in          Cube to filter.
  @param    halfw       Half-width for filter.
  @param    rejmin      Number of min pixels to reject.
  @param    rejmax      Number of max pixels to reject.
  @param    background  Double array to store computed background.
  @return   int 0 if Ok, -1 otherwise

  This function performs a 3d rejection filter on the input cube.
  Each time line is extracted on +/-halfw. On this line of sight, all
  pixels are first normalized by subtracting the median value of the
  plane they belong to. The highest and lowest values are then removed
  and the rest is averaged to yield a value which is subtracted from
  the initial input pixel.

  The background array is optional, provide NULL if you have no use
  for it. This array is expected to be already allocated, it should
  have space for at least as many doubles as there are input planes.
  The array will contain the average value which has been subtracted
  to pixels in each plane. It is usually a good indicator of the
  average subtracted background value if you use this filter to subtract
  an infrared sky background.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_3dfilt_runminmax(
        cube_t  **  in,
        int         halfw,
        int         rejmin,
        int         rejmax,
        double  *   background) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    3d-filtering on a cube with minmax and central rejection.
  @param    in          Cube to filter.
  @param    halfw       Half-width for filter.
  @param    rejmin      Number of min pixels to reject.
  @param    rejmax      Number of max pixels to reject.
  @param    background  Double array to store computed background.
  @return   int 0 if Ok, -1 otherwise

  This function performs a 3d rejection filter on the input cube.
  Each time line is extracted on +/-halfw. On this line of sight, all
  pixels are first normalized by subtracting the median value of the
  plane they belong to. The central, highest and lowest values are then
  removed and the rest is averaged to yield a value which is subtracted
  from the initial input pixel.

  The background array is optional, provide NULL if you have no use
  for it. This array is expected to be already allocated, it should
  have space for at least as many doubles as there are input planes.
  The array will contain the average value which has been subtracted
  to pixels in each plane. It is usually a good indicator of the
  average subtracted background value if you use this filter to subtract
  an infrared sky background.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_3dfilt_runminmax_central(
        cube_t  **  in,
        int         halfw,
        int         rejmin,
        int         rejmax,
        double  *   background);
/* </python> */

/*-------------------------------------------------------------------------*/
/**
  @brief    3d-filtering on a cube with minmax rejection.
  @param    in          Cube to filter.
  @param    halfw       Half-width for filter.
  @param    rejmin      Number of min pixels to reject.
  @param    rejmax      Number of max pixels to reject.
  @param    background  Double array to store computed background.
  @return   int 0 if Ok, -1 otherwise

  This function performs a 3d rejection filter on the input cube.
  Each time line is extracted on +/-halfw. On this line of sight, all
  pixels are first normalized by subtracting the median value of the
  plane they belong to. The highest and lowest values are then removed
  and the rest is averaged to yield a value which is subtracted from
  the initial input pixel.

  The background array is optional, provide NULL if you have no use
  for it. This array is expected to be already allocated, it should
  have space for at least as many doubles as there are input planes.
  The array will contain the average value which has been subtracted
  to pixels in each plane. It is usually a good indicator of the
  average subtracted background value if you use this filter to subtract
  an infrared sky background.
 */
/*--------------------------------------------------------------------------*/
int cube_3dfilt_runminmax_by_quad(
        cube_t  **  in,
        int         halfw,
        int         rejmin,
        int         rejmax,
        double  *   background) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Sky estimation and correction with median method
  @param    cube        Input cube with objects and sky frames
  @param    sky_flags   Flags to identify the sky frames
  @return   the estimated sky
    
  This function take the median of all the sky frames and subtract the
  resulting frame from all object frames.
  If all input frames are object frames, the median is computed on the object
  frames.
  At the end, the object frames are normalized (divided by their median)
  The estimated sky frame is returned
 */
/*--------------------------------------------------------------------------*/
image_t * cube_subtract_median_sky(
        cube_t  *   cube,
        int     *   sky_flags) ;

#endif
