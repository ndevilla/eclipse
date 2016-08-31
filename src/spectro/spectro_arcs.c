/*-------------------------------------------------------------------------*/
/**
   @file	spectro_arcs.c
   @author	T.Rogon
   @date	October 1999
   @version	$Revision: 1.43 $
   @brief	spectroscopy routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: spectro_arcs.c,v 1.43 2003/05/15 07:44:30 yjung Exp $
	$Author: yjung $
	$Date: 2003/05/15 07:44:30 $
	$Revision: 1.43 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <limits.h>
#include "spectro_arcs.h"
#include "spectral_lines.h"

/*-------------------------------------------------------------------------
					Prototypes for private functions
 -------------------------------------------------------------------------*/
static double3 * find_arc_fine_pos(image_t *, intimage *, int *, int) ;
static int threshold_one_dim(image_t *, pixelvalue, pixelvalue *, int,
		pixelvalue) ;
static int select_valid_arcs(int, int, int, detected *, int *, int **, int **) ;
static double3 ** get_positions(image_t *, int, int *, intimage *, 
		detected *, int) ;
static detected * detect_arcs(image_t *, int *, int **, int **, 
		intimage **, int, int, int, int, int) ;
static void mask_obj(image_t *, image_t *, intimage *, int) ;
static int get_extreme_obj_coor(detected *, int *, int, int *, int *, int *, 
		int *) ;

/*---------------------------------------------------------------------------
                                Defines
 ---------------------------------------------------------------------------*/

#define	ARC_RANGE_FACT		3.0
#define	TRESH_MEDIAN_MIN	0.0
#define	TRESH_SIGMA_MAX  	200.0

/*---------------------------------------------------------------------------
                                Function code
 ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/**
  @brief	Refine the position of an arc. 
  @param 	in				gray image
  @param 	label_image		corresponding label image
  @param 	arc_start_pos	starting coordinate of the arc (y for horiz. arcs)
  @param 	arc_middle_pos	coord. of the arc middle (x for horiz. arcs)	
  @param 	position 		the computed fine position of the arc  
  @return new position 

  The fine arc position is a gravity center.
  The arc_start_pos parameter is a pointer whose value is increased until
  the arc limit is reached.
  ARCS are supposed to be VERTICAL
*/
/*---------------------------------------------------------------------------*/
static double3 * find_arc_fine_pos(
		image_t		*	in,
		intimage 	*	label_image, 
		int 		*	arc_start_pos,
		int 			arc_middle_pos)
{
	double3	*	position ;
	int			objnum ;
	int			curr_obj ;
	int			start_pos ;
	double 		grav_c ;
	double		sum ;
	double		max ;
	double		val ;
	int			maxpos ;
	int			im_extrem ;
	double		arc_pos ;

	/* Test of input parameters */
	if (arc_start_pos == NULL) return NULL ;
	if (*arc_start_pos < 0) {
		e_error("illegal start position: arc_start_pos=%d", *arc_start_pos) ;
		return NULL ;
	}
	
	/* Initialize */
	grav_c = 0.0 ;
	sum    = 0.0 ;
	start_pos = *arc_start_pos ;
	maxpos = start_pos ;
	max    = in->data[arc_middle_pos * in->lx + start_pos] ;
	objnum = label_image->data[arc_middle_pos*label_image->lx+start_pos] ;
	im_extrem = in->lx ;
	
	/* While we stay in the same object... */
	do {
		val = in->data[arc_middle_pos * in->lx + *arc_start_pos] ;
		
		if (*arc_start_pos == 0) grav_c = 0.0 ;
		else grav_c += *arc_start_pos * val ;
		
		sum += val ;
		if (val > max) {
			max = val ;
			maxpos = *arc_start_pos ;
		}

		/* Next point */
		(*arc_start_pos)++ ;
	
		/* Test if the bordure is reached */
		if (*arc_start_pos >= im_extrem) {
			e_warning("border reached objnum %d : %d", objnum, *arc_start_pos) ;
			break ;
		}
		
		curr_obj = label_image->data[arc_middle_pos*in->lx + *arc_start_pos] ;
	} while (curr_obj == objnum) ;

	/* Returned position is the gravity center or the max in bad cases */
	if ((fabs(grav_c) < 1.0e-40) || (fabs(sum) < 1.0e-40)) {
		arc_pos = maxpos ;
	} else {
		arc_pos = grav_c / sum ;
		if (fabs(arc_pos) >= *arc_start_pos) arc_pos = maxpos ;
	}

	/* Allocate position */
	position = double3_new(1) ;
	
	/* Give the final arc fine position */
	position->x[0] = arc_pos ;
	position->y[0] = arc_middle_pos ;
	return position ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	Selects arcs in a spectral image according to given criteria.
  @param 	min_arc_length	Minimal arc length
  @param 	max_arc_width	maximal arc length
  @param    arc_sat         Arc saturation value
  @param 	det				detected object 
  @param	validarcs		array to identify the valid arcs
  @param 	arc2obj		 	look-up table giving the object nb of a det arc
  @param 	obj2arc		 	look-up table giving the arc number of an object 
  @return 	n_arcs 			the number of valid arcs found
	
  C-style coordinates are assumed (i.e. bottom,left=0,0)
  A valid arc has a length > min_arc_length and width < max_arc_width.
  Indexes (relative to connected component numbers) of valid objects are 
  returned in cc2arc, their number in validarcs
  Arcs are supposed to be vertical
*/
/*---------------------------------------------------------------------------*/
static int select_valid_arcs(
		int				min_arc_length,
		int				max_arc_width,
        int             arc_sat,
		detected	*	det,
		int 		*	validarcs,
		int 		**	obj2arc,
		int 		**	arc2obj)
{
	int 		*	tl ;
	double			arclen,
					archeight,
					edge ;
	int				i,
					j ;

	/* Allocate look-up table between objects and arcs */
	*obj2arc = malloc(det->nbobj*sizeof(int)) ;
	
	/* Count the number of valid arcs and update obj2arc array */	
	/* A valid arc is long enough, and is not saturated */
	*validarcs = 0 ;
	j = 0 ;
	for (i=0 ; i<det->nbobj ; i++) {
		arclen = det->top_y[i] - det->bottom_y[i] + 1 ;
		archeight = det->right_x[i] - det->left_x[i] + 1 ;
		edge = det->left_x[i] ;
	
		/* Test if the current object is a valid arc */
		if ((arclen>min_arc_length) && (archeight<max_arc_width) 
				&& (edge>0) && (det->obj_mean[i] < arc_sat)) {
			(*obj2arc)[i] = j++ ;
			(*validarcs)++ ;
		} else {
			(*obj2arc)[i] = -1 ;
		}
	}
			
	/* No valid arc found */
	if (!(*validarcs)) return 0 ;

	/* Allocate look-up table between arcs and objects */
	*arc2obj = calloc(*validarcs, sizeof(int)) ;

	/* Associate to each arc, its object number */
	tl = *arc2obj ;
	for (i=0 ; i<det->nbobj ; i++) {
		arclen = det->top_y[i] - det->bottom_y[i] + 1 ;
		archeight = det->right_x[i] - det->left_x[i] + 1 ;
		edge = det->left_x[i] ;
		/* Test if the current object is a valid arc */
		if ((arclen>min_arc_length) && (archeight<max_arc_width) 
				&& (edge>0) && (det->obj_mean[i]<arc_sat)) *tl++ = i ;
	}
	return 0 ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	Creates a 2-D deformation grid for polynomial fitting
  @param 	in 				input image
  @param	n_arcs 			the number of vertical arcs found
  @param 	obj2arc		 	a lut giving the arc number of an object
  @param 	label_image		corresponding label_image 
  @param	det				detected object
  @param	nb_samples		nb of samples i.e. horizontal cuts
  @return 	a 2-D array n_arcs * nb_samples for subsequent poly fitting

  Creates a grid based on maxima along the arcs. The grid is irregular
  i.e. n_calib equally spaced samples between each arcs individual
  top and bottom.
  Arcs are assumed vertical
*/
/*---------------------------------------------------------------------------*/
static double3 ** get_positions(
		image_t		*	in,
		int				n_arcs,
		int 		*	obj2arc,
		intimage	*	label_image, 	
		detected	*	det,
		int				nb_samples)
{
	image_t 	*	filt_img ;
	double3		**	pos ;
	double3		*	position ;
	int			*	arc_coord ;		
	int			*	obj_done ;
	int			*	calib ; 
	double			arclen ;
	int		 		use_this_arc ;
	int				obj ;
	int 			i,
					j,
					k ;
	
	/* Allocate positions (pos. of n_arcs*nb_samples pts on the arcs) */
	pos = calloc(n_arcs, sizeof(double3*)) ;
	for (i=0 ; i<n_arcs ; i++) pos[i] = double3_new(nb_samples) ;
	
	/* Median filter on input image */
	filt_img = image_filter_median(in) ;
	
	/* Allocate memory	 */
	/* Measured Arcs coordinates along curvature */
	arc_coord = malloc(det->nbobj*nb_samples*sizeof(int)) ;   
	/* Arcs already found along transversal */
	obj_done  = calloc(det->nbobj, sizeof(int)) ;
	/* Sample number along a given arc */
	calib     = calloc(det->nbobj, sizeof(int)) ;
	
	/* Find out the Y coordinates along the arcs  */
	for (j=0 ; j<det->nbobj; j++) {
		arclen = det->top_y[j] - det->bottom_y[j] ;
		for (i=0 ; i<nb_samples ; i++) {
			arc_coord[i+j*nb_samples] = (int)
			(det->bottom_y[j] + (arclen*(i+0.5)) / (double)nb_samples);
		}
	}
	
	/* Find out the X coord. at nb_samples Y positions on all arcs */
	for (i=0 ; i<label_image->ly ; i++) {
		for (j=0 ; j<label_image->lx ; j++) {
			/* use_this_arc is set to 1 if we are on the arc at a y */
			/* coordinate where the x coord should be found */
			obj = label_image->data[i * label_image->lx + j] ;
			/* Handle background */
			if (obj==0)
				continue ;
			else
				/* Decrease by one to index the array from 0 */
				obj-- ;

			use_this_arc = 0 ;
			for (k=0 ; k<nb_samples ; k++) {
				if (arc_coord[k+obj*nb_samples] == i) {
					use_this_arc = 1 ;
					break ; 
				}
			}

			/* a=obj2arc[b] : a is an arc id or -1, b is any object  */
			if (use_this_arc && obj>=0 && (obj2arc[obj] != -1)) {
				/* Find x coordinate of obj at the calib[obj] Y coord. */
				if ((position = find_arc_fine_pos(	filt_img, 
													label_image, 
													&j, 
													i)) == NULL) {  
					e_error("cannot find fine arc position") ;
					image_del(filt_img);
					free(calib);
					free(obj_done);
					free(arc_coord);
					for (i=0 ; i<n_arcs ; i++) double3_del(pos[i]) ;
					free(pos) ;
					return NULL ;
				} else {
					pos[obj2arc[obj]]->x[calib[obj]] = position->x[0] ;
					pos[obj2arc[obj]]->y[calib[obj]] = position->y[0] ;
					double3_del(position) ;
					obj_done[obj] = 1 ;
				}
			}
		}

		/* Do not search twice a X coord at the same Y for the same arc	 */
		for (j=0 ; j<det->nbobj; j++) {
			if ((obj_done[j]) && (calib[j] < nb_samples)) calib[j]++ ;
			/* Re-initialize obj_done */
			obj_done[j] = 0 ;
		}
	}

	/* Free and return */
	image_del(filt_img);
	free(calib);
	free(obj_done);
	free(arc_coord);
	return pos ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief 	Thresholds rows/columns in an image 
  @param 	in 				input image 
  @param	threshold 		binarization theshold
  @param	line 			a test line
  @param 	orientation 	arc orientation (0 for VERTICAL, 1 for HORIZONTAL) 
  @param	val				replacement value
  @return 	0 if OK, -1 if not

  The image is modified destructively.
*/
/*---------------------------------------------------------------------------*/
static int threshold_one_dim(
		image_t 		*	in,
		pixelvalue			threshold,
		pixelvalue		*	line,
		int					orientation,
		pixelvalue			val)
{
	int 			i, 
					j ;

	switch(orientation) {
		case 0:
		for (i=0 ; i<in->lx ; i++)
			if (line[i] < threshold) {
				for (j=0 ; j<in->ly ; j++) in->data[j*in->lx+i] = val ;
			}
		break ;
	
		case 1:
		for (j=0 ; j<in->ly ; j++) 
			if (line[j] < threshold) {
				for (i=0 ; i<in->lx ; i++) in->data[j*in->lx+i] = val ;
			}
		break;
		
		default:
		e_error("threshold_one_dim: unknown orientation %d", orientation) ;
		return -1 ;
	}

	return 0 ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief 	Detect vertical or horizontal arcs in a spectral image
  @param 	in 				input image containing arcs
  @param 	n_arcs 			the number of arcs found
  @param 	arc2obj		 	lut giving the object number of a detected arc 
  @param 	obj2arc		 	lut giving the arc number of an object  
  @param	label_image		label image (return param)
  @param    arc_sat         Arc saturation value
  @param	xmin		
  @param	ymin			Define the region of interest
  @param	xmax
  @param	ymax
  @return	det				detected object 

  Algorithm:

  - Clear bad zones
  - 1-D median filter in detection direction
  - Thresholding
  - Labelisation 
  - Detection / Rejection of invalid clusters

  The arcs are assumed to be vertical
*/
/*---------------------------------------------------------------------------*/
static detected * detect_arcs(
		image_t		*	in,
		int			*	n_arcs,
		int 		**	arc2obj,
		int 		**	obj2arc,
		intimage	**	label_image,
        int             arc_sat,
		int				xmin,
		int				ymin,
		int				xmax,
		int				ymax)
{
	image_t 	*	filt_img,
				*	collapsed ;
	pixelmap	*	thresh ;
	pixelvalue 		threshold,
					fillval ;
	double			median_val,
					sigma ;
	int				min_arclen = 0 ;
	detected	*	det ;
	int				nobj ;
	
	/* Default values for output parameters */
	*arc2obj      = NULL ;
	*obj2arc      = NULL ;
	*label_image  = NULL ;
	
	/* Clear zones to be ignored (to avoid false detections) */	
	median_val = image_median_stat(in, &sigma) ;
	fillval = median_val-sigma/2.0 ;
	if (ymin>0) image_fillrect(in, fillval, 0, in->lx-1, 0, ymin) ;
	if (ymax<in->ly-1) 
        image_fillrect(in, fillval, 0, in->lx-1, ymax, in->ly-1) ;
	if (xmin>0) image_fillrect(in, fillval, 0, xmin, 0, in->ly-1) ;
	if (xmax<in->lx-1) 
        image_fillrect(in, fillval, xmax, in->lx-1, 0, in->ly-1) ;
	
	/* Median filter */
	filt_img = image_filter_vertical_median(in, ARC_MEDIAN_SIZE) ; 

	/* Subtract a low-pass */
	if (image_sub_lowpass(filt_img, 0, ARC_WINDOWSIZE) == -1) {
		image_del(filt_img) ;
		return NULL ;
	}
	
	/* Get relevant stats for thresholding */	
    median_val = image_median_stat(filt_img, &sigma) ;

	/* Collapse the image */
	collapsed = image_collapse_median(filt_img, 0, 0, 0) ;

	/* Correct median_val and sigma if necessary */
	if (median_val < TRESH_MEDIAN_MIN) median_val = TRESH_MEDIAN_MIN ;
	if (sigma > TRESH_SIGMA_MAX) sigma = TRESH_SIGMA_MAX ;
	
	/* Set the threshold */
	threshold = median_val + sigma*ARC_THRESHFACT ;

	/* Threshold to keep only the arcs - use of the collapsed image */	
	if (threshold_one_dim(filt_img,
							median_val,
							collapsed->data,
							0,
							0.0) == -1) {
		image_del(filt_img) ;
		image_del(collapsed);
		return NULL ;
	}
	image_del(collapsed);

	/* Binarize the image */
	thresh = image_threshold2pixelmap(filt_img, threshold, MAX_PIX_VALUE) ; 
	image_del(filt_img) ;
	if (thresh == NULL) return NULL ;
	
	/* Test if there are enough good pixels */
	if (thresh->ngoodpix < ARC_MINGOODPIX) {
		e_error("too few (%d) white pixels", thresh->ngoodpix) ;
		pixelmap_del(thresh) ;
		return NULL ;
	}

	/* Labelize pixel map to a label image */
	*label_image = intimage_labelize_pixelmap(thresh, &nobj) ;
	pixelmap_del(thresh) ;

	/* Compute statistics on objects */
	if ((det = detected_compute_objstat(in, *label_image, nobj)) == NULL) {
		e_error("cannot compute objects stats") ;
		intimage_del(*label_image) ;
		*label_image = NULL ;
		return NULL ;
	}

	/* Set min_arclen */
	min_arclen = (int)(in->ly/ARC_MINARCLENFACT) ;

	/* Select only relevant arcs, create corresponding LUT's */	
	if (select_valid_arcs(min_arclen,
							ARC_MAXARCWIDTH,
                            arc_sat,
							det, 
							n_arcs,
							obj2arc, 
							arc2obj) == -1) {
		intimage_del(*label_image) ;
		*label_image = NULL ;
		detected_del(det) ;
		return NULL ;
	}
	if (*n_arcs < ARC_MINNBARCS) {
		intimage_del(*label_image) ;
		*label_image = NULL ;
		if (*obj2arc != NULL) free(*obj2arc) ;
		*obj2arc = NULL ; 
        if (*arc2obj != NULL) free(*arc2obj) ;
		*arc2obj = NULL ; 
		detected_del(det) ;
		return NULL ;
	}
	return det ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief 	Computes the inverse dist polynomium of an image containing arcs
  @param 	in 	grey level image containing arcs i.e deformed parallel lines 
  @param	xmin
  @param	ymin		the region to consider
  @param	xmax
  @param	ymax
  @param    arc_sat Arc saturation value
  @param	nb_arcs	Nb of arcs used for calibration
  @param	arcs	Arcs positions
  @return a 2-D polynomial poly2d

  Computes the inverse distortion polynomium c, x, y, xy, x^2, y^2 in order 
  of appearance.
  C-style convention for coordinates is used, i.e.  top,left=0,0.
  The input image is assumed dark subtracted.
  Arcs are assumed vertical
 */ 
/*---------------------------------------------------------------------------*/
poly2d * compute_distortion(
		image_t		    *	in,
		int					xmin,
		int					ymin,
		int					xmax,
		int					ymax,
        int                 arc_sat,
		int				*	nb_arcs,
		double			**	arcs)
{
	double		*	distpoly ;
	poly2d		*	poly_u ;

	/* Apply the distortion computation engine */
	if ((distpoly = dist_engine(in, 
							xmin,
							ymin,
							xmax,
							ymax,
                            arc_sat,
							nb_arcs,
							arcs)) == NULL) {
		return NULL ;
	}
	
	/* Construct fill and return the output object (poly2d) */
	poly_u=malloc(sizeof(poly2d)) ;
	poly_u->nc = 6 ; 
	poly_u->px=calloc(6, sizeof(int)) ;
	poly_u->py=calloc(6, sizeof(int)) ;
	poly_u->px[0] = 0 ;
	poly_u->px[1] = 1 ;
	poly_u->px[2] = 0 ;
	poly_u->px[3] = 1 ;
	poly_u->px[4] = 2 ;
	poly_u->px[5] = 0 ;
	poly_u->py[0] = 0 ;
	poly_u->py[1] = 0 ;
	poly_u->py[2] = 1 ;
	poly_u->py[3] = 1 ;
	poly_u->py[4] = 0 ;
	poly_u->py[5] = 2 ;
	poly_u->c = distpoly ;
	return poly_u ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	This is the low level function called by compute_distortion
  @param 	in 			the arc image
  @param	xmin
  @param	ymin		the region to consider
  @param	xmax
  @param	ymax
  @param    arc_sat Arc saturation value
  @param	nb_arcs	Nb of arcs used for calibration
  @param	arcs	Arcs positions
  @return 	a 2-D polynomial of size 6 double
	
  Input image is assumed dark subtracted
  Arcs are assumed vertical
*/
/*---------------------------------------------------------------------------*/
double * dist_engine( 
		image_t 		*	org,
		int					xmin,
		int					ymin,
		int					xmax,
		int					ymax,
        int                 arc_sat,
		int				*	nb_arcs,
		double			**	arcs)
{
	double3		**	grid2d ;
	double3		*	lamgrid ;
	detected	*	det ;
	intimage	*	label_image ; 
	image_t		*	in,
				* 	masked,
				*	collapsed ;
	double		* 	refgrid ;
	double 		*	distpoly ;
	double			mean_squared_error ;
	int				n_arcs ;
	int			*	arc2obj,
				*	obj2arc ;
	int				ncoeffs ;
	int				min_arc_range ;
	int				left, 
					right, 
					top, 
					bottom ;
	int				grid_refpoint ;
	int				n_calib ;
	
	int 			i, j ;

	/* Initialize */
	grid_refpoint = ARC_GRID_REF_1DMAX ;
	n_calib = ARC_NBSAMPLES ;					
	
	/* Test input data */
	if (org == NULL) return NULL ;

	/* Local copy of input image */
	in = image_copy(org);
	
	/* Detect the arcs in the input frame */
	e_comment(2, "detecting arcs");
	if ((det = detect_arcs(in,
						&n_arcs,
						&arc2obj,
						&obj2arc,
						&label_image,
                        arc_sat,
						xmin,
						ymin,
						xmax,
						ymax)) == NULL) {
		image_del(in);
		return NULL;
	}
	
	/* Find out the limit objects positions */
	get_extreme_obj_coor(det, arc2obj, n_arcs, &left, &right, &top, &bottom) ;

	/* Abort if the detected arcs are too concentrated in the same zone */
	min_arc_range = (int)(in->lx / ARC_RANGE_FACT) ;
	if (right-left < min_arc_range) {
		e_error("too narrow range (%d-%d)<%d", right, left, min_arc_range) ;
		intimage_del(label_image) ;
		free(obj2arc) ;
		free(arc2obj) ;
		detected_del(det) ;
		image_del(in) ;
		return NULL ;
	}
	
	/* Create a 2-D deformation grid with detected arcs */
	e_comment(2, "creating deformation grid");
	grid2d = get_positions(	in, 
							n_arcs, 
							obj2arc, 
							label_image, 
							det, 
							n_calib) ;
	free(obj2arc);
	if (grid2d==NULL) {
		intimage_del(label_image);
		free(arc2obj) ;
		detected_del(det) ;
		image_del(in);
		return NULL;
	}
	
	/* refgrid contains the arc real positions, ie the Xs of lamgrid */
	refgrid = calloc(n_arcs, sizeof(double));
	
	switch(grid_refpoint) {
		
	case ARC_GRID_REF_1DMAX:
		masked = image_copy(in);
		
		for (i=0 ; i<n_arcs ; i++) {
			/* Keep only the current arc - mask the others */
			mask_obj(in, masked, label_image, arc2obj[i]);
		
			/* Collapse the image */
			collapsed = image_collapse(masked, 0);
			
			/* First estimation of the current arc position */
			refgrid[i] = det->x[arc2obj[i]] ;
			
			/* Refining of the current arc position */
			if ((refgrid[i] > 5) && (refgrid[i] < in->lx-5)) {
				refgrid[i] += function1d_find_centroid( 
						&(collapsed->data[(int)refgrid[i]-5]), 11) - 5 ;
			}
			image_del(collapsed) ;
		}
		image_del(masked) ;
		break;
		
	case ARC_GRID_REF_GRAV_CENT:
		
		for (i=0 ; i<n_arcs ; i++) refgrid[i] = det->x[arc2obj[i]] ;
		break;
	
	default:
		e_error("unknown grid reference point type %d", grid_refpoint) ;
		free(refgrid) ;
		for (i=0 ; i<n_arcs ; i++) double3_del(grid2d[i]) ;
		free(grid2d) ;
		free(arc2obj) ;
		detected_del(det) ;
		image_del(in) ;
		intimage_del(label_image);
		return NULL;
	}
	intimage_del(label_image);
	free(arc2obj) ;
	detected_del(det) ;

	if ((nb_arcs != NULL) && (arcs != NULL)) {
		/* Fill arcs array */
		*nb_arcs = n_arcs ;
		*arcs = malloc(*nb_arcs * sizeof(double)) ;
		for (i=0 ; i<*nb_arcs ; i++) {
			(*arcs)[i] = refgrid[i] ;
		}
	}

	/* lamgrid is (x,y,X) where (x,y) is the arc coord., X is the ideal pos */
	lamgrid = double3_new(n_calib * n_arcs);
	
	/* Fill lamgrid with refgrid and grid2d */
	for (i=0 ; i<n_arcs ; i++) {
		for (j=0 ; j<n_calib ; j++) {
			lamgrid->x[i*n_calib+j] = refgrid[i] ;
			lamgrid->y[i*n_calib+j] = grid2d[i]->y[j] ; 
			lamgrid->z[i*n_calib+j] = grid2d[i]->x[j] ;
		}
		double3_del(grid2d[i]) ;
	}
	free(grid2d);
	free(refgrid);

	/* Compute the 2D polynomial fit */
	e_comment(2, "computing surface fit");
	distpoly = fit_surface_polynomial(lamgrid,
									"(0,0) (1,0) (0,1) (1,1) (2,0) (0,2)",
									2,
									&ncoeffs, 
									&mean_squared_error) ;
	
	/* Free and return the 2D polynomial */
	double3_del(lamgrid) ;
	image_del(in) ;
	return distpoly ;
}


/*---------------------------------------------------------------------------*/
/**
  @brief	masks (i.e. sets to maskval) areas not occurring in objlist
  @param	in      input grey image
  @param	out     masked output image (may be the same as in)
  @param	l       corresponding label image
  @param	objid	Label of object to keep.
  @return 	0 if OK -1 if not
*/
/*---------------------------------------------------------------------------*/
static void mask_obj(
		image_t    *   in,
		image_t    *   out,
		intimage   *   l,
		int			   objid)
{
    int i ;
	for (i=0 ; i<in->lx * in->ly ; i++) {
		if (l->data[i]==(1+objid)) {
			out->data[i] = in->data[i] ;
		} else {
			out->data[i] = (pixelvalue)0;
		}
	}
    return ;
}


/*--------------------------------------------------------------------------*/
/**
  @brief  Find bounding rectangle of a list of objects
  @param  det                 detected object 
  @param  selected_objlist    selected objects (NULL for all)
  @param  nselectedobjs       number of selected objects 
  @param  left                pointer to leftmost coordinate
  @param  right               pointer to rightmost coordinate 
  @param  top                 pointer to topmost coordinate
  @param  bottom              pointer to bottommost coordinate
  @return 0 if OK -1 if not
  
  Finds the extreme coordinates of a list of
  objects, i.e the corrdinates of the box bounding all objects 
  in the list.
*/  
/*--------------------------------------------------------------------------*/
static int get_extreme_obj_coor(
		detected *   det, 
		int      *   selected_objlist,
		int          nselectedobjs,
		int      *   left,
		int      *   right,
		int      *   top, 
		int      *   bot)
{   
    int     i, l, r, b, t ;

    if (det == NULL) return -1 ;
    if (det->nbobj <= 0) {
        e_error("get_extreme_obj_coor: nobjs %d", det->nbobj) ;
        return -1 ;
    }
    if (selected_objlist == NULL) {
        l = det->left_x[0] ;
        r = det->right_x[0] ;
        t = det->top_y[0] ;
        b = det->bottom_y[0] ;
        for (i=0 ; i<det->nbobj ; i++) {
            if (det->left_x[i] < l) l = det->left_x[i] ;
            if (det->right_x[i] > r) r = det->right_x[i] ;
            if (det->bottom_y[i] < b) b = det->bottom_y[i] ;
            if (det->top_y[i] > t) t = det->top_y[i] ;
        }
    } else {
        if (!nselectedobjs) return -1 ;
        i = selected_objlist[nselectedobjs-1] ;
        l = det->left_x[i] ;
        r = det->right_x[i] ;
        t = det->top_y[i] ;
        b = det->bottom_y[i] ;
        while (--nselectedobjs) {
            i = selected_objlist[nselectedobjs] ;
            if (det->left_x[i] < l) l = det->left_x[i] ;
            if (det->right_x[i] > r) r = det->right_x[i] ;
            if (det->bottom_y[i] < b) b = det->bottom_y[i] ;
            if (det->top_y[i] > t) t = det->top_y[i] ;
        }
    }

    *left = l ;
    *right = r ;
    *bot = b ;
    *top = t ;

    return 0 ;
}


