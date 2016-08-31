/*----------------------------------------------------------------------------*/
/**
   @file	generate.c
   @author	Nicolas Devillard
   @date	Sept 28, 1995
   @version	$Revision: 1.37 $
   @brief	pattern image generation
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: generate.c,v 1.37 2003/02/21 14:30:48 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/21 14:30:48 $
	$Revision: 1.37 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "generate.h"
#include "polygon.h"
#include "random.h"
#include "pi.h"
#include "image_intops.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define NB_PHOTONS  			100
#define REGULAR_SAMPLE_X    	10
#define REGULAR_SAMPLE_Y    	10
#define INV9                    ((double)1.0/(double)9.0)

/* Seconds to radians conversion */
#define SEC2RAD					(206265)

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/* These functions are only used to compute the  telescope OTF  */
static double PSF_H1(double f, double u, double v);
static double PSF_H2(double f, double u) ;
static double PSF_G(double f, double u) ;
static double PSF_sinc(double x) ;
static double PSF_TelOTF(double f, double u) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the value of a 2d Airy function at a given point.
  @param	x			x where to compute the function.
  @param	y			y where to compute the function.
  @param	max_pix		Airy function amplitude.
  @param	airy_size	Airy first lobe width.
  @return	1 double.

  This function computes the value of a 2d Airy function at a given point, with
  the requested amplitude and first lobe width.
 */
/*----------------------------------------------------------------------------*/
static double airy_2d(
		double	x, 
		double 	y, 
		double 	max_pix, 
		double 	airy_size)
{
    double  radius ;
    double  result ;

    radius = sqrt(x*x + y*y) ;
    if (radius < 1e-4)
        return (double)1.0 ;
    radius /= airy_size ;

    result = j1(radius) / radius ;
    result = 4.0 * max_pix * result * result ;

    return result ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image containing an Airy pattern.
  @param	lx			Generated image size in x.
  @param	ly			Generated image size in y.
  @param	center_x	x-coordinate of Airy pattern center.
  @param	center_y	y-coordinate of Airy pattern center.
  @param	max_pix		Max pixel value of the Airy function.
  @param	airy_size	Diameter of the main lobe.
  @return	1 newly allocated image.

  This function generates an image containing an Airy 2d function centered on 
  the requested pixel (might be a non-integer position), with the specified 
  maximum amplitude and first lobe width.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_airy(
    	int         lx,
    	int         ly,
    	double      center_x,
    	double      center_y,
    	pixelvalue  max_pix,
    	double      airy_size)
{
    image_t		*	airy_patt ;
    double          accu ;
    double          x, y ;
    int             i, j, k, l ;

    if ((max_pix>MAX_PIX_VALUE) || (max_pix<1)) {
        e_error("invalid maximum pixel value: %f aborting generation",max_pix);
        return NULL ;
    }

    /* Create image structure to fill in    */
    airy_patt = image_new(lx, ly) ;
	if (airy_patt==NULL) return NULL ;
    for (j=0 ; j<ly ; j++) {
        for (i=0 ; i<lx ; i++) {
            accu = 0.0 ;
            for (l=0 ; l<REGULAR_SAMPLE_Y ; l++) {
                for (k=0 ; k<REGULAR_SAMPLE_X ; k++) {
                    x = (double)i - (double)center_x + 
                            (double)k/(double)REGULAR_SAMPLE_X ; 
                    y = (double)j - (double)center_y + 
                            (double)l/(double)REGULAR_SAMPLE_Y ;
                    accu += airy_2d(x,y, (double)max_pix, (double)airy_size) ;
                }
            }
            accu /= (double)(REGULAR_SAMPLE_X * REGULAR_SAMPLE_Y) ;
            airy_patt->data[i+j*lx] = (pixelvalue)accu ;
        }
    }
    return airy_patt ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief		Compute the value of a Gaussian function at a given point.
  @param	x		x coordinate where to compute the function.
  @param	y		y coordinate where to compute the function.
  @param	sigma	Sigma for the Gauss distribution.
  @return	1 double
  Compute the value of a 2d Gaussian function at a given point.
 */
/*----------------------------------------------------------------------------*/
static double gaussian_2d(
		double	x, 
		double 	y, 
		double 	sigma)
{
    double  radius ;
    double  result ;

    radius =  x*x + y*y  ;
    
    result = 1 / (sigma * sqrt(2.0 * PI_NUMB) ) ;
    result = result * exp(- radius  / (2.0 * sigma * sigma)) ; 
    return result ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image of a 2d gaussian function.
  @param	lx			x size of the generated image.
  @param	ly			y size of the generated image.
  @param	center_x	x position of the gaussian center.
  @param	center_y	y position of the gaussian center.
  @param	sigma		Sigma for the gaussian distribution.
  @return	1 newly allocated image.

  This function generates an image of a 2d gaussian. The gaussian is
  defined by the position of its center, given in pixel coordinates inside
  the image with the FITS convention (x from 1 to lx, y from 1 to ly), and
  the value of sigma.

  The returned image must be deallocated using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_gauss(
    	int         lx,
    	int         ly,
    	double       center_x,
    	double       center_y,
    	double       sigma)
{
    image_t	*	gauss_patt ;
    double          x, y ;
    int             i, j ;

    gauss_patt = image_new(lx, ly) ;
	if (gauss_patt==NULL) return NULL ;
    for (j=0 ; j<ly ; j++) {
        for (i=0 ; i<lx ; i++) {
            x = (double)i - (double) center_x ;
            y = (double)j - (double) center_y ;
            gauss_patt->data[i+j*lx] = (pixelvalue)gaussian_2d(x,y,sigma) ;
        }
    }
    return gauss_patt ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the value of a Lorentzian function at a given point.
  @param	x			x coordinate where to compute the function.
  @param	y			y coordinate where to compute the function.
  @param	intensity	Lorentzian intensity.
  @param	dispersion	Lorentzian dispersion.
  @return	1 double
  Compute the value of a 2d Lorentzian function at a given point.
 */
/*----------------------------------------------------------------------------*/
static double lorentzian_2d(
		double	x, 
		double 	y, 
		double 	intensity, 
		double 	dispersion) 
{
    double  radius ;
    radius =  x*x + y*y  ;
    return intensity / ( 1 + radius * dispersion ) ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image of a lorentzian pattern.
  @param	lx			x size of the generated image.
  @param	ly			y size of the generated image.
  @param	center_x	x position of the lorentzian center.
  @param	center_y	y position of the lorentzian center.
  @param	intensity	Lorentzian intensity.
  @param	dispersion	Lorentzian dispersion.
  @return	1 newly allocated image.

  This function generates an image of a 2d lorentzian. The lorentzian is
  defined by the position of its center, given in pixel coordinates inside
  the image with the FITS convention (x from 1 to lx, y from 1 to ly), and
  the value of intensity and dispersion.

  The returned image must be deallocated using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_lorentz( 
     	int 	lx, 
     	int 	ly,
     	double 	center_x,
     	double 	center_y,
     	double 	intensity,
    	double 	dispersion) 
{
    image_t	*	lorentz_patt ;
    double          x, y ;
    int             i, j ;

    if ( intensity<0 || dispersion<0 ){
		e_error("intensity and dispersion can only be positive") ;
		return NULL ;
    }

    lorentz_patt = image_new(lx, ly) ;
	if (lorentz_patt==NULL) return NULL ;
    for (j=0 ; j<ly ; j++) {
        for (i=0 ; i<lx ; i++) {
            x = (double)i - (double) center_x ;
            y = (double)j - (double) center_y ;
            lorentz_patt->data[i+j*lx] = (pixelvalue)
				lorentzian_2d(x,y,intensity,dispersion) ;
        }
    }
    return lorentz_patt ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image with uniform random noise distribution.
  @param	lx			x size of the generated image.
  @param	ly			y size of the generated image.
  @param	min_pix		Minimum output pixel value.
  @param	max_pix		Maximum output pixel value.
  @return	1 newly allocated image.

  Generate an image with a uniform random noise distribution. Pixel values are 
  within the provided bounds.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_random_uniform(
    	int         lx,
    	int         ly,
    	pixelvalue  min_pix,
    	pixelvalue  max_pix)
{
    image_t	*	rand_patt ;
    int         	i, j ;

    if (    (max_pix > MAX_PIX_VALUE)    || 
            (max_pix < MIN_PIX_VALUE)    ||
            (min_pix > MAX_PIX_VALUE)    ||
            (min_pix < MIN_PIX_VALUE)    ||
            (min_pix > max_pix)) {
		e_error("invalid interval [%g %g]: aborting image generation",
				(double)min_pix,
				(double)max_pix) ;	
        return(NULL) ;
    }
    rand_patt = image_new(lx, ly) ;
	if (rand_patt==NULL) return NULL ;
    for (j=0 ; j<ly ; j++)
        for (i=0 ; i<lx ; i++)
            rand_patt->data[i+j*lx] = 
                (pixelvalue)(min_pix + drand48() * (max_pix-min_pix)) ;

    return rand_patt ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image with gaussian noise distribution.
  @param	size_x		x size of the generated image.
  @param	size_y		y size of the generated image.
  @param	sigma		Sigma of the gaussian distribution.
  @param	mean		Mean of the gaussian distribution.
  @return	1 newly allocated image.

  Generates an image containing a gaussian noise distribution. To get the
  default sigma (1/sqrt(2)), give a negative value for sigma. No default mean 
  value is provided.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_random_gauss(
		int		size_x,
		int		size_y,
		double	sigma,
		double	mean)
{
	image_t	*	gauss_noise_image ;
	int				i ;

	if (sigma < 1e-8) {
		sigma = 1.0 / sqrt(2.0) ;
		e_warning("using default sigma value: %f", sigma) ;
	}

	gauss_noise_image = image_new(size_x, size_y) ;
	if (gauss_noise_image==NULL) return NULL ;
	for (i=0 ; i<(gauss_noise_image->lx * gauss_noise_image->ly) ; i++) {
		gauss_noise_image->data[i] = 
			(pixelvalue)(random_gauss(sigma) + mean) ;
	}
	return gauss_noise_image ;
}	


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image with lorentzian noise distribution.
  @param	size_x		x size of the generated image.
  @param	size_y		y size of the generated image.
  @param	dispersion	Dispersion of the lorentzian distribution.
  @param	mean		Mean of the gaussian distribution.
  @return	1 newly allocated image.

  Generates an image containing a lorentzian noise distribution. To get the
  default dispersion (1.0), give a negative value. No default mean value is
  provided.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_random_lorentz(
		int		size_x,
		int		size_y,
		double	dispersion,
		double	mean)
{
	image_t	*	lorentz_noise_image ;
	int				i ;

	if (dispersion < 1e-8) {
		dispersion = 1.0 ;
	}

	lorentz_noise_image = image_new(size_x, size_y) ;
	if (lorentz_noise_image==NULL) return NULL ;

	for (i=0 ; i<(lorentz_noise_image->lx * lorentz_noise_image->ly) ; i++) {
		lorentz_noise_image->data[i] = 
			(pixelvalue)(random_lorentz(dispersion) + mean) ;
	}

	return lorentz_noise_image ;
}	


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image of an ideal Optical Transfer Function.
  @param	m1			Diameter of the M1 mirror in meters.
  @param	m2			Diameter of the M2 mirror in meters.
  @param	lam			Central wavelength in microns.
  @param	dlam		Filter bandwidth in microns.
  @param	size		Generated image size (image will be square).
  @param	pscale		Pixel scale on the sky in arcseconds.
  @return	1 newly generated image.

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
image_t * image_gen_otf(
    	double  m1,
    	double  m2,
    	double  lam,
    	double  dlam,
    	int  	size,
    	double  pscale)
{
    image_t    *	otf_image ;
    double   		obs_ratio ;  /* m1 / m2    */
    double   		f_max ;      /* cut-off frequency        */
    int     		pix0 ;      /* Pixel corresponding to the zero frequency */
    int     		i, j, k ;
    double  		a, x, y ;
    double  		f, r, fc, invfc, lambda ;
    double  		sincy ;
    double  		invsize ;
    register int	 pos ;

    /* No test is made at this point to see if the values are correctly */
    /* set, it is up to the calling function to check that. */

    /* Convert pixel scale in radians, microns in meters    */
    pscale /= (double)SEC2RAD ;
    lam /= (double)1.0e6 ;
    dlam /= (double)1.0e6 ;

    /* Obscuration ratio    */
    obs_ratio = m2 / m1 ;
    
    /* Pixel corresponding to the zero frequency    */
    pix0 = size/2 ;
    invsize = (double)1.0 / (double)size ;

    /* Cut-off frequency in pixels  */
    f_max = m1 * pscale * (double)size / lam ;

    /* Allocate for output image    */
    otf_image = image_new(size, size) ;
	if (otf_image==NULL) return NULL ;

    /* Now compute the OTF  */
    /* OPTIMIZED CODE !!! LIMITED READABILITY !!!   */

    for (k=1 ; k<=9 ; k++) {    /* iteration on the wavelength  */
        /* Compute intermediate cut-off frequency   */
        lambda = (double)(lam - dlam*(double)(k-5)/8.0) ;
        fc = (double)f_max * (double)lam / lambda ; 
        invfc = 1.0 / fc ;
    
        /* Convolution with the detector pixels */
        pos = 0 ;
        for (j=0 ; j<size ; j++) {
            y = (double)(j-pix0) ;
            sincy = PSF_sinc(PI_NUMB * y * invsize) ;
            pos = j * size ;
            for (i=0 ; i<size ; i++) {
                x = (double)(i-pix0) ;
                r = sqrt(x*x + y*y) ;
                f = r * invfc ;
                if (f<1.0) {
                    if (r<0.1)
                        a = 1.0 ;
                    else {
                        a = PSF_TelOTF(f,obs_ratio) * 
                            PSF_sinc(PI_NUMB * x * invsize) * sincy ;
                    }
                } else {
                    a = 0.0 ;
                }
                otf_image->data[pos++] += (pixelvalue)(a * INV9) ;
            }
        }
    }
    return otf_image ;
}


/*----------------------------------------------------------------------------*
 *       These functions are only used to compute the telescope OTF 
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 * H1 function
 *----------------------------------------------------------------------------*/
static double PSF_H1(
		double	f, 
		double 	u, 
		double 	v)
{
  double 	e ;

  if (fabs(1.0-v)<1.0e-12)  e = 1.0 ; /* e = 1.0 if v = 1.0 */
  else                      e = -1.0 ;

  return((v*v/PI_NUMB)*acos((f/v)*(1.0+e*(1.0-u*u)/(4.0*f*f))));
}


/*----------------------------------------------------------------------------*
 * H2 function
 *----------------------------------------------------------------------------*/
static double PSF_H2(
		double	f, 
		double	u)
{
    double  tmp1, tmp2, ret ;

    tmp1 = (2.0 * f) / (1.0 + u) ; 
    tmp2 = (1.0 - u) / (2.0 * f) ;
    ret = -1.0 * (f/PI_NUMB) * (1.0+u) * sqrt((1.0-tmp1*tmp1)*(1.0-tmp2*tmp2));
    return ret ;
}


/*----------------------------------------------------------------------------*
 * G function
 *----------------------------------------------------------------------------*/
static double PSF_G(
		double	f, 
		double	u)
{
    if (f<=(1.0-u)/2.0) return(u*u);
    if (f>=(1.0+u)/2.0) return(0.0);
    else return(PSF_H1(f,u,1.0) + PSF_H1(f,u,u) + PSF_H2(f,u));
}


/*----------------------------------------------------------------------------*
 * sinc function
 *----------------------------------------------------------------------------*/
static double PSF_sinc(double x)
{
  if(fabs(x) < 1e-4) return(1.0);
  return(sin(x)/x) ;
}


/*----------------------------------------------------------------------------*
 * Telescope OTF function
 *----------------------------------------------------------------------------*/
static double PSF_TelOTF(
		double	f, 
		double 	u)
{ 
    return((PSF_G(f,1.0)+u*u*PSF_G(f/u,1.0)-2.0*PSF_G(f,u))/(1.0-u*u)); 
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the ideal PSF for a given telescope+instrument.
  @param	m1			Diameter of the M1 mirror in meters.
  @param	m2			Diameter of the M2 mirror in meters.
  @param	lam			Central wavelength in microns.
  @param	dlam		Filter bandwidth in microns.
  @param	size		Generated image size (image will be square).
  @param	pscale		Pixel scale on the sky in arcseconds.
  @return	1 newly generated image.

  This function computes the ideal PSF for a given telescope and instrument. 
  The PSF is computed by first generated the ideal OTF for the provided 
  conditions, and applying a Fourier transform to it to bring it back to real 
  space. The returned PSF is normalized to unity flux, to help Strehl ratio 
  computations.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_psf(
    	double   m1,
    	double   m2,
    	double   lam,
    	double   dlam,
    	double   pscale,
    	int		 size)
{
    image_t    *	otf_image ;
    cube_t     *	complex_psf ;
    cube_t     *	polar_psf ;
    image_t    *	psf_image ;
	image_t	   *    norm ;

    /* Generate Optical Transfer Function of the telescope  */
    otf_image = image_gen_otf(m1, m2, lam, dlam, size, pscale) ;
    if (otf_image == NULL) {
        e_error("cannot generate OTF: aborting PSF generation") ;
        return NULL ;
    }

    /* The generated image is in Fourier space: back into reality   */
    complex_psf = image_fft(otf_image, NULL, FFT_FORWARD) ;
    image_del(otf_image) ;
    if (complex_psf == NULL) {
        e_error("cannot FFT OTF: aborting PSF generation") ;
        return NULL ;
    }

    /* Convert from (x,y) coordinates to (r, theta) */

    polar_psf = cube_conv_xy_rtheta(complex_psf) ;
    cube_del(complex_psf) ;
    if (polar_psf == NULL) {
        e_error("cannot convert coordinates: aborting PSF generation") ;
        return NULL ;
    }

    /* Now get first plane as PSF and swap quadrants in output  */
    psf_image = image_copy(polar_psf->plane[0]) ;
    cube_del(polar_psf) ;
    if (psf_image == NULL) {
        e_error("cannot extract image from polar PSF: aborting generation") ;
        return NULL ;
    }
    image_swapquad(psf_image) ;

    /* Now normalize PSF to get flux=1  */
	norm = image_normalize(psf_image, NORM_FLUX);
	image_del(psf_image);
	if (norm==NULL) {
        e_error("cannot normalize PSF to unity flux: aborting") ;
    }
    return norm ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a disk in a binary image.
  @param	size_x		x size of the generated image
  @param	size_y		y size of the generated image
  @param	center_x	x-pos of the of the binary disk center in the image.
  @param	center_y	y-pos of the of the binary disk center in the image.
  @param	radius		Disk radius in pixels.
  @return	1 newly allocated image.

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
    	double  radius)
{
    pixelmap    *	disk ;
    int		     	i, j ;
    double       	sq_radius, dist ;

    disk = pixelmap_new(size_x, size_y) ;
	if (disk==NULL) return NULL ;

    /* Conversion to internal coordinates: first pixel at (0,0) */
    center_x-= 1.0 ;
    center_y-= 1.0 ;
    sq_radius = radius * radius ;
    for (j=0 ; j<size_y ; j++) {
        for (i=0 ; i<size_x ; i++) {
            dist =  ((double)i - center_x) * ((double)i - center_x) +
                    ((double)j - center_y) * ((double)j - center_y) ;
            if (dist <= sq_radius) {
                disk->data[i+j*size_x] = PIXELMAP_1 ;
            } else {
                disk->data[i+j*size_x] = PIXELMAP_0 ;
                disk->ngoodpix -- ;
            }   
        }
    }
    return disk ;
}   


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a rectangle in a binary image.
  @param	size_x		x size of the generated image
  @param	size_y		y size of the generated image
  @param	llx			x-coordinate of the rectangle lower left corner.
  @param	lly			y-coordinate of the rectangle lower left corner.
  @param	urx			x-coordinate of the rectangle upper right corner.
  @param	ury			y-coordinate of the rectangle upper right corner.
  @return	1 newly allocated image.

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
    	int     ury)
{
    pixelmap    *	rect ;
    int		     	i, j;

    rect = pixelmap_new(size_x, size_y) ;
	if (rect==NULL) return NULL ;

    /* Conversion to internal coordinates: first pixel at (0,0) */
    llx -- ; lly -- ; urx -- ; ury -- ;

    for (j=0 ; j<size_y ; j++) {
        for (i=0 ; i<size_x ; i++) {
            if ((i>=llx) && (i<=urx) && (j>=lly) && (j<=ury)) {
                rect->data[i+j*size_x] = PIXELMAP_1 ;
            } else {
                rect->data[i+j*size_x] = PIXELMAP_0 ;
                rect->ngoodpix -- ;
            }
        }
    }   
    return rect ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate the image of a 2nd-degree polynomial in (x,y).
  @param	lx	x size of generated image.
  @param	ly	y size of generated image.
  @param	c	Array of 6 doubles containing the polynomial coefficients.
  @return	1 newly allocated image.

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
image_t * image_gen_poly2d(
		int			lx,
		int			ly,
		double	*	c)
{
	image_t	*	ret ;
	int				i, j ;
	double			x, y, z ;

	ret = image_new(lx, ly) ;
	if (ret==NULL) return NULL ;

	for (j=0 ; j<ly ; j++) {
		y = (double)j + 1.00 ;
		for (i=0 ; i<lx ; i++) {
			x = (double)i + 1.00 ;
			z = c[0] * x * x +
				c[1] * y * y +
				c[2] * x * y +
				c[3] * x +
				c[4] * y +
				c[5] ;
			ret->data[i+j*lx] = (pixelvalue)z ;
		}
	}
	return ret ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image of a polynomial in x and y.
  @param	lx			x size of the generated image.
  @param	ly			y size of the generated image.
  @param	c			Polynomial coefficients.
  @param	nc			Number of polynomial coefficients.
  @param	poly_deg	Degree of the polynomial.
  @param	poly_string	Polynomial definition string.
  @return	1 newly allocated image.

  Generates the image of a polynomial. The input polynomial in (x,y) is defined
  by a string, a list of coefficients, and the number of coefficients in the 
  list. The string defining the polynomial is the same as the one used by 
  fitting functions (see fit_curve.h).
 */
/*--------------------------------------------------------------------------*/
image_t * image_gen_polynomial(
		int			lx,
		int			ly,
		double	*	c,
		int			nc,
		int			poly_deg,
		char	*	poly_string)
{
	image_t	*	gen ;
	int				i, j, k ;
	double			x, y, z ;
	double			xp, yp ;
	int			*	tdx,
				*	tdy ;
	int				nc2 ;

	gen = image_new(lx, ly) ;
	if (gen==NULL) return NULL ;

	nc2 = (1+poly_deg)*(2+poly_deg)/2 ;
	tdx = malloc(nc2 * sizeof(int)) ;
	tdy = malloc(nc2 * sizeof(int)) ;

	nc2 = buildup_polytab_from_string(poly_string,
									  poly_deg,
									  tdx,
									  tdy) ;
	if (nc2!=nc) {
		e_error("control string and provided # of coeffs do not match\n"
				"%d provided but %d found in string\n"
				"string is %s",
				nc, nc2, poly_string) ;
		free(tdx) ;
		free(tdy) ;
		image_del(gen) ;
		return NULL ;
	}

	/*
	 * Need to store the results of ipow() in local variables xp, yp
	 * due to a bug in Solaris compiler...
	 */
	for (j=0 ; j<ly ; j++) {
		y = (double)j+1.00 ;
		for (i=0 ; i<lx ; i++) {
			x = (double)i+1.00 ;
			z = 0.00 ;
			for (k=0 ; k<nc ; k++) {
				xp = ipow(x, tdx[k]) ;
				yp = ipow(y, tdy[k]) ;
				z += c[k] * xp * yp ; 
			}
			gen->data[i+j*lx] = (pixelvalue)z ;
		}
	}
	free(tdx) ;
	free(tdy) ;
	return gen ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image of a polynomial in x and y in double prec.
  @param	lx			x size of the generated image.
  @param	ly			y size of the generated image.
  @param	c			Polynomial coefficients.
  @param	nc			Number of polynomial coefficients.
  @param	poly_deg	Degree of the polynomial.
  @param	poly_string	Polynomial definition string.
  @return	1 newly allocated image.

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
double * image_gen_polynomial_double(
		int			lx,
		int			ly,
		double	*	c,
		int			nc,
		int			poly_deg,
		char	*	poly_string)
{
	double	*	gen ;
	int			i, j, k ;
	double		x, y, z ;
	double		xp, yp ;
	int		*	tdx,
			*	tdy ;
	int			nc2 ;
	double		average ;

	gen = malloc(lx * ly * sizeof(double));
	if (gen==NULL) return NULL ;

	nc2 = (1+poly_deg)*(2+poly_deg)/2 ;
	tdx = malloc(nc2 * sizeof(int)) ;
	tdy = malloc(nc2 * sizeof(int)) ;

	nc2 = buildup_polytab_from_string(poly_string,
									  poly_deg,
									  tdx,
									  tdy) ;
	if (nc2!=nc) {
		e_error("control string and provided # of coeffs do not match\n"
				"%d provided but %d found in string\n"
				"string is %s",
				nc, nc2, poly_string) ;
		free(tdx) ;
		free(tdy) ;
		free(gen) ;
		return NULL ;
	}

	/*
	 * Need to store the results of ipow() in local variables xp, yp
	 * due to a bug in Solaris compiler...
	 */
	for (j=0 ; j<ly ; j++) {
		y = (double)j+1.00 ;
		for (i=0 ; i<lx ; i++) {
			x = (double)i+1.00 ;
			z = 0.00 ;
			for (k=0 ; k<nc ; k++) {
				xp = ipow(x, tdx[k]) ;
				yp = ipow(y, tdy[k]) ;
				z += c[k] * xp * yp ; 
			}
			gen[i+j*lx] = z ;
		}
	}
	free(tdx) ;
	free(tdy) ;

	/* Normalize the image so that it has an average value of 1.0 */
	average = 0.00 ;
	for (i=0 ; i<lx*ly ; i++) average += gen[i] ;
	average = (double)lx * (double)ly / average ;
	for (i=0 ; i<lx*ly ; i++) gen[i] *= average ;
	return gen ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a pixelmap containing a polygon.
  @param	lx		x size of the generated image.
  @param	ly		y size of the generated image.
  @param	polygon	Polygon vertices as a double3.
  @param	pval	Assign value, must be 0 or 1.
  @return	1 newly allocated pixelmap.

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
		int			lx,
		int			ly,
		double3	*	polygon,
		int			pval)
{
	pixelmap	*	map_out ;
	int				i, j ;

	map_out = pixelmap_new(lx, ly);
	for (j=0 ; j<ly ; j++) {
		for (i=0 ; i<lx ; i++) {
			if (polygon_contains_point(polygon, i-1, j-1)) {
				map_out->data[i+j*lx] = pval ;
			} else {
				map_out->data[i+j*lx] = !pval ;
			}
		}
	}
	return map_out ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief 	Generate an image for testing purposes.
  @return	1 newly allocated image.
  Generates a reference pattern for testing purposes only.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_testimage(void)
{
	image_t	*	testim ;
	int				i, j ;
	double			x, y, z ;
	double			ax, bx, ay, by ;

	/* Hardcode everything: this must be kept constant over time */

	/* 1024x512 allows testing of x/y properties */
	testim = image_new(1024, 512) ;
	if (testim==NULL) return NULL ;

	/* Generate sine waves into this image */
	ax = (2.0 * PI_NUMB) / (double)(testim->lx -1) ;
	bx = PI_NUMB ;

	ay = (2.0 * PI_NUMB) / (double)(testim->ly -1);
	by = PI_NUMB ;

	for (j=0 ; j<testim->ly ; j++) {
		y = ay * (double)j + by ;
		for (i=0 ; i<testim->lx ; i++) {
			x = ax * (double)i + bx ;
			z = sin(2.0 * x) * cos(y) ;
			testim->data[i+j*testim->lx] = (pixelvalue)(1000.0 * z);
		}
	}
	return testim ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate an image for jitter testing purposes.
  @param	lx		Size in x of the image to generate
  @param	ly		Size in y of the image to generate
  @param	obj		Position of objects to insert in the frame.
  @param	dx		Frame offset in X
  @param	dy		Frame offset in Y
  @return	1 pointer to a newly allocated image.

  This function produces one jitter frame for jitter testing purposes. It does 
  not make sense alone, it must be used to produce a whole batch of frames.
 */
/*----------------------------------------------------------------------------*/
image_t * image_gen_jitterimage(
		int			lx,
		int			ly,
		double3	*	obj,
		double		dx,
		double		dy)
{
	image_t	*	gen ;
	image_t	*	one_object ;
	int				i ;
	double			pos_x,
					pos_y ;
	int				pix, npix ;

	if (obj==NULL) return NULL ;

	/* Generate a noisy background */
	gen = image_gen_random_gauss(lx, ly, 1.0, 0.0);

	/* Add objects one by one */
	npix = lx * ly ;
	for (i=0 ; i<obj->n ; i++) {
		pos_x = (double)(lx/2)+obj->x[i]+dx ;
		pos_y = (double)(ly/2)+obj->y[i]+dy ;
		one_object = image_gen_gauss(lx, ly, pos_x, pos_y, 2.0);
		for (pix=0 ; pix<npix ; pix++) {
			gen->data[pix] +=
				(pixelvalue)((double)one_object->data[pix] * obj->z[i]);
		}
		image_del(one_object);
	}
	return gen ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a cube for jitter testing purposes.
  @param	nframes		Number of frames to generate
  @param	lx			Size in X of each frame.
  @param	ly			Size in Y of each frame.
  @param	nobj		Number of objects to generate in each frame.
  @param	p_homog		Poisson homogeneity
  @param	ampl		Offset amplitude in percentage between 0 and 1.
  @return	1 newly allocated cube.
  This function generates a cube for jitter testing purposes.
  FIXME: TO BE DOCUMENTED.
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_gen_jittercube(
		int		nframes,
		int		lx,
		int		ly,
		int		nobj,
		int		p_homog,
		double	ampl)
{
	cube_t	*	gen ;
	int			p ;
	double3	*	obj ;
	double3	*	offs ;
	int			gen_zone[4] ;

	if (nframes<1 || lx<1 || ly<1) return NULL ;

	/* Compute valid zone for offset generation */
	gen_zone[0] = -(int)(0.5+(double)lx*ampl); /* xmin */
	gen_zone[1] =  (int)(0.5+(double)lx*ampl); /* xmax */
	gen_zone[2] = -(int)(0.5+(double)ly*ampl); /* ymin */
	gen_zone[3] =  (int)(0.5+(double)ly*ampl); /* ymax */

	/* Generate Poisson offsets */
	offs = generate_rect_poisson_points(gen_zone, nframes, p_homog);
	if (offs==NULL) {
		e_error("generating offsets: aborting");
		return NULL ;
	}

	/* Compute valid zone for object generation */
	gen_zone[0] = -lx/2 ; /* xmin */
	gen_zone[1] =  lx/2 ; /* xmax */
	gen_zone[2] = -ly/2 ; /* ymin */
	gen_zone[3] =  ly/2 ; /* ymax */

	/* Generate objects with Poisson scattering */
	obj = generate_rect_poisson_points(gen_zone, nobj, nobj);
	if (obj==NULL) {
		e_error("generating objects: aborting");
		double3_del(offs);
		return NULL ;
	}
	/* Set random amplitudes */
	for (p=0 ; p<obj->n ; p++) {
		obj->z[p] = drand48() * (2300.77) + 10.0 ;
	}

	/* Generate empty cube */
	gen = cube_new(lx, ly, nframes);

	/* Generate frame by frame and store into cube */
	for (p=0 ; p<nframes ; p++) {
		compute_status("generating planes...", p, nframes, 0);
		gen->plane[p] = image_gen_jitterimage(lx,
											  ly,
											  obj,
											  offs->x[p],
											  offs->y[p]);
	}
	double3_del(offs);
	double3_del(obj);
	return gen ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a Mandelbrot set image.
  @param	sx		Size in X of the generated image
  @param	sy		Size in Y of the generated image
  @param	loc		Location in complex space, or NULL for default values.
  @return	1 newly allocated image

  This function generates an image of a Mandelbrot set. The location of
  the image on the set is specified by loc, a pointer to an array of
  4 doubles giving respectively xmin, xmax, ymin and ymax. If this array
  is NULL, default values are used.
 */
/*----------------------------------------------------------------------------*/
/** Default xmin for Mandelbrot set */
#define MANDEL_XMIN     -2.0
/** Default xmax for Mandelbrot set */
#define MANDEL_XMAX      1.0
/** Default ymin for Mandelbrot set */
#define MANDEL_YMIN     -1.5
/** Default ymax for Mandelbrot set */
#define MANDEL_YMAX      1.5
/** Default value for divergence of a series */
#define MANDEL_BAILOUT   4.0
/** Max number of iterations for a series */
#define MANDEL_ITERMAX   1000
image_t * image_gen_mandelbrot(
		int			sx, 
		int 		sy, 
		double	* 	loc)
{
    image_t *   mandel ;
    int         i, j ;
    int         iter ;
    double      c_re, c_im ;
    double      z_re, z_im ;
    double      zi_re, zi_im ;
    double      x_step, y_step ;
    double      mod ;
	double		xmin, ymin ;

	if (loc==NULL) {
		x_step = (MANDEL_XMAX - MANDEL_XMIN) / (double)(sx-1) ;
		y_step = (MANDEL_YMAX - MANDEL_YMIN) / (double)(sy-1) ;
		xmin = MANDEL_XMIN ;
		ymin = MANDEL_YMIN ;
	} else {
		x_step = (loc[1] - loc[0]) / (double)(sx-1) ;
		y_step = (loc[3] - loc[2]) / (double)(sy-1) ;
		xmin = loc[0] ;
		ymin = loc[2] ;
	}
    mandel = image_new(sx, sy);
    for (j=0 ; j<sy ; j++) {
        compute_status("computing fractal...", j, sy, 1);
        c_im = ymin + (double)j * y_step ;
        for (i=0 ; i<sx ; i++) {
            c_re = xmin + (double)i * x_step ;
            /* Initialize z0 */
            z_re = c_re ;
            z_im = c_im ;
            for (iter=0 ; iter<MANDEL_ITERMAX ; iter++) {
                zi_re = (z_re*z_re)-(z_im*z_im)+c_re ;
                zi_im = 2.0 * z_re * z_im + c_im ;
                z_re = zi_re ;
                z_im = zi_im ;
                mod = z_re*z_re + z_im*z_im ;
                if (mod>MANDEL_BAILOUT)
                    break ;
            }
            mandel->data[i+j*sx] = (pixelvalue)mod ;
        }
    }
    return mandel ;
}
#undef MANDEL_XMIN
#undef MANDEL_XMAX
#undef MANDEL_YMIN
#undef MANDEL_YMAX
#undef MANDEL_BAILOUT
#undef MANDEL_ITERMAX

