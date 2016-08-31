/*-------------------------------------------------------------------------*/
/**
   @file	corner.c
   @author	N. Devillard
   @date	Nov 2000
   @version	$Revision: 1.4 $
   @brief	Corner detector.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: corner.c,v 1.4 2001/10/26 14:33:44 yjung Exp $
	$Author: yjung $
	$Date: 2001/10/26 14:33:44 $
	$Revision: 1.4 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

#include "image_handling.h"
#include "doubles.h"

/*---------------------------------------------------------------------------
   								Define
 ---------------------------------------------------------------------------*/

#define	SUSAN_MASK_PIX		37
#define THRESH 				0.1

/*---------------------------------------------------------------------------
   								Macros
 ---------------------------------------------------------------------------*/

#define square(x)	((x)*(x))

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Detect corners in an image.
  @param	in		Input image.
  @param
  @return	1 newly allocated list of pixel positions.

  This function applies a corner detector to an image and returns a list of
  pixel positions where corners have been located.
 */
/*--------------------------------------------------------------------------*/
image_t * image_detect_corners(image_t * in)
{
	int			pos ;
	double		sum ;
	pixelvalue	p1, p2 ;
	image_t *	sum_im ;
	image_t *	corner_im ;
	image_t *	final ;
	pixelvalue	max_sum ;
	double3	 *	pts_list ;
	int			i, j, k ;
	
	int			susan_mask_x[SUSAN_MASK_PIX] =
	{		-1, 0, 1,
		-2, -1, 0, 1, 2,
	-3, -2, -1, 0, 1, 2, 3,
	-3, -2, -1, 0, 1, 2, 3,
	-3, -2, -1, 0, 1, 2, 3,
		-2, -1, 0, 1, 2,
			-1, 0, 1
	};
	int			susan_mask_y[SUSAN_MASK_PIX] =
	{
			 3,  3,  3,
		 2,  2,  2,  2,  2,
	 1,  1,  1,  1,  1,  1,  1,
	 0,  0,  0,  0,  0,  0,  0,
	-1, -1, -1, -1, -1, -1, -1,
		-2, -2, -2, -2, -2,
			-1, -1, -1
	};

	int			mask5_x[24] = {
		-2, -1, 0, 1, 2,
		-2, -1, 0, 1, 2,
		-2, -1,    1, 2,
		-2, -1, 0, 1, 2,
		-2, -1, 0, 1, 2
	};
	int			mask5_y[24] = {
		 2,  2,  2,  2,  2,
		 1,  1,  1,  1,  1,
		 0,  0,      0,  0,
		-1, -1, -1, -1, -1,
		-2, -2, -2, -2, -2
	};

	if (in==NULL) return NULL ;

	/* Compute the corner criterion over the input image */
	sum_im = image_new(in->lx, in->ly);
	max_sum = 0 ;
	/* Loop through the image */
	for (j=3 ; j<(in->ly-3) ; j++) {
		compute_status("computing corner criterion", j, in->ly, 0);
		for (i=3 ; i<(in->lx-3) ; i++) {
			sum = 0 ;
			p1 = in->data[i+j*in->lx];
			/* Use points within SUSAN mask */
			for (k=0 ; k<SUSAN_MASK_PIX ; k++) {
				pos = (i+susan_mask_x[k]) +
					  (j+susan_mask_y[k]) * in->lx ;
				p2 = in->data[pos] ;
				sum += exp(-square((double)(p1-p2)/THRESH));
			}
			sum_im->data[i+j*sum_im->lx] = sum ;
			if (sum>max_sum)
				max_sum = sum ;
		}
	}
	max_sum *= 0.5 ;

	if (debug_active()>1)
		image_save_fits(sum_im, "sum.fits", BPP_DEFAULT);

	/* Invert the image to get corners as local maxima */
	corner_im = image_new(in->lx, in->ly);
	for (j=0 ; j<in->ly ; j++) {
		for (i=0 ; i<in->lx ; i++) {
			p1 = sum_im->data[i+j*sum_im->lx];
			if (p1<max_sum) {
				corner_im->data[i+j*corner_im->lx] = max_sum - p1 ;
			}
		}
	}
	image_del(sum_im);

	if (debug_active()>1)
		image_save_fits(corner_im, "corners.fits", BPP_DEFAULT);

	/* Locate local maxima */
	pts_list = double3_new(1024) ;
	final = image_new(in->lx, in->ly);
	for (j=2 ; j<(in->ly-2) ; j++) {
		compute_status("locating corners", j, (in->ly-2), 0);
		for (i=2 ; i<(in->lx-2) ; i++) {
			p1 = corner_im->data[i+j*corner_im->lx];
			final->data[i+j*final->lx] = 0 ;
			if (p1>1e-2) {
				for (k=0 ; k<24 ; k++) {
					pos = (i+mask5_x[k]) +
						  (j+mask5_y[k]) * corner_im->lx ;
					p2 = corner_im->data[pos];
					if ((p2-p1)>-1e-4) {
						final->data[i+j*final->lx] = 0 ;
						break ;
					} else {
						final->data[i+j*final->lx] = 1 ;
					}
				}
			}
		}
	}
	image_del(corner_im);
	if (debug_active()>1)
		image_save_fits(final, "final.fits", BPP_DEFAULT);

	/* Return list of points */
	return final ;
}

