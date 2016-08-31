/*-------------------------------------------------------------------------*/
/**
   @file	image_arith.c
   @author	Nicolas Devillard
   @date	Aug 11, 1995
   @version	$Revision: 1.32 $
   @brief	basic arithmetic functions over images
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: image_arith.c,v 1.32 2003/10/30 12:44:01 llundin Exp $
	$Author: llundin $
	$Date: 2003/10/30 12:44:01 $
	$Revision: 1.32 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "image_arith.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Add two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_add(
		image_t	*	image1,
		image_t	*	image2) 
{
	
	image_t	*	image_out ;
	int				i ;
	register
	pixelvalue	*	p1,
				*	p2,
				*	outp ;

	if (image1==NULL || image2==NULL) return NULL ;

	/* Input data images shall have the same sizes */
	if (	(image1->lx != image2->lx) ||
			(image1->ly != image2->ly)	) {
		e_error("cannot add images of different sizes\n") ;
		return NULL ;
	}

	image_out = image_new(image1->lx, image1->ly) ;

	/* Add two images */
	p1 = image1->data ;
	p2 = image2->data ;
	outp = image_out->data ;

	for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
		*outp++ = *p1++ + *p2++ ;

	return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_sub(
		image_t *image1,
		image_t *image2)
{
    image_t    *image_out ;
    int			i;
	register
	pixelvalue	*p1, *p2, *outp ;

	if (image1==NULL || image2==NULL) return NULL ;

    /* Input data images shall have the same sizes */
    if (    (image1->lx != image2->lx) ||
            (image1->ly != image2->ly)    ) {
		e_error("cannot subtract images of different sizes\n") ;
		return NULL ;
    }
 
    /* Allocations  */
    image_out = image_new(image1->lx, image1->ly) ;
 
    /* Subtract two images */
	p1 = image1->data ;
	p2 = image2->data ;
	outp = image_out->data ;

    for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
		*outp++ = *p1++ - *p2++ ;
 
    return image_out ;
}
  

/*-------------------------------------------------------------------------*/
/**
  @brief    Multiply two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_mul(
		image_t *image1,
		image_t *image2)
{
    image_t    *	image_out ;
    int		   		i	;
	register
	pixelvalue	*	p1, 
				*	p2, 
				*	outp ;

	if (image1==NULL || image2==NULL) return NULL ;

    /* Input data images shall have the same sizes */
    if (    (image1->lx != image2->lx) ||
            (image1->ly != image2->ly)    ) {
		e_error("cannot multiply images of different sizes\n") ;
        return NULL ;
    }
 
    /* Allocations  */
    image_out = image_new(image1->lx, image1->ly) ;
 
    /* Multiply two images */
	p1 = image1->data ;
	p2 = image2->data ;
	outp = image_out->data ;
 
    for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
		*outp++ = *p1++ * *p2++ ;
 
    return image_out ;
}
 

/*-------------------------------------------------------------------------*/
/**
  @brief    Divide two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_div(
		image_t *image1,
		image_t *image2)
{
    image_t *  image_out ;
    int		   	i;
	register
	pixelvalue	*p1, *p2, *outp ;
 
	if (image1==NULL || image2==NULL) return NULL ;

    /* Input data images shall have the same sizes */
    if (    (image1->lx != image2->lx) ||
            (image1->ly != image2->ly)    ) {
        e_error("cannot divide images of different size\n") ;
        return NULL ;
    }

    /* Allocations  */
    image_out = image_new(image1->lx, image1->ly) ;

    /* Divide two images */
	p1 = image1->data ;
	p2 = image2->data ;
	outp = image_out->data ;
     
    for (i=0 ; i<(image_out->lx * image_out->ly) ; i++) {

		if (fabs((double)*p2) < (double)1e-10) {
			*outp = MAX_PIX_VALUE ;
			outp++ ;
			p1++ ;
			p2++ ;
		} else {
			*outp++ = *p1++ / *p2++ ;
		}
	}
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Add two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_add_local(
		image_t * im1,
		image_t * im2)
{
	register pixelvalue * p1,
						* p2 ;
	register int		  i ;					

	p1 = im1->data ;
	p2 = im2->data ;
	for (i=0 ; i<(im1->lx * im1->ly) ; i++) {
		*p1++ += *p2++ ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_sub_local(
		image_t * im1,
		image_t * im2)
{
	register pixelvalue * p1,
						* p2 ;
	register int		  i ;					

	p1 = im1->data ;
	p2 = im2->data ;
	for (i=0 ; i<(im1->lx * im1->ly) ; i++) {
		*p1++ -= *p2++ ;
	}
	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Multiply two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_mul_local(
		image_t * im1,
		image_t * im2)
{
	register pixelvalue * p1,
						* p2 ;
	register int		  i ;					

	p1 = im1->data ;
	p2 = im2->data ;
	for (i=0 ; i<(im1->lx * im1->ly) ; i++) {
		*p1++ *= *p2++ ;
	}
	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Divide two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_div_local(
		image_t * im1,
		image_t * im2)
{
	register pixelvalue * p1,
						* p2 ;
	register int		  i ;					

	p1 = im1->data ;
	p2 = im2->data ;
	for (i=0 ; i<(im1->lx * im1->ly) ; i++) {
		if (fabs(*p2)>(double)1e-30) {
			*p1 /= *p2 ;
		} else {
			*p1 = (pixelvalue)0.0 ;
		}
		p1++ ; p2++ ;
	}
	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief   	Subtract and divide an image, store the result in the first im. 
  @param    im1     first image
  @param    im2     image subtracted
  @param    im3     image divided
  @return   int 0 if Ok, -1 otherwise.
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_subdiv_local(
		image_t * im1,
		image_t * im2,
		image_t * im3)
{
	register pixelvalue * p1,
						* p2,
						* p3 ;
	register int		  i ;					

	p1 = im1->data ;
	p2 = im2->data ;
	p3 = im3->data ;
	for (i=0 ; i<(im1->lx * im1->ly) ; i++) {
		*p1 -= *p2 ;
		if (fabs(*p3)>(double)1e-30) {
			*p1 /= *p3 ;
		} else {
			*p1 = (pixelvalue)0.0 ;
		}
		p1++ ; p2++ ; p3++ ;
	}
	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Divide an image by an intimage, store the result in the first 
  			image.
  @param    im1     first operand.
  @param    im2     second operand (int image).
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_div_intimage_local(
		image_t		* im1,
		intimage 	* im2)
{
	register pixelvalue * p1 ;
	register intpix		* p2 ;
	register int		  i ;					

	p1 = im1->data ;
	p2 = im2->data ;
	for (i=0 ; i<(im1->lx * im1->ly) ; i++) {
		if ((*p2) != 0) {
			*p1 /= (pixelvalue)*p2 ;
		} else {
			*p1 = (pixelvalue)0.0 ;
		}
		p1++ ; p2++ ;
	}
	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract im2 from im1, and multiply the result by a constant.
  @param    im1     first operand.
  @param    im2     second operand.
  @param    fact    multiplicative factor.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_submul_local(
		image_t		*	im1,
		image_t		*	im2,
		pixelvalue 		fact) 
{
	pixelvalue 	*p1, *p2 ;
	int			i ;
	int 		err=0;

	if (im1==NULL || im2==NULL) return -1 ;

	if ((im1->lx != im2->lx) || (im1->ly != im2->ly)) {
		e_error("sub_and_mult_img_local : inconsistent image sizes");
		return -1;
	}
	p1 = im1->data ;
	p2 = im2->data ;
	i=(im1->lx * im1->ly); 
	while (i--)
		p1[i] = (p1[i] - p2[i]) * fact ; 
	return err ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Add im2 (1D signal) to each row or column of im1.
  @param	im1		image to modify
  @param	im2		1D signal to add
  @return	int 0 if ok -1 otherwise.
  	
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_add_1d_local(
		image_t	*	im1,
		image_t	*	im2)
{
	int			orientation ;
	
	int			i,
				j ;
	
	/* Test input data */
	if ((im1 == NULL) || (im2 == NULL)) {
		return -1 ;
	}
	
	/* Test if one and only one size of im2 is equal to 1 */
	if (im2->lx == 1) {
		if (im2->ly == 1) {
			e_error("1D signal contains only one value - abort") ;
			return -1 ;
		} else {
			orientation = 0 ;
		}
	} else if (im2->ly != 1) {
		e_error("The second parameter is not a 1D signal - abort") ;
		return -1 ;
	} else {
		orientation = 1 ;
	}

	if (orientation==0) {
		/* 1D signal is vertical */
		for (j=0 ; j<im1->ly ; j++)
            for (i=0 ; i<im1->lx ; i++)
                im1->data[j*im1->lx+i] += im2->data[j] ;
	} else {
		/* 1D signal is horizontal */
		for (j=0 ; j<im1->ly ; j++)
			for (i=0 ; i<im1->lx ; i++)
				im1->data[j*im1->lx+i] += im2->data[i] ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Subtract im2 (1D signal) from each row or column of im1.
  @param	im1		image to modify
  @param	im2		1D signal to subtract
  @return	int 0 if ok -1 otherwise.
  	
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_sub_1d_local(
		image_t	*	im1,
		image_t	*	im2)
{
	int			orientation ;
	
	int			i,
				j ;
	
	/* Test input data */
	if ((im1 == NULL) || (im2 == NULL)) {
		return -1 ;
	}
	
	/* Test if one and only one size of im2 is equal to 1 */
	if (im2->lx == 1) {
		if (im2->ly == 1) {
			e_error("1D signal contains only one value - abort") ;
			return -1 ;
		} else orientation = 0 ;
	} else if (im2->ly != 1) {
		e_error("The second parameter is not a 1D signal - abort") ;
		return -1 ;
	} else orientation = 1 ;

	if (orientation==0) {
		/* 1D signal is vertical */
		for (j=0 ; j<im1->ly ; j++)
            for (i=0 ; i<im1->lx ; i++)
                im1->data[j*im1->lx+i] -= im2->data[j] ;
	} else {
		/* 1D signal is horizontal */
		for (j=0 ; j<im1->ly ; j++)
			for (i=0 ; i<im1->lx ; i++)
				im1->data[j*im1->lx+i] -= im2->data[i] ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Multiply im2 (1D signal) to each row or column of im1.
  @param	im1		image to modify
  @param	im2		1D sgnal to multiply 
  @return	int 0 if ok -1 otherwise.
  	
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_mul_1d_local(
		image_t	*	im1,
		image_t	*	im2)
{
	int			orientation ;
	
	int			i,
				j ;
	
	/* Test input data */
	if ((im1 == NULL) || (im2 == NULL)) {
		return -1 ;
	}
	
	/* Test if one and only one size of im2 is equal to 1 */
	if (im2->lx == 1) {
		if (im2->ly == 1) {
			e_error("1D signal contains only one value - abort") ;
			return -1 ;
		} else orientation = 0 ;
	} else if (im2->ly != 1) {
		e_error("The second parameter is not a 1D signal - abort") ;
		return -1 ;
	} else orientation = 1 ;

	if (orientation==0) {
		/* 1D signal is vertical */
		for (j=0 ; j<im1->ly ; j++)
            for (i=0 ; i<im1->lx ; i++)
                im1->data[j*im1->lx+i] *= im2->data[j] ;
	} else {
		/* 1D signal is horizontal */
		for (j=0 ; j<im1->ly ; j++)
			for (i=0 ; i<im1->lx ; i++)
				im1->data[j*im1->lx+i] *= im2->data[i] ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Divide each row or column of im1 by im2 (1D signal).
  @param	im1		image to modify
  @param	im2		1D signal 
  @return	int 0 if ok -1 otherwise.
  	
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_div_1d_local(
		image_t	*	im1,
		image_t	*	im2)
{
	int			orientation ;
	
	int			i,
				j ;
	
	/* Test input data */
	if ((im1 == NULL) || (im2 == NULL)) {
		return -1 ;
	}
	
	/* Test if one and only one size of im2 is equal to 1 */
	if (im2->lx == 1) {
		if (im2->ly == 1) {
			e_error("1D signal contains only one value - abort") ;
			return -1 ;
		} else orientation = 0 ;
	} else if (im2->ly != 1) {
		e_error("The second parameter is not a 1D signal - abort") ;
		return -1 ;
	} else orientation = 1 ;

	if (orientation==0) {
		/* 1D signal is vertical */
		for (j=0 ; j<im1->ly ; j++)
            for (i=0 ; i<im1->lx ; i++)
				if (fabs(im2->data[j])>(double)1e-30) {
					im1->data[j*im1->lx+i] /= im2->data[j] ;
				} else {
					im1->data[j*im1->lx+i] = (pixelvalue)0.0 ;
				}
	} else {
		/* 1D signal is horizontal */
		for (j=0 ; j<im1->ly ; j++)
			for (i=0 ; i<im1->lx ; i++)
				if (fabs(im2->data[i])>(double)1e-30) {
					im1->data[j*im1->lx+i] /= im2->data[i] ;
				} else {
					im1->data[j*im1->lx+i] = (pixelvalue)0.0 ;
				}
	}

	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Arithmetic between an image and a constant.
  @param    image_in    Image operand.
  @param    constant    Constant operand.
  @param    operation   Operation to perform.
  @return   1 newly allocated image.
 
  Performs an operation between an image and a constant, returns a newly
  allocated image containing the result. Possible operations are:
 
  \begin{itemize}
  \item '+' Addition
  \item '-' Subtraction
  \item '*' Multiplication
  \item '/' Division
  \item 'l' Logarithm
  \item '^' Power
  \item 'e' Exponentiation
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
image_t * image_cst_op(
		image_t		*	image_in,
		double			constant,
		int				operation)
{
    image_t    *	image_out ;
    int		   		i ;
	double			invlog ;
	double			dexp ;
	double			invconst ;

    /* Error handling   */
	if (image_in == NULL) return NULL ;
    if ((fabs((double)constant) < 1e-10) && (operation == '/')){
        e_error("division by zero requested in image/constant operation") ;
        return NULL ;
    }

	image_out = image_new(image_in->lx, image_in->ly) ;
	switch(operation)
	{
		case '+':
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = 
				(pixelvalue)((double)image_in->data[i] + constant) ;
		break ;
		 
		case '-':
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = 
				(pixelvalue)((double)image_in->data[i] - constant) ;
		break ;
		 
		case '*':
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = 
				(pixelvalue)((double)image_in->data[i] * constant) ;
		break ;
		 
		case '/':
		/* Multiplications are faster than divisions !	*/
		invconst = (double)1.0 / constant ;
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = 
				(pixelvalue)((double)image_in->data[i] * invconst) ;
		break ;

		case 'l':
		invlog = (double)(1.0 / log((double)constant)) ;
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = (pixelvalue)
								log((double)image_in->data[i])*invlog ;
		break ;

		case '^':
		dexp = (double)constant ;
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = (pixelvalue)
								pow((double)image_in->data[i], dexp) ;
		break ;

		case 'e':
		dexp = (double)constant ;
		for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
			image_out->data[i] = (pixelvalue)
								pow(dexp, (double)image_in->data[i]) ;
		break ;
	
		default:
		e_error("unrecognized requested operation : aborting") ;
		break ;
	}
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Arithmetic between an image and a constant.
  @param    image_in    Image operand.
  @param    constant    Constant operand.
  @param    operation   Operation to perform.
  @return   int 0 if Ok, -1 otherwise.
 
  Performs an operation between an image and a constant, stores the
  results in the input image buffer. Possible operations are:
 
  \begin{itemize}
  \item '+' Addition
  \item '-' Subtraction
  \item '*' Multiplication
  \item '/' Division
  \item 'l' Logarithm
  \item '^' Power
  \item 'e' Exponentiation
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
int image_cst_op_local(
		image_t	*	image_in,
		double		constant,
		int			operation)
{
    int	   	i ;
	double	invlog ;
	double	dexp ;
	double	invconst ;

    /* Error handling   */
	if (image_in == NULL) return -1 ;
    if ((fabs((double)constant) < 1e-10) && (operation == '/')){
        e_error("division by zero requested in image/constant operation") ;
        return -1 ;
    }
	switch(operation)
	{
		case '+':
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] =
				(pixelvalue)((double)image_in->data[i] + constant) ;
		break ;
		 
		case '-':
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] = 
				(pixelvalue)((double)image_in->data[i] - constant) ;
		break ;
		 
		case '*':
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] = 
				(pixelvalue)((double)image_in->data[i] * constant) ;
		break ;
		 
		case '/':
		/* Multiplications are faster than divisions !	*/
		invconst = (double)1.0 / constant ;
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] = 
				(pixelvalue)((double)image_in->data[i] * invconst) ;
		break ;

		case 'l':
		invlog = (double)(1.0 / log((double)constant)) ;
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] = (pixelvalue)
								log((double)image_in->data[i])*invlog ;
		break ;

		case '^':
		dexp = (double)constant ;
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] = (pixelvalue)
								pow((double)image_in->data[i], dexp) ;
		break ;

		case 'e':
		dexp = (double)constant ;
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++)
			image_in->data[i] = (pixelvalue)
								pow(dexp, (double)image_in->data[i]) ;
		break ;
	
		default:
		e_error("unrecognized requested operation : aborting") ;
		return -1 ;
	}
    return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Normalize pixels in an image.
  @param    image_in    Image operand.
  @param    mode        Normalization mode.
  @return   1 newly allocated image.
 
  Normalizes an image according to a given criterion, stores the results
  in a newly allocated image and returns it. The returned image must be
  freed using image_del().
 
  Possible normalizations are:
  \begin{itemize}
  \item NORM_SCALE sets the pixel interval to [0,1].
  \item NORM_MEAN sets the mean value to 1.
  \item NORM_FLUX sets the flux to 1.
  \item NORM_AFLUX sets the absolute flux to 1.
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
image_t * image_normalize(
		image_t	*	image_in, 
		int 		mode)
{
	image_t	*	image_out ;
	image_stats	*	stats ;
	double			gain ;
	int		 		i ;

	if (image_in==NULL) return NULL ;
	stats = image_getstats(image_in) ;
	if (stats == NULL) {
		e_error("cannot extract image stats: aborting normalization") ;
		return NULL;
	}
	switch (mode) {
		case NORM_SCALE:
		gain = (double)(stats->max_pix - stats->min_pix) ;
		if (fabs((double)gain) < 1e-10) {
			e_error("interval is too small to normalize to unity interval") ;
			free(stats);
			return NULL ;
		}		
		gain = 1.0 / gain ;
		image_out = image_new(image_in->lx, image_in->ly) ;
		for (i=0 ; i<(image_in->lx * image_in->ly) ; i++) {
			image_out->data[i] = (pixelvalue)
					(gain * (double)(image_in->data[i] - stats->min_pix)) ;
		}
		break ;

		case NORM_MEAN:
		if (fabs(stats->avg_pix) < 1e-10) {
			e_error("zero mean value : cannot normalize to unity mean") ;
			free(stats);
			return NULL ;
		}		
		image_out=image_cst_op(image_in,stats->avg_pix,'/') ;
		break ;

		case NORM_FLUX:
		if (fabs((double)stats->flux) < 1e-10) {
			e_error("flux is too small: cannot normalize to unity flux") ;
			free(stats);
			return NULL ;
		}
		image_out = image_cst_op(image_in, stats->flux, '/');
		break ;

		case NORM_AFLUX:
		if (fabs((double)stats->absflux) < 1e-10) {
			e_error("abs flux is too small: cannot normalize to unity aflux") ;
			free(stats);
			return NULL ;
		}
		image_out = image_cst_op(image_in, stats->absflux, '/');
		break ;

		default:
		e_error("unrecognized normalization mode: aborting normalization") ;
		free(stats);
		return NULL ;
	}
	free(stats) ;
	return image_out ;
}
		

/*-------------------------------------------------------------------------*/
/**
  @brief    Threshold an image to a given interval.
  @param    image_in        Image to threshold.
  @param    lo_cut          Lower bound.
  @param    hi_cut          Higher bound.
  @param    assign_lo_cut   Value to assign to pixels below low bound.
  @param    assign_hi_cut   Value to assign to pixels above high bound.
  @return   1 newly allocated image.
 
  Pixels outside of the provided interval are assigned the given
  values. To specify no threshold on lower bound, set lo_cut to
  MIN_PIX_VALUE. To specify no threshold on higher bound, set
  hi_cut to MAX_PIX_VALUE.
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_threshold(
    	image_t 	*	image_in,
    	pixelvalue  	lo_cut,
    	pixelvalue  	hi_cut,
    	pixelvalue  	assign_lo_cut,
    	pixelvalue  	assign_hi_cut)
{
	image_t	*	image_out ;
	pixelvalue	*	pos ;
	int				i ;

	if (image_in==NULL) return NULL ;

	/* Out Image creation	*/

	image_out = image_new(image_in->lx, image_in->ly) ;
	if (image_out==NULL) return NULL ;

	if (assign_lo_cut == MIN_PIX_VALUE)
		assign_lo_cut = lo_cut ;
	if (assign_hi_cut == MAX_PIX_VALUE)
		assign_hi_cut = hi_cut ;
	
	pos = image_in->data ;
	for (i=0 ; i<(image_out->lx * image_out->ly) ; i++) {
		if (*pos > hi_cut) {
			image_out->data[i] = assign_hi_cut ;
		} else {
			if (*pos < lo_cut) {
				image_out->data[i] = assign_lo_cut ;
			} else {
				image_out->data[i] = *pos ;
			}
		}
		pos++ ;
	}
	return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Take the absolute value of an image.
  @param    image_in    Image operand.
  @return   1 newly allocated image.
 
  For each pixel, out = abs(in). The returned image must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_abs(image_t * image_in)
{
	image_t	*	image_out ;
	int				i ;

	if (image_in==NULL) return NULL ;
	image_out = image_new(image_in->lx, image_in->ly) ;
	if (image_out==NULL) return NULL ;

	for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
		image_out->data[i] = (pixelvalue)fabs((double)image_in->data[i]) ;

	return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Build the average of two images.
  @param    image_1     First image operand.
  @param    image_2     Second image operand.
  @return   1 newly allocated image.
 
  Builds the average of two images and returns a newly allocated image,
  to be freed using image_del(). The average is arithmetic, i.e.
  outpix=(pix1+pix2)/2
 */
/*--------------------------------------------------------------------------*/
image_t * image_mean(
		image_t	*	image_1,
		image_t	*	image_2)
{
	image_t	*	image_out ;
	int				i ;

	/* Error handling : check entries	*/
	if (image_1==NULL || image_2==NULL) return NULL ;
	image_out = image_new(image_1->lx, image_1->ly) ;
	if (image_out==NULL) return NULL ;

	for (i=0 ; i<(image_out->lx * image_out->ly) ; i++)
		image_out->data[i] = (pixelvalue)0.5 *
								(image_1->data[i] + image_2->data[i]) ;
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract min value from all pixels.
  @param    image_in    Image to modify.
  @return   int 0 if Ok, -1 otherwise.
 
  Find the minimum value of an image, and subtract it from all pixels.
  This ends up making all pixels in the image non-negative.
 
  The input image is modified.
 */
/*--------------------------------------------------------------------------*/
int image_submin(image_t * image_in)
{
	int				i ;
	pixelvalue	*	current ;
	pixelvalue		min ;
    
	if (image_in==NULL) return -1 ;
	min = image_getmin(image_in);
	current = image_in->data ;
	for (i=0 ; i < (image_in->lx * image_in->ly) ; i++){
		*current -= min ;
		current++ ;
	}
	return 0 ;
} 


/*-------------------------------------------------------------------------*/
/**
  @brief    Build an integer-only image from an image.
  @param    image_in
  @return   1 newly allocated image.
 
  For each pixel in input, the output pixel is computed with the floor()
  function, i.e. all output pixels are integer-valued. floor() finds the
  closest least integer.
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_floor(image_t * image_in) 
{
	image_t		*	floor_image ;
	int				i ;
	register
	pixelvalue	*	current_in,
  				*	current_fl ;

	if (image_in==NULL) return NULL ;
	floor_image = image_new(image_in->lx,image_in->ly);
	if (floor_image==NULL) return NULL ;
	current_in = image_in->data ;
	current_fl = floor_image->data ;
	for ( i = 0 ; i < (image_in->lx * image_in->ly) ; i++){
		*current_fl = (pixelvalue)floor((double)*current_in);
		current_fl++ ;
		current_in++ ;
	}
	return floor_image ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the reciprocal of an image, i.e. 1/image.
  @param    image_in    Image operand.
  @return   1 newly allocated image.
 
  Compute the reciprocal image, out = 1/in. Notice that zero-valued pixels
  in input are also zero-valued in output.
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_recip(image_t * image_in)
{
	int				i ;
	image_t	*	image_out ;
	
	if (image_in==NULL) return NULL ;
	
	image_out = image_new(image_in->lx, image_in->ly) ;
	for (i=0 ; i<(image_in->lx * image_in->ly) ; i++) {
		if (fabs((double)image_in->data[i]) < 1e-10) {
			image_out->data[i] = (pixelvalue)0 ;
		} else {
			image_out->data[i] =
				(pixelvalue)(1.00 / (double)image_in->data[i]) ;
		}
	}
	return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Inverts all pixels in an image, i.e. image = -image.
  @param    in  Image to modify.
  @return   int 0 if Ok, -1 otherwise.
 
  Operates in place: all pixels are inverted, i.e. image=-image.
 */
/*--------------------------------------------------------------------------*/
int image_invert(image_t * in)
{
	int		i ;
	if (in==NULL) return -1 ;
	if (in->data==NULL) return -1 ;
	for (i=0 ; i<(in->lx * in->ly) ; i++) in->data[i] = - in->data[i] ;
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract the median of each row from all pixels in the row.
  @param    in  Input image.
  @return   int 0 if Ok, -1 otherwise.
 
  For each row in the input image, compute the median value of all
  pixels in the row, and remove it from all pixels in the row. This
  effectively brings all row to have a zero median value and slightly
  modifies the flux of the input image. This algorithm is used
  to remove row saturation effects.
 
  The input image is modified.
 */
/*--------------------------------------------------------------------------*/
int image_sub_rowmedian(image_t * in)
{
	pixelvalue	row_median ;
	pixelvalue* row ;
	pixelvalue* first_in_row ;
	int			i, j ;

	if (in==NULL) return -1 ;
	if (in->data==NULL) return -1 ;

	first_in_row = in->data ;
	row = malloc(in->lx * sizeof(pixelvalue)) ;
	for (j=0 ; j<in->ly ; j++) {
		memcpy(row, first_in_row, in->lx * sizeof(pixelvalue)) ;
		row_median = median_pixelvalue(row, in->lx) ;
		for (i=0 ; i<in->lx ; i++) {
			first_in_row[i] -= row_median ;
		}
		first_in_row += in->lx ;
	}
	free(row) ;
	return 0 ;
}


/*---------------------------------------------------------------------------*/
/**
 	@brief	Subtract a lowpas
    @param  in              input image (modified)
    @param  orientation     0 horizontal median, 1 for vertical median
    @param  window_size     the window size for the lowpasss
    @return 0 if OK, -1 if not
    
    Subtracts a low pass filtered 1-d median from image.
	The image is modified destructively.
*/
/*---------------------------------------------------------------------------*/
int image_sub_lowpass(
		image_t	*	in,
		int       	orientation,
		int       	window_size)
{
    image_t			*	image1d ;
    int                 maxi ;
    pixelvalue      *   avglinehi,
                    *   avglinelo ;
    int                 xsize,
                        ysize ;

    int                 i ;

    /* Find low-pass filtered 1-d transversal */
    switch(orientation){

        /* Horizontal median */
        case 0:
        avglinelo = image_getmedian_mov_horz(in,
                                            in->ly/4,
                                            window_size) ;
        avglinehi = image_getmedian_mov_horz(in,
                                            (3*in->ly)/4,
                                            window_size) ;
        maxi = in->lx ;
        xsize = in->lx ;
        ysize = 1 ;
        break;

        /* Vertical median */
        case 1:
        avglinelo = image_getmedian_mov_vert(in,
                                            in->lx/4,
                                            window_size) ;
        avglinehi = image_getmedian_mov_vert(in,
                                            (3*in->lx)/4,
                                            window_size) ;
        maxi = in->ly ;
        xsize = 1 ;
        ysize = in->ly ;
        break;

        default:
        e_error("unknown orientation %d", orientation) ;
        return -1 ;
    }

    /* Even out by subracting low-pass filtered 1-d transversal */
    for (i=0 ; i<maxi ; i++) avglinehi[i] = (avglinehi[i] + avglinelo[i])/2.0 ;
    free(avglinelo) ;

    /* Create the image image1d */
    image1d = image_new(xsize, ysize) ;
    memcpy(image1d->data, avglinehi, maxi*sizeof(pixelvalue)) ;
    free(avglinehi) ;

    /* Subtraction */
    image_sub_1d_local(in, image1d) ;
    image_del(image1d) ;

    return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Collapse a vig along its rows or columns.
  @param	inimage     Image to collapse.
  @param	llx			lower left x coord.
  @param	lly			lower left y coord
  @param	urx			upper right x coord
  @param	ury			upper right y coord
  @param    direction   Collapsing direction.
  @return	collapsed image	
  	
  llx, lly, urx, ury are vig coordinates in FITS convention 
 */
/*--------------------------------------------------------------------------*/
image_t * image_collapse_vig(
		image_t	*	in,
		int			llx,
		int			lly,
		int			urx,
		int			ury,
		int			direction)
{
	image_t	*	extracted ;
	image_t	*	collapsed ;
		
	/* First extract the image to collapse */
	if ((extracted = image_getvig(in,
										llx,
										lly,
										urx,
										ury)) == NULL) {
		e_error("cannot extract image - abort") ;
		return NULL ;
	}

	/* Collapse now the extracted image */
	if ((collapsed = image_collapse(extracted, direction)) == NULL) {
		e_error("cannot collapse image - abort") ;
		image_del(extracted) ;
		return NULL ;
	}
	image_del(extracted) ;
	
	return collapsed ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Collapse an image along its rows or columns.
  @param    inimage     Image to collapse.
  @param    direction   Collapsing direction.
  @return   1 newly allocated image having 1 row or 1 column.
 
  Collapsing an image means building up a 1d signal by adding up all pixels
  on the same row or column.
 
  \begin{verbatim}
  Collapse along y:
 
  p7  p8  p9     Input image is a 3x3 image containing 9 pixels.
  p4  p5  p6     The output is an image containing one row with
  p1  p2  p3     3 pixels A, B, C, where:
  ----------
 
  A   B   C      A = p1+p4+p7
                 B = p2+p5+p8
                 C = p3+p6+p9
  \end{verbatim}
 
  Provide the collapsing direction as an int. Give 0 to collapse along y
  (sum of rows) and get an image with a single row in output, or give 1
  to collapse along x (sym of columns) to get an image with a single
  column in output.
 
 */
/*--------------------------------------------------------------------------*/
image_t * image_collapse(
		image_t	* 	inimage,
		int			direction)
{
    image_t		*	image1D ;
    double          pixval ;
    int             i ;

    if (direction == 0) {
        image1D = image_new(inimage->lx, 1) ;
        for (i=0 ; i<image1D->lx ; i++) {
            pixval=image_getsumpix_vig(inimage,
                                            i+1,
                                            1,
                                            i+1,
                                            inimage->ly) ;
            image1D->data[i] = (pixelvalue)pixval ;
        }
    } else if (direction == 1) {
        image1D = image_new(1, inimage->ly) ;
        for (i=0 ; i<image1D->ly ; i++) {
            pixval=image_getsumpix_vig(inimage,
                                            1,
                                            i+1,
                                            inimage->lx,
                                            i+1) ;
            image1D->data[i] = (pixelvalue)pixval ;
        }
    } else {
        e_error("unrecognized direction : [%d]", direction) ;
        return NULL ;
    }
    return image1D ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Collapse an image along its rows and columns, with filtering.
  @param    in          Input image.
  @param    direction   Collapsing direction.
  @param    discard_lo  Low rejection parameter.
  @param    discard_hi  High rejection parameter.
  @return   1 newly allocated image having 1 row or 1 column.
  @see      collapse_image
 
  Collapsing is done as for collapse_image(). The difference is that pixels
  are not just summed along rows or columns but instead all sorted, the
  highest and lowest pixel values are removed, and the remaining values
  are linearly averaged to produce the output value.
 
 */
/*--------------------------------------------------------------------------*/
image_t * image_collapse_median(
		image_t		*	in,
		int				direction,
		int				discard_lo,
		int				discard_hi)
{
	image_t		*	collapsed ;
	pixelvalue	*	line ;
	int				i, j ;
	int				width ;

	if (in==NULL) return NULL ;
	if (direction == 1) {
		/* Collapsing the image in the x direction */
		if ((discard_lo+discard_hi)>=in->ly) {
			e_error("discard bounds: %d+%d >= %d",
					discard_lo, discard_hi, in->ly);
			return NULL ;
		}
		collapsed = image_new(1, in->ly);
		width = in->lx - discard_lo - discard_hi ;
		line = malloc(width * sizeof(pixelvalue)) ;
		for (j=0 ; j<in->ly ; j++) {
			for (i=0 ; i<width ; i++) {
				line[i] = in->data[(i+discard_lo)+j*in->lx];
			}
			collapsed->data[j] = median_pixelvalue(line, width);
		}
		free(line);
	} else if (direction==0) {
		/* Collapsing the image in the y direction */
		if ((discard_lo+discard_hi)>=in->lx) {
			e_error("discard bounds: %d+%d >= %d",
					discard_lo, discard_hi, in->lx);
			return NULL ;
		}
		collapsed = image_new(in->lx, 1);
		width = in->ly - discard_lo - discard_hi ;
		line = malloc(width * sizeof(pixelvalue)) ;
		for (i=0 ; i<in->lx ; i++) {
			for (j=0 ; j<width ; j++) {
				line[j] = in->data[i+(j+discard_lo)*in->lx] ;
			}
			collapsed->data[i] = median_pixelvalue(line, width);
		}
		free(line);
	} else {
		e_error("unknown direction for collapsing: [%d]", direction);
		collapsed = NULL ;
	}
	return collapsed ;
}

