/*----------------------------------------------------------------------------*/
/**
   @file    detector.h
   @author  Yves Jung
   @date    June 2001
   @version $Revision: 1.10 $
   @brief   All detector check functions
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: detector.h,v 1.10 2003/08/18 14:21:28 yjung Exp $
    $Author: yjung $
    $Date: 2003/08/18 14:21:28 $
    $Revision: 1.10 $
*/

#ifndef _DETECTOR_H_
#define _DETECTOR_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "comm.h" 
#include "xmemory.h"
#include "cube_handling.h"
#include "fit_curve.h"
#include "image_arith.h"
#include "image_stats.h"

/*-----------------------------------------------------------------------------
   						Function ANSI C prototypes	
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute a flat-field out of a set of exposures.
  @param    twilight    Input cube.
  @return   1 newly allocated array of 3 newly allocated images.
  @see      cube_create_gainmap() cube_create_gainmap_proportional()

  The input is assumed to be a cube containing planes of different intensities 
  (usually increasing or decreasing). Typical inputs are: twilight data sets, 
  halogen lamp, or skies of different airmasses in the thermal regime.

  The output is a set of 3 images. The first image contains a regression map, 
  i.e. for each pixel position on the detector, a curve is plotted of the 
  pixel intensity in each plane against the median intensity of the plane. A 
  slope is fit, and the gain factor is stored into this first image.

  The second image contains the y-intercepts of the slope fit. It is usually 
  good to check it out in case of failures.

  The third image contains the sum of squared errors for each fit.

  The fit is using a robust least-squares criterion rejecting outliers. This 
  is the algorithm to use with big telescopes like the VLT, which collect so 
  much light that objects are actually seen in the twilight sky.  It is also 
  recommended to jitter the twilight acquisition in this case (this is what is 
  done on ISAAC).

  The returned result is an array of 3 image pointers, that must be deallocated
  using free(). Each of the returned image pointers must have been previously 
  deallocated using image_del().

  Example:
  \begin{verbatim}
  image_t ** slopefit ;

  slopefit = cube_create_gainmap_robust(cube);
  ...
  if (slopefit[0]!=NULL) image_del(slopefit[0]);
  if (slopefit[1]!=NULL) image_del(slopefit[1]);
  if (slopefit[2]!=NULL) image_del(slopefit[2]);
  free(slopefit);
  \end{verbatim}
 */
/*----------------------------------------------------------------------------*/
image_t ** cube_create_gainmap_robust(cube_t * twilight) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute a flat-field out of a set of exposures.
  @param    twilight    Input cube.
  @return   1 newly allocated array containing 2 newly allocated images.
  @see      cube_create_gainmap
  @see      cube_create_gainmap_robust

  The input is assumed to be a cube containing planes of different intensities 
  (usually increasing or decreasing), from which any source of bias has been 
  removed. Typical inputs are: twilight data sets, halogen lamp, or skies of 
  different airmasses in the thermal regime. The input frame should have been 
  dark-subtracted or de-biased before entering this function.

  The output is an array of 2 images. The first image contains a regression map,
  i.e. for each pixel position on the detector, a curve is plotted of the pixel
  intensity in each plane against the median intensity of the plane. A slope is
  fit assuming a zero y-intercept, and the gain factor is stored into this 
  first image.

  The second image contains the sum of squared errors for each fit.

  The fit is using a robust slope fit criterion rejecting outliers. The slope 
  of each pixel is computed in all the input planes, and only the median slope
  is stored in output.

  The returned result is an array of 2 image pointers, that must be deallocated
  using free(). Each of the returned image pointers must have been previously 
  deallocated using image_del().

  Example:
  \begin{verbatim}
  image_t ** slopefit ;

  slopefit = cube_create_gainmap_proportional(cube);
  ...
  if (slopefit[0]!=NULL) image_del(slopefit[0]);
  if (slopefit[1]!=NULL) image_del(slopefit[1]);
  free(slopefit);
  \end{verbatim}

 */
/*----------------------------------------------------------------------------*/
image_t ** cube_create_gainmap_proportional(cube_t * twilight) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the readout noise in a rectangle.
  @param    diff    Input image, usually a difference frame.
  @param    zone    Zone where the readout noise is to be computed.
  @param    ron_hsize   to specify half size of squares (>0 to use default)
  @param    ron_nsamp   to specify the nb of samples (>0 to use default)
  @param    noise   Output parameter: noise in the frame.
  @param    error   Output parameter: error on the noise.
  @return   int 0 if Ok, -1 otherwise.

  This function is meant to compute the readout noise in a frame by means of a 
  MonteCarlo approach. The input is a frame, usually a difference between two 
  frames taken with the same settings for the acquisition system, although no 
  check is done on that, it is up to the caller to feed in the right kind of 
  frame.

  The provided zone is an array of four integers specifying the zone to take 
  into account for the computation. The integers specify ranges as xmin, xmax, 
  ymin, ymax, where these coordinates are given in the FITS notation (x from 1 
  to lx, y from 1 to ly and bottom to top). Specify NULL instead of an array of
  four values to use the whole frame in the computation.

  The algorithm will create typically 100 9x9 windows on the frame, scattered 
  optimally using a Poisson law. In each window, the standard deviation of all 
  pixels in the window is computed and this value is stored. The readout noise 
  is the median of all computed standard deviations, and the error is the 
  standard deviation of the standard deviations.

  Both values (noise and error) are returned by modifying a passed double. If 
  you do not care about the error, pass NULL.
 */
/*----------------------------------------------------------------------------*/
int image_rect_readout_noise(
        image_t *   diff,
        int     *   zone_def,
        int         ron_hsize,
        int         ron_nsamp,
        double  *   noise,
        double  *   error) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the readout noise in a ring.
  @param    diff    Input image, usually a difference frame.
  @param    zone    Center coordinates and ring radiuses (4 int)
  @param    noise   Output parameter: noise in the frame.
  @param    error   Output parameter: error on the noise.
  @return   int 0 if Ok, -1 otherwise.

  Same as image_rect_readout_noise, but Poisson dist. in a ring.

  This function is meant to compute the readout noise in a frame by means of a 
  MonteCarlo approach. The input is a frame, usually a difference between two 
  frames taken with the same settings for the acquisition system, although no 
  check is done on that, it is up to the caller to feed in the right kind of 
  frame.

  The provided zone is an array of four integers specifying the zone to take 
  into account for the computation. The integers specify ranges as x, y, r1, r2
  where these coordinates are given in the FITS notation (x from 1 to lx, y 
  from 1 to ly). The zone is a ring in this case.

  The algorithm will create typically 50 9x9 windows on the frame, scattered 
  optimally using a Poisson law in the ring. In each window, the standard 
  deviation of all pixels in the window is computed and this value is stored. 
  The readout noise is the median of all computed standard deviations, and the 
  error is the standard deviation of the standard deviations.

  Both values (noise and error) are returned by modifying a passed double. If 
  you do not care about the error, pass NULL.
 */
/*----------------------------------------------------------------------------*/
int image_ring_readout_noise(
        image_t *   diff,
        int     *   zone_def,
        double  *   noise,
        double  *   error) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the linearity of the detector
  @param    in  input cube
  @param    dit list of DIT values
  @param    deg degree of the fit
  @return   cube with 4 images (3 coeffs & rms)
 */
/*----------------------------------------------------------------------------*/
cube_t * detector_linearity_fit(
        cube_t  *   in,
        double  *   dit,
        int         deg) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Correct detector non linearity
  @param    in  input cube to correct (modified)
  @param    coeff_a     a coefficients
  @param    coeff_b     b coefficients
  @param    coeff_c     c coefficients
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int cube_correct_detlin(
        cube_t  *   in,
        image_t *   coeff_a,
        image_t *   coeff_b,
        image_t *   coeff_c) ;

#endif
