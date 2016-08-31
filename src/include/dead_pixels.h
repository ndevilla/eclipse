/*-------------------------------------------------------------------------*/
/**
   @file    dead_pixels.h
   @author  Nicolas Devillard   
   @date    Sept 15, 1995
   @version $Revision: 1.14 $
   @brief   dead pixel localization/elimination
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: dead_pixels.h,v 1.14 2001/10/26 14:30:35 yjung Exp $
    $Author: yjung $
    $Date: 2001/10/26 14:30:35 $
    $Revision: 1.14 $
*/

#ifndef _DEAD_PIXELS_H_
#define _DEAD_PIXELS_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "xmemory.h"
#include "cube_defs.h"
#include "cube_handling.h"
#include "cube_arith.h"
#include "cube2image.h"
#include "image_filters.h"
#include "pixelmaps.h"

/*---------------------------------------------------------------------------
  						Function ANSI C prototypes
 --------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Clean an image of its dead pixels.
  @param    dirty       Image to clean.
  @param    deadpixmap  Dead pixel map.
  @return   Newly allocated, clean image.

  Replace dead pixels by an average of the correct neighbors in the
  3x3 neighborhood around each pixel. If no correct pixel can be found
  in the 8 neighbors, the pixel is set to zero.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_clean_deadpix(
        image_t     *   dirty,
        pixelmap    *   deadpixmap) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Clean out a cube of its bad pixels.
  @param    in          Cube to clean.
  @param    deadpixmap  Dead pixel map.
  @return   int 0 if Ok, -1 otherwise.

  Applies a dead pixel cleaning function, according to the passed
  pixelmap, to all planes in the input cube. The input cube is
  modified.
 */
/*--------------------------------------------------------------------------*/
int cube_clean_deadpix(
        cube_t      *   in,
        pixelmap    *   deadpixmap) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Detect bad pixels in a single image by median filtering.
  @param    dirty               Image to use for detection.
  @param    median_threshold    Threshold for detection.
  @return   1 newly allocated pixelmap.

  The list of bad pixels is detected by thresholding the difference
  between the original image and a median-filtered version of it. This
  method is extremely sensitive to the input signal and is likely to
  require interaction with a user to iterate until an acceptable pixel
  map is found. A robust dead pixel detection should not be based on
  this method.
 */
/*--------------------------------------------------------------------------*/
pixelmap * image_detect_deadpix_median(
        image_t     *   dirty,
        pixelvalue      median_threshold) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Detect bad pixels in a cube using a median method.
  @param    skyname             Name of the cube to use for detection.
  @param    median_threshold    Threshold for detection.
  @return   1 newly allocated pixelmap.

  A median detection is used on every plane of the input cube. The
  final pixel map is an AND of all pixel maps found (1 for each input
  plane). This means that for a pixel to be declared good, it has to
  be declared good in all produced pixelmaps.

  This method is as unreliable in automatic mode as the
  image_detect_deadpix_median() method. It is likely to require interaction
  with a user to reach an acceptable threshold. The AND condition is
  maybe too restrictive to get usable pixel maps in output.
 */
/*--------------------------------------------------------------------------*/
pixelmap * cube_detect_deadpix_median(
        char        *   skyname,
        pixelvalue      median_threshold) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find out bad pixels by observing pixel behaviour in time.
  @param    skyname         Input cube name.
  @param    sigma_width     Sigma threshold.
  @return   1 newly allocated pixelmap.

  An image of the standard deviations of the pixels along time is
  computed. This image expresses the variability of the pixels along
  time in the cube. This standard deviation image is then thresholded
  using the provided threshold to yield a map of the most agitated
  pixels. These pixels are declared bad.

  This method is not reliable, do not use it in automatic mode.
 */
/*--------------------------------------------------------------------------*/
pixelmap * cube_detect_deadpix_z(
        char    *   skyname,
        double      sigma_width) ;

#endif
