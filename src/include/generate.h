/*----------------------------------------------------------------------------*/
/**
   @file    generate.h
   @author  Nicolas Devillard
   @date    Sept 28, 1995
   @version $Revision: 1.23 $
   @brief   pattern image generation
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: generate.h,v 1.23 2002/10/18 14:52:59 yjung Exp $
    $Author: yjung $
    $Date: 2002/10/18 14:52:59 $
    $Revision: 1.23 $
*/

#ifndef _GENERATE_H_
#define _GENERATE_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "xmemory.h"
#include "cube_defs.h"
#include "image_handling.h"
#include "fit_curve.h"
#include "fourier.h"
#include "image_arith.h"
#include "pixelmaps.h"
#include "random.h"

/*-----------------------------------------------------------------------------
   						Function ANSI C prototypes
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image containing an Airy pattern.
  @param    lx          Generated image size in x.
  @param    ly          Generated image size in y.
  @param    center_x    x-coordinate of Airy pattern center.
  @param    center_y    y-coordinate of Airy pattern center.
  @param    max_pix     Max pixel value of the Airy function.
  @param    airy_size   Diameter of the main lobe.
  @return   1 newly allocated image.

  This function generates an image containing an Airy 2d function centered on 
  the requested pixel (might be a non-integer position), with the specified 
  maximum amplitude and first lobe width.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_airy(
        int         lx,
        int         ly,
        double      center_x,
        double      center_y,
        pixelvalue  max_pix,
        double      airy_size) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image of a 2d gaussian function.
  @param    lx          x size of the generated image.
  @param    ly          y size of the generated image.
  @param    center_x    x position of the gaussian center.
  @param    center_y    y position of the gaussian center.
  @param    sigma       Sigma for the gaussian distribution.
  @return   1 newly allocated image.

  This function generates an image of a 2d gaussian. The gaussian is
  defined by the position of its center, given in pixel coordinates inside
  the image with the FITS convention (x from 1 to lx, y from 1 to ly), and
  the value of sigma.

  The returned image must be deallocated using image_del().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_gauss(
        int         lx,
        int         ly,
        double       center_x,
        double       center_y,
        double       sigma) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image of a lorentzian pattern.
  @param    lx          x size of the generated image.
  @param    ly          y size of the generated image.
  @param    center_x    x position of the lorentzian center.
  @param    center_y    y position of the lorentzian center.
  @param    intensity   Lorentzian intensity.
  @param    dispersion  Lorentzian dispersion.
  @return   1 newly allocated image.

  This function generates an image of a 2d lorentzian. The lorentzian is
  defined by the position of its center, given in pixel coordinates inside
  the image with the FITS convention (x from 1 to lx, y from 1 to ly), and
  the value of intensity and dispersion.

  The returned image must be deallocated using image_del().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_lorentz(
        int     lx,
        int     ly,
        double  center_x,
        double  center_y,
        double  intensity,
        double  dispersion) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image with uniform random noise distribution.
  @param    lx          x size of the generated image.
  @param    ly          y size of the generated image.
  @param    min_pix     Minimum output pixel value.
  @param    max_pix     Maximum output pixel value.
  @return   1 newly allocated image.

  Generate an image with a uniform random noise distribution. Pixel values are 
  within the provided bounds.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_random_uniform(
        int         lx,
        int         ly,
        pixelvalue  min_pix,
        pixelvalue  max_pix) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image with gaussian noise distribution.
  @param    size_x      x size of the generated image.
  @param    size_y      y size of the generated image.
  @param    sigma       Sigma of the gaussian distribution.
  @param    mean        Mean of the gaussian distribution.
  @return   1 newly allocated image.

  Generates an image containing a gaussian noise distribution. To get the
  default sigma (1/sqrt(2)), give a negative value for sigma. No default mean 
  value is provided.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_random_gauss(
        int     size_x,
        int     size_y,
        double  sigma,
        double  mean) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image with lorentzian noise distribution.
  @param    size_x      x size of the generated image.
  @param    size_y      y size of the generated image.
  @param    dispersion  Dispersion of the lorentzian distribution.
  @param    mean        Mean of the gaussian distribution.
  @return   1 newly allocated image.

  Generates an image containing a lorentzian noise distribution. To get the
  default dispersion (1.0), give a negative value. No default mean value is
  provided.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_random_lorentz(
        int     size_x,
        int     size_y,
        double  dispersion,
        double  mean) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image of an ideal Optical Transfer Function.
  @param    m1          Diameter of the M1 mirror in meters.
  @param    m2          Diameter of the M2 mirror in meters.
  @param    lam         Central wavelength in microns.
  @param    dlam        Filter bandwidth in microns.
  @param    size        Generated image size (image will be square).
  @param    pscale      Pixel scale on the sky in arcseconds.
  @return   1 newly generated image.

  This code has been taken from a program called otf_theo.c written by Francois
  Rigaut and Jean-Luc Beuzit. The code was deeply modified to fit in here.

  Based on the paper "Amplitude estimation from speckle interferometry" by 
  Christian Perrier in "Diffraction-limited imaging with very large telescopes",
  NATO ASI Series C, Vol. 274, edited by D. Alloin and J.-M. Mariotti, 1989 
  (p. 99).

  Default values are set for the ESO 3.6m telescope in la Silla. Provide -1.0 
  for any parameter to get the default value used.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_otf(
        double  m1,
        double  m2,
        double  lam,
        double  dlam,
        int     size,
        double  pscale) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the ideal PSF for a given telescope+instrument.
  @param    m1          Diameter of the M1 mirror in meters.
  @param    m2          Diameter of the M2 mirror in meters.
  @param    lam         Central wavelength in microns.
  @param    dlam        Filter bandwidth in microns.
  @param    size        Generated image size (image will be square).
  @param    pscale      Pixel scale on the sky in arcseconds.
  @return   1 newly generated image.

  This function computes the ideal PSF for a given telescope and instrument. 
  The PSF is computed by first generated the ideal OTF for the provided 
  conditions, and applying a Fourier transform to it to bring it back to real 
  space. The returned PSF is normalized to unity flux, to help Strehl ratio 
  computations.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_psf(
        double   m1,
        double   m2,
        double   lam,
        double   dlam,
        double   pscale,
        int      size) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a disk in a binary image.
  @param    size_x      x size of the generated image
  @param    size_y      y size of the generated image
  @param    center_x    x-pos of the of the binary disk center in the image.
  @param    center_y    y-pos of the of the binary disk center in the image.
  @param    radius      Disk radius in pixels.
  @return   1 newly allocated image.

  This function generates a pixelmap containing a white disk (1) over a black 
  background (0). Disk center coordinates are given in the FITS convention: x 
  going from 1 to lx and y going from 1 to ly.
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_gen_disk(
        int     size_x,
        int     size_y,
        double  center_x,
        double  center_y,
        double  radius) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a rectangle in a binary image.
  @param    size_x      x size of the generated image
  @param    size_y      y size of the generated image
  @param    llx         x-coordinate of the rectangle lower left corner.
  @param    lly         y-coordinate of the rectangle lower left corner.
  @param    urx         x-coordinate of the rectangle upper right corner.
  @param    ury         y-coordinate of the rectangle upper right corner.
  @return   1 newly allocated image.

  This function generates a pixelmap containing a white rectangle (1) over
  a black background (0). Rectangle coordinates are given in the FITS
  convention: x going from 1 to lx and y going from 1 to ly. The rectangle
  corners are included in the white zone.

  The returned image must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_gen_rectangle(
        int     size_x,
        int     size_y,
        int     llx,
        int     lly,
        int     urx,
        int     ury) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate the image of a 2nd-degree polynomial in (x,y).
  @param    lx  x size of generated image.
  @param    ly  y size of generated image.
  @param    c   Array of 6 doubles containing the polynomial coefficients.
  @return   1 newly allocated image.

  The coordinate system for the polynomial follows the FITS convention, x
  growing from 1 to lx and y growing from 1 to ly. Lower left corner is (1,1), 
  x increasing from left to right and y from bottom to top.

  Coefficients for the polynomial must be stored as:
    - c[0] for x^2
    - c[1] for y^2
    - c[2] for x.y
    - c[3] for x
    - c[4] for y
    - c[5] for 1
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_poly2d(
        int         lx,
        int         ly,
        double  *   c) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image of a polynomial in x and y.
  @param    lx          x size of the generated image.
  @param    ly          y size of the generated image.
  @param    c           Polynomial coefficients.
  @param    nc          Number of polynomial coefficients.
  @param    poly_deg    Degree of the polynomial.
  @param    poly_string Polynomial definition string.
  @return   1 newly allocated image.

  Generates the image of a polynomial. The input polynomial in (x,y) is defined
  by a string, a list of coefficients, and the number of coefficients in the 
  list. The string defining the polynomial is the same as the one used by 
  fitting functions (see fit_curve.h).
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * image_gen_polynomial(
        int         lx,
        int         ly,
        double  *   c,
        int         nc,
        int         poly_deg,
        char    *   poly_string) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image of a polynomial in x and y in double prec.
  @param    lx          x size of the generated image.
  @param    ly          y size of the generated image.
  @param    c           Polynomial coefficients.
  @param    nc          Number of polynomial coefficients.
  @param    poly_deg    Degree of the polynomial.
  @param    poly_string Polynomial definition string.
  @return   1 newly allocated image.

  Generates the image of a polynomial, in double precision pixels. The input 
  polynomial in (x,y) is defined by a string, a list of coefficients, and the 
  number of coefficients in the list. The string defining the polynomial is 
  the same as the one used by fitting functions (see fit_curve.h).

  The generated image is a 2d array of doubles stored in a 1d list as image
  pixel buffers. This function should only be needed when float pixels do not 
  have enough precision. The returned image is normalized so that it has a mean
  value of 1.0.
 */
/*----------------------------------------------------------------------------*/
/* </python> */
double * image_gen_polynomial_double(
        int         lx,
        int         ly,
        double  *   c,
        int         nc,
        int         poly_deg,
        char    *   poly_string) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a pixelmap containing a polygon.
  @param    lx      x size of the generated image.
  @param    ly      y size of the generated image.
  @param    polygon Polygon vertices as a double3.
  @param    pval    Assign value, must be 0 or 1.
  @return   1 newly allocated pixelmap.

  This function creates a new pixel map containing a polygon. The polygon is 
  defined as its list of vertices in a double3.  All points inside the polygon 
  are assigned the value pval, all points outside are assigned !pval (pval 
  must be 0 or 1).

  Polygon vertice coordinates are expected in the FITS convention: x running 
  from 1 to lx (left to right), y running from 1 to ly (bottom to top), lower 
  left pixel is (1,1).
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_gen_polygon(
        int         lx,
        int         ly,
        double3 *   polygon,
        int         pval) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image for testing purposes.
  @return   1 newly allocated image.
  Generates a reference pattern for testing purposes only.
 */
/*----------------------------------------------------------------------------*/
/* </python> */
image_t * image_gen_testimage(void) ;
/* </python> */


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate an image for jitter testing purposes.
  @param    lx      Size in x of the image to generate
  @param    ly      Size in y of the image to generate
  @param    obj     Position of objects to insert in the frame.
  @param    dx      Frame offset in X
  @param    dy      Frame offset in Y
  @return   1 pointer to a newly allocated image.

  This function produces one jitter frame for jitter testing purposes. It does 
  not make sense alone, it must be used to produce a whole batch of frames.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_jitterimage(
        int         lx,
        int         ly,
        double3 *   obj,
        double      dx,
        double      dy) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a cube for jitter testing purposes.
  @param    nframes     Number of frames to generate
  @param    lx          Size in X of each frame.
  @param    ly          Size in Y of each frame.
  @param    nobj        Number of objects to generate in each frame.
  @param    p_homog     Poisson homogeneity
  @param    ampl        Offset amplitude in percentage between 0 and 1.
  @return   1 newly allocated cube.
  This function generates a cube for jitter testing purposes.
  FIXME: TO BE DOCUMENTED.
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_gen_jittercube(
        int     nframes,
        int     lx,
        int     ly,
        int     nobj,
        int     p_homog,
        double  ampl) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a Mandelbrot set image.
  @param    sx      Size in X of the generated image
  @param    sy      Size in Y of the generated image
  @param    loc     Location in complex space, or NULL for default values.
  @return   1 newly allocated image

  This function generates an image of a Mandelbrot set. The location of
  the image on the set is specified by loc, a pointer to an array of
  4 doubles giving respectively xmin, xmax, ymin and ymax. If this array
  is NULL, default values are used.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_mandelbrot(
        int         sx,
        int         sy,
        double  *   loc) ;

#endif
