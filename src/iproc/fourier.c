/*----------------------------------------------------------------------------*/
/**
   @file	fourier.c
   @author	Nicolas Devillard
   @date	Oct 1995
   @version	$Revision: 1.16 $
   @brief	fourier transform routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: fourier.c,v 1.16 2003/02/20 14:05:29 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/20 14:05:29 $
	$Revision: 1.16 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "fourier.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute a Fast Fourier Transform on an image.
  @param	real_img		Image of real values.
  @param	imaginary_img	Image of imaginary values.
  @param	sign			Transform direction.
  @return	1 newly allocated cube containing 2 planes.

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
		image_t 	*	real_img,
		image_t		*	imaginary_img,
		int				sign)
{
    cube_t  	*	out ;
	dcomplex	*	cbuffer ;
	unsigned		dim[2] ;
    int        		i;
    int        		size;
	double			surface ;

    if (real_img==NULL) return NULL ;
 
    /* only square images, no test to see if it is a power of 2 */
    if (real_img->lx != real_img->ly) {
        e_error("can only FFT square images (this one: %d x %d): aborting",
				real_img->lx, real_img->ly) ;
        return NULL ;
    }
    size = (int)real_img->lx ;
 
    /* Allocate buffer to work */
    /* Allocate for double size : each pixel will get complex   */

	cbuffer = calloc((real_img->lx*real_img->ly), sizeof(dcomplex)) ;

	/* Allocate out Cube	*/ 
    out = cube_new(size, size, 2) ;
	out->plane[0] = image_new(real_img->lx, real_img->ly) ;
	out->plane[1] = image_new(real_img->lx, real_img->ly) ;

	/* OK, now fill buffer with Image values and 0's for imaginary	*/
	if (imaginary_img == NULL) {
		for (i=0 ; i<(real_img->lx*real_img->ly) ; i++) {
			cbuffer[i].x = (double)real_img->data[i] ;
			cbuffer[i].y = (double)0.0 ;
		}															
	} else {
		for (i=0 ; i<(real_img->lx*real_img->ly) ; i++) {
			cbuffer[i].x = (double)real_img->data[i] ;
			cbuffer[i].y = (double)imaginary_img->data[i] ;
		}															
	}

	/* set up image dimensions	*/
	dim[0] = dim[1] = size ;

	/* Now when can go: HERE IS THE FFT !!!	 
	 ****************************************/
		fftn(cbuffer, dim, 2, sign) ;
	/****************************************/
	
	/* Supposed to be OK, now copy results into out cube	*/

	surface = (double)size;
	for (i=0 ; i<(real_img->lx * real_img->ly) ; i++) {
		out->plane[0]->data[i] = (pixelvalue)(cbuffer[i].x / surface);
		out->plane[1]->data[i] = (pixelvalue)(cbuffer[i].y / surface);
	}

	free(cbuffer) ;
	return out ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Convert a 2-plane cube from (real,imag) to (ampl,phase).
  @param	cube_in		Input cube (containing 2 planes)
  @return	1 newly allocated cube containing 2 planes.

  The input cube is expected to contain two planes: first one is the real part 
  of a complex image, second one is the imaginary part of the same image. The 
  returned cube contains two planes: first one is the complex amplitude of the 
  image, second one is the phase.
  The returned cube must be deallocated using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_conv_xy_rtheta(cube_t * cube_in)
{
	cube_t 	*	cube_out ;
	int				i, j ;
	double			re, im ;
	pixelvalue		mod, phase ;

	/* Error handling : test entries	*/
	if (cube_in==NULL) return NULL ;
	if (cube_in->np != 2) {
		e_error("input cube has %d planes, should be 2 for conversion",
				cube_in->np) ;
		return NULL ;
	}

	/* Allocate cube_out */
	cube_out = cube_new(cube_in->lx, cube_in->ly, 2) ;
	cube_out->plane[0] = image_new(cube_in->lx, cube_in->ly) ;
	cube_out->plane[1] = image_new(cube_in->lx, cube_in->ly) ;

	/* Convert */
	for (j=0 ; j<cube_in->ly ; j++) {
		for (i=0 ; i<cube_in->lx ; i++) {
			re = (double)cube_in->plane[0]->data[i+j*cube_in->lx] ;
			im = (double)cube_in->plane[1]->data[i+j*cube_in->lx] ;
			mod = (pixelvalue)(sqrt(re*re + im*im)) ;
			if (re != 0.0)
				phase = (pixelvalue)atan2(im, re) ;
			else 
				phase = 0.0 ;
			cube_out->plane[0]->data[i+j*cube_out->lx] = mod ; 
			cube_out->plane[1]->data[i+j*cube_out->lx] = phase ; 
		}
	}
	return cube_out ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Convert a 2-plane cube from (ampl,phase) to (real,imag).
  @param	cube_in		Input cube (containing 2 planes)
  @return	1 newly allocated cube containing 2 planes.

  The input cube is expected to contain two planes: first one is the
  amplitude of a complex image, second one is the phase. The returned cube
  contains two planes: first one is the real part of the image, second one
  is the imaginary part.

  The returned cube must be deallocated using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_conv_rtheta_xy(cube_t * cube_in)
{
	cube_t 	*	cube_out ;
	int				i, j ;
	pixelvalue		re, im ;
	double			mod, phase ;

	if (cube_in==NULL) return NULL ;
	if (cube_in->np != 2) {
		e_error("input cube has %d planes, should be 2 for conversion",
				cube_in->np) ;
		return NULL ;
	}

	cube_out = cube_new(cube_in->lx, cube_in->ly, 2) ;
	cube_out->plane[0] = image_new(cube_in->lx, cube_in->ly) ;
	cube_out->plane[1] = image_new(cube_in->lx, cube_in->ly) ;

	/* Convert */
	for (j=0 ; j<cube_in->ly ; j++) {
		for (i=0 ; i<cube_in->lx ; i++) {
			mod = (double)cube_in->plane[0]->data[i+j*cube_in->lx] ;
			phase = (double)cube_in->plane[1]->data[i+j*cube_in->lx] ;
			re = (pixelvalue)(mod * cos(phase));
			im = (pixelvalue)(mod * sin(phase));
			cube_out->plane[0]->data[i+j*cube_out->lx] = re ; 
			cube_out->plane[1]->data[i+j*cube_out->lx] = im ; 
		}
	}
	return cube_out ;
}

