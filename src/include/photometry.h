/*-------------------------------------------------------------------------*/
/**
   @file    photometry.h
   @author  Nicolas Devillard
   @date    Jun 23, 1997
   @version $Revision: 1.10 $
   @brief   photometry measurement routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: photometry.h,v 1.10 2002/06/27 13:55:22 yjung Exp $
    $Author: yjung $
    $Date: 2002/06/27 13:55:22 $
    $Revision: 1.10 $
*/

#ifndef _PHOTOMETRY_H_
#define _PHOTOMETRY_H_

/*---------------------------------------------------------------------------
  								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cube_handling.h"
#include "image_handling.h"
#include "pixel_handling.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define BG_METHOD_AVERAGE		1
#define BG_METHOD_MEDIAN		2
#define BG_METHOD_AVER_REJ		3

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the flux in a disk in a given image.
  @param    in          Input image.
  @param    x_center    x center of the disk.
  @param    y_center    y center of the disk.
  @param    radius      Disk radius.
  @param    background  Optional background value.
  @return   1 double.

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
        image_t     *   in,
        double          x_center,
        double          y_center,
        double          radius,
        pixelvalue      background) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Computes the flux per pixel in a given ring in an image.
  @param    in          Input image.
  @param    x_center    x center of the ring.
  @param    y_center    y center of the ring.
  @param    rad_int     Internal radius of the ring.
  @param    rad_ext     External radius of the ring.
  @param    method      Computing method.
  @return   1 double

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
        image_t     *   in,
        double          x_center,
        double          y_center,
        double          rad_int,
        double          rad_ext,
        int             method) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Estimate the background in an image.
  @param    in              Input image.
  @param    stop_thr        Threshold to stop procedure.
  @param    max_it          Maximum number of iterations to perform.
  @return   1 double.

  This function is a fairly simple background estimator. It assumes the
  input image to be made mainly of background pixels, with some high-flux
  signal in it (in minority). It computes iteratively the mean value of the
  image, rejecting at each pass all pixels above 3 sigmas. It stops when
  the improvement to the mean value between two iterations is lower than
  the provided stop_thr  value.

  The iterations will either stop because the mean improvement is lower
  than the requested value, or because the maximal number of iterations has
  been reached.

  Returns 0.0 in case of error.
 */
/*--------------------------------------------------------------------------*/
double image_estimate_background(
        image_t *   in,
        double      stop_thr,
        int         max_it) ;

#endif
