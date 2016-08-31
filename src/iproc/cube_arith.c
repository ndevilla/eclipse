/*----------------------------------------------------------------------------*/
/**
   @file	cube_arith.c
   @author	Nicolas Devillard
   @date	Aug 02, 1995
   @version	$Revision: 1.31 $
   @brief	cube arithmetic routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: cube_arith.c,v 1.31 2005/01/26 12:13:08 yjung Exp $
	$Author: yjung $
	$Date: 2005/01/26 12:13:08 $
	$Revision: 1.31 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include "cube_arith.h"
#include "image_stats.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define FLAT_LOW_THRESH     0.5
#define FLAT_HIGH_THRESH    2.0

/*-----------------------------------------------------------------------------
   								Macros
 -----------------------------------------------------------------------------*/

#ifndef min
#define min(a,b) (((a)<=(b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a)>=(b)) ? (a) : (b))
#endif

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    4 arithmetic operations between two cubes.
  @param    cube1       First argument.
  @param    cube2       Second argument.
  @param    operation   Arithmetic operation to perform.
  @return   int 0 if Ok, -1 otherwise.
 
  The first argument is modified to contain the results of the
  operation. Supported operations are '+', '-', '*', '/'.
 
  \begin{verbatim}
  cube_op(&c1, c2, '+') is equivalent to c1+=c2
  \end{verbatim}
 */
/*----------------------------------------------------------------------------*/
int cube_op(
		cube_t	**	cube1, 
		cube_t 	* 	cube2, 
		int 		operation) 
{
	int			status ;
	if (*cube1==NULL || cube2==NULL) return -1 ;

	e_comment(1, "performing cube arithmetic") ;
	switch(operation) {
		case '+':
		status = cube_add(*cube1, cube2) ;
		break ;

		case '-':
		status = cube_sub(*cube1, cube2) ;
		break ;

		case '*':
		status = cube_mul(*cube1, cube2) ;
		break ;

		case '/':
		status = cube_div(*cube1, cube2) ;
		break ;

		default:
		e_error("illegal requested operation: aborting cube arithmetic") ;
		status = -1 ;
	}

	return status ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Arithmetic operations between a cube and a constant.
  @param    cube1       First operand.
  @param    constant    Second operand.
  @param    operation   Operator.
  @return   int 0 if Ok, -1 otherwise.
 
  Performs an arithmetic operation on all pixels on the input cube.
  Possible operators are:
 
  \begin{tabular}{lll}
  Operator          & Symbol    &   Operation \\
  \\
  Addition          & +         & out = in + c \\
  Subtraction       & -         & out = in - c \\
  Multiplication    & *         & out = in * c \\
  Division          & /         & out = in * (1/c) \\
  Logarithm         & l         & out = log(in)/log(c) \\
  Power             & ^         & out = in ^ c \\
  Exponentiation    & e         & out = c ^ in
  \end{tabular}
 
  Notice that the division is already optimized by multiplying all
  pixels by the reciprocal of the given constant. No need to do it
  yourself, i.e.
 
  \begin{verbatim}
  cube_cst_op(cube1, cst, '/');
  is just as efficient as:
  cube_cst_op(cube1, 1.0/cst, '*');
  \end{verbatim}
 
  Every pixel in the input cube is modified.
 */
/*----------------------------------------------------------------------------*/
int cube_cst_op(
		cube_t	* 	cube1, 
		double 		constant, 
		int 		operation)
{
	int	p ;

	if (cube1==NULL) return -1 ;
    if ((fabs(constant)< 1e-16) && (operation == '/')) {
        e_error("division by zero requested in cube/constant operation\n") ;
        return -1 ;
    }
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("operating arithmetic on planes", p, cube1->np, 2) ;
		image_cst_op_local(cube1->plane[p], constant, operation) ;
	}
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Normalize all pixels in a cube.
  @param    cube1   Input cube to normalize.
  @param    mode    Normalization mode.
  @return   int 0 if Ok, -1 if error occurred.
 
  Normalize all planes of a cube. See normalization mode definitions
  in image_arith.h.
 
  The input cube is modified.
 */
/*----------------------------------------------------------------------------*/
int cube_normalize(
		cube_t	* 	cube1, 
		int 		mode) 
{
	image_t	*	normalized ;
	int				p ;

	if (cube1==NULL) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("normalizing planes", p, cube1->np, 2) ;
        normalized = image_normalize(cube1->plane[p], mode) ;
		if (normalized==NULL) {
			e_error("normalizing plane %d in cube", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p] = normalized ;
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Scale all images to a given value or to the flux in the
            first plane.
  @param    cube1       Cube to scale.
  @param    to_flux      Flux to scale to.
  @return   int 0 if Ok, -1 otherwise.
 
  All images in the input cube are modified so that their fluxes
  corresponds to the given value. The flux of an image is the sum of
  all its pixel values.
 
  If the input value for 'to_flux' is zero, all plane fluxes will be
  scaled (except the first one) to be equal to the flux in the first
  plane.
 
  The input cube is modified.
 */
/*----------------------------------------------------------------------------*/
int cube_scale_flux(
		cube_t	*	cube1, 
		double 		to_flux)
{
	image_t *	scaled ;
	int			p ;
	int			start_plane ;
	double		scaling ;
	double		local_flux ;

	if (cube1==NULL) return -1 ;
	
	/* Find out if we scale everything according to the first */
	/* plane's flux, or if we scale according to the to_flux parameter */

	if (to_flux < 1e-20) {
		to_flux = (double)image_getsumpix(cube1->plane[0]) ; 
		if (to_flux <= 1e-10) {
			e_error("first plane has wrong flux (%g): aborting", to_flux) ;
			return -1 ;
		}
		start_plane = 1 ;
	} else {
		start_plane = 0 ;
	}

	for (p=start_plane ; p<cube1->np ; p++) {
		compute_status("flux scaling", p, cube1->np, 2) ;
		local_flux = (double)image_getsumpix(cube1->plane[p]) ;
		if (local_flux < 1e-20) {
			e_error("wrong flux (%g) in plane %d: cannot scale",local_flux,p+1);
			return -1 ;
		}	
		/* scale flux in current plane	*/
		scaling = to_flux / local_flux ;
        scaled = image_cst_op(cube1->plane[p], scaling, '*');
		if (scaled==NULL) {
			e_error("scaling flux in plane %d", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p] = scaled ;
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Threshold all pixel values to an interval.
  @param    cube1           Cube to threshold.
  @param    lo_cut          Lower threshold value.
  @param    hi_cut          Higher threshold value.
  @param    assign_lo_cut   Value to assign to pixels lower than lo_cut.
  @param    assign_hi_cut   Value to assign to pixels higher than hi_cut.
  @return   int 0 if Ok, -1 otherwise.
 
  Threshold all pixel values in cube frames to a given interval. If
  you do not want to threshold with a lower bound, specify
  MIN_PIX_VALUE for lower threshold. If you do not want to threshold
  with a higher bound, specify MAX_PIX_VALUE for higher threshold.
 
  Values lower than lo_cut will be assigned the value 'assign_lo_cut'.
  Values higher than hi_cut will be assigned the value
  'assign_hi_cut'.
 
  The input cube is modified.
 */
/*----------------------------------------------------------------------------*/
int cube_threshold(
		cube_t		*	cube1,
		pixelvalue		lo_cut,
		pixelvalue		hi_cut,
		pixelvalue		assign_lo_cut,
		pixelvalue		assign_hi_cut)
{
    image_t  *	thresholded ;
    int         p ;

    /* Error handling : test entries    */
	if (cube1==NULL) return -1 ;
	if (lo_cut>=hi_cut) {
		e_error("in cuts: low is %g, high is %g: cannot threshold",
				lo_cut, hi_cut) ;
		return -1 ;
	}
    for (p=0 ; p<cube1->np ; p++) {
        compute_status("Thresholding planes", p, cube1->np, 2) ;
		thresholded = image_threshold(cube1->plane[p], 
									  lo_cut,
									  hi_cut,
									  assign_lo_cut,
									  assign_hi_cut) ;
		if (thresholded == NULL) {
			e_error("thresholding plane %d in cube: aborting", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p] = thresholded ;
    }
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Subtract one cube from another
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*----------------------------------------------------------------------------*/
int cube_sub(
		cube_t	*	c1, 
		cube_t	* 	c2)
{
	int	i, np, nbpix ;

	if ((c1->lx != c2->lx) ||
		(c1->ly != c2->ly)) {
		e_error("incompatible size: cannot subtract") ;
		return -1 ;
	}

	if ((c2->np != c1->np) &&
		(c2->np != 1)) {
		e_error("cannot compute with these number of planes") ;
		return -1 ;
	}
		
	nbpix = c1->plane[0]->lx * c1->plane[0]->ly ;
	if (c1->np == c2->np) {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				c1->plane[np]->data[i] -= c2->plane[np]->data[i] ;
			}
		}
	} else {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				c1->plane[np]->data[i] -= c2->plane[0]->data[i] ;
			}
		}
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Add two cubes.
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*----------------------------------------------------------------------------*/
int cube_add(
		cube_t	*	c1, 
		cube_t 	* 	c2)
{
	int	i, np, nbpix ;

	if ((c1->lx != c2->lx) ||
		(c1->ly != c2->ly)) {
		e_error("incompatible size: cannot add") ;
		return -1 ;
	}

	if ((c2->np != c1->np) &&
		(c2->np != 1)) {
		e_error("cannot compute with these number of planes") ;
		return -1 ;
	}

	nbpix = c1->plane[0]->lx * c1->plane[0]->ly ;
	if (c1->np == c2->np) {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				c1->plane[np]->data[i] += c2->plane[np]->data[i] ;
			}
		}
	} else {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				c1->plane[np]->data[i] += c2->plane[0]->data[i] ;
			}
		}
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Multiplies two cubes.
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*----------------------------------------------------------------------------*/
int cube_mul(
		cube_t	* 	c1, 
		cube_t 	* 	c2)
{
	int i, np, nbpix ;

	if ((c1->lx != c2->lx) ||
		(c1->ly != c2->ly)) {
		e_error("incompatible size: cannot multiply") ;
		return -1 ;
	}

	if ((c2->np != c1->np) &&
		(c2->np != 1)) {
		e_error("cannot compute with these number of planes") ;
		return -1 ;
	}
		
	nbpix = c1->plane[0]->lx * c1->plane[0]->ly ;
	if (c1->np == c2->np) {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				c1->plane[np]->data[i] *= c2->plane[np]->data[i] ;
			}
		}
	} else {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				c1->plane[np]->data[i] *= c2->plane[0]->data[i] ;
			}
		}
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Divide a cube by another.
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*----------------------------------------------------------------------------*/
int cube_div(
		cube_t	* 	c1, 
		cube_t 	* 	c2)
{
	int i, np, nbpix ;

	if ((c1->lx != c2->lx) ||
		(c1->ly != c2->ly)) {
		e_error("incompatible size: cannot divide") ;
		return -1 ;
	}

	if ((c2->np != c1->np) &&
		(c2->np != 1)) {
		e_error("cannot compute with these number of planes") ;
		return -1 ;
	}
		
	nbpix = c1->plane[0]->lx * c1->plane[0]->ly ;
	if (c1->np == c2->np) {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				if (fabs((double)c2->plane[np]->data[i]) < 1e-10)
					c1->plane[np]->data[i] = 0.0 ;
				else
					c1->plane[np]->data[i] /= c2->plane[np]->data[i] ;
			}
		}
	} else {
		for (np=0 ; np < c1->np ; np++) {
			for (i=0 ; i< nbpix ; i++) {
				if (fabs((double)c2->plane[0]->data[i]) < 1e-10)
					c1->plane[np]->data[i] = 0.0 ;
				else
					c1->plane[np]->data[i] /= c2->plane[0]->data[i] ;
			}
		}
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Add an image to all planes in a cube.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*----------------------------------------------------------------------------*/
int cube_add_im(
		cube_t	* 	cu, 
		image_t * 	im)
{
	int i ;

	if (cu==NULL || im==NULL) return -1 ;
	if ((cu->lx != im->lx) || (cu->ly != im->ly)) {
		e_error("incompatible size: cannot add image to cube");
		return -1 ;
	}
	for (i=0 ; i<cu->np ; i++) {
		image_add_local(cu->plane[i], im);
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Subtract an image from all planes in a cube.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*----------------------------------------------------------------------------*/
int cube_sub_im(
		cube_t	* 	cu, 
		image_t * 	im)
{
	int	i ;

	if (cu==NULL || im==NULL) return -1 ;
	if ((cu->lx != im->lx) || (cu->ly != im->ly)) {
		e_error("incompatible size: cannot subtract image from cube");
		return -1 ;
	}
	for (i=0 ; i<cu->np ; i++) {
		image_sub_local(cu->plane[i], im);
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Multiply all planes in a cube by an image.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*----------------------------------------------------------------------------*/
int cube_mul_im(
		cube_t	*	cu, 
		image_t * 	im)
{
	int	i ;

	if (cu==NULL || im==NULL) return -1 ;
	if ((cu->lx != im->lx) || (cu->ly != im->ly)) {
		e_error("incompatible size: cannot multiply image by cube");
		return -1 ;
	}
	for (i=0 ; i<cu->np ; i++) {
		image_mul_local(cu->plane[i], im);
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Divide all planes in a cube by an image.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*----------------------------------------------------------------------------*/
int cube_div_im(
		cube_t	* 	cu, 
		image_t * 	im)
{
	int	i ;

	if (cu==NULL || im==NULL) return -1 ;
	if ((cu->lx != im->lx) || (cu->ly != im->ly)) {
		e_error("incompatible size: cannot subtract image from cube");
		return -1 ;
	}
	for (i=0 ; i<cu->np ; i++) {
		image_div_local(cu->plane[i], im);
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Subtract and Divide all planes in a cube by two images.
  @param    cu      Cube to modify.
  @param    im1     Image to subtract
  @param    im2     Image to divide 
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*----------------------------------------------------------------------------*/
int cube_subdiv_im(
		cube_t	* 	cu, 
		image_t * 	im1,
		image_t	*	im2)
{
	int	i ;

	if (cu==NULL || im1==NULL || im2==NULL) return -1 ;
	if ((cu->lx != im1->lx) || (cu->ly != im1->ly) || 
			(cu->lx != im2->lx) || (cu->ly != im2->ly)) {
		e_error("incompatible size between cube and images");
		return -1 ;
	}
	for (i=0 ; i<cu->np ; i++) {
		image_subdiv_local(cu->plane[i], im1, im2);
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Computes a standard deviation image from a cube.
  @param    cube1   Input cube.
  @return   1 newly allocated image.
 
  This function takes the same pixel position in all frames of the
  input cube, computes the standard deviation over time for this
  pixel, and output it into an image (at the same position).
 
  The returned image is thus an image of standard deviations in the
  cube, it does not share the input cube pixel value units.
 */
/*----------------------------------------------------------------------------*/
image_t * cube_stdev_z(cube_t * cube1) 
{     
	image_t	*	result ;
	image_t	*	abs_result ;
	image_t	*	in_plane ;
	image_t	*	m_img ;
	int			i ;
	int			n_plane ;
	double	*	sq_sum ;
	double		diff_pix ;
	double		invsurface ;

	if (cube1==NULL) return NULL ;
	result = image_new(cube1->lx, cube1->ly) ;

	sq_sum = malloc(result->lx * result->ly * sizeof(double));
	m_img = cube_avg_linear(cube1) ;
	if (m_img == NULL) {
		e_error("in averaging : aborting time stdev on cube") ;
		image_del(result) ;
		free(sq_sum) ;
		return NULL ;
	}

	invsurface = (double)1.0 / (double)cube1->np ;
	for (i=0 ; i<(result->lx * result->ly) ; i++) {
		sq_sum[i] = (double)0.0 ;
	}

	e_comment(1, "extracting standard deviation on cube") ;
	for (n_plane=0 ; n_plane<cube1->np ; n_plane++) {
		compute_status("standard deviation", n_plane, cube1->np,2) ; 
		in_plane = cube1->plane[n_plane] ;
		for (i=0 ; i<(in_plane->lx * in_plane->ly) ; i++) {
			diff_pix = (double)in_plane->data[i] - (double)m_img->data[i] ; 
			sq_sum[i] += diff_pix * diff_pix ;
		}
	}
	image_del(m_img) ;
	for (i=0 ; i<(result->lx * result->ly) ; i++) {
		result->data[i] = (pixelvalue)(sq_sum[i] * invsurface) ;
	}
	free(sq_sum) ;
	abs_result = image_abs(result) ;
	image_del(result) ;
	return abs_result ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Transform a cube in its reciprocal.
  @param	c1		Cube to transform.
  @return	int 0 if Ok, -1 otherwise.

  Applies for each pixel p in the cube p=1/p. If p is zero, leave it as
  zero.
 */
/*----------------------------------------------------------------------------*/
int cube_recip(cube_t * c1)
{
	image_t	*	recip ;
	int			p ;

	if (c1==NULL) return -1 ;
	for (p=0 ; p<c1->np ; p++) {
		recip = image_recip(c1->plane[p]);
		if (recip==NULL) {
			e_error("in cube reciprocal: aborting");
			return -1 ;
		}
		image_del(c1->plane[p]);
		c1->plane[p] = recip ;
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Transform a cube in its negation.
  @param	c1		Cube to transform.
  @return	int 0 if Ok, -1 otherwise.

  Applies for each pixel p in the cube p=-p.
 */
/*----------------------------------------------------------------------------*/
int cube_invert(cube_t * c1)
{
	int p ;

	if (c1==NULL) return -1 ;
	for (p=0 ; p<c1->np ; p++) {
		image_invert(c1->plane[p]);
	}
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    apply dark subtraction and ff division, replace bad pixels
  @param    in  pointer to allocated cube
  @param    ff      flat field
  @param    dark    dark 
  @return   badpix  bad pixel 
 */
/*----------------------------------------------------------------------------*/
int cube_correct_ff_dark_badpix(
    cube_t      *   in,
    image_t     *   ff,
    image_t     *   dark,
    pixelmap    *   badpix)
{
    if (in == NULL) return -1 ;
    if ((dark == NULL) && (ff == NULL) && (badpix == NULL)) return -1 ;

    if (dark != NULL) {
        if ((dark->lx != in->lx) || (dark->ly != in->ly))
            return -1 ;
    }
    if (ff != NULL) {
        if ((ff->lx != in->lx) || (ff->ly != in->ly))
            return -1 ;
    }
   
    /* only dark was provided */
    if (dark && !ff) {
        e_comment(1, "applying dark subtraction") ;
        cube_sub_im(in, dark) ;
        e_comment(1, "no flat-field provided: skipping") ;
    }

    /* only flat-field was provided */
    if (!dark && ff) {
        e_comment(1, "no dark provided: skipped") ;
        e_comment(1, "applying flat-field division") ;
        cube_div_im(in, ff) ;
    }

    /* both dark and flat-field have been provided */
    if (dark && ff) {
        e_comment(1, "applying dark subtraction and flat-field division");
        cube_subdiv_im(in, dark, ff);
    }

    /* Apply now bad pixel correction if needed */
    if (badpix != NULL) {
        e_comment(1, "applying dead pixel correction") ;
        cube_clean_deadpix(in, badpix) ;
    }
    return 0 ;
}

