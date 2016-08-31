/*-------------------------------------------------------------------------*/
/**
   @file	spectro_detect.c
   @author	Thomas Rogon
   @date	October 1999
   @version	$Revision: 1.34 $
   @brief	spectroscopy routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: spectro_detect.c,v 1.34 2001/11/23 15:04:07 yjung Exp $
	$Author: yjung $
	$Date: 2001/11/23 15:04:07 $
	$Revision: 1.34 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "spectro_detect.h"
#include "spectral_lines.h"

/*-------------------------------------------------------------------------
					Prototypes for private functions
 -------------------------------------------------------------------------*/

static int valid_spectrum(int, int, detected *, int, image_t *, int, 
		spec_detect_mode_t) ;
static int select_valid_spectra(int, int, detected *, int *, int**, int, 
		image_t *, spec_detect_mode_t) ;
static detected * detect_spectra_1d(image_t *, int *, int **, int, 
		spec_detect_mode_t) ;

/*---------------------------------------------------------------------------
   								Define
 ---------------------------------------------------------------------------*/

#define  	MIN_NB_SPECS		1
#define  	TRESH_MEDIAN_MIN	0.0
#define  	TRESH_SIGMA_MAX    	200.0

#define		TRESH_SIGMA_MIN  	1.0
#define 	THRESH_NOISE_LEV 	0.6
#define		MIN_THRESH_FACT	 	0.9
#define		MAX_THRESH_FACT	 	1.1
#define		MEDIAN_SIZE		 	5

/*---------------------------------------------------------------------------
   								Function code
 ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/**
  @brief	Helper function to select_valid_spectra 
  @param 	min_spec_length 	Minimal spectrum length
  @param	max_spec_width		Maximum spectrum width
  @param	det					detected object
  @param 	char_dist 			dist. to the two "negative" shadows 
  @param 	in 					spectral image 
  @param	objnum				Relevant spectrum identifier
  @param	spec_detect_mode	spec_detect_mode_t object
  @return 1 if valid 0 if not
  
  Spectra are assumed to be horizontal
  C-style coordinates are assumed.
*/
/*---------------------------------------------------------------------------*/
static int valid_spectrum(
		int					min_spec_length,
		int					max_spec_width,
		detected		*	det,
		int 				char_dist,
		image_t			*	in,
		int					objnum,
		spec_detect_mode_t	spec_detect_mode)
{
	int			darkover,
				darkunder ;
	int			objlen   = -1,
				objwidth = -1 ;
	double		valover,
				valunder,
				valcenter ;

	/* Find out objlen and objwidth */
	objlen = det->right_x[objnum] - det->left_x[objnum] + 1 ;
	objwidth = det->top_y[objnum] - det->bottom_y[objnum] + 1 ;
	if (objwidth > max_spec_width) {
		e_error("object is too wide") ;
		return 0 ;
	}
	
	/* Object is too small */
	if (det->obj_nbpix[objnum] < 2) return 0 ;

	/* char_dist==0 => the no shadow required */
	if (!char_dist) return 1 ;

	/* Get the median of the object (valcenter) */
	valcenter = det->obj_median[objnum] ;

	/* Get the black shadows positions and medians */
	/* Over position */
	darkover = (int)(det->y[objnum] - char_dist) ;
	if (darkover < 0) darkover = in->ly - char_dist ;
	else darkover = - char_dist ;
	/* Under position */
	darkunder = (int)(det->y[objnum] + char_dist) ;
	if (darkunder >= in->ly) darkunder = char_dist - in->ly ;
	else darkunder = char_dist ;
	/* Get the black shadows medians (valunder and valover) */
	valunder = image_getmedian_vig(in,
								   det->bottom_x[objnum]+1,
								   det->bottom_y[objnum]+darkunder,
								   det->top_x[objnum]+1,
								   det->top_y[objnum]+darkunder);
	valover  = image_getmedian_vig(in,
								   det->bottom_x[objnum]+1,
								   det->bottom_y[objnum]+darkover,
								   det->top_x[objnum]+1,
								   det->top_y[objnum]+darkover);

	switch(spec_detect_mode){
	
		case EQUALLY_SPACED_SHADOW_SPECTRA:	
		if ((valunder < -fabs(valcenter/SPEC_SHADOW_FACT)) && 
			(valover < -fabs(valcenter/SPEC_SHADOW_FACT))	&&
			(valunder/valover > 0.5) &&
			(valunder/valover < 2.0)) return 1 ;
		else return 0 ;
		
		case ONE_SHADOW_SPECTRUM:
		if ((valunder < -fabs(valcenter/SPEC_SHADOW_FACT)) || 
			(valover < -fabs(valcenter/SPEC_SHADOW_FACT))) return 1 ;
		else return 0 ;

		case NO_SHADOW_SPECTRUM:
		return 1 ;
		
		default:
		e_error("unknown spec_detect_mode %d", (int)spec_detect_mode) ;
		break ;
	}
	
	return 0 ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	Selects spectra in a spectral image.
  @param 	min_spec_length		minimal spectrum length
  @param	max_spec_width 		maximal spectrum width
  @param	det					detected object						
  @param	label_image			labeled image
  @param 	validspecs 			the number of vertical specs found
  @param 	lut_spec_2_cc	lut giving the object number of a found spectrum.
  @param 	char_dist 		the characteristic length i.e. the distance to the
  							two "negative" shadows of the bright spectrum
  @param 	in 				spectral image
  @param	spec_detect_mode	spec_detect_mode_t object

  Spectra are supposed to be horizontal
  A valid spectrum has length >min_spec_length and width < max_spec_length 
  and two shadow (negative spectra) at the characteristic distance.
  If the characteristic distance is zero, the last condition is ignored.
  C-style coordinates are assumed.
*/
/*---------------------------------------------------------------------------*/
static int select_valid_spectra(
		int					min_spec_length,
		int					max_spec_width,
		detected		*	det,
		int 			*	validspecs,
		int 			**	lut_spec_2_cc,
		int 				char_dist,
		image_t			*	in,
		spec_detect_mode_t	spec_detect_mode)
{
	int *	tl ;
	int 	nobjs ;
	int		i, j ;

	/* Initialize */
	*lut_spec_2_cc = NULL ;
	nobjs = det->nbobj ;
	*validspecs = 0 ;
	tl = NULL ;

	if (nobjs < 1) return -1 ;
	
	/* Count nb of valid specs */	
	/* Identify valid specs */	
	j = 0 ;
	for (i=0 ; i<nobjs ; i++)
		if (valid_spectrum(min_spec_length,
							max_spec_width, 
							det, 
							char_dist, 
							in,
							i,
							spec_detect_mode)) (*validspecs)++ ;
	
	/* Associate to each spectrum, its object number */
	if (*validspecs) {
		*lut_spec_2_cc = calloc(*validspecs, sizeof(int)) ;
		tl = *lut_spec_2_cc ;
		for (i=0 ; i<nobjs ; i++) 
			if (valid_spectrum(min_spec_length,
								max_spec_width, 
								det, 
								char_dist, 
								in,
								i,
								spec_detect_mode)) *tl++ = i ;
	} else return -1 ;
	
	return 0 ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	Detects vertical or horizontal specs in a collapsed spectral image
  @param 	in 				1d	spectral image 
  @param 	n_specs 		the number of spectra found (return param)
  @param 	lut_spec_2_cc	lut giving the connected component number of a 
  							detected spectrum (return param)
  @param	label_image		label image (return param)
  @param 	char_dist 		the characteristic length i.e. the distance to the
  							two "negative" shadows of the bright spectrum
  @param	spec_detect_mode	spec_detect_mode_t object						
  @return	detected object 

  Spectra are supposed to be horizontal.
  Algorithm:\begin{enumerate}
  \item   Clear bad zones
  \item   1-D median filter in detection direction
  \item   Thresholding
  \item   labelisation
  \item   Detection / Rejection of invalid clusters
  \end{enumerate}
*/
/*---------------------------------------------------------------------------*/
static detected * detect_spectra_1d(
		image_t			*	in,
		int				*	n_specs,
		int 			**	lut_spec_2_cc,
		int					char_dist,
		spec_detect_mode_t	spec_detect_mode)
{
	image_t		*	filt_img,
				*	image1d ;
	pixelvalue	*	lowpassline ;
	detected	*	det ;
	pixelmap	*	thresh ;
	intimage	*	lab ;
	int				nobj ;
	double			threshold,
					sigma ;
	double			thresh_fact ;
	double			median_val;
	image_stats	*	st ;
	int				ret ;
	
	/* Default values for output parameters */
	*lut_spec_2_cc = NULL ;
	
	/* Filters */
	/* Get rid of very high frequencies  */
	filt_img = image_filter_vertical_median(in, MEDIAN_SIZE) ;
	/* Subtract low frequency signal */
	lowpassline = image_getmedian_mov_vert(filt_img, 0, SPEC_PARAM_WINDOWSIZE) ;
	/* Create image1d */
	image1d = image_new(1, filt_img->ly) ;
	memcpy(image1d->data, lowpassline, filt_img->ly * sizeof(pixelvalue)) ;
	free(lowpassline) ;
	/* Subtraction */
	image_sub_1d_local(filt_img, image1d) ;
	image_del(image1d) ;    

    /* Get relevant stats for thresholding */
    median_val = image_median_stat(filt_img, &sigma) ;
    st = image_getstats(filt_img) ;

    /* Correct median_val and sigma if necessary  */
    if (median_val < TRESH_MEDIAN_MIN) median_val = TRESH_MEDIAN_MIN ;
    thresh_fact = sigma / st->stdev ;
    if (sigma > TRESH_SIGMA_MAX) sigma = TRESH_SIGMA_MAX ;
    if (thresh_fact > THRESH_NOISE_LEV) {
        sigma = st->stdev ;
        if (sigma < TRESH_SIGMA_MIN) sigma = TRESH_SIGMA_MIN ;
    } else if (sigma < TRESH_SIGMA_MIN) sigma = st->stdev ;

    /* Set the threshold */
    threshold = median_val + sigma * (1.0 + thresh_fact) ;
    if (threshold > MIN_THRESH_FACT * st->max_pix)
        threshold = MIN_THRESH_FACT * st->max_pix ;
    if (threshold < MAX_THRESH_FACT * st->avg_pix)
        threshold = MAX_THRESH_FACT * st->avg_pix ;
    free(st) ;

    /* Binarize the image */
    thresh = image_threshold2pixelmap(filt_img, threshold, MAX_PIX_VALUE);
	if (thresh==NULL) {
		image_del(filt_img) ;
        return NULL ;
    }

    /* Test if there are enough good pixels */
    if (thresh->ngoodpix < 1) {
        e_error("not enough signal to detect spectra");
        pixelmap_del(thresh) ;
		image_del(filt_img) ;
        return NULL ;
    }

	/* Detect objects and compute stats on them */
	lab = intimage_labelize_pixelmap(thresh, &nobj);
    pixelmap_del(thresh) ;
	if ((det = detected_compute_objstat(in, lab, nobj)) == NULL) {
		e_error("cannot compute objects statistics") ;
		image_del(filt_img) ;
		intimage_del(lab);
		return NULL ;
	}
	intimage_del(lab);
	
	/* Select only relevant specs, create corresponding LUT's */	
	ret = select_valid_spectra(	0,
								SPEC_MAXWIDTH,
								det, 
								n_specs,
								lut_spec_2_cc,
								char_dist, 
								filt_img,
								spec_detect_mode);
	image_del(filt_img) ;
	if (ret==-1) {
		detected_del(det) ;
		return NULL ;
	}
	
	if (*n_specs < MIN_NB_SPECS) { 
        free(*lut_spec_2_cc) ;
		*lut_spec_2_cc = NULL ; 
		detected_del(det) ;
		det = NULL ;
	}

	return det ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	Finds the brightest spectrum in an image 
  @param 	in 		spectral image with spectra conforming to the other param.
  @param 	main_offset_diff the characteristic diff. bet. pos. and neg. spec.
  @param 	spec_detect_mode the spectrum detection method 
  @param 	min_bright 		min. bright. required for a spec. to be detected.
  @return The coordinates of the found spectrum, NULL if error.

  Finds the brightest spectrum in an image by collapsing the image
  orthogonally to the spectrum orientation.
  Spectra are assumed to be horizontal.
  C-style convention for coordinates is used, i.e.  top,left=0,0.
*/
/*---------------------------------------------------------------------------*/
double3 * find_brightest_spectrum_1d(
		image_t			*	in,
		int 				main_offset_diff,
		spec_detect_mode_t	spec_detect_mode,
		double				min_bright)
{
	image_t		*	collapsed ;
	detected	*	det ;
	double3		*	p ;
	int				nb_spec ;
	int				brightest ;
	double			brightness ;
	int 		*	lut_spec_2_cc ;
	int				i ;
	
	/* Initialize */
	lut_spec_2_cc = NULL ;

	/* Collapse the image */
	if ((collapsed = image_collapse_median(in, 1, 0, 0)) == NULL) {
		e_error("collapsing image: aborting spectrum detection");
		return NULL ;
	}

	/* Spectrum detection */
	det = detect_spectra_1d(collapsed, 
							&nb_spec,
							&lut_spec_2_cc, 
							main_offset_diff,
							spec_detect_mode) ;
	image_del(collapsed) ;
	if (det==NULL) return NULL ;

	/* Allocate space for the brightest spectrum position */
	p = double3_new(1) ;

	/* Look for the brightest, among the detected spectra */
	p->x[0] = det->x[lut_spec_2_cc[0]] ;
	p->y[0] = det->y[lut_spec_2_cc[0]] ;
	brightest = lut_spec_2_cc[0] ;
	brightness = det->obj_mean[lut_spec_2_cc[0]] *
				det->obj_nbpix[lut_spec_2_cc[0]] ;
	for (i=0 ; i<nb_spec ; i++) {
		if (det->obj_mean[lut_spec_2_cc[i]] *
				det->obj_nbpix[lut_spec_2_cc[i]] > brightness) {
			p->x[0] = det->x[lut_spec_2_cc[i]] ;
			p->y[0] = det->y[lut_spec_2_cc[i]] ;
			brightest = lut_spec_2_cc[i] ;
			brightness = det->obj_mean[lut_spec_2_cc[i]] *
						det->obj_nbpix[lut_spec_2_cc[i]] ;
		}
	}
	detected_del(det) ;
	free(lut_spec_2_cc) ;

	/* Minimum brightness required */
	if (brightness < min_bright) {
		e_comment(1,"brightness %f too low <%f", brightness, min_bright) ;
		double3_del(p) ;
		return NULL ;
	}

	/* Return */
	return p;
}



