/*-------------------------------------------------------------------------*/
/**
   @file	resampling.c
   @author	Nicolas Devillard
   @date	Jan 04, 1996
   @version	$Revision: 1.21 $
   @brief	resampling routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: resampling.c,v 1.21 2002/03/26 12:43:41 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/03/26 12:43:41 $
	$Revision: 1.21 $
*/

/*---------------------------------------------------------------------------
  								Includes
 ---------------------------------------------------------------------------*/

#include "resampling.h"
#include "pi.h"

/*---------------------------------------------------------------------------
  							Private functions
 ---------------------------------------------------------------------------*/

static void reverse_tanh_kernel(double * data, int nn) ;

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Generate an interpolation kernel to use in this module.
  @param	kernel_type		Type of interpolation kernel.
  @return	1 newly allocated array of doubles.

  Provide the name of the kernel you want to generate. Supported kernel
  types are:

  \begin{tabular}{ll}
  NULL			&	default kernel, currently "tanh" \\
  "default"		&	default kernel, currently "tanh" \\
  "tanh"		&	Hyperbolic tangent \\
  "sinc2"		&	Square sinc \\
  "lanczos"		&	Lanczos2 kernel \\
  "hamming"		&	Hamming kernel \\
  "hann"		&	Hann kernel
  \end{tabular}

  The returned array of doubles is ready of use in the various re-sampling
  functions in this module. It must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * generate_interpolation_kernel(char * kernel_type)
{
    double  *	tab ;
    int     	i ;
    double  	x ;
	double		alpha ;
	double		inv_norm ;
    int     	samples = KERNEL_SAMPLES ;

	if (kernel_type==NULL) {
		tab = generate_interpolation_kernel("tanh") ;
	} else if (!strcmp(kernel_type, "default")) {
		tab = generate_interpolation_kernel("tanh") ;
	} else if (!strcmp(kernel_type, "sinc")) {
		tab = malloc(samples * sizeof(double)) ;
		tab[0] = 1.0 ;
		tab[samples-1] = 0.0 ;
		for (i=1 ; i<samples ; i++) {
			x = (double)KERNEL_WIDTH * (double)i/(double)(samples-1) ;
			tab[i] = sinc(x) ;
		}
	} else if (!strcmp(kernel_type, "sinc2")) {
		tab = malloc(samples * sizeof(double)) ;
		tab[0] = 1.0 ;
		tab[samples-1] = 0.0 ;
		for (i=1 ; i<samples ; i++) {
			x = 2.0 * (double)i/(double)(samples-1) ;
			tab[i] = sinc(x) ;
			tab[i] *= tab[i] ;
		}
	} else if (!strcmp(kernel_type, "lanczos")) {
		tab = malloc(samples * sizeof(double)) ;
		for (i=0 ; i<samples ; i++) {
			x = (double)KERNEL_WIDTH * (double)i/(double)(samples-1) ;
			if (fabs(x)<2) {
				tab[i] = sinc(x) * sinc(x/2) ;
			} else {
				tab[i] = 0.00 ;
			}
		}
	} else if (!strcmp(kernel_type, "hamming")) {
		tab = malloc(samples * sizeof(double)) ;
		alpha = 0.54 ;
		inv_norm  = 1.00 / (double)(samples - 1) ;
		for (i=0 ; i<samples ; i++) {
			x = (double)i ;
			if (i<(samples-1)/2) {
				tab[i] = alpha + (1-alpha) * cos(2.0*PI_NUMB*x*inv_norm) ;
			} else {
				tab[i] = 0.0 ;
			}
		}
	} else if (!strcmp(kernel_type, "hann")) {
		tab = malloc(samples * sizeof(double)) ;
		alpha = 0.50 ;
		inv_norm  = 1.00 / (double)(samples - 1) ;
		for (i=0 ; i<samples ; i++) {
			x = (double)i ;
			if (i<(samples-1)/2) {
				tab[i] = alpha + (1-alpha) * cos(2.0*PI_NUMB*x*inv_norm) ;
			} else {
				tab[i] = 0.0 ;
			}
		}
	} else if (!strcmp(kernel_type, "tanh")) {
		tab = generate_tanh_kernel(TANH_STEEPNESS) ;
	} else {
		e_error("unrecognized kernel type [%s]: aborting generation",
				kernel_type) ;
		return NULL ;
	}
    return tab ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Cardinal sine.
  @param	x	double value.
  @return	1 double.

  Compute the value of the function sinc(x)=sin(pi*x)/(pi*x) at the
  requested x.
 */
/*--------------------------------------------------------------------------*/
double sinc(double x)
{
    if (fabs(x)<1e-4)
        return (double)1.00 ;
    else
        return ((sin(x * (double)PI_NUMB)) / (x * (double)PI_NUMB)) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Warp an image according to a linear transformation.
  @param	image_in		Image to warp.
  @param	param			Linear transformation definition.
  @param	kernel_type		Interpolation kernel to use.
  @return	1 newly allocated image.

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
    	image_t    	*	image_in,
    	double      *	param,
		char		*	kernel_type)
{	
    image_t    *	image_out ;
    int         	i, j, k ;
    int         	lx_out, ly_out ;
    double       	cur ;
    double      *	invert_transform ;
    double       	neighbors[16] ;
    double       	rsc[8],
					sumrs ;
    double       	x, y ;
    int     		px, py ;
    int     		pos ;
    int         	tabx, taby ;
    double      *	kernel ;
    double       	zoom ;
    int		      	leaps[16] ;

	if ((image_in==NULL) || (param==NULL)) return NULL ;

    invert_transform = invert_linear_transform(param) ;
    if (invert_transform == NULL) {
        e_error("cannot compute invert transform: aborting warping") ;
        return NULL ;
    }

    /* Generate default interpolation kernel */
    kernel = generate_interpolation_kernel(kernel_type) ;
    if (kernel == NULL) {
        e_error("cannot generate kernel: aborting resampling") ;
        return NULL ;
    }

    /* Compute new image size   */
    zoom = (double)sqrt(fabs((double)(param[0]*param[4] - param[1]*param[3])));
    lx_out = (int)(image_in->lx * zoom) ;
    ly_out = (int)(image_in->ly * zoom) ;

    image_out = image_new(lx_out, ly_out) ;

    /* Pre compute leaps for 16 closest neighbors positions */

    leaps[0] = -1 - image_in->lx ;
    leaps[1] =    - image_in->lx ;
    leaps[2] =  1 - image_in->lx ;
    leaps[3] =  2 - image_in->lx ;

    leaps[4] = -1 ;
    leaps[5] =  0 ;
    leaps[6] =  1 ;
    leaps[7] =  2 ;

    leaps[8] = -1 + image_in->lx ;
    leaps[9] =      image_in->lx ;
    leaps[10]=  1 + image_in->lx ;
    leaps[11]=  2 + image_in->lx ;

    leaps[12]= -1 + 2*image_in->lx ;
    leaps[13]=      2*image_in->lx ;
    leaps[14]=  1 + 2*image_in->lx ;
    leaps[15]=  2 + 2*image_in->lx ;

    /* Double loop on the output image  */
    for (j=0 ; j < ly_out ; j++) {
        for (i=0 ; i< lx_out ; i++) {
            /* Compute the original source for this pixel   */

            x = invert_transform[0] * (double)i +
				invert_transform[1] * (double)j +
				invert_transform[2] ; 

            y = invert_transform[3] * (double)i +
				invert_transform[4] * (double)j +
				invert_transform[5] ; 

            /* Which is the closest integer positioned neighbor?    */
            px = (int)x ;
			py = (int)y ;

            if ((px < 1) ||
                (px > (image_in->lx-3)) ||
                (py < 1) ||
                (py > (image_in->ly-3)))
                image_out->data[i+j*lx_out] = (pixelvalue)0.0 ;
            else {
                /* Now feed the positions for the closest 16 neighbors  */
                pos = px + py * image_in->lx ;
                for (k=0 ; k<16 ; k++)
                    neighbors[k] = 
					(double)(image_in->data[(int)(pos+leaps[k])]) ;

                /* Which tabulated value index shall we use?    */
                tabx = (int)((x - (double)px) * (double)(TABSPERPIX)) ; 
                taby = (int)((y - (double)py) * (double)(TABSPERPIX)) ; 

                /* Compute resampling coefficients  */
                /* rsc[0..3] in x, rsc[4..7] in y   */

                rsc[0] = kernel[TABSPERPIX + tabx] ;
                rsc[1] = kernel[tabx] ;
                rsc[2] = kernel[TABSPERPIX - tabx] ;
                rsc[3] = kernel[2 * TABSPERPIX - tabx] ;
                rsc[4] = kernel[TABSPERPIX + taby] ;
                rsc[5] = kernel[taby] ;
                rsc[6] = kernel[TABSPERPIX - taby] ;
                rsc[7] = kernel[2 * TABSPERPIX - taby] ;

                sumrs = (rsc[0]+rsc[1]+rsc[2]+rsc[3]) *
                        (rsc[4]+rsc[5]+rsc[6]+rsc[7]) ;

                /* Compute interpolated pixel now   */
                cur =   rsc[4] * (  rsc[0]*neighbors[0] +
                                    rsc[1]*neighbors[1] +
                                    rsc[2]*neighbors[2] +
                                    rsc[3]*neighbors[3] ) +
                        rsc[5] * (  rsc[0]*neighbors[4] +
                                    rsc[1]*neighbors[5] +
                                    rsc[2]*neighbors[6] +
                                    rsc[3]*neighbors[7] ) +
                        rsc[6] * (  rsc[0]*neighbors[8] +
                                    rsc[1]*neighbors[9] +
                                    rsc[2]*neighbors[10] +
                                    rsc[3]*neighbors[11] ) +
                        rsc[7] * (  rsc[0]*neighbors[12] +
                                    rsc[1]*neighbors[13] +
                                    rsc[2]*neighbors[14] +
                                    rsc[3]*neighbors[15] ) ; 

                /* Affect the value to the output image */
                image_out->data[i+j*lx_out] = (pixelvalue)(cur/sumrs) ;
                /* done ! */
            }       
        }
    }
    free(kernel) ;
    free(invert_transform) ;
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Warp an image according to a polynomial transformation.
  @param	image_in		Image to warp.
  @param	kernel_type		Interpolation kernel to use.
  @param	poly_u			Polynomial transform in U.
  @param	poly_v			Polynomial transform in V.
  @return	1 newly allocated image.

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
    	image_t    	*	image_in,
		char		*	kernel_type,
		poly2d		*	poly_u,
		poly2d		*	poly_v)
{
    image_t    *	image_out ;
    int         	i, j, k ;
    int         	lx_out, ly_out ;
    double       	cur ;
    double       	neighbors[16] ;
    double       	rsc[8],
					sumrs ;
    double       	x, y ;
    int     		px, py ;
    int     		pos ;
    int         	tabx, taby ;
    double      *	kernel ;
    int		      	leaps[16] ;

    if (image_in == NULL) return NULL ;

    /* Generate default interpolation kernel */
    kernel = generate_interpolation_kernel(kernel_type) ;
    if (kernel == NULL) {
        e_error("cannot generate kernel: aborting resampling") ;
        return NULL ;
    }

    /* Compute new image size   */
    lx_out = (int)image_in->lx ;
    ly_out = (int)image_in->ly ;

    image_out = image_new(lx_out, ly_out) ;

    /* Pre compute leaps for 16 closest neighbors positions */

    leaps[0] = -1 - image_in->lx ;
    leaps[1] =    - image_in->lx ;
    leaps[2] =  1 - image_in->lx ;
    leaps[3] =  2 - image_in->lx ;

    leaps[4] = -1 ;
    leaps[5] =  0 ;
    leaps[6] =  1 ;
    leaps[7] =  2 ;

    leaps[8] = -1 + image_in->lx ;
    leaps[9] =      image_in->lx ;
    leaps[10]=  1 + image_in->lx ;
    leaps[11]=  2 + image_in->lx ;

    leaps[12]= -1 + 2*image_in->lx ;
    leaps[13]=      2*image_in->lx ;
    leaps[14]=  1 + 2*image_in->lx ;
    leaps[15]=  2 + 2*image_in->lx ;

    /* Double loop on the output image  */
    for (j=0 ; j < ly_out ; j++) {
        for (i=0 ; i< lx_out ; i++) {
            /* Compute the original source for this pixel   */

			x = poly2d_compute(poly_u, (double)i, (double)j);
			y = poly2d_compute(poly_v, (double)i, (double)j);

            /* Which is the closest integer positioned neighbor?    */
            px = (int)x ;
			py = (int)y ;

            if ((px < 1) ||
                (px > (image_in->lx-3)) ||
                (py < 1) ||
                (py > (image_in->ly-3)))
                image_out->data[i+j*lx_out] = (pixelvalue)0.0 ;
            else {
                /* Now feed the positions for the closest 16 neighbors  */
                pos = px + py * image_in->lx ;
                for (k=0 ; k<16 ; k++)
                    neighbors[k] = 
					(double)(image_in->data[(int)(pos+leaps[k])]) ;

                /* Which tabulated value index shall we use?    */
                tabx = (int)((x - (double)px) * (double)(TABSPERPIX)) ;
                taby = (int)((y - (double)py) * (double)(TABSPERPIX)) ;

                /* Compute resampling coefficients  */
                /* rsc[0..3] in x, rsc[4..7] in y   */

                rsc[0] = kernel[TABSPERPIX + tabx] ;
                rsc[1] = kernel[tabx] ;
                rsc[2] = kernel[TABSPERPIX - tabx] ;
                rsc[3] = kernel[2 * TABSPERPIX - tabx] ;
                rsc[4] = kernel[TABSPERPIX + taby] ;
                rsc[5] = kernel[taby] ;
                rsc[6] = kernel[TABSPERPIX - taby] ;
                rsc[7] = kernel[2 * TABSPERPIX - taby] ;

                sumrs = (rsc[0]+rsc[1]+rsc[2]+rsc[3]) *
                        (rsc[4]+rsc[5]+rsc[6]+rsc[7]) ;

                /* Compute interpolated pixel now   */
                cur =   rsc[4] * (  rsc[0]*neighbors[0] +
                                    rsc[1]*neighbors[1] +
                                    rsc[2]*neighbors[2] +
                                    rsc[3]*neighbors[3] ) +
                        rsc[5] * (  rsc[0]*neighbors[4] +
                                    rsc[1]*neighbors[5] +
                                    rsc[2]*neighbors[6] +
                                    rsc[3]*neighbors[7] ) +
                        rsc[6] * (  rsc[0]*neighbors[8] +
                                    rsc[1]*neighbors[9] +
                                    rsc[2]*neighbors[10] +
                                    rsc[3]*neighbors[11] ) +
                        rsc[7] * (  rsc[0]*neighbors[12] +
                                    rsc[1]*neighbors[13] +
                                    rsc[2]*neighbors[14] +
                                    rsc[3]*neighbors[15] ) ; 

                /* Affect the value to the output image */
                image_out->data[i+j*lx_out] = (pixelvalue)(cur/sumrs) ;
                /* done ! */
            }       
        }
    }
    free(kernel) ;
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Invert a linear transformation.
  @param	trans	Transformation to invert.
  @return	1 newly allocated array of 6 doubles.

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
double * invert_linear_transform(double * trans)
{
    double   *	i_trans ;
    double   	det ;

	if (trans==NULL) return NULL ;
    det = (trans[0] * trans[4]) - (trans[1] * trans[3]) ;
    if (fabs(det) < 1e-6) {
        e_error("NULL determinant: cannot invert transform") ;
        return NULL ;
    }

    i_trans = calloc(6, sizeof(double)) ;

    i_trans[0] =  trans[4] / det ; 
    i_trans[1] = -trans[1] / det ;
    i_trans[2] = ((trans[1] * trans[5]) - (trans[2] * trans[4])) / det ;
    i_trans[3] = -trans[3] / det ;
    i_trans[4] =  trans[0] / det ;
    i_trans[5] = ((trans[2] * trans[3]) - (trans[0] * trans[5])) / det ;

    return i_trans ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Warp an image according to a linear transformation.
  @param	image_in		Image to warp.
  @param	param			Linear transformation definition.
  @param	kernel_type		Interpolation kernel to use.
  @return	1 newly allocated image.

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
    	image_t    	*	image_in,
    	double      *	param,
		char		*	kernel_type)
{
    image_t    *	image_out ;
    int         	i, j, k ;
    int         	lx_out, ly_out ;
    double       	cur ;
    double       *	i_trans ;
    double       	neighbors[16] ;
    double       	rsc[8], sumrs ;
    double       	x, y ;
    int     		px, py ;
    int     		pos ;
    int         	tabx, taby ;
    double       *	kernel ;
    double       	zoom ;
    int		      	leaps[16] ;

    if (image_in == NULL) return NULL ;
    if ((i_trans = invert_linear_transform(param)) == NULL) {
        e_error("cannot compute invert transform: aborting warping") ;
        return NULL ;
    }

    /* Generate Sinc kernel */
    if ((kernel = generate_interpolation_kernel(kernel_type))==NULL) {
        e_error("cannot generate kernel: aborting resampling") ;
		free(i_trans);
        return NULL ;
    }

    /* Compute new image size   */
    zoom = (double)sqrt((double)(param[0]*param[4] - param[1]*param[3])) ;
    lx_out = (int)(image_in->lx * zoom) ;
    ly_out = (int)(image_in->ly * zoom) ;

    image_out = image_new(lx_out, ly_out) ;

    /* Pre compute leaps for 16 closest neighbors positions */

    leaps[0] = -1 - image_in->lx ;
    leaps[1] =    - image_in->lx ;
    leaps[2] =  1 - image_in->lx ;
    leaps[3] =  2 - image_in->lx ;

    leaps[4] = -1 ;
    leaps[5] =  0 ;
    leaps[6] =  1 ;
    leaps[7] =  2 ;

    leaps[8] = -1 + image_in->lx ;
    leaps[9] =      image_in->lx ;
    leaps[10]=  1 + image_in->lx ;
    leaps[11]=  2 + image_in->lx ;

    leaps[12]= -1 + 2*image_in->lx ;
    leaps[13]=      2*image_in->lx ;
    leaps[14]=  1 + 2*image_in->lx ;
    leaps[15]=  2 + 2*image_in->lx ;

    /* Double loop on the output image  */
    for (j=0 ; j < ly_out ; j++) {
        for (i=0 ; i< lx_out ; i++) {
            /* Compute the original source for this pixel   */
            x = i_trans[0] * (double)i + i_trans[1] * (double)j + i_trans[2] ; 
            y = i_trans[3] * (double)i + i_trans[4] * (double)j + i_trans[5] ; 

            /* Which is the closest integer positioned neighbor?    */
            px = (int)x ; py = (int)y ;

            if ((px < 1) ||
                (px > (image_in->lx-3)) ||
                (py < 1) ||
                (py > (image_in->ly-3)))
                image_out->data[i+j*lx_out] = (pixelvalue)0.0 ;
            else {
                /* Now feed the positions for the closest 16 neighbors  */
                pos = px + py * image_in->lx ;
                for (k=0 ; k<16 ; k++)
                    neighbors[k] =
					(double)(image_in->data[(int)(pos+leaps[k])]) ;

                /* Which tabulated value index shall we use?    */
                tabx = (int)((x - (double)px) * (double)(TABSPERPIX)) ; 
                taby = (int)((y - (double)py) * (double)(TABSPERPIX)) ; 

                /* Compute resampling coefficients  */
                /* rsc[0..3] in x, rsc[4..7] in y   */

                rsc[0] = kernel[TABSPERPIX + tabx] ;
                rsc[1] = kernel[tabx] ;
                rsc[2] = kernel[TABSPERPIX - tabx] ;
                rsc[3] = kernel[2 * TABSPERPIX - tabx] ;
                rsc[4] = kernel[TABSPERPIX + taby] ;
                rsc[5] = kernel[taby] ;
                rsc[6] = kernel[TABSPERPIX - taby] ;
                rsc[7] = kernel[2 * TABSPERPIX - taby] ;

                sumrs = (rsc[0]+rsc[1]+rsc[2]+rsc[3]) *
                        (rsc[4]+rsc[5]+rsc[6]+rsc[7]) ;

                /* Compute interpolated pixel now   */
                cur =   rsc[4] * (  rsc[0]*neighbors[0] +
                                    rsc[1]*neighbors[1] +
                                    rsc[2]*neighbors[2] +
                                    rsc[3]*neighbors[3] ) +
                        rsc[5] * (  rsc[0]*neighbors[4] +
                                    rsc[1]*neighbors[5] +
                                    rsc[2]*neighbors[6] +
                                    rsc[3]*neighbors[7] ) +
                        rsc[6] * (  rsc[0]*neighbors[8] +
                                    rsc[1]*neighbors[9] +
                                    rsc[2]*neighbors[10] +
                                    rsc[3]*neighbors[11] ) +
                        rsc[7] * (  rsc[0]*neighbors[12] +
                                    rsc[1]*neighbors[13] +
                                    rsc[2]*neighbors[14] +
                                    rsc[3]*neighbors[15] ) ; 

                /* Affect the value to the output image */
                image_out->data[i+j*lx_out] = (pixelvalue)(cur/sumrs) ;
                /* done ! */
            }       
        }
    }

    free(kernel) ;
    free(i_trans) ;
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Generate a hyperbolic tangent kernel.
  @param	steep	Steepness of the hyperbolic tangent parts.
  @return	1 pointer to a newly allocated array of doubles.

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
#define hk_gen(x,s) (((tanh(s*(x+0.5))+1)/2)*((tanh(s*(-x+0.5))+1)/2))
double * generate_tanh_kernel(double steep)
{
    double  *   kernel ;
    double  *   x ;
    double      width ;
    double      inv_np ;
    double      ind ;
    int         i ;
    int         np ;
    int         samples ;

    width   = (double)TABSPERPIX / 2.0 ; 
    samples = KERNEL_SAMPLES ;
    np      = 32768 ; /* Hardcoded: should never be changed */
    inv_np  = 1.00 / (double)np ;

    /*
     * Generate the kernel expression in Fourier space
     * with a correct frequency ordering to allow standard FT
     */
    x = malloc((2*np+1)*sizeof(double)) ;
    for (i=0 ; i<np/2 ; i++) {
        ind      = (double)i * 2.0 * width * inv_np ;
        x[2*i]   = hk_gen(ind, steep) ;
        x[2*i+1] = 0.00 ;
    }
    for (i=np/2 ; i<np ; i++) {
        ind      = (double)(i-np) * 2.0 * width * inv_np ;
        x[2*i]   = hk_gen(ind, steep) ;
        x[2*i+1] = 0.00 ;
    }

    /* 
     * Reverse Fourier to come back to image space
     */
    reverse_tanh_kernel(x, np) ;

    /*
     * Allocate and fill in returned array
     */
    kernel = malloc(samples * sizeof(double)) ;
    for (i=0 ; i<samples ; i++) {
        kernel[i] = 2.0 * width * x[2*i] * inv_np ;
    }
    free(x) ;
    return kernel ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Bring a hyperbolic tangent kernel from Fourier to normal space.
  @param	data	Kernel samples in Fourier space.
  @param	nn		Number of samples in the input kernel.
  @return	void

  Bring back a hyperbolic tangent kernel from Fourier to normal space. Do
  not try to understand the implementation and DO NOT MODIFY THIS FUNCTION.
 */
/*--------------------------------------------------------------------------*/
#define KERNEL_SW(a,b) tempr=(a);(a)=(b);(b)=tempr
static void reverse_tanh_kernel(double * data, int nn)
{
    unsigned long   n,
					mmax,
					m,
					i, j,
					istep ;
    double  wtemp,
            wr,
            wpr,
            wpi,
            wi,
            theta;
    double  tempr,
            tempi;

    n = (unsigned long)nn << 1;
    j = 1;
    for (i=1 ; i<n ; i+=2) {
        if (j > i) {
            KERNEL_SW(data[j-1],data[i-1]);
            KERNEL_SW(data[j],data[i]);
        }
        m = n >> 1;
        while (m>=2 && j>m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax = 2;
    while (n > mmax) {
        istep = mmax << 1;
        theta = 2 * PI_NUMB / mmax;
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr  = 1.0;
        wi  = 0.0;
        for (m=1 ; m<mmax ; m+=2) {
            for (i=m ; i<=n ; i+=istep) {
                j = i + mmax;
                tempr = wr * data[j-1] - wi * data[j];
                tempi = wr * data[j]   + wi * data[j-1];
                data[j-1] = data[i-1] - tempr;
                data[j]   = data[i]   - tempi;
                data[i-1] += tempr;
                data[i]   += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}
#undef KERNEL_SW


/*-------------------------------------------------------------------------*/
/**
  @brief	Print out an interpolation kernel values on stdout.
  @param	kernel_name		Name of the kernel to print out.
  @return	void

  Takes in input a kernel name, generates the corresponding kernel and
  prints it out on stdout, then discards the generated kernel.

  For debugging purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void show_interpolation_kernel(char * kernel_name)
{
	double	*	ker ;
	int			i ;
	double		x ;


	ker = generate_interpolation_kernel(kernel_name) ;
	if (ker == NULL)
		return ;

	(void)fprintf(stdout, "# Kernel is %s\n", kernel_name) ;
	x = 0.00 ;
	for (i=0 ; i<KERNEL_SAMPLES ; i++) {
		(void)fprintf(stdout, "%g %g\n", x, ker[i]) ;
		x += 1.00 / (double)TABSPERPIX ;
	}
	free(ker) ;
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Subsample an image by a factor 2.
  @param	in		Image to subsample
  @return	1 newly allocated image

  Subsamples an image by a factor 2, i.e. 4 pixels become 1 pixel.
  A triangular filter (1 2 1) is applied to smooth high
  frequencies.
  The returned image must be freed using image_del(). 
 */
/*--------------------------------------------------------------------------*/
image_t * image_subsample(image_t * in)
{
	int				i, j ;
	pixelvalue 	* 	line_i,
				*	line_o ;
	image_t		*	pass1,
				*	pass2 ;
	double			p0, p1, p2 ;	
	double			out ;
	int				contrib ;
			
	if (in==NULL) return NULL ;

	/* First pass: apply a 1 2 1 filter in X */
	pass1 = image_new(in->lx/2, in->ly) ;
	for (j=0 ; j<pass1->ly ; j++) {
		line_i = in->data + j*in->ly ;
		line_o = pass1->data + j*pass1->lx ;
		for (i=0 ; i<pass1->lx ; i++) {
			p0 = 0 ;
			p1 = (double)line_i[2*i];
			p2 = 0 ;
			contrib=2 ;
			if ((2*i-1)>=0) {
				p0 = (double)line_i[2*i-1] ;
				contrib++ ;
			}
			if ((2*i+1)<in->lx) {
				p2 = (double)line_i[2*i+1] ;
				contrib++;
			}
			out = (p0 + 2.0*p1 + p2)/(double)contrib ;
			line_o[i] = (pixelvalue)out ;
		}
	}

	/* Second pass: apply a 1 2 1 filter in Y */
	pass2 = image_new(in->lx/2, in->ly/2);
	for (i=0 ; i<pass2->lx ; i++) {
		for (j=0 ; j<pass2->ly ; j++) {
			p0 = 0 ;
			p1 = (double)pass1->data[i+2*j*pass1->lx];
			p2 = 0 ;
			contrib=2 ;
			if ((2*j-1)>=0) {
				p0 = (double)pass1->data[i+(2*j-1)*pass1->lx];
				contrib ++ ;
			}
			if ((2*j+1)<pass1->ly) {
				p2 = (double)pass1->data[i+(2*j+1)*pass1->lx];
				contrib ++ ;
			}
			out = (p0 + 2.0*p1 + p2)/(double)contrib ;
			pass2->data[i+j*pass2->lx] = (pixelvalue)out ;
		}
	}
	image_del(pass1);
	return pass2 ;
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Subsample an image by a factor 4.
  @param	in		Image to subsample
  @return	1 newly allocated image

  Subsamples an image by a factor 4, i.e. 16 pixels become 1 pixel.
  A triangular filter (1 2 1) is applied twice to smooth high
  frequencies.
  The returned image must be freed using image_del(). 
 */
/*--------------------------------------------------------------------------*/
image_t * image_subsample4(image_t * in)
{
    image_t *   sub2 ;
    image_t *   sub4 ;

    sub4 = NULL ;
    sub2 = image_subsample(in);
    if (sub2!=NULL) {
        sub4 = image_subsample(sub2);
        image_del(sub2);
    }
    return sub4 ;
}
