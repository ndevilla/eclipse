/*----------------------------------------------------------------------------*/
/**
   @file    fourier.h
   @author  Nicolas Devillard
   @date    Oct 1995
   @version $Revision: 1.12 $
   @brief   fourier transform routines
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: fourier.h,v 1.12 2003/02/20 14:05:38 yjung Exp $
    $Author: yjung $
    $Date: 2003/02/20 14:05:38 $
    $Revision: 1.12 $
*/

#ifndef _FOURIER_H_
#define _FOURIER_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include "xmemory.h"
#include "cube_defs.h"
#include "cube_handling.h"
#include "fft_base.h"

/*-----------------------------------------------------------------------------
   						Function ANSI C prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute a Fast Fourier Transform on an image.
  @param    real_img        Image of real values.
  @param    imaginary_img   Image of imaginary values.
  @param    sign            Transform direction.
  @return   1 newly allocated cube containing 2 planes.

  This function computes the FFT of an input complex image. Complex pixels are 
  not supported in eclipse. Instead, a complex type is simulated by providing 
  two images: the first contains the real part of the complex pixels, the 
  second contains the imaginary part. It is Ok to provide a NULL pointer 
  instead of an imaginary image.

  To perform a forward transform (resp. inverse), set sign to FFT_FORWARD
  (resp. FFT_INVERSE).

  The returned cube contains two images: the first one is the real part of
  the returned image, the second one is the imaginary part. Scaling has
  already been applied, so this function should be reversible.

  The returned cube must be deallocated using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * image_fft(
        image_t     *   real_img,
        image_t     *   imaginary_img,
        int             sign) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Convert a 2-plane cube from (real,imag) to (ampl,phase).
  @param    cube_in     Input cube (containing 2 planes)
  @return   1 newly allocated cube containing 2 planes.

  The input cube is expected to contain two planes: first one is the real part 
  of a complex image, second one is the imaginary part of the same image. The 
  returned cube contains two planes: first one is the complex amplitude of the 
  image, second one is the phase.
  The returned cube must be deallocated using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_conv_xy_rtheta(cube_t * cube_in) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Convert a 2-plane cube from (ampl,phase) to (real,imag).
  @param    cube_in     Input cube (containing 2 planes)
  @return   1 newly allocated cube containing 2 planes.

  The input cube is expected to contain two planes: first one is the
  amplitude of a complex image, second one is the phase. The returned cube
  contains two planes: first one is the real part of the image, second one
  is the imaginary part.

  The returned cube must be deallocated using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_conv_rtheta_xy(cube_t * cube_in) ;

#endif
