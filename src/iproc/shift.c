/*----------------------------------------------------------------------------*/
/**
   @file	shift.c
   @author	Y. Jung
   @date	Jan. 2001
   @version	$Revision: 1.24 $
   @brief	Shift related routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: shift.c,v 1.24 2003/04/11 08:32:54 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/11 08:32:54 $
	$Revision: 1.24 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "shift.h"
#include "resampling.h"
#include "doubles.h"
#include "dstats.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift an image by a given (non-integer) 2d offset.
  @param    image_in        Image to shift.
  @param    shift_x         Shift in x.
  @param    shift_y         Shift in y.
  @param    interp_kernel   Interpolation kernel to use.
  @return   1 newly allocated image.

  This function shifts an image by a non-integer offset, using
  interpolation. You can either generate an interpolation kernel once and
  pass it to this function, or let it generate a default kernel. In the
  former case, use generate_interpolation_kernel() to generate an
  appropriate kernel. In the latter case, pass NULL as last argument. A
  default interpolation kernel is then generated then discarded before this
  function returns.

  The returned image is a newly allocated object, it must be deallocated
  using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t * image_shift(
		image_t	*	image_in,
		double			shift_x,
		double			shift_y,
		double		*	interp_kernel)
{
    image_t    *   shifted ;
    pixelvalue  *   first_pass ;
    pixelvalue  *   second_pass ;
    int             samples = KERNEL_SAMPLES ;
    int             i, j ;
    double           fx, fy ;
    double           rx, ry ;
    int             px, py ;
    int             tabx, taby ;
    double           value ;
    int	          pos ;
    register pixelvalue     *   pix ;
    register pixelvalue     *   pixint ;
    int             mid;
    double          norm ;
    double       *  ker ;
    int             freeKernel = 1 ;

    /* error handling: test entries */
    if (image_in==NULL) return NULL ;

    /* Shifting by a zero offset returns a copy of the input image */
    if ((fabs(shift_x)<1e-2) && (fabs(shift_y)<1e-2))
        return image_copy(image_in) ;

    /* See if a kernel needs to be generated */
    if (interp_kernel == NULL) {
        ker = generate_interpolation_kernel("default") ;
        if (ker == NULL) {
            e_error("kernel generation failure: aborting resampling") ;
            return NULL ;
        }
    } else {
        ker = interp_kernel ;
        freeKernel = 0 ;
    }

    mid = (int)samples/(int)2 ;
    first_pass = calloc(image_in->lx, image_in->ly*sizeof(pixelvalue)) ;
    shifted = image_new(image_in->lx, image_in->ly) ;
    second_pass = shifted->data ;

    pix = image_in->data ;
    for (j=0 ; j<image_in->ly ; j++) {
        for (i=1 ; i<image_in->lx-2 ; i++) {
            fx = (double)i-shift_x ;
            px = (int)fx ;
            rx = fx - (double)px ;

            pos = px + j * image_in->lx ;

            if ((px>1) && (px<(image_in->lx-3))) {
                tabx = (int)(fabs((double)mid * rx)) ;
                /* 
                 * Sum up over 4 closest pixel values, 
                 * weighted by interpolation kernel values
                 */
                value =     (double)pix[pos-1] * ker[mid+tabx] +
                            (double)pix[pos] * ker[tabx] +
                            (double)pix[pos+1] * ker[mid-tabx] +
                            (double)pix[pos+2] * ker[samples-tabx-1] ;
                /*
                 * Also sum up interpolation kernel coefficients
                 * for further normalization
                 */
                norm =      (double)ker[mid+tabx] +
                            (double)ker[tabx] +
                            (double)ker[mid-tabx] +
                            (double)ker[samples-tabx-1] ;
                if (fabs(norm) > 1e-4) {
                    value /= norm ;
                }
            } else {
                value = 0.0 ;
            }
            /* 
             * There may be a problem of rounding here if pixelvalue
             * has not enough bits to sustain the accuracy.
             */
            first_pass[i+j*image_in->lx] = (pixelvalue)value ;
        }
    }
    pixint = first_pass ;
    for (i=0 ; i<image_in->lx ; i++) {
        for (j=1 ; j<image_in->ly-3 ; j++) {
            fy = (double)j - shift_y ;
            py = (int)fy ;
            ry = fy - (double)py ;
            pos = i + py * image_in->lx ;

            taby = (int)(fabs((double)mid * ry)) ;

            if ((py>(int)1) && (py<(image_in->ly-2))) {
                /* 
                 * Sum up over 4 closest pixel values, 
                 * weighted by interpolation kernel values
                 */
                value = (double)pixint[pos-image_in->lx] * ker[mid+taby] +
                        (double)pixint[pos] * ker[taby] +
                        (double)pixint[pos+image_in->lx] * ker[mid-taby] +
                        (double)pixint[pos+2*image_in->lx]*ker[samples-taby-1];
                /*
                 * Also sum up interpolation kernel coefficients
                 * for further normalization
                 */
                norm =      (double)ker[mid+taby] +
                            (double)ker[taby] +
                            (double)ker[mid-taby] +
                            (double)ker[samples-taby-1] ;

                if (fabs(norm) > 1e-4) {
                    value /= norm ;
                }
            } else {
                value = 0.0 ;
            }
            second_pass[i+j*image_in->lx] = (pixelvalue)value ;
        }
    }

    free(first_pass) ;
    if (freeKernel)
        free(ker) ;
    return shifted ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift an image by an integer 2d offset.
  @param    image_in    Image to shift.
  @param    shift_x     Shift in X.
  @param    shift_y     Shift in Y.
  @return   1 newly allocated image.

  Shifts an image by an integer offset. The returned object is a newly
  allocated image, it must be deallocated using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t * image_shift_int(
		image_t    	*   image_in,
		int             shift_x,
		int             shift_y)
{
    image_t  * shifted ;
    int         i, j ;

    if (image_in==NULL) return NULL ;
    if ((shift_x==0) && (shift_y==0))
        return image_copy(image_in);
    shifted = image_new(image_in->lx, image_in->ly) ;
    for (j=0 ; j<image_in->ly ; j++) {
        for (i=0 ; i<image_in->lx ; i++) {
            if (((i+shift_x)>0) &&
                ((i+shift_x)<image_in->lx) &&
                ((j+shift_y)>0) &&
                ((j+shift_y)<image_in->ly)) {
                shifted->data[(i+shift_x)+(j+shift_y)*image_in->lx] =
                    image_in->data[i+j*image_in->lx] ;
            }
        }
    }
    return shifted ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @return   int 0 if Ok, -1 otherwise.
 
  Runs cube_shift_int_slice over the whole cube.
 */
/*----------------------------------------------------------------------------*/
int cube_shift_int(
        cube_t  *   to_shift,
        double3 *   offsets)
{
	/* No multithreading support yet */
	return cube_shift_int_slice(to_shift, offsets, -1, -1);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @param	from_p		Index of first plane to shift.
  @param	to_p		Index of last plane to shift.
  @return   int 0 if Ok, -1 otherwise.
 
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! Shifted planes are replacing
  one by one the initial planes.

  The input list of offsets is not allowed to contain invalid measurements,
  i.e. cross-correlation distances lower than 0. The input list must
  contain as many offsets as there are planes in the input cube.
 
  The rule is that if an offset (dx,dy) is given, the image is shifted
  with a (-dx,-dy) shift, to stay consistent with the results returned
  from the cross-correlation functions.
 
  The offsets used for shifting are rounded up from the input offsets,
  to the closest integer value. The shifting does not involve any
  resampling, only integer pixel shifting.
 
  This is much faster than cube_shift() but of course troublesome,
  since it only handles offsets to pixel resolution.
 */
/*----------------------------------------------------------------------------*/
int cube_shift_int_slice(
		cube_t  	*   to_shift,
		double3 	*   offsets,
		int				from_p,
		int				to_p)
{
    image_t    *   shifted_image ;
    int             i ;
    int             idx, idy ;

    /* Error handling: test entries */
    if (to_shift==NULL || offsets==NULL) return -1 ;
    if (to_shift->np > offsets->n) {
        e_error("not enough offsets measurements to shift planes");
        return -1 ;
    }

	/* Check indices */
	if (from_p<0) from_p=0 ;
	if (to_p<0) to_p=to_shift->np ;

	/* Clip indices */
	if (from_p>(to_shift->np-1)) from_p=to_shift->np-1 ;
	if (to_p>to_shift->np) to_p=to_shift->np ;

    /*
     * Check that all input offsets are valid, i.e. z>-1
     */
    for (i=from_p ; i<to_p ; i++) {
        if (offsets->z[i] < -0.5) {
            e_error("input offset list contains invalid offsets");
            return -1 ;
        }
    }

	e_comment(1, "shifting planes (integer) %d-%d...", from_p+1, to_p);
    for (i=from_p ; i<to_p ; i++) {
        /*
         * The received offset is correct: shift the image
         */
        idx = (int)floor(0.5+(double)offsets->x[i]) ;
        idy = (int)floor(0.5+(double)offsets->y[i]) ;
        shifted_image = image_shift_int(to_shift->plane[i], -idx, -idy) ;
        if (shifted_image == NULL) {
            e_error("in (integer) cube shift at plane %d: aborting", i+1);
            return -1 ;
        }
        image_del(to_shift->plane[i]) ;
        to_shift->plane[i] = shifted_image ;
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @return   a contribution map 
    
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! 
  The final size is defined by the union of all the shifted planes. Each
  input plane is placed in such a huge blank frame.
 
  The planes are first shifted with the decimal part of the offsets, and 
  then, cube_shift_int_expand is used to shift the planes with the E() part 
  of the offsets.
 
 */
/*----------------------------------------------------------------------------*/
intimage * cube_shift_expand(
        cube_t         **   to_shift,
        double3         *   offsets)
{
	double3         *   frac_offs ;
	int					i ;
	intimage		*	contrib ;
	
	/* Determine fractions of offsets */
	frac_offs = double3_new(offsets->n) ;
	for (i=0 ; i<offsets->n ; i++) {
		frac_offs->x[i] = offsets->x[i] - (int)(offsets->x[i]) ;
		frac_offs->y[i] = offsets->y[i] - (int)(offsets->y[i]) ;
	}

	/* Shift with the fractions */
	if (cube_shift(*to_shift, frac_offs, "default") == -1) {
		e_error("cannot shift the cube") ;
		double3_del(frac_offs) ;
		return NULL ;
	}

	/* Shift with the offsets integer parts */
	if ((contrib = cube_shift_int_expand(to_shift, offsets)) == NULL) {
		e_error("cannot shift the cube") ;
		double3_del(frac_offs) ;
		return NULL ;
	}
	double3_del(frac_offs) ;
	return contrib ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @return   a contribution map 
    
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! 
  The final size is defined by the union of all the shifted planes. Each
  input plane is placed in such a huge blank frame.
  
  The rule is that if an offset (dx,dy) is given, the image is shifted
  with a (-E(dx),-E(dy)) shift, to stay consistent with the results returned
  from the cross-correlation functions (not (E(dx), E(dy))).
 
  Here, only the int shift is done. We suppose that the offsets decimal part 
  has already been used to shift the frames. 
  
  The input list of offsets is not allowed to contain invalid measurements,
  i.e. cross-correlation distances lower than 0. The input list must
  contain as many offsets as there are planes in the input cube.
 */
/*----------------------------------------------------------------------------*/
intimage * cube_shift_int_expand(
        cube_t         **   to_shift,
        double3         *   offsets)
{
    cube_t     *   huge_cube ;
    image_t    *   huge_blank_image ;
    image_t    *   huge_image ;
    intimage    *   contr_map ;

    int         x_size,
                y_size ;

    int         x_position,
                y_position ;

    double      off_min_x,
                off_min_y,
                off_max_x,
                off_max_y ;

    int         i ;

    /* Error handling: test entries */
    if ((*to_shift)==NULL || offsets==NULL) return NULL ;
    if ((*to_shift)->np > offsets->n) {
        e_error("not enough offsets measurements to shift planes");
        return NULL ;
    }

    /*
     * Check that all input offsets are valid, i.e. z>-1
     */
    for (i=0 ; i<offsets->n; i++) {
        if (offsets->z[i] < -0.5) {
            e_error("input offset list contains invalid offsets");
            return NULL ;
        }
    }

    /* Identify offsets min and max */
    off_min_x = off_max_x = offsets->x[0] ;
    off_min_y = off_max_y = offsets->y[0] ;
    for (i=1 ; i<offsets->n ; i++) {
        if (offsets->x[i] < off_min_x) off_min_x = offsets->x[i] ;
        if (offsets->x[i] > off_max_x) off_max_x = offsets->x[i] ;
        if (offsets->y[i] < off_min_y) off_min_y = offsets->y[i] ;
        if (offsets->y[i] > off_max_y) off_max_y = offsets->y[i] ;
    }

    /* Find out the HUGE frame size */
    /* SHIFT_REJECT_x is the number of pixels to reject due to the  */
    /* subpixel shift resampling that sets the frame borders to 0 */
    x_size = (*to_shift)->lx + (int)(off_max_x) - (int)(off_min_x)
        - SHIFT_REJECT_R - SHIFT_REJECT_L ;
    y_size = (*to_shift)->ly + (int)(off_max_y) - (int)(off_min_y)
        - SHIFT_REJECT_T - SHIFT_REJECT_B ;

    /* Create a blank contribution map */
    contr_map = intimage_new(x_size, y_size) ;
    if (contr_map == NULL) {
        e_error("cannot create intimage") ;
        return NULL ;
    }

    /* Create a HUGE blank frame     */
    huge_blank_image = image_new(x_size, y_size) ;
    if (huge_blank_image == NULL) {
        e_error("cannot create a huge blank image") ;
        intimage_del(contr_map) ;
        return NULL ;
    }

    /* Create the HUGE output cube */
    huge_cube = cube_new(x_size, y_size, (*to_shift)->np) ;
    if (huge_cube == NULL) {
        e_error("cannot create the new HUGE cube") ;
        intimage_del(contr_map) ;
        image_del(huge_blank_image) ;
        return NULL ;
    }

    /* Loop on all the input frames */
    for (i=0 ; i<(*to_shift)->np ; i++) {

        /* Find out the position of the initial frame in the HUGE one */
        x_position = (int)(off_max_x) - (int)(offsets->x[i]) ;
        y_position = (int)(off_max_y) - (int)(offsets->y[i]) ;

        /* Paste the input frame */
        huge_image = image_paste_vig(huge_blank_image,
                                    (*to_shift)->plane[i],
                                    x_position+1,
                                    y_position+1,
                                    SHIFT_REJECT_L + 1,
                                    SHIFT_REJECT_B + 1,
                                    (*to_shift)->lx - SHIFT_REJECT_R,
                                    (*to_shift)->ly - SHIFT_REJECT_T) ;
        if (huge_image == NULL) {
            e_error("cannot paste the frame in the HUGE image") ;
            image_del(huge_blank_image) ;
            intimage_del(contr_map) ;
            cube_del(huge_cube) ;
            return NULL ;
        }

        /* Update the contribution map */
        if (intimage_increment_zone(contr_map,
                        x_position,
                        y_position,
                        (*to_shift)->lx-SHIFT_REJECT_L-SHIFT_REJECT_R,
                        (*to_shift)->ly-SHIFT_REJECT_B-SHIFT_REJECT_T) == -1) {
            e_error("cannot update the contribution map") ;
            image_del(huge_blank_image) ;
            intimage_del(contr_map) ;
            cube_del(huge_cube) ;
            return NULL ;
        }

        /* Store the result in the output cube */
        huge_cube->plane[i] = huge_image ;
    }
    image_del(huge_blank_image) ;

    /* Replace the input cube by the HUGE one */
    cube_del(*to_shift) ;
    *to_shift = huge_cube ;

    /* Free and return the contribution map */
    return contr_map ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @param    kernel      Kernel to use for resampling.
  @return   int 0 if Ok, -1 otherwise.
 
  Runs cube_shift_slice over the whole cube.
 */
/*----------------------------------------------------------------------------*/
int cube_shift(
		cube_t  *   to_shift,
		double3 *   offsets,
		char    *   kernel)
{
	/* No multithreading support yet */
	return cube_shift_slice(to_shift, offsets, kernel, -1, -1);
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @param    kernel      Kernel to use for resampling.
  @param	from_p		Index of first plane to shift.
  @param	to_p		Index of last plane to shift.
  @return   int 0 if Ok, -1 otherwise.
 
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! Shifted planes are replacing
  one by one the initial planes.
 
  The rule is that if an offset (dx,dy) is given, the image is shifted
  with a (-dx,-dy) shift, to stay consistent with the results returned
  from the cross-correlation functions.
 
  The input list of offsets is not allowed to contain invalid measurements,
  i.e. cross-correlation distances lower than 0. The input list must
  contain as many offsets as there are planes in the input cube.
 
  There is a faster version (shift_cube_int) but only handling offsets
  to pixel resolution, i.e. without resampling.

  The only planes which will be shifted are the ones between index 'from_p'
  and 'to_p' (including 'from_p', not 'to_p'). Nevertheless, the provided
  list of offsets must be consistent with the input cube, i.e. have as many
  offsets as there are planes in the cube.  To shift the whole cube, use
  the macro cube_shift(). If from_p is negative, it is assumed to be 0 (the
  first plane). If to_p is negative, it is assumed to be to_shift->np (the
  last plane). This slicing is meant for multi-threading support.
 */
/*----------------------------------------------------------------------------*/
int cube_shift_slice(
		cube_t  *   to_shift,
		double3 *   offsets,
		char    *   kernel,
		int			from_p,
		int			to_p)
{
    image_t     *   shifted_image ;
    int             i ;
    double      *   interp_kernel ;

    /* Error handling: test entries */
    if (to_shift == NULL) return -1  ;
    if (offsets  == NULL) return -1 ;    
	if (to_shift->np != offsets->n) {
        e_error("inconsistency between provided offsets and cube");
        return -1 ;
    }

	/* Check indices */
	if (from_p<0) from_p=0 ;
	if (to_p<0) to_p=to_shift->np ;

	/* Clip indices */
	if (from_p>(to_shift->np-1)) from_p=to_shift->np-1 ;
	if (to_p>to_shift->np) to_p=to_shift->np ;

    /*
     * Check that all input offsets are valid, i.e. z>-1
     */
    for (i=from_p ; i<to_p ; i++) {
        if (offsets->z[i] < -0.5) {
            e_error("input offset list contains invalid offsets");
            return -1 ;
        }
    }

    interp_kernel = generate_interpolation_kernel(kernel) ;
    if (interp_kernel == NULL) {
        return -1 ;
    }

	e_comment(1, "shifting planes %d-%d...", from_p+1, to_p);
    for (i=from_p ; i<to_p ; i++) {
        if (offsets->z[i] > -0.5) {
            /*
             * The received offset is correct: shift the image
             */
            shifted_image = image_shift(to_shift->plane[i],
                                        -offsets->x[i],
                                        -offsets->y[i],
                                        interp_kernel) ;
            if (shifted_image == NULL) {
                e_error("in cube shift at plane %d: aborting", i+1) ;
                free(interp_kernel) ;
                return -1 ;
            }
            image_del(to_shift->plane[i]) ;
            to_shift->plane[i] = shifted_image ;
        }
    }
    free(interp_kernel) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Shift and add a cube to a single frame.
  @param	in			Cube to process.
  @param	offs		List of offsets between frames.
  @param	kernel		Interpolation kernel to use.
  @param	rejmin		Number of min pixels to reject in stacking
  @param	rejmax		Number of max pixels to reject in stacking
  @param    union_flag  Flag to create a union image.
  @return	1 newly allocated image.

  This function does everything related to the final shift-and-add of
  a stack of frames. It takes in input a cube and a list of offsets
  to apply to the cube to register all frames to a common position,
  applies an interpolation kernel to resample the frames to sub-pixel
  accuracy and accumulates them into an output image, using 3d filtering
  if requested.

  If the union flag is non-zero, the final frame is a union of all input
  frames, i.e. it is always bigger than the input image. If union_flag
  is set to zero, only the intersection of all frames will be built,
  i.e. all pixels in the final image have been seen by all input
  frames.

  The returned frame is a newly allocated object, to be deallocated
  using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t	* cube_shiftandadd(
		cube_t  * in,
		double3 * offs,
		char	* kernel,
		int		  rejmin,
		int		  rejmax,
        int       union_flag)
{
	int				i, j, k, p ;
	int				Lx, Ly ;
	image_t		*	final ;
	double			ox_min, oy_min, ox_max, oy_max ;
	pixelvalue 	*	acc ;
	pixelvalue		interp ;
	int				ncontrib ;
	double			x, y ;
	int				px, py ;
	int				tabx, taby ;
	double			rsc[8] ;
	double			sumrs ;
	double			finpix ;
	double		*	interp_kernel ;
	int				leaps[16] ;
	double			neighbors[16] ;
	int				pos ;
	int				rejtot ;

    double      *   offx ;
    double      *   offy ;

    int             start_x,
                    start_y ;

	/* Test inputs */
	if (in==NULL || offs==NULL) return NULL ;
	if (in->np != offs->n) {
		e_error("not enough offsets to shift&add cube");
		return NULL ;
	}
	for (i=0 ; i<offs->n ; i++) {
		if (offs->z[i]<-0.5) {
			e_error("in shift& add: invalid offset measurement in input");
			return NULL ;
		}
	}

    if (in->np==1) {
        e_warning("single image in input of shift-and-add: doing nothing");
        return image_copy(in->plane[0]);
    }


    /* Test rejection parameters */
    if (in->np <= 3) {
        e_warning("less than 3 frames in input: no rejection applied");
        rejmin=0 ;
        rejmax=0 ;
    }
    if (in->np<=(2*(rejmin + rejmax))) {
        e_warning("rejection set to %d-%d but %d planes in input\n"
                  "rejection will not be applied", rejmin, rejmax, in->np) ;
        rejmin=0 ;
        rejmax=0 ;
    }
	rejtot = rejmin + rejmax ;
    
	/* Find out size of output image */
    /* List all offsets */
    offx = malloc(offs->n * sizeof(double));
    offy = malloc(offs->n * sizeof(double));

    memcpy(offx, offs->x, offs->n * sizeof(double));
    memcpy(offy, offs->y, offs->n * sizeof(double));

    double_qsort(offx, offs->n);
    double_qsort(offy, offs->n);


    /* Compute output image size for union/intersection */
    if (union_flag) {
        ox_min = offx[rejtot];
        ox_max = offx[offs->n-rejtot-1];
        oy_min = offy[rejtot];
        oy_max = offy[offs->n-rejtot-1];
        Lx = (int)(in->lx + ox_max - ox_min) +1 ;
        Ly = (int)(in->ly + oy_max - oy_min) +1 ;
        start_x = ox_min ;
        start_y = oy_min ;
    } else { 
        ox_min = offx[0] ;
        ox_max = offx[offs->n-1];
        oy_min = offy[0] ;
        oy_max = offy[offs->n-1];

        Lx = (int)(in->lx - ox_max + ox_min) +1;
        Ly = (int)(in->ly - oy_max + oy_min) +1;
        start_x = ox_max - ox_min ;
        start_y = oy_max - oy_min ;
    }
    free(offx);
    free(offy);

	/* Generate interpolation kernel */
	interp_kernel = generate_interpolation_kernel(kernel);
	if (interp_kernel==NULL) {
		e_error("generating interpolation kernel: aborting shift&add");
		return NULL ;
	}

	/* Pre-compute leaps for 16 closest neighbor positions */
    leaps[0] = -1 - in->lx ;
    leaps[1] =    - in->lx ;
    leaps[2] =  1 - in->lx ;
    leaps[3] =  2 - in->lx ;

    leaps[4] = -1 ;
    leaps[5] =  0 ;
    leaps[6] =  1 ;
    leaps[7] =  2 ;

    leaps[8] = -1 + in->lx ;
    leaps[9] =      in->lx ;
    leaps[10]=  1 + in->lx ;
    leaps[11]=  2 + in->lx ;

    leaps[12]= -1 + 2*in->lx ;
    leaps[13]=      2*in->lx ;
    leaps[14]=  1 + 2*in->lx ;
    leaps[15]=  2 + 2*in->lx ;


	/* Create output image */
	final = image_new(Lx, Ly);

	/* Triple loop */
	acc = malloc(in->np * sizeof(pixelvalue));
	for (j=0 ; j<Ly ; j++) {
		compute_status("shift and add...", j, Ly, 1);
		for (i=0 ; i<Lx ; i++) {
			ncontrib=0 ;
			for (p=0 ; p<in->np ; p++) {
				x = (double)i - offs->x[p] + start_x ;
				y = (double)j - offs->y[p] + start_y ;

				px = (int)x ;
				py = (int)y ;

				/* If original pixel is present in the current plane */
				if (px>1 && px<(in->lx-2) && py>1 && py<(in->ly-2)) {
					/* Interpolate from input image */
					pos = px + py * in->lx ;
					for (k=0 ; k<16 ; k++) {
						neighbors[k] =
							(double)in->plane[p]->data[(int)pos+leaps[k]] ;
					}
					/* Which tabulated value index shall be used? */
					tabx = (int)(0.5+(x-(double)px)*(double)(TABSPERPIX)) ;
					taby = (int)(0.5+(y-(double)py)*(double)(TABSPERPIX)) ;

					/* Compute resampling coefficients */
					/* rsc[0..3] in x, rsc[4..7] in y  */

					rsc[0] = interp_kernel[TABSPERPIX + tabx] ;
					rsc[1] = interp_kernel[tabx] ;
					rsc[2] = interp_kernel[TABSPERPIX - tabx] ;
					rsc[3] = interp_kernel[2*TABSPERPIX - tabx] ;
					rsc[4] = interp_kernel[TABSPERPIX + taby] ;
					rsc[5] = interp_kernel[taby] ;
					rsc[6] = interp_kernel[TABSPERPIX - taby] ;
					rsc[7] = interp_kernel[2*TABSPERPIX - taby] ;

					sumrs = (rsc[0]+rsc[1]+rsc[2]+rsc[3]) *
							(rsc[4]+rsc[5]+rsc[6]+rsc[7]) ;

					/* Compute interpolated pixel now   */
					interp= rsc[4] * (  rsc[0]*neighbors[0] +
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
                    interp /= sumrs ;
					acc[ncontrib] = interp ;
					ncontrib ++ ;
				}
			}
			finpix = 0 ;
			if ((ncontrib>0) && (ncontrib>rejtot)) {
				pixel_qsort(acc, ncontrib);
				for (p=rejmin ; p<(ncontrib-rejmax) ; p++) {
					finpix += (double)acc[p];
				}
				finpix /= (double)(ncontrib-rejtot) ;
			}
			final->data[i+j*Lx] = finpix ;
		}
	}
	free(interp_kernel);
	free(acc);
	return final ;
}

