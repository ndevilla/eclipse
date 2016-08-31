/*-------------------------------------------------------------------------*/
/**
   @file    resampling.h
   @author  Nicolas Devillard
   @date    Jan 04, 1996
   @version $Revision: 1.16 $
   @brief   resampling routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: resampling.h,v 1.16 2002/03/26 12:43:41 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/03/26 12:43:41 $
    $Revision: 1.16 $
*/

#ifndef _RESAMPLING_H_
#define _RESAMPLING_H_

/*---------------------------------------------------------------------------
  								Includes
 ---------------------------------------------------------------------------*/

#include "image_handling.h"
#include "matrix.h"
#include "poly2d.h"

/*---------------------------------------------------------------------------
  								Defines
 ---------------------------------------------------------------------------*/

#define TRANSFO_AFFINE          0
#define TRANSFO_DEG2            1
#define TRANSFO_HOMOGRAPHIC     2

/*
 * Kernel definition in terms of sampling
 */

/* Number of tabulations in kernel  */
#define TABSPERPIX      (1000)
#define KERNEL_WIDTH    (2.0)
#define KERNEL_SAMPLES  (1+(int)(TABSPERPIX * KERNEL_WIDTH))

#define TANH_STEEPNESS	(5.0)

/*---------------------------------------------------------------------------
 						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Generate an interpolation kernel to use in this module.
  @param    kernel_type     Type of interpolation kernel.
  @return   1 newly allocated array of doubles.

  Provide the name of the kernel you want to generate. Supported kernel
  types are:

  \begin{tabular}{ll}
  NULL          &   default kernel, currently "tanh" \\
  "default"     &   default kernel, currently "tanh" \\
  "tanh"        &   Hyperbolic tangent \\
  "sinc2"       &   Square sinc \\
  "lanczos"     &   Lanczos2 kernel \\
  "hamming"     &   Hamming kernel \\
  "hann"        &   Hann kernel
  \end{tabular}

  The returned array of doubles is ready of use in the various re-sampling
  functions in this module. It must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * generate_interpolation_kernel(char * kernel_type) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Cardinal sine.
  @param    x   double value.
  @return   1 double.

  Compute the value of the function sinc(x)=sin(pi*x)/(pi*x) at the
  requested x.
 */
/*--------------------------------------------------------------------------*/
double sinc(double x) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Warp an image according to a linear transformation.
  @param    image_in        Image to warp.
  @param    param           Linear transformation definition.
  @param    kernel_type     Interpolation kernel to use.
  @return   1 newly allocated image.

  Warp an image according to a linear transformation. The transform is
  given as a set of 6 doubles, such as:

  \begin{verbatim}
  u = t[0].x + t[1].y + t[2]
  v = t[3].x + t[4].y + t[5]
  \end{verbatim}

  Where (u,v) are the coordinates of a pixel in the warped image, and (x,y)
  are the coordinates of a pixel in the original image.
  The transformation must be invertible for this function to work. The
  warping algorithm is implemented as a reverse warping.

  See the function generate_interpolation_kernel() for possible kernel
  types. If you want to use a default kernel, provide NULL for kernel type.

  The returned image is a newly allocated object, it must be deallocated
  using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_warp_linear(
        image_t     *   image_in,
        double      *   param,
        char        *   kernel_type) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Warp an image according to a polynomial transformation.
  @param    image_in        Image to warp.
  @param    kernel_type     Interpolation kernel to use.
  @param    poly_u          Polynomial transform in U.
  @param    poly_v          Polynomial transform in V.
  @return   1 newly allocated image.

  Warp an image according to a polynomial transform. Provide two
  polynomials (see poly2d.h for polynomials in this library) Pu and Pv such
  as:

  \begin{verbatim}
  x = poly2d_compute(Pu, u, v)
  y = poly2d_compute(Pv, u, v)
  \end{verbatim}

  Attention! The polynomials define a reverse transform. (u,v) are
  coordinates in the warped image and (x,y) are coordinates in the original
  image. The transform you provide is used to compute from the warped
  image, which pixels contributed in the original image.

  The output image will have strictly the same size as in the input image.
  Beware that for extreme transformations, this might lead to blank images
  as result.

  See the function generate_interpolation_kernel() for possible kernel
  types. If you want to use a default kernel, provide NULL for kernel type.

  The returned image is a newly allocated objet, use image_del() to
  deallocate it.

 */
/*--------------------------------------------------------------------------*/
image_t * image_warp_generic(
        image_t     *   image_in,
        char        *   kernel_type,
        poly2d      *   poly_u,
        poly2d      *   poly_v) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Invert a linear transformation.
  @param    trans   Transformation to invert.
  @return   1 newly allocated array of 6 doubles.

  Given 6 parameters a, b, c, d, e, f defining a linear transform such as:
  \begin{verbatim}
  u = ax + by + c
  v = dx + ey + f
  \end{verbatim}

  The inverse transform is also linear, and is defined by:
  \begin{verbatim}
  x = Au + Bv + C
  y = Du + Ev + F
  \end{verbatim}

  where:

  \begin{verbatim}
  if G = (ae-bd)

  A =  e/G
  B = -b/G
  C = (bf-ce)/G
  D = -d/G
  E =  a/G
  F = (cd-af)/G
  \end{verbatim}

  Notice that if G=0 (ae=bd) the transform cannot be reversed.

  The returned array must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * invert_linear_transform(double * trans) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Warp an image according to a linear transformation.
  @param    image_in        Image to warp.
  @param    param           Linear transformation definition.
  @param    kernel_type     Interpolation kernel to use.
  @return   1 newly allocated image.

  Warp an image according to a linear transformation. The transform is
  given as a set of 6 doubles, such as:

  \begin{verbatim}
  u = t[0].x + t[1].y + t[2]
  v = t[3].x + t[4].y + t[5]
  \end{verbatim}

  Where (u,v) are the coordinates of a pixel in the warped image, and (x,y)
  are the coordinates of a pixel in the original image.
  The transformation must be invertible for this function to work. The
  warping algorithm is implemented as a reverse warping.

  See the function generate_interpolation_kernel() for possible kernel
  types. If you want to use a default kernel, provide NULL for kernel type.

  The returned image is a newly allocated object, it must be deallocated
  using image_del().

  This function is strictly the same as image_warp_linear. Only difference
  is that the code should be hopefully faster, but not completely tested...
 */
/*--------------------------------------------------------------------------*/
image_t * image_warp_linear_opt(
        image_t     *   image_in,
        double      *   param,
        char        *   kernel_type) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Generate a hyperbolic tangent kernel.
  @param    steep   Steepness of the hyperbolic tangent parts.
  @return   1 pointer to a newly allocated array of doubles.

  The following function builds up a good approximation of a box filter. It
  is built from a product of hyperbolic tangents. It has the following
  properties:

  \begin{itemize}
  \item It converges very quickly towards +/- 1.
  \item The converging transition is very sharp.
  \item It is infinitely differentiable everywhere (i.e. smooth).
  \item The transition sharpness is scalable.
  \end{itemize}

  The returned array must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * generate_tanh_kernel(double steep) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Print out an interpolation kernel values on stdout.
  @param    kernel_name     Name of the kernel to print out.
  @return   void

  Takes in input a kernel name, generates the corresponding kernel and
  prints it out on stdout, then discards the generated kernel.

  For debugging purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void show_interpolation_kernel(char * kernel_name) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subsample an image by a factor 2.
  @param    in      Image to subsample
  @return   1 newly allocated image

  Subsamples an image by a factor 2, i.e. 4 pixels become 1 pixel.
  A triangular filter (1 2 1) is applied to smooth high
  frequencies.
  The returned image must be freed using image_del(). 
 */
/*--------------------------------------------------------------------------*/
image_t * image_subsample(image_t * in) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Subsample an image by a factor 4.
  @param    in      Image to subsample
  @return   1 newly allocated image

  Subsamples an image by a factor 4, i.e. 16 pixels become 1 pixel.
  A triangular filter (1 2 1) is applied twice to smooth high
  frequencies.
  The returned image must be freed using image_del(). 
 */
/*--------------------------------------------------------------------------*/
image_t * image_subsample4(image_t * in);

#endif
