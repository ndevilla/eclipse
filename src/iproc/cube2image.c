/*-------------------------------------------------------------------------*/
/**
   @file	cube2image.c
   @author	Nicolas Devillard
   @date	Sept 14, 1995
   @version	$Revision: 1.33 $
   @brief	cube averaging to a single plane
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: cube2image.c,v 1.33 2002/07/31 14:35:20 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/07/31 14:35:20 $
	$Revision: 1.33 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "cube2image.h"
#include "median.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Average a cube to another cube or an image.
  @param	name_in			Name of the input file.
  @param	name_out		Name of the output file.
  @param	amethod			Cut method definition (see cube2image.h).
  @param	amethod			Average method definition (see cube2image.h).
  @param	cycle_step		Cycle step (see below).
  @param	run_hw			Running half-width (see below).
  @param	lo_rej			Number of low rejects.
  @param	hi_rej			Number of high rejects.
  @return	int 0 if Ok, -1 otherwise.

  This engine is actually calling cube_average() underneath after having
  loaded the input cube, and will save its result as indicated. See
  cube_average() for a complete doc about possible averaging methods and
  associated parameters.
 */
/*--------------------------------------------------------------------------*/
int average_engine(
		char		*	name_in,
		char		*	name_out,
		cut_method		cmethod,
		average_method	amethod,
		int				cycle_step,
		int				run_hw,
		int				lo_rej,
		int				hi_rej)
{
	cube_t		*	cube_in ;
	cube_t		*	cube_out ;

	/* Test inputs */
	if (name_in[0] == (char)0) {
		e_error("no input name was specified: aborting average") ;
		return -1 ;
	}
	if (name_out[0] == (char)0) {
		sprintf(name_out, "%s_avg.fits", get_rootname(name_in)) ;
	}

	/* Load input cube */
	cube_in = cube_load(name_in) ;
	if (cube_in == NULL) {
		e_error("cannot load cube [%s]: aborting average", name_in);
		return -1 ;
	}

	/* Apply average */
	if ((cube_out = cube_average(cube_in, cmethod, amethod, cycle_step, 
					run_hw, lo_rej, hi_rej)) == NULL) {
		e_error("cannot average the cube") ;
		return -1 ;
	}
		
	/* Save averaged cube */
	cube_save_fits_hdrcopy(cube_out, name_out, name_in) ;
	cube_del(cube_out) ;
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Average a cube
  @param	cube_in		Input cube to average
  @param	cmethod		Cut method
  @param	amethod		Average method
  @param	cycle_step	Cycle step for cut="cycle"
  @param	run_hw		Running half-width for cut="running"
  @param	lo_rej		Low rejection for method="filtered"
  @param	hi_rej		High rejection for method="filtered"
  @return	1 newly allocated cube

  Averages are separated in two dimensions: cut method and average
  method.

  Cutting tells which planes in the cube are to be considered. See 
  cube2image.h for details.

  Averaging over the third dimension can be done in a number of ways.
  Basically, the idea is that the output pixel is determined from a
  time line of input pixels lying at the same position on the
  detector. The way the output pixel is computed from the list of
  input pixels completely specifies the kind of average which is
  performed. See cube2image.h for details.

  The additional parameters run_hw, cycle_step, lo_rej and hi_rej are
  specific to each kind of averaging.

  Notice that all combinations are not yet implemented. Check out the
  source code to see if your specific average need is there.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_average(
		cube_t		*	cube_in,
		cut_method		cmethod,
		average_method	amethod,
		int				cycle_step,
		int				run_hw,
		int				lo_rej,
		int				hi_rej)
{
	cube_t		*	cube_out ;
	image_t 	*	image_out ;

	/* Test inputs */
	if (cube_in==NULL) return NULL ;
	 
	/* Case by case, apply one or another average method */
	if ((cmethod == cut_whole) && (amethod == avg_linear)) {
		image_out = cube_avg_linear(cube_in) ;
		cube_del(cube_in) ;
		if (image_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
		cube_out = cube_from_image(image_out) ;
		image_del(image_out) ;
	} else if ((cmethod == cut_whole) && (amethod == avg_median)) {
		image_out = cube_avg_median(cube_in) ;
		cube_del(cube_in) ;
		if (image_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
		cube_out = cube_from_image(image_out) ;
		image_del(image_out) ;
	} else if ((cmethod == cut_whole) && (amethod == avg_sum)) {
		image_out = cube_avg_sum(cube_in) ;
		cube_del(cube_in) ;
		if (image_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
		cube_out = cube_from_image(image_out) ;
		image_del(image_out) ;
	} else if ((cmethod == cut_whole) && (amethod == avg_filtered)) {
		image_out = cube_avg_reject(cube_in, lo_rej, hi_rej) ;
		if (image_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
        cube_del(cube_in) ;
        cube_out = cube_from_image(image_out) ;
        image_del(image_out) ;
	} else if ((cmethod == cut_cycle) && (amethod == avg_linear)) {
		cube_out = cube_avgcyc_linear(cube_in, cycle_step) ;
		cube_del(cube_in) ;
		if (cube_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
	} else if ((cmethod == cut_cycle) && (amethod == avg_sum)) {
		cube_out = cube_avgcyc_sum(cube_in, cycle_step) ;
		cube_del(cube_in) ;
		if (cube_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
	} else if ((cmethod == cut_cycle) && (amethod == avg_median)) {
		cube_out = cube_avgcyc_median(cube_in, cycle_step) ;
		cube_del(cube_in) ;
		if (cube_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
	} else if ((cmethod == cut_running) && (amethod == avg_linear)) {
		cube_out = cube_avgrun_linear(cube_in, run_hw) ;
		cube_del(cube_in) ;
		if (cube_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
	} else if ((cmethod == cut_running) && (amethod == avg_sum)) {
		cube_out = cube_avgrun_sum(cube_in, run_hw) ;
		cube_del(cube_in) ;
		if (cube_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
	} else if ((cmethod == cut_running) && (amethod == avg_median)) {
		cube_out = cube_avgrun_median(cube_in, run_hw) ;
		cube_del(cube_in) ;
		if (cube_out==NULL) {
			e_error("averaging cube: aborting");
			return NULL ;
		}
	} else {
		e_error("unsupported method: aborting average") ;
		return NULL ;
	}
	return cube_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Linear average over a whole cube to a single image.
  @param	incube	Cube to average.
  @return	Newly allocated image object.

  Probably the simplest and most intuitive average type: stack a whole
  cube to a single image.
 */
/*--------------------------------------------------------------------------*/
image_t	* cube_avg_linear(cube_t * incube)
{
	image_t	*	sum_image ;
	int				i ;
	pixelvalue		inv ;

	if (incube==NULL) return NULL ;
	e_comment(1, "averaging cube to one image") ;
	sum_image = image_new(incube->lx, incube->ly) ;
	if (sum_image==NULL) return NULL ;
	/* Loop on all planes */
	for (i=0 ; i<incube->np ; i++) {
		compute_status("linear averaging", i, incube->np, 2) ;
		image_add_local(sum_image, incube->plane[i]) ;
	}
	/* average on number of planes */
	inv = (pixelvalue)(1.0 / (double)incube->np) ;
	for (i=0 ; i<(sum_image->lx * sum_image->ly) ; i++) {
		sum_image->data[i] *= inv ; 
	}
	return(sum_image) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Median a cube with rejection.
  @param	incube		Cube to average.
  @param	lo_rej		Number of min pixels to reject.
  @param	hi_rej		Number of max pixels to reject.
  @return	Newly allocated image object.

  This median averaging applies to the whole cube. Every time line is
  extracted, sorted out, then the lowest and highest values are
  rejected, and the median of the rest is found to yield the output
  pixel.

  Rejection levels are given as a number of pixels to reject on each side.
 */
/*--------------------------------------------------------------------------*/
image_t * cube_avg_medreject(
		cube_t	*	incube,
		int			lo_rej,
		int			hi_rej)
{
	image_t	*		avg ;
	int				pos ;
	int				i, j ;
	pixelvalue		rejavg ;

	/* Error handling: test entries	*/
	if (incube==NULL) return NULL ;
	if ((lo_rej+hi_rej)>=incube->np)
		return NULL ;
	if (incube->np-lo_rej-hi_rej<3) {
		e_error("not enough planes in cube to apply rejection");
		return NULL ;
	}
	if (lo_rej<0) lo_rej=0 ;
	if (hi_rej<0) hi_rej=0 ;

	avg = image_new(incube->lx, incube->ly) ;
	for (j=0 ; j<incube->ly ; j++) {
		compute_status("median averaging with rejection", j, incube->ly, 1) ;
		for (i=0 ; i<incube->lx ; i++) {
			pos = i + j * incube->lx ;
			rejavg = cube_z_medreject(incube, pos, lo_rej, hi_rej) ;
			avg->data[pos] = rejavg ;
		}
	}
	return avg ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Median-average pixel values on a time line, with rejection.
  @param	in_cube		Input cube to use for average computation.
  @param	pos			Detector position to use for averaging.
  @param	lo_rej		Number of min pixels to reject.
  @param	hi_rej		Number of max pixels to reject.
  @return	1 pixelvalue.

  This function takes a cube in input and a detector position. The
  detector position is a single number, expected to be of the form
  i+j*lx, where (i,j) is the position on the detector and lx the image
  width.
 */
/*--------------------------------------------------------------------------*/
pixelvalue cube_z_medreject(
		cube_t	*	in_cube, 
		int 		pos, 
		int 		lo_rej, 
		int 		hi_rej)
{
	int				plane ;
	pixelvalue	*	timeline ;
	double			acc_val ; 

	/* Test entries */
	if (in_cube == NULL) return -1 ;
	if (in_cube->np < 3) return -1 ;
	if ((lo_rej+hi_rej) >= in_cube->np) return -1 ;
	
	timeline = calloc(in_cube->np, sizeof(pixelvalue)) ;

	/* Get all the pixel values along timeline */
	for (plane=0 ; plane<in_cube->np ; plane++) {
		timeline[plane] = in_cube->plane[plane]->data[pos] ;
	}
	
	/* Now sort out the timeline for this pixel	*/
	pixel_qsort(timeline, in_cube->np) ;
	acc_val = 0.0 ;

	/* Get the middle values, reject lower and upper pixel proportion */
	acc_val = median_pixelvalue(&timeline[lo_rej], in_cube->np-lo_rej-hi_rej) ;
	free(timeline) ;
	return (pixelvalue)acc_val ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Average a cube with rejection.
  @param	incube		Cube to average.
  @param	lo_rej		Number of min pixels to reject.
  @param	hi_rej		Number of max pixels to reject.
  @return	Newly allocated image object.

  This averaging applies to the whole cube. Every time line is
  extracted, sorted out, then the lowest and highest values are
  rejected, and the rest is linearly averaged to yield the output
  pixel.
 */
/*--------------------------------------------------------------------------*/
image_t * cube_avg_reject(
		cube_t	*	incube, 
		int 		lo_rej, 
		int 		hi_rej)
{
	image_t		*	avg ;
	int				pos ;
	int				i, j ;
	pixelvalue		rejavg ;

	/* Error handling: test entries	*/
	if (incube==NULL) return NULL ;
	if ((lo_rej+hi_rej)>=incube->np)
		return NULL ;
	if (incube->np-lo_rej-hi_rej<3) {
		e_error("not enough planes in cube to apply rejection");
		return NULL ;
	}
	if (lo_rej<0) lo_rej=0 ;
	if (hi_rej<0) hi_rej=0 ;

	avg = image_new(incube->lx, incube->ly) ;
	for (j=0 ; j<incube->ly ; j++) {
		compute_status("averaging with rejection", j, incube->ly, 1) ;
		for (i=0 ; i<incube->lx ; i++) {
			pos = i + j * incube->lx ;
			rejavg = cube_z_reject(incube, pos, lo_rej, hi_rej) ;
			avg->data[pos] = rejavg ;
		}
	}
	return avg ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Average pixel values on a time line, with rejection.
  @param	in_cube		Input cube to use for average computation.
  @param	pos			Detector position to use for averaging.
  @param	lo_rej		Number of min pixels to reject.
  @param	hi_rej		Number of max pixels to reject.
  @return	1 pixelvalue.

  This function takes a cube in input and a detector position. The
  detector position is a single number, expected to be of the form
  i+j*lx, where (i,j) is the position on the detector and lx the image
  width.
 */
/*--------------------------------------------------------------------------*/
pixelvalue cube_z_reject(
		cube_t	*	in_cube, 
		int 		pos, 
		int 		lo_rej, 
		int 		hi_rej)
{
	int				plane ;
	pixelvalue	*	timeline ;
	double			acc_val ; 
	int				i ;
	int				nv ;

	/* Test entries */
	if (in_cube == NULL) return -1 ;
	if (in_cube->np < 3) {
		e_error("median average has no meaning with less than 3 planes");
		return -1 ;
	}
	if (lo_rej+hi_rej>=in_cube->np) return -1 ;
	
	timeline = calloc(in_cube->np, sizeof(pixelvalue)) ;

	/* Get all the pixel values along timeline */
	for (plane=0 ; plane<in_cube->np ; plane++) {
		timeline[plane] = in_cube->plane[plane]->data[pos] ;
	}
	
	/* Now sort out the timeline for this pixel	*/
	pixel_qsort(timeline, in_cube->np) ;
	acc_val = 0.0 ;

	/*
	 * Get only the middle values, rejecting the lower and upper pixel
	 * proportion as requested
	 */
	nv = 0 ;
	for (i=lo_rej ; i<(in_cube->np - hi_rej) ; i++) {
		acc_val += (double)timeline[i] ;
		nv++ ;
	}
	free(timeline) ;
	acc_val /= (double)nv ;
	return (pixelvalue)acc_val ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Sum a cube to a single image.
  @param	incube	Cube to sum.
  @return	Newly allocated image object.

  The output image is a sum of all planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
image_t	* cube_avg_sum(cube_t * incube)
{
	image_t	*	sum_image ;
	int				i ;

	/* Test entries */
	if (incube==NULL) return NULL ;

	e_comment(1, "averaging cube to one image") ;
	sum_image = image_new(incube->lx, incube->ly) ;
	if (sum_image == NULL) return NULL ;
	/* Loop on all planes */
	for (i=0 ; i<incube->np ; i++) {
		compute_status("sum averaging", i, incube->np, 2) ;
		image_add_local(sum_image, incube->plane[i]) ;
	}
	return(sum_image) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Average a cube to a median image.
  @param	to_average	Cube to average.
  @return	Newly allocated image object.

  The returned image is a median image of the input cube. See the
  convention for median used in the case of an even number of
  elements, in math/median.c
 */
/*--------------------------------------------------------------------------*/
image_t * cube_avg_median(cube_t * to_average)
{
	image_t	*	avg ;
	int				plane, nplanes;
	int				i, j, pos, offset ;
	pixelvalue	*	timeline ;

	/* Error handling: test entries	*/
	if (to_average == NULL) return NULL ;
	if (to_average->np<3) {
	    e_error("median average has no meaning with less than 3 planes") ;
	    return NULL ;
	}

	avg = image_new(to_average->lx, to_average->ly) ;
	nplanes = to_average->np;
	timeline = calloc(nplanes, sizeof(pixelvalue)) ;

	for (j=0 ; j<to_average->ly ; j++) {
	     offset = j*to_average->lx ;
	     for (i=0 ; i<to_average->lx ; i++) {
	          pos = i + offset;
		  for (plane=0 ; plane<nplanes ; plane++) {
		       timeline[plane] = to_average->plane[plane]->data[pos] ;
		  }
		  avg->data[pos] = median_pixelvalue(timeline, nplanes) ;
	     }
	}
	free(timeline);
	return avg ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Cycle average a cube linearly.
  @param    incube  Cube to average.
  @param    cycle   Cycle length.
  @return   Newly allocated cube.
 
  The cube is averaged by cycles: sub-cubes of 'cycle' planes are
  extracted from the input cube and each sub-cube is linearly
  averaged.
 
  Example: the input cube has 120 planes, the cycle is 30, the output
  cube will have 120/3=4 planes, each plane being an average of 30
  consecutive planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_avgcyc_linear(
		cube_t	* 	incube, 
		int 		cycle)
{
    cube_t   *	avg_cube;
    int       	i, j, k ;
	pixelvalue	inv ;
	int			nbpix ;

	/* error handling: test entries */
	if (incube == NULL) return NULL;
	if ((cycle<1) || (cycle>incube->np)) {
		e_error("illegal cycle step [%d]: aborting", cycle) ;
		return NULL ;
	}
	
	/* Get number of planes and check it fits the cycle step	*/
    if (incube->np % cycle != 0) {
            e_warning("The number of planes in the cube is not a multiple of") ;
            e_warning("the block size you've given the last incomplete block") ;
            e_warning("will be ignored") ;
	}
    avg_cube = cube_new(incube->lx, incube->ly, incube->np / cycle);
	inv = (pixelvalue)(1.0 / (double)cycle) ;
	for (i=0 ; i<avg_cube->np ; i++) {
		avg_cube->plane[i] = image_copy(incube->plane[i*cycle]) ;
		for (j=1 ; j<cycle ; j++) {
			image_add_local(avg_cube->plane[i], incube->plane[i*cycle+j]) ;
		}
		nbpix = avg_cube->plane[i]->lx * avg_cube->plane[i]->ly ;
		for (k=0 ; k<nbpix ; k++) {
			avg_cube->plane[i]->data[k] *= (pixelvalue)inv ;
		}
	}
    return avg_cube ;    
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Cycle average a cube with sums.
  @param    incube  Cube to average
  @param    cycle   Cycle length.
  @return   Newly allocated cube.
 
  The cube is averaged by cycles: sub-cubes of 'cycle' planes are
  extracted from the input cube and each sub-cube is summed to a
  single plane.
 
  Example: the input cube has 120 planes, the cycle is 30, the output
  cube will have 120/3=4 planes, each plane being a sum of 30
  consecutive planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_avgcyc_sum(
		cube_t	*	incube, 
		int 		cycle)
{
    cube_t   *	avg_cube;
    int       	i, j ;

	/* error handling: test entries */
	if (incube == NULL) return NULL ;
	if ((cycle<1) || (cycle>incube->np)) {
		e_error("illegal cycle step [%d]: aborting", cycle) ;
		return NULL ;
	}
	
	/* Get number of planes and check it fits the cycle step	*/
    if (incube->np % cycle != 0) {
            e_warning("The number of planes in the cube is not a multiple of") ;
            e_warning("the block size you've given the last incomplete block") ;
            e_warning("will be ignored") ;
	}
    avg_cube = cube_new(incube->lx, incube->ly, incube->np / cycle);
	for (i=0 ; i<avg_cube->np ; i++) {
		avg_cube->plane[i] = image_copy(incube->plane[i*cycle]) ;
		for (j=0 ; j<cycle ; j++) {
			image_add_local(avg_cube->plane[i], incube->plane[i*cycle+j]) ;
		}
	}
    return(avg_cube) ;    
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Cycle median average a cube.
  @param    incube  Cube to average
  @param    cycle   Cycle length.
  @return   Newly allocated cube.
 
  The cube is averaged by cycles: sub-cubes of 'cycle' planes are
  extracted from the input cube and each sub-cube is median averaged
  to a single plane.
 
  Example: the input cube has 120 planes, the cycle is 30, the output
  cube will have 120/3=4 planes, each plane being a median average of
  30 consecutive planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_avgcyc_median(
		cube_t	*	incube, 
		int 		cycle)
{
    cube_t	*	avg_cube,
			*	intermediate ;
	int			prev_verb ;
    int       	i, j ;

    /* error handling: test entries */
	if (incube==NULL) return NULL ;
	if ((cycle<1) || (cycle>incube->np)) {
		e_error("illegal cycle step [%d]: aborting", cycle) ;
		return NULL ;
	}

	/* Get number of planes and check it fits the cycle step	*/
    if (incube->np % cycle != 0) {
            e_warning("The number of planes in the cube is not a multiple of") ;
            e_warning("the block size you've given the last incomplete block") ;
            e_warning("will be ignored") ;
	}
    avg_cube = cube_new(incube->lx, incube->ly, incube->np / cycle);

	for (i=0 ; i<avg_cube->np ; i++) {
		compute_status("computing cycle median...", i, avg_cube->np, 1);
		intermediate = cube_new(incube->lx, incube->ly, cycle) ;
		for (j=0 ; j<cycle ; j++) {
			intermediate->plane[j] = incube->plane[i*cycle+j] ;
		}
		/* Disable verbose during average */
		prev_verb = verbose_active();
		set_verbose(0);
		avg_cube->plane[i] = cube_avg_median(intermediate);
		set_verbose(prev_verb);
		cube_del_shallow(intermediate);
		if (avg_cube->plane[i]==NULL) {
			e_error("during cycle median average: aborting");
			cube_del(avg_cube);
			return NULL ;
		}
	}
    return avg_cube ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the median value of a pixel position along time.
  @param    in_cube     Cube from which the line is extracted.
  @param    pos         Position on the detector as i+j*lx.
  @return   Median pixelvalue along this time line.
 
  Provide a cube and a detector position as one number: i+j*lx, where
  (i,j) specifies the position on the detector with the C convention
  (i and j start at zero). The returned value is the median of the
  pixels on this detector position along time.
 
 */
/*--------------------------------------------------------------------------*/
pixelvalue cube_z_median(
		cube_t		*	in_cube,
		int				pos)
{
	int				plane ;
	pixelvalue	*	timeline ;
	pixelvalue		median_value ; 

	/* Test entries */
	if (in_cube==NULL) return (pixelvalue)-1 ;
	if (in_cube->np < 3) {
		e_error("median extraction has no meaning with less than 3 values");
		return (pixelvalue)-1 ;
	}

	timeline = calloc(in_cube->np, sizeof(pixelvalue)) ;

	/* Get all the pixel values along timeline */
	for (plane=0 ; plane<in_cube->np ; plane++) {
		timeline[plane] = in_cube->plane[plane]->data[pos] ;
	}
	
	/* Now sort out the timeline for this pixel	*/
	median_value = median_pixelvalue(timeline, in_cube->np) ; 
	free(timeline) ;
	return median_value ;
}
		

/*-------------------------------------------------------------------------*/
/**
  @brief	Running linear average of a cube.
  @param	incube		Input cube.
  @param	half_cycle	Half cycle definition.
  @return	1 newly allocated cube.

  A running average is computing plane averages along the cube.
  The returned cube has as many planes as the input cube. Each output
  plane is an average of the planes around [-halfcycle, +halfcycle]
  around the current plane position. For the beginning and end planes,
  only existing plane positions are taken into account.

  Example: a running average of a cube containing 7 planes, with a
  halfcycle of 2, would be done as follows:
  \begin{verbatim}

    output plane:       average of input planes:
    1                   1 2 3
    2                   1 2 3 4
    3                   1 2 3 4 5
    4                     2 3 4 5 6
    5                       3 4 5 6 7
    6                         4 5 6 7
    7                           5 6 7
  \end{verbatim}

  As can be seen, the number of planes truly used to compute one given
  output plane is between halfcycle+1 (on the edges) and 2*halfcycle+1
  (away from the edges). In this example, only planes 3, 4, and 5 will
  be computed using 2*halfcycle+1 frames.

  This function is not optimized for speed to cover the greatest
  possible number of cases. Since it needs to browse the input cube
  back and forth, it is CPU and memory intensive.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_avgrun_linear(
 		cube_t	*	incube,
		int			half_cycle)
{
	cube_t		*	outcube ;
	int				from, to ;
	int				i, j, k ;
	pixelvalue		inv ;
	int				nbpix ;

	e_comment(1, "running linear average on cube") ;
	if (half_cycle > incube->np-1) {
		e_error("half cycle is too big: %d (%d planes in cube)",
				half_cycle, incube->np) ;
		return NULL ;
	}
	outcube = cube_new(incube->lx, incube->ly, incube->np);

	for (i=0 ; i<incube->np ; i++) {

        compute_status("running linear average", i, incube->np, 2) ;

		from = i - half_cycle ;
		to   = i + half_cycle ;

		if (from<0) from=0 ;
		if (to>incube->np-1) to=incube->np-1 ;

		inv = (pixelvalue)1.00 ;

		outcube->plane[i] = image_copy(incube->plane[from]) ;
		for (j=from+1 ; j<=to ; j++) {
			image_add_local(outcube->plane[i], incube->plane[j]) ;
			inv += 1.0 ;
		}
		inv = (pixelvalue)(1.00 / (double)inv) ;
		nbpix = outcube->plane[i]->lx * outcube->plane[i]->ly ;
		for (k=0 ; k<nbpix ; k++) {
			outcube->plane[i]->data[k] *= (pixelvalue)inv ;
		}

    }
	return outcube ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Running sum of a cube.
  @param	incube		Cube to sum.
  @param	half_cycle	Half cycle definition.
  @return	1 newly allocated cube.

  See above function cube_avgrun_linear for a description of
  a running filter. This variant of average is only summing planes,
  not performing a linear average.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_avgrun_sum (
		cube_t	*	incube,
		int			half_cycle)
{
	cube_t		*	outcube ;
	int				from, to ;
	int				i, j ;

	e_comment(1, "running linear average on cube") ;
	if (half_cycle > incube->np-1) {
		e_error("half cycle is too big: %d (%d planes in cube)",
				half_cycle, incube->np) ;
		return NULL ;
	}
	outcube = cube_new(incube->lx, incube->ly, incube->np);

	for (i=0 ; i<incube->np ; i++) {

        compute_status("running sum average", i, incube->np, 2) ;

		from = i - half_cycle ;
		to   = i + half_cycle ;

		if (from<0) from=0 ;
		if (to>incube->np-1) to=incube->np-1 ;

		outcube->plane[i] = image_copy(incube->plane[from]) ;
		for (j=from+1 ; j<=to ; j++) {
			image_add_local(outcube->plane[i], incube->plane[j]) ;
		}
    }
	return outcube ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Running median average of a cube.
  @param	incube		Cube to median average.
  @param	half_cycle	Half cycle definition.
  @return	1 newly allocated cube.

  See above function cube_avgrun_linear for a description of
  a running filter. This variant of average uses a median average for
  each batch of planes.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_avgrun_median(
		cube_t	*	incube,
		int			half_cycle)
{
	cube_t		*	outcube ;
	cube_t		*	intermediate ;
	int				i , j;
	int				from, to ;
	int				prev_verb ;

	e_comment(1, "running linear average on cube") ;
	if (half_cycle > incube->np-1) {
		e_error("half cycle is too big: %d (%d planes in cube)",
				half_cycle, incube->np) ;
		return NULL ;
	}
	outcube = cube_new(incube->lx, incube->ly, incube->np);

	for (i=0 ; i<incube->np ; i++) {

        compute_status("running median average", i, incube->np, 2) ;

		from = i - half_cycle ;
		to   = i + half_cycle ;

		if (from<0) from=0 ;
		if (to>incube->np-1) to=incube->np-1 ;

		intermediate = cube_new(incube->lx, incube->ly, (to-from+1)) ; 
		for (j=from ; j<=to ; j++) {
			intermediate->plane[j-from] = incube->plane[j] ;
		}
		/* Disable verbose during the average */
		prev_verb = verbose_active();
		set_verbose(0);
		outcube->plane[i] = cube_avg_median(intermediate);
		set_verbose(prev_verb);
		cube_del_shallow(intermediate);
		if (outcube->plane[i]==NULL) {
			e_error("computing running median: aborting");
			cube_del(outcube);
			return NULL ;
		}
    }
	return outcube ;
}
