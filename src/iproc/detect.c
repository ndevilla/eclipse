/*----------------------------------------------------------------------------*/
/**
   @file	detect.c
   @author	Y.Jung 
   @date	May 2001
   @version	$Revision: 1.31 $
   @brief	Object detection in an astronomical image
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: detect.c,v 1.31 2003/11/28 09:33:11 llundin Exp $
	$Author: llundin $
	$Date: 2003/11/28 09:33:11 $
	$Revision: 1.31 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "detect.h"
#include "xmemory.h"
#include "comm.h"
#include "doubles.h"
#include "dstats.h"
#include "image_handling.h"
#include "photometry.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

static double3 * detected_finepos_engine(image_t *, int, int, int, double) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Constructor for detected object.
  @return	1 newly allocated detected object.

  This function only allocates the main pointer. No information is stored in 
  there yet. The returned object must be deleted using detected_del().
 */
/*----------------------------------------------------------------------------*/
detected * detected_new(void)
{
	return (detected*)calloc(1, sizeof(detected));
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Destructor for detected object.
  @param	det		Object to delete.
  @return	void

  This function deallocates all possibly allocated arrays inside the given 
  object, then deallocates the main pointer.
 */
/*----------------------------------------------------------------------------*/
void detected_del(detected * det)
{
	if (det==NULL) return ;

	if (det->x!=NULL) 			free(det->x);
	if (det->y!=NULL) 			free(det->y);
	if (det->obj_nbpix!=NULL) 	free(det->obj_nbpix);
	if (det->bottom_x!=NULL) 	free(det->bottom_x);
	if (det->bottom_y!=NULL) 	free(det->bottom_y);
	if (det->top_x!=NULL) 		free(det->top_x);
	if (det->top_y!=NULL) 		free(det->top_y);
	if (det->left_x!=NULL) 		free(det->left_x);
	if (det->left_y!=NULL) 		free(det->left_y);
	if (det->right_x!=NULL) 	free(det->right_x);
	if (det->right_y!=NULL) 	free(det->right_y);
	if (det->min_x!=NULL) 		free(det->min_x);
	if (det->min_y!=NULL) 		free(det->min_y);
	if (det->max_x!=NULL) 		free(det->max_x);
	if (det->max_y!=NULL) 		free(det->max_y);
	if (det->min_i!=NULL) 		free(det->min_i);
	if (det->max_i!=NULL) 		free(det->max_i);
	if (det->obj_mean!=NULL) 	free(det->obj_mean);
	if (det->obj_stdev!=NULL) 	free(det->obj_stdev);
	if (det->obj_median!=NULL) 	free(det->obj_median);
	if (det->fine_x!=NULL) 		free(det->fine_x);
	if (det->fine_y!=NULL) 		free(det->fine_y);
	if (det->fwhm_x!=NULL) 		free(det->fwhm_x);
	if (det->fwhm_y!=NULL) 		free(det->fwhm_y);
	if (det->obj_flux!=NULL)	free(det->obj_flux);
	if (det->obj_background!=NULL) free(det->obj_background);
	free(det);
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Dump a detected object to an opened file pointer.
  @param	det		Detected object to dump
  @param	fp		Opened file pointer, ready to receive data
  @return	void

  This function dumps all informations contained into a detected object, to the
  passed (opened) file pointer. It is Ok to pass stdout or stderr. If the 
  object is unallocated or contains nothing, this function does nothing.
 */
/*----------------------------------------------------------------------------*/
void detected_dump(detected * det, FILE * fp)
{
	int	    i ;

	if (det==NULL || fp==NULL) return ;
	if (det->nbobj<1) return ;

	fprintf(fp, "#        X      Y");
	if (det->obj_nbpix!=NULL)   fprintf(fp, "    pix");
	if (det->obj_mean!=NULL)    fprintf(fp, "   mean");
	if (det->obj_stdev!=NULL)   fprintf(fp, "    dev");
	if (det->obj_median!=NULL)  fprintf(fp, "    med");
	if (det->min_i!=NULL)       fprintf(fp, "    min");
	if (det->max_i!=NULL)       fprintf(fp, "    max");
	if (det->fwhm_x!=NULL)     fprintf(fp, "     fx     fy     fa");
	if (det->obj_flux!=NULL)    fprintf(fp, "     flux");

	fprintf(fp, "\n");

	for (i=0 ; i<det->nbobj ; i++) {
		fprintf(fp, "% 3d %6.1f %6.1f", i+1,
            det->fine_x ? det->fine_x[i] : det->x[i],
            det->fine_y ? det->fine_y[i] : det->y[i]);
		if (det->obj_nbpix!=NULL)   fprintf(fp, " % 6d", det->obj_nbpix[i]);
		if (det->obj_mean!=NULL)    fprintf(fp, " %6.2f", det->obj_mean[i]);
		if (det->obj_stdev!=NULL)   fprintf(fp, " %6.2f", det->obj_stdev[i]);
		if (det->obj_median!=NULL) fprintf(fp, " %6.2f", det->obj_median[i]);
		if (det->min_i!=NULL)       fprintf(fp, " %6.2f", det->min_i[i]);
		if (det->max_i!=NULL)       fprintf(fp, " %6.2f", det->max_i[i]);
		if (det->fwhm_x!=NULL) {
			fprintf(fp, " %6.2f %6.2f %6.2f", det->fwhm_x[i], det->fwhm_y[i], 
                    sqrt(det->fwhm_x[i] * det->fwhm_y[i]));
		}
		if (det->obj_flux!=NULL) fprintf(fp, " %8.2f", det->obj_flux[i]);
		fprintf(fp, "\n");
	}
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	kappasigma detection and objects statistics computation.
  @param	in		Image to examine.
  @param    kappa   kappa for detection
  @return 	a detected object	

  This function will detect astronomical objects in the image and fill up a
  detected structure accordingly. This version uses default parameters for 
  all settings.
 */
/*----------------------------------------------------------------------------*/
detected * detected_ks_withstats(
		image_t 	* 	in, 
		double 			kappa)
{
	detected	*	det ;

	/* Search method is kappa-sigma clipping */
	if ((det = detected_ks_engine(in,
								 kappa,
								 0)) == NULL) {
		e_error("in kappa-sigma clipping");
		return NULL ;
	}
	if (det->nbobj<1) return det ;

	/* Compute fine positioning */
	if (detected_compute_finepos(det,
							   in,
							   DETECTED_FPOS_STAR,
							   DETECTED_FPOS_INT,
							   DETECTED_FPOS_EXT) != 0) {
		e_warning("fine positioning failed");
	}

	/* Compute FWHMs */
	if (detected_compute_fwhm(det, in) != 0) {
		e_warning("computing FWHMs failed");
	}

	/* Compute photometry */
	if (detected_compute_phot(det,
							in,
							DETECTED_PHOT_STAR,
							DETECTED_PHOT_INT,
							DETECTED_PHOT_EXT) != 0) {
		e_warning("computing photometry failed");
	}
	return det ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	squares-method object detection and statistics computation.
  @param	in		Image to examine.
  @param	hx		Halfsize of the square in X direction 
  @param	hy		Halfsize of the square in Y direction 
  @return 	a detected object	

  This function will detect astronomical objects in the image and fill up a
  detected structure accordingly. This version uses default parameters for 
  all settings.
 */
/*----------------------------------------------------------------------------*/
detected * detected_sq_withstats(
		image_t 	* 	in, 
		int				hx,
		int				hy)
{
	detected	*	det ;

	/* Search method is kappa-sigma clipping */
	if ((det = detected_sq_engine(in,
								 hx,
								 hy)) == NULL) {
		e_error("in kappa-sigma clipping");
		return NULL ;
	}
	if (det->nbobj<1) return det ;

	/* Compute fine positioning */
	if (detected_compute_finepos(det,
							   in,
							   DETECTED_FPOS_STAR,
							   DETECTED_FPOS_INT,
							   DETECTED_FPOS_EXT) != 0) {
		e_warning("fine positioning failed");
	}

	/* Compute FWHMs */
	if (detected_compute_fwhm(det, in) != 0) {
		e_warning("computing FWHMs failed");
	}

	/* Compute photometry */
	if (detected_compute_phot(det,
							in,
							DETECTED_PHOT_STAR,
							DETECTED_PHOT_INT,
							DETECTED_PHOT_EXT) != 0) {
		e_warning("computing photometry failed");
	}
	return det ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute object statistics for all labelled objects in an image.
  @param	ref		Reference image.
  @param	lab		Label image.
  @param	nb		Number of objects
  @return	a detected object
 */
/*----------------------------------------------------------------------------*/
detected * detected_compute_objstat(
		image_t		*	ref,
		intimage	*	lab,
		int				nb)
{
	detected	*	det ;
	double	 	*	sum ;
	double	 	*	sqsum ;
	pixelvalue		pix ;
	int				npix ;
	pixelvalue 	* 	storemed ;
	int				count ;
	int				pixwin[4] ;
	int				jl ;
	int				i, j, k;

	/* Review input parameters */
	if (ref==NULL || lab==NULL) return NULL ;
	if (nb < 0) return NULL ;

	/* Create a detected object */
	det = detected_new() ;
	det->nbobj = nb ;
	if (det->nbobj == 0) return det ;
	
	/* Allocate data holders */
	det->x 			= calloc(nb, sizeof(double));
	det->y 			= calloc(nb, sizeof(double));
	det->obj_nbpix 	= calloc(nb, sizeof(int));

	det->bottom_x 	= calloc(nb, sizeof(int));
	det->bottom_y 	= calloc(nb, sizeof(int));
	det->top_x		= calloc(nb, sizeof(int));
	det->top_y		= calloc(nb, sizeof(int));
	det->left_x		= calloc(nb, sizeof(int));
	det->left_y		= calloc(nb, sizeof(int));
	det->right_x	= calloc(nb, sizeof(int));
	det->right_y	= calloc(nb, sizeof(int));

	det->min_x		= calloc(nb, sizeof(int));
	det->min_y		= calloc(nb, sizeof(int));
	det->max_x		= calloc(nb, sizeof(int));
	det->max_y		= calloc(nb, sizeof(int));
	det->min_i		= calloc(nb, sizeof(double));
	det->max_i		= calloc(nb, sizeof(double));

	det->obj_mean	= calloc(nb, sizeof(double));
	det->obj_stdev	= calloc(nb, sizeof(double));
	det->obj_median	= calloc(nb, sizeof(double));

	sum 			= calloc(nb, sizeof(double));
	sqsum 			= calloc(nb, sizeof(double));

	/* Initialize features */
	for (k=0 ; k<nb ; k++) {
		/* Enough to initialize only min/max x */
		det->min_x[k] = -1 ;
		det->max_x[k] = -1 ;

		det->top_y[k] = -1 ;
		det->right_x[k] = -1 ;
		det->left_x[k] = lab->lx ;
		det->bottom_y[k] = lab->ly ;
	}

	for (j=0 ; j<lab->ly ; j++) {
		for (i=0 ; i<lab->lx ; i++) {
			k = (int)lab->data[i+j*lab->lx]-1 ;
			/* Background: do nothing */
			if (k==-1) continue ;

			/* Accumulate weighted position */
			det->x[k] += (double)i ;
			det->y[k] += (double)j ;
			/* Increase number of pixels */
			det->obj_nbpix[k] ++ ;
			/* Store object extremities */
			if (j<det->bottom_y[k]) {
				det->bottom_x[k]=i ;
				det->bottom_y[k]=j ;
			}
			if (j>det->top_y[k]) {
				det->top_x[k]=i;
				det->top_y[k]=j;
			}
			if (i>det->right_x[k]) {
				det->right_x[k]=i;
				det->right_y[k]=j;
			}
			if (i<det->left_x[k]) {
				det->left_x[k]=i;
				det->left_y[k]=j;
			}

			/* Store pixel sum and squared sum */
			pix = ref->data[i+j*ref->lx];
			sum[k] += pix ;
			sqsum[k] += (pix*pix);

			/* Check min/max pos and value */
			if (((double)pix<det->min_i[k]) || (det->min_x[k]==-1)) {
				det->min_i[k] = (double)pix ;
				det->min_x[k] = i ;
				det->min_y[k] = j ;
			}
			if (((double)pix>det->max_i[k]) || (det->max_x[k]==-1)) {
				det->max_i[k] = (double)pix ;
				det->max_x[k] = i ;
				det->max_y[k] = j ;
			}
		}
	}

	/* Compute average and std dev for each object, normalize centers */
	for (k=0 ; k<nb ; k++) {
		npix = det->obj_nbpix[k] ;
		det->obj_mean[k] = sum[k] / (double)npix ;
		if (npix>1) {
            /* Rounding errors can cause the variance to be negative */
            det->obj_stdev[k] = (sqsum[k] - ((sum[k]*sum[k])/(double)npix))
                              / ((double)npix-1.0);
            det->obj_stdev[k] = det->obj_stdev[k] > 0
                              ? sqrt(det->obj_stdev[k]) : 0;
		} else {
			det->obj_stdev[k] = 0.0 ;
		}
		det->x[k] /= (double)npix ;
		det->y[k] /= (double)npix ;
	}
	free(sum);
	free(sqsum);

	/* Compute median for each object */
	for (k=0 ; k<nb ; k++) {
		pixwin[0] = det->left_x[k] ;
		pixwin[1] = det->right_x[k]+1 ;
		pixwin[2] = det->bottom_y[k] ;
		pixwin[3] = det->top_y[k]+1;
		storemed = malloc(det->obj_nbpix[k] * sizeof(pixelvalue));
		count=0 ;
		for (j=pixwin[2] ; j<pixwin[3] ; j++) {
			jl = j * lab->lx ;
			for (i=pixwin[0] ; i<pixwin[1] ; i++) {
				if (lab->data[i+jl]==(k+1)) {
					storemed[count++] = ref->data[i+j*ref->lx] ;
				}
			}
		}
		det->obj_median[k] = median_pixelvalue(storemed, count);
		free(storemed);
	}
	return det ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute fine positioning for all detected objects.
  @param	det			Detected structure to fill up.
  @param	ref			Reference image.
  @param	fpos_star	Radius for star.
  @param	fpos_int	Internal radius for background.
  @param	fpos_ext	External radius for background.
  @return	int 0 if Ok, -1 otherwise.
 */
/*----------------------------------------------------------------------------*/
int detected_compute_finepos(
		detected	*	det,
		image_t		*	ref,
		double			fpos_star,
		double			fpos_int,
		double			fpos_ext)
{
	double3		*	fine ;
    double  		background ;
    int     		i ;

	if (det==NULL || ref==NULL) return -1 ;
	if ((det->x == NULL) || (det->y == NULL)) return -1 ;
    if ((fpos_star<1.0) || (fpos_int<1.0)  || (fpos_ext<1.0)  ||
        (fpos_int<fpos_star) || (fpos_ext<fpos_star) || (fpos_ext<fpos_int)) {
        return -1 ;
    }
	
	det->fine_x = malloc(det->nbobj * sizeof(double));
	det->fine_y = malloc(det->nbobj * sizeof(double));

    for (i=0 ; i<det->nbobj ; i++) {
        /* First, determine the background value for each position */
		background = detected_compute_background(ref,
												det->x[i],
												det->y[i],
												fpos_int,
												fpos_ext) ;
		
        /* Determine the barycenter in the circle of radius fpos_star  */
		if ((fine = detected_finepos_engine(ref,
										det->x[i],
										det->y[i],
										fpos_star,
										background)) == NULL) {
			e_error("cannot compute fine positions") ;
			free(det->fine_x) ;
			free(det->fine_y) ;
			det->fine_x = det->fine_y = NULL ;
			return -1 ;
		}
		det->fine_x[i] = fine->x[0] ;
		det->fine_y[i] = fine->y[0] ;
		double3_del(fine) ; 
   }
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the background value around an object
  @param	in			input image	
  @param	x_pos		Object x position
  @param	y_pos		Object y position
  @param	rad1		small radius
  @param	rad2		big radius
  @return	the background value	
 */
/*----------------------------------------------------------------------------*/
double detected_compute_background(
		image_t		*	in,
		int				x_pos,
		int				y_pos,
		double			rad1,
		double			rad2)
{
	image_t	*	local ;
    double  	background ;
    int     	countpix ;
	double		sq_rad,
				sq_rad1, 
				sq_rad2 ;
	int			loc_x_pos,
				loc_y_pos ;
	int			llx, lly, urx, ury ;			
	int			i, j ;

	/* Initialize  */
	sq_rad1 = (double)(rad1 * rad1) ;
	sq_rad2 = (double)(rad2 * rad2) ;
	
	/* First extract the small image containing the point */
	llx = (int)(x_pos - rad2) ;
	lly = (int)(y_pos - rad2) ;
	urx = (int)(x_pos + rad2) ;
	ury = (int)(y_pos + rad2) ;
	/* Verify the positions */
	if (llx < 0) llx = 0 ;	
	if (lly < 0) lly = 0 ;
	if (urx >= in->lx) urx = in->lx - 1 ;
	if (ury >= in->ly) ury = in->ly - 1 ;
	/* Extract	 */
	if ((local = image_getvig(in, llx+1, lly+1, urx+1, ury+1)) == NULL) {
		e_error("cannot extract image") ;
		return 0.00 ;
	}
	loc_x_pos = x_pos - llx ;
	loc_y_pos = y_pos - lly ;

	/* Compute the background */
	countpix = 0 ;
	background = 0.00 ;
	for (j=0 ; j<local->ly ; j++) {
		for (i=0 ; i<local->lx ; i++) {
			sq_rad = (double)((i - loc_x_pos) * (i - loc_x_pos) +
				(j - loc_y_pos) * (j - loc_y_pos)) ;
			if ((sq_rad >= sq_rad1) && (sq_rad <= sq_rad2)) {
				countpix ++ ;
				background += (double)local->data[i+j*local->lx] ;
			}
		}
	}
	image_del(local) ;
    
	if (countpix<=0) return 0.00 ;
	else background /= (double)countpix ;
	
	/* Return */
	return background ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the corrected position of an object
  @param	in			input image	
  @param	x_pos		Object rough x position
  @param	y_pos		Object rough y position
  @param	radius		Object radius
  @param	background	the background
  @return	the object position (coordinates respect C conv. : (0,0)...)	
 */
/*----------------------------------------------------------------------------*/
static double3 * detected_finepos_engine(
		image_t	*	in,
		int			xpos,
		int			ypos,
		int			radius,
		double		background)
{
	double3	*	fine ;
	image_t	*	local ;
    double  	corr_x, 
				corr_y ;
    double  	sum_weights ;
    double  	curpix ;
    double  	sq_radius ;
    double  	sq_rad ;
	int			loc_x_pos,
				loc_y_pos ;
	int			llx, lly, urx, ury ;			
    int     	i, j ;

	/* Initialize  */
	corr_x = corr_y = 0.00 ;
	sum_weights = 0.00 ;
	sq_radius = (double)(radius * radius) ;
	
	/* First extract the small image containing the point */
	llx = (int)(xpos - radius) ;
	lly = (int)(ypos - radius) ;
	urx = (int)(xpos + radius) ;
	ury = (int)(ypos + radius) ;
	/* Verify the positions */
	if (llx < 0) llx = 0 ;	
	if (lly < 0) lly = 0 ;
	if (urx >= in->lx) urx = in->lx - 1 ;
	if (ury >= in->ly) ury = in->ly - 1 ;
	/* Extract	 */
	if ((local = image_getvig(in, llx+1, lly+1, urx+1, ury+1)) == NULL) {
		e_error("cannot extract image") ;
		return NULL ;
	}
    
	/* Positions in extracted image */
	loc_x_pos = xpos - llx ;
	loc_y_pos = ypos - lly ;

	/* Compute the positions */
	for (j=0 ; j<local->ly ; j++) {
		for (i=0 ; i<local->lx ; i++) {
			sq_rad = (double)((i - loc_x_pos) * (i - loc_x_pos) +
				(j - loc_y_pos) * (j - loc_y_pos)) ;
			if (sq_rad <= sq_radius) {
				curpix = (double)local->data[i+j*local->lx] - background ;
				sum_weights += curpix ;
				corr_x += (double)i * curpix ;
				corr_y += (double)j * curpix ;
			}
		}
	}
	image_del(local) ;
	
	/* Create fine */
	fine = double3_new(1) ;	

	/* Normalize */
	if (fabs(sum_weights)>1e-10) {
		corr_x /= sum_weights ;
		corr_y /= sum_weights ;
		fine->x[0] = corr_x + llx ;
		fine->y[0] = corr_y + lly ;
	} else {
		double3_del(fine) ;
		return NULL ;
	}
	return fine ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute FWHM of all objects in a detected structure.
  @param	det		Detected structure to fill up.
  @param	ref		Image to examine.
  @return	void

  This function computes the FWHM for all objects contained into the passed 
  detected structure. It writes the results into the detected structure, 
  assuming the FWHM fields have already been allocated.
  It also computes the median FWHM of all objects in the image.
 */
/*----------------------------------------------------------------------------*/
int detected_compute_fwhm(detected * det, image_t * ref)
{
    pixelvalue  *   column,
                *   row ;
    pixelvalue      half_max ;
    double          thres_value ;
	int				j, k ;
    int         	nvalid ;
    double  	*   fwhmx,
            	*   fwhmy,
            	*   fwhma ;

    if (ref==NULL || det==NULL) return -1;
	if (det->nbobj<1) return -1 ;
	if ((det->max_x==NULL) || (det->max_y==NULL) || (det->max_i==NULL)) 
		return -1 ;

	/* Allocated storage */
	det->fwhm_x = calloc(det->nbobj, sizeof(double));
	det->fwhm_y = calloc(det->nbobj, sizeof(double));
	
	/* Compute FWHM on all objects */
	nvalid = 0 ;
	for (k=0 ; k<det->nbobj ; k++) {
		det->fwhm_x[k] = -1.0 ;
		det->fwhm_y[k] = -1.0 ;
		/* extract two arrays centered on the maximum */
		if ((row = image_getrow(ref, (int)det->max_y[k])) == NULL) continue ;
		if ((column = image_getcol(ref, (int)det->max_x[k])) == NULL) {
			free(row) ;
			continue ;
		}

		/* Find out threshold */
		thres_value =
			find_noise_level_around_peak(row, ref->lx, (int)det->max_y[k]) ;
		thres_value += 
			find_noise_level_around_peak(column, ref->ly, (int)det->max_x[k]) ;
		thres_value *= 0.5 ;
		half_max = (det->max_i[k] + thres_value ) * 0.5 ;

		if (half_max > det->max_i[k]) {
			free(row);
			free(column);
			continue;
		}

        det->fwhm_x[k] = get_fullwidth_on_y_linear(row,
												   ref->lx,
												   (int)det->max_x[k],
												   half_max);
        det->fwhm_y[k] = get_fullwidth_on_y_linear(column,
												   ref->ly,
												   (int)det->max_y[k],
												   half_max);
		if ((det->fwhm_x[k]>0) && (det->fwhm_y[k]>0)) nvalid ++ ;
		free(row) ;
		free(column) ;
	}
	
    /* Compute fwhm_medx fwhm_medy fwhm_meda */
    det->fwhm_medx = -1.0 ;
	det->fwhm_medy = -1.0 ;
	det->fwhm_meda = -1.0 ;
	if (nvalid<1) return -1 ;
    if (nvalid < 3) {
        e_warning("not enough values to compute a median") ;
    } else {
        /* Allocate the valid fwhm arrays */
        fwhmx = malloc(nvalid * sizeof(double)) ;
        fwhmy = malloc(nvalid * sizeof(double)) ;
        fwhma = malloc(nvalid * sizeof(double)) ;

        /* Fill the valid fwhm arrays */
        j=0 ;
        for (k=0 ; k<det->nbobj ; k++) {
            if ((det->fwhm_x[k]>0) && (det->fwhm_y[k]>0)) {
                fwhmx[j] = det->fwhm_x[k] ;
                fwhmy[j] = det->fwhm_y[k] ;
                fwhma[j] = (det->fwhm_x[k] + det->fwhm_y[k]) * 0.5 ;
                j++ ;
            }
        }
        /* Compute the median */
        det->fwhm_medx = double_median(fwhmx, nvalid);
        free(fwhmx);
        det->fwhm_medy = double_median(fwhmy, nvalid);
        free(fwhmy);
        det->fwhm_meda = double_median(fwhma, nvalid);
        free(fwhma);
    } 
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute image quality
  @param    det     Detected structure to examine.
  @param    pscale  Pixel scale in arcsec/pixel
  @param    srange  Seeing range in arcsec as an array of 2 doubles.
  @return   1 double, negative if error occurred.

  This function tries to estimate the image quality in an image.

  This function expects a detected structure with filled FWHM fields (fwhm_x, 
  fwhm_y), a pixel scale in arcsec/pixel and possibly a seeing range in arcsec,
  given as an array of 2 doubles (may be NULL).

  The algorithm is the following:

  - Reject all measurements for which fwhm_x differs from fwhm_y by more than 
    a pre-set threshold (see SEEING_FWHM_VAR).
  - Reject all measurements for which FWHM is outside of the given seeing range.
  - Return the median of the remaining fwhm_a values.

  The provided seeing range may be NULL, in which case a default seeing range 
  of 0.1 to 5 arcseconds will be used.
  This function returns a negative value in case of error.
 */
/*----------------------------------------------------------------------------*/
/* 20% variation allowed between fwhm_x and fwhm_y */
#define SEEING_FWHM_VAR     (0.2)
/* Default seeing range if none is provided */
#define SEEING_MIN_ARCSEC   (0.1)
#define SEEING_MAX_ARCSEC   (5.0)
double detected_compute_iq(
        detected    *   det,
        double          pscale,
        double      *   srange)
{
    double      fx, fy, fr ;
    double      f_min, f_max ;
    double  *   fwhm_keep ;
    double      iq ;
    int         i, j ;

    /* Bulletproof */
    if (det==NULL || pscale<1e-6) return -1 ;
    if (det->fwhm_x==NULL || det->fwhm_y==NULL) return -1 ;
    if (det->nbobj<1) return -1 ;

    if (srange==NULL) {
        f_min = SEEING_MIN_ARCSEC / pscale ;
        f_max = SEEING_MAX_ARCSEC / pscale ;
    } else {
        f_min = srange[0] / pscale ;
        f_max = srange[1] / pscale ;
    }
    /*
     * Reject all measurements for which the ratio between fwhm_x and
     * fwhm_y is above a given threshold, or the estimated seeing
     * is outside of the possible range.
     */
    fwhm_keep = malloc(det->nbobj * sizeof(double));
    j=0 ;
    for (i=0 ; i<det->nbobj ; i++) {

        fx = det->fwhm_x[i];
        fy = det->fwhm_y[i];
        fr = 2.0 * fabs(fx-fy) / (fx+fy) ;

        if ((fr < SEEING_FWHM_VAR) &&
            (fx < f_max) && (fx > f_min) && (fy < f_max) && (fy > f_min)) {
            fwhm_keep[j] = (fx+fy)*0.5 ;
            j++ ;
        }
    }

    if (j<1) {
        /* No value passed the tests */
        free(fwhm_keep);
        return -1 ;
    }

    if (j<3) {
        e_warning("not enough values to compute a median for iq");
        iq = pscale * fwhm_keep[0] ;
    } else {
        /* Compute median of the selected values */
        iq = pscale * double_median(fwhm_keep, j);
    }
    free(fwhm_keep);
    return iq ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute photometry of all objects in an image.
  @param	det			Detected struct
  @param	ref			Image to examine
  @param	phot_star	Photometry: star radius
  @param	phot_int	Photometry: internal background radius
  @param	phot_ext	Photometry: external background radius
  @return	void

  This function computes the photometry for each object declared into the 
  detected structure. It assumes the photometry arrays in the input detected 
  object to be already allocated.
 */
/*----------------------------------------------------------------------------*/
int detected_compute_phot(
		detected	*	det,
		image_t		*	ref,
		double			phot_star,
		double			phot_int,
		double			phot_ext)
{
	double xpos, ypos ;
    int    k ;

	if (det==NULL || ref==NULL) return -1 ;
	if (det->nbobj<1) return -1 ;
	if ((det->x == NULL) || (det->y == NULL)) return -1 ;

	/* Make sure FWHM have been computed */
	if ((det->fwhm_x == NULL) || (det->fwhm_y == NULL)) {
		if (detected_compute_fwhm(det, ref)!=0) {
			e_error("need FWHM to compute photometry");
			return -1 ;
		}
	}

	/* Allocate storage */
	det->obj_flux 		= calloc(det->nbobj, sizeof(double));
	det->obj_background = calloc(det->nbobj, sizeof(double));
    for (k=0 ; k<det->nbobj ; k++) {
		if ((det->fine_x != NULL) && (det->fine_y != NULL)) {
			xpos = det->fine_x[k] ;
			ypos = det->fine_y[k] ;
		} else {
			xpos = det->x[k] ;
			ypos = det->y[k] ;
		}
		det->obj_background[k] =
			image_get_disk_background(ref,
									  xpos,
									  ypos,
									  phot_int,
									  phot_ext,
									  BG_METHOD_MEDIAN) ;
		if (det->obj_background[k] < 0.0)
			det->obj_background[k] = 0.0;

        det->obj_flux[k] = image_get_disk_flux(ref,
                                xpos,
                                ypos,
                                phot_star,
                                det->obj_background[k]) ;
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Convert a detected object in a double3 object
  @param    det		detected object 
  @return	the double3 object	
 */
/*----------------------------------------------------------------------------*/
double3 * detected2double3(detected * det) 
{
	double3		*	out ;
	int				i ;

    if (det==NULL) return NULL ;
    if (det->nbobj<1) return NULL ;

	out = double3_new(det->nbobj) ;
	for (i=0 ; i<out->n ; i++) {
		out->x[i] = det->x[i] ;
		out->y[i] = det->y[i] ;
	}
	return out ;
}

