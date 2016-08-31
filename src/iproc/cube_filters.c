/*-------------------------------------------------------------------------*/
/**
   @file	cube_filters.c
   @author	Nicolas Devillard
   @date	Aug 29, 1995
   @version	$Revision: 1.31 $
   @brief	various cube filters in image domain
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: cube_filters.c,v 1.31 2004/03/03 08:51:18 yjung Exp $
	$Author: yjung $
	$Date: 2004/03/03 08:51:18 $
	$Revision: 1.31 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "cube_filters.h"
#include "dstats.h"
#include "image_intops.h"
#include "image_stats.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply an image filter to all planes of a cube.
  @param	cube_in		Cube to modify.
  @param	filter		Filter name.
  @param	filtval		(optional) filter values.
  @return	int 0 if Ok, -1 otherwise.

  Every plane in the input cube is modified by applying a filter on
  it. The possible filter names are:

  \begin{itemize}
  \item user-linear
  \item mean3
  \item dx
  \item dy
  \item d2x
  \item d2y
  \item contour1
  \item contour2
  \item contour3
  \item contrast1
  \item mean5
  \item min
  \item max
  \item median
  \item max-min
  \item user-morpho
  \item 3x1
  \item flat
  \end{itemize}

  Only 'user-linear', 'user-morpho' and '3x1' actually request filter
  values to be provided through the 'filtval' array of doubles.

  The 'flat' filter requires a single value, an integer representing the
  half-size of the kernel to use. This integer is actually passed as the
  first double in the filtval list. If the passed value is not truly an
  integer, it is rounded up to the closest integer.
 */
/*--------------------------------------------------------------------------*/
int cube_filter(
		cube_t 	* 	cube_in, 
		char 	* 	filter, 
		double 	* 	filtval)
{
    double * ker ;
    int      nval ;
    int      morpho ;
	int		 status ;

	if (cube_in==NULL || filter==NULL) return -1 ;

    /* Try to obtain a filter definition for the provided string */
    ker = image_filter_getkernel(filter, &nval, &morpho);
    if (ker!=NULL) {
        if (morpho) {
            status = cube_filter_morpho(cube_in, ker);
        } else {
            switch (nval) {
                case 9:
                status = cube_filter_3x3(cube_in, ker);
                break ;

                case 25:
                status = cube_filter_5x5(cube_in, ker);
                break ;

                default:
                e_error("invalid filter definition for [%s]", filter);
                return -1 ;
            }
        }
    } else {
        if (!strcmp(filter, "median")) {
            status = cube_filter_median(cube_in);
        } else if (!strcmp(filter, "user-linear")) {
            if (filtval == NULL) {
                e_error("no provided values for user-defined filter");
                status=-1 ;
            } else {
                status = cube_filter_3x3(cube_in, filtval) ;
            }
        } else if (!strcmp(filter, "user-morpho")) {
            if (filtval == NULL) {
                e_error("no provided values for user-defined filter");
                status=-1 ;
            } else {
                status = cube_filter_morpho(cube_in, filtval) ;
            }
        } else if (!strcmp(filter, "3x1")) {
            if (filtval == NULL) {
                e_error("no provided values for user-defined filter");
                status=-1 ;
            } else {
                status = cube_filter_3x1(cube_in, filtval) ;
            }
        } else if (!strcmp(filter, "flat")) {
            status = cube_filter_flat(cube_in, (int)(filtval[0]+0.5));
        } else {
            e_error("unsupported filter: [%s]", filter) ;
            status=-1 ;
        }
    }
	return status ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a 3x3 filter to all planes of a cube.
  @param	cube1			Cube to modify.
  @param	filter_array	Array of 9 filter coefficients.
  @return	int 0 if Ok, -1 otherwise.

  Filter coefficients are spread over a 3x3 matrix. If the matrix is:

  \begin{verbatim}
  f0  f1  f2
  f3  f4  f5
  f6  f7  f8
  \end{verbatim}

  Then the filter is given as a pointer to 9 doubles as {f0,...,f8}.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
int cube_filter_3x3(
		cube_t	* 	cube1, 
		double 	* 	filter_array) 
{
	image_t	* filtered ;
	int		  p ;
	
	if (cube1==NULL || filter_array==NULL) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("filtering planes", p, cube1->np, 2) ;
		filtered = image_filter3x3(cube1->plane[p], filter_array) ;
		if (filtered==NULL) {
			e_error("applying filter on plane %d: aborting", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p] = filtered ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a 3x1 filter to all planes of a cube.
  @param    cube1           Cube to modify.
  @param    filter_array    Array of 3 filter coefficients.
  @return   int 0 if Ok, -1 otherwise.
 
  Filter coefficients are spread over a 3x1 matrix: {f1, f2, f3}.
 
  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
int cube_filter_3x1(
		cube_t 	* 	cube1, 
		double 	* 	filter_array)
{
	image_t	*	filtered ;
	int			p ;
	
	if (cube1==NULL || filter_array==NULL) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("filtering planes", p, cube1->np, 2) ;
		filtered = image_filter3x1(cube1->plane[p], filter_array) ;
		if (filtered==NULL) {
			e_error("applying filter on plane %d: aborting", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p] = filtered ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a 5x5 filter to all planes of a cube.
  @param	cube1			Cube to modify.
  @param	filter_array	Array of 25 filter coefficients.
  @return	int 0 if Ok, -1 otherwise.

  Filter coefficients are spread over a 5x5 matrix. If the matrix is:

  \begin{verbatim}
  f0  f1  f2  f3  f4
  f5  f6  f7  f8  f9
  f10 f11 f12 f13 f14
  f15 f16 f17 f18 f19
  f20 f21 f22 f23 f24
  \end{verbatim}

  Then the filter is given as a pointer to 25 doubles as {f0,...,f24}.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
int cube_filter_5x5(
		cube_t 	* 	cube1, 
		double 	* 	filter_array) 
{
    image_t     *	filtered ;
    int             p ;
 
	if (cube1==NULL || filter_array==NULL) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("filtering planes", p, cube1->np, 2) ;
		filtered = image_filter5x5(cube1->plane[p], filter_array) ;
		if (filtered==NULL) {
			e_error("filtering plane %d: aborting operation", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p]=filtered ;
	}
	return 0 ;
}    


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a morpho filter to all planes of a cube.
  @param	cube1			Cube to modify
  @param	filter_array	Array of 9 filter coefficients.
  @return	int 0 if Ok, -1 otherwise.

  Filter coefficients are spread over a 3x3 matrix. If the matrix is:

  \begin{verbatim}
  f0  f1  f2
  f3  f4  f5
  f6  f7  f8
  \end{verbatim}

  Then the filter is given as a pointer to 9 doubles as {f0,...,f8}.

  The first filter element applies to the min value in the 3x3
  neighbourhood of the current pixel, the 9th element of the filter
  matrix applies to the maximum, and other elements are sorted
  according to their position.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
int cube_filter_morpho(
		cube_t 	* 	cube1, 
		double 	* 	filter_array)
{
	image_t	*	filtered ;
    int             p ;

	if (cube1==NULL || filter_array==NULL) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("filtering planes", p, cube1->np, 2) ;
		filtered = image_filter_morpho(cube1->plane[p],filter_array);
		if (filtered==NULL) {
			e_error("filtering plane %d: aborting operation", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p]=filtered ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a median filter to all planes of a cube.
  @param	cube1			Cube to modify
  @return	int 0 if Ok, -1 otherwise.

  A 3x3 median filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
int cube_filter_median(cube_t * cube1)
{
	image_t	* 	filtered ;
    int             p ;

	if (cube1==NULL) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("median filtering planes", p, cube1->np, 2) ;
		filtered = image_filter_median(cube1->plane[p]);
		if (filtered==NULL) {
			e_error("filtering plane %d: aborting operation", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p]=filtered ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a flat filter to all planes of a cube.
  @param	cube1			Cube to modify.
  @param	kern_hsize		Kernel half-size
  @return	int 0 if Ok, -1 otherwise.

  The same filter is applied to every plane in the input cube. The
  input cube is modified in the process.
 */
/*--------------------------------------------------------------------------*/
int cube_filter_flat(
		cube_t 	* 	cube1, 
		int 		kern_hsize) 
{
    image_t     *	filtered ;
    int             p ;
 
	if (cube1==NULL || kern_hsize<1) return -1 ;
	for (p=0 ; p<cube1->np ; p++) {
		compute_status("filtering planes", p, cube1->np, 2) ;
		filtered = image_filter_flat(cube1->plane[p], kern_hsize) ;
		if (filtered==NULL) {
			e_error("filtering plane %d: aborting operation", p+1);
			return -1 ;
		}
		image_del(cube1->plane[p]);
		cube1->plane[p]=filtered ;
	}
	return 0 ;
}    


/*-------------------------------------------------------------------------*/
/**
  @brief	3d-filtering on a cube with minmax rejection.
  @param	in			Cube to filter.
  @param	halfw		Half-width for filter.
  @param	rejmin		Number of min pixels to reject.
  @param	rejmax		Number of max pixels to reject.
  @param	background	Double array to store computed background.
  @return	int 0 if Ok, -1 otherwise

  This function performs a 3d rejection filter on the input cube.
  Each time line is extracted on +/-halfw. On this line of sight, all
  pixels are first normalized by subtracting the median value of the
  plane they belong to. The highest and lowest values are then removed
  and the rest is averaged to yield a value which is subtracted from
  the initial input pixel.

  The background array is optional, provide NULL if you have no use
  for it. This array is expected to be already allocated, it should
  have space for at least as many doubles as there are input planes.
  The array will contain the average value which has been subtracted
  to pixels in each plane. It is usually a good indicator of the
  average subtracted background value if you use this filter to subtract
  an infrared sky background.
 */
/*--------------------------------------------------------------------------*/
int cube_3dfilt_runminmax(
		cube_t	**	in,
		int			halfw,
		int			rejmin,
		int			rejmax,
		double	*	background)
{
	cube_t		*	filtres ;
	double		*	medians ;
	int				pos, npix ;
	int				p ;
	int				np ;
	int				fr_p, to_p ;
	int				n_curp ;
	double	 	*	localwin ;
	double			out ;
	int				i ;
	pixelvalue		one_med ;

	if (in==NULL || (*in)==NULL) return -1 ;
	/* Tests on validity of rejection parameters */
	if (((rejmin+rejmax)>=halfw) || (halfw<1) || (rejmin<0) || (rejmax<0)) {
		e_error("cannot run filter with rejection parms %d (%d-%d)",
				halfw,
				rejmin,
				rejmax);
		return -1 ;
	}

	/* Pre-compute median value in each plane */
	np = (*in)->np ;
	medians = calloc(np, sizeof(double));
	for (p=0 ; p<np ; p++) {
		compute_status("computing medians...", p, np, 1);
		medians[p] = (double)image_getmedian((*in)->plane[p]);
	}
	/* Allocate output cube */
	filtres = cube_new((*in)->lx, (*in)->ly, np);

	/* Allocate local window */
	localwin = malloc((2*halfw+1)*sizeof(double));

	/* Main loop over input planes */
	npix = (*in)->lx * (*in)->ly ;
	for (p=0 ; p<np ; p++) {
		compute_status("3d filtering on cube...", p, np, 1);
		/* Allocate output plane */
		filtres->plane[p] = image_new((*in)->lx, (*in)->ly);
		/* Compute border indices */
		fr_p = p - halfw ;
		to_p = p + halfw ;

		if (fr_p<0) fr_p=0 ;
		if (to_p>(np-1)) to_p=np-1 ;

		/* Number of valid planes to consider after edge effects */
		n_curp = to_p - fr_p + 1 ;

		if (background!=NULL)
			background[p]=0 ;
		/* Loop over all pixels */
		for (pos=0 ; pos<npix ; pos++) {
			/* Fill up local window */
			for (i=0 ; i<n_curp ; i++) {
				localwin[i]=
					(double)(*in)->plane[i+fr_p]->data[pos] - medians[i+fr_p];
			}
			/* Sort window */
			double_qsort(localwin, n_curp);
			/* Reject min and max, accumulate other pixels */
			out = 0 ;
			for (i=rejmin ; i<(n_curp-rejmax) ; i++) {
				out += localwin[i];
			}
			/* Take the mean */
			out /= (double)(n_curp - rejmin - rejmax);
			/* Assign value */
			filtres->plane[p]->data[pos] =
				(*in)->plane[p]->data[pos] - (pixelvalue)(out + medians[p]);

			if (background!=NULL)
				background[p] += (out+medians[p]);
		}
		if (background!=NULL)
			background[p] /= (double)npix ;

		/* Free plane whenever possible */
		if (p-halfw>=0) {
			image_del((*in)->plane[p-halfw]) ;
			(*in)->plane[p-halfw] = NULL ;
		}
	}
	free(localwin);
	free(medians);

	/* Subtract median from each frame */
	for (p=0 ; p<np ; p++) {
		compute_status("computing medians...", p, np, 1);
		one_med = image_getmedian(filtres->plane[p]);
		image_cst_op_local(filtres->plane[p], (double)one_med, '-');
	}
	cube_del(*in);
	*in = filtres ;
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	3d-filtering on a cube with minmax and central rejection.
  @param	in			Cube to filter.
  @param	halfw		Half-width for filter.
  @param	rejmin		Number of min pixels to reject.
  @param	rejmax		Number of max pixels to reject.
  @param	background	Double array to store computed background.
  @return	int 0 if Ok, -1 otherwise

  This function performs a 3d rejection filter on the input cube.
  Each time line is extracted on +/-halfw. On this line of sight, all
  pixels are first normalized by subtracting the median value of the
  plane they belong to. The central, highest and lowest values are then
  removed and the rest is averaged to yield a value which is subtracted
  from the initial input pixel.

  The background array is optional, provide NULL if you have no use
  for it. This array is expected to be already allocated, it should
  have space for at least as many doubles as there are input planes.
  The array will contain the average value which has been subtracted
  to pixels in each plane. It is usually a good indicator of the
  average subtracted background value if you use this filter to subtract
  an infrared sky background.
 */
/*--------------------------------------------------------------------------*/
int cube_3dfilt_runminmax_central(
		cube_t	**	in,
		int			halfw,
		int			rejmin,
		int			rejmax,
		double	*	background)
{
	cube_t		*	filtres ;
	double		*	medians ;
	int				pos, npix ;
	int				p ;
	int				np ;
	int				fr_p, to_p ;
	int				n_curp ;
	double	 	*	localwin ;
	double			out ;
	int				i, j ;
	pixelvalue		one_med ;

	if (in==NULL || (*in)==NULL) return -1 ;
	/* Tests on validity of rejection parameters */
	if (((rejmin+rejmax)>=halfw) || (halfw<1) || (rejmin<0) || (rejmax<0)) {
		e_error("cannot run filter with rejection parms %d (%d-%d)",
				halfw,
				rejmin,
				rejmax);
		return -1 ;
	}

	/* Pre-compute median value in each plane */
	np = (*in)->np ;
	medians = calloc(np, sizeof(double));
	for (p=0 ; p<np ; p++) {
		compute_status("computing medians...", p, np, 1);
		medians[p] = (double)image_getmedian((*in)->plane[p]);
	}
	/* Allocate output cube */
	filtres = cube_new((*in)->lx, (*in)->ly, np);

	/* Allocate local window */
	localwin = malloc(2 * halfw * sizeof(double));

	/* Main loop over input planes */
	npix = (*in)->lx * (*in)->ly ;
	for (p=0 ; p<np ; p++) {
		compute_status("3d filtering on cube...", p, np, 1);
		/* Allocate output plane */
		filtres->plane[p] = image_new((*in)->lx, (*in)->ly);
		/* Compute border indices */
		fr_p = p - halfw ;
		to_p = p + halfw ;

		if (fr_p<0) fr_p=0 ;
		if (to_p>(np-1)) to_p=np-1 ;

		/* Number of valid planes to consider after edge effects */
		n_curp = to_p - fr_p ;

		if (background!=NULL)
			background[p]=0 ;

		/* Loop over all pixels */
		for (pos=0 ; pos<npix ; pos++) {
			/* Fill up local window */
            j=0 ;
			for (i=fr_p ; i<=to_p ; i++) {
                if (i!=p) {
                    localwin[j]=
					(double)(*in)->plane[i]->data[pos] - medians[i];
                    j++;
                }
			}
			/* Sort window */
			double_qsort(localwin, n_curp);
			/* Reject min and max, accumulate other pixels */
			out = 0 ;
			for (i=rejmin ; i<(n_curp-rejmax) ; i++) {
				out += localwin[i];
			}
			/* Take the mean */
			out /= (double)(n_curp - rejmin - rejmax);
			/* Assign value */
			filtres->plane[p]->data[pos] =
				(*in)->plane[p]->data[pos] - (pixelvalue)(out + medians[p]);

			if (background!=NULL)
				background[p] += (out+medians[p]);
		}
		if (background!=NULL)
			background[p] /= (double)npix ;

		/* Free plane whenever possible */
		if (p-halfw>=0) {
			image_del((*in)->plane[p-halfw]) ;
			(*in)->plane[p-halfw] = NULL ;
		}
	}
	free(localwin);
	free(medians);

	/* Subtract median from each frame */
	for (p=0 ; p<np ; p++) {
		compute_status("computing medians...", p, np, 1);
		one_med = image_getmedian(filtres->plane[p]);
		image_cst_op_local(filtres->plane[p], (double)one_med, '-');
	}
	cube_del(*in);
	*in = filtres ;
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	3d-filtering on a cube with minmax rejection.
  @param	in			Cube to filter.
  @param	halfw		Half-width for filter.
  @param	rejmin		Number of min pixels to reject.
  @param	rejmax		Number of max pixels to reject.
  @param	background	Double array to store computed background.
  @return	int 0 if Ok, -1 otherwise

  This function performs a 3d rejection filter on the input cube.
  Each time line is extracted on +/-halfw. On this line of sight, all
  pixels are first normalized by subtracting the median value of the
  plane they belong to. The highest and lowest values are then removed
  and the rest is averaged to yield a value which is subtracted from
  the initial input pixel.

  The background array is optional, provide NULL if you have no use
  for it. This array is expected to be already allocated, it should
  have space for at least as many doubles as there are input planes.
  The array will contain the average value which has been subtracted
  to pixels in each plane. It is usually a good indicator of the
  average subtracted background value if you use this filter to subtract
  an infrared sky background.
 */
/*--------------------------------------------------------------------------*/
int cube_3dfilt_runminmax_by_quad(
		cube_t	**	in,
		int			halfw,
		int			rejmin,
		int			rejmax,
		double	*	background)
{
	int			i, j ;
	cube_t	*	quad ;
	double	*	bg[4] ;
	int			lx, ly, np ;
	int			window[4][4] ;
	image_t	*	pasted ;

	if (in==NULL || (*in)==NULL) return -1 ;
	/* Tests on validity of rejection parameters */
	if (((rejmin+rejmax)>=halfw) || (halfw<1) || (rejmin<0) || (rejmax<0)) {
		e_error("cannot run filter with rejection parms %d (%d-%d)",
				halfw,
				rejmin,
				rejmax);
		return -1 ;
	}

	lx = (*in)->lx ;
	ly = (*in)->ly ;
	np = (*in)->np ;

	/* Set coordinates for upper-left quadrant */
	window[0][0] = 1 ;
	window[0][1] = 1 + ly/2 ;
	window[0][2] = lx/2 ;
	window[0][3] = ly ;

	/* Set coordinates for upper-right quadrant */
	window[1][0] = 1 + lx/2 ;
	window[1][1] = 1 + ly/2 ;
	window[1][2] = lx ;
	window[1][3] = ly ;

	/* Set coordinates for lower-right quadrant */
	window[2][0] = 1 + lx/2 ;
	window[2][1] = 1 ;
	window[2][2] = lx ;
	window[2][3] = ly/2 ;

	/* Set coordinates for lower-left quadrant */
	window[3][0] = 1 ;
	window[3][1] = 1 ;
	window[3][2] = lx/2 ;
	window[3][3] = ly/2 ;

	/* Loop on all quadrants */
	for (i=0 ; i<4 ; i++) {
		e_comment(1, "sky filtering quadrant %d...", i+1);
		/* Extract lower-left quadrant */
		quad = cube_getvig(*in,
							window[i][0],
							window[i][1],
							window[i][2],
							window[i][3]);
		bg[i] = malloc(np * sizeof(double)) ;
		/* Filter cube */
		cube_3dfilt_runminmax(&quad, halfw, rejmin, rejmax, bg[i]);
		/* Replace pixels in initial cube */
		for (j=0 ; j<np ; j++) {
			pasted = image_paste(	(*in)->plane[j],
									quad->plane[j],
									window[i][0],
									window[i][1]);
			image_del((*in)->plane[j]);
			(*in)->plane[j] = pasted ;
		}
		/* Delete intermediate cube */
		cube_del(quad);
	}
	if (background!=NULL) {
		for (i=0 ; i<np ; i++) {
			background[i] = (bg[0][i]+bg[1][i]+bg[2][i]+bg[3][i])*0.25 ;
		}
	}
	free(bg[0]);
	free(bg[1]);
	free(bg[2]);
	free(bg[3]);

	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Sky estimation and correction with median method
  @param    cube        Input cube with objects and sky frames
  @param    sky_flags   Flags to identify the sky frames
  @return   the estimated sky
    
  This function take the median of all the sky frames and subtract the
  resulting frame from all object frames.
  If all input frames are object frames, the median is computed on the object
  frames.
  At the end, the object frames are normalized (divided by their median)
  The estimated sky frame is returned
 */
/*--------------------------------------------------------------------------*/
image_t * cube_subtract_median_sky(
    cube_t  *   cube,
    int     *   sky_flags)
{
    cube_t  *   sky_cube ;
    image_t *   sky ;
    double      median_pix ;
    int         nb_sky ;
    int         i, j ;

    if (cube==NULL || sky_flags==NULL)
        return NULL ;

    e_comment(2, "building sky frame");
    /* Count the number of sky frames */
    nb_sky = 0 ;
    for (i=0 ; i<cube->np ; i++) if (sky_flags[i] == 1) nb_sky++ ;

    /* Build the cube with sky frames, or with obj frames if none */
    if (nb_sky != 0) {
        sky_cube = cube_new(cube->lx, cube->ly, nb_sky) ;
        j = 0 ;
        for (i=0 ; i<cube->np ; i++) {
            if (sky_flags[i] == 1) {
                sky_cube->plane[j] = cube->plane[i] ;
                j++ ;
            }
        }
        /* Average Median of the cube to construct the sky  */
        sky = cube_avg_median(sky_cube) ;

        /* Destroy the sky cube */
        cube_del_shallow(sky_cube) ;

    } else if (nb_sky == 0) {
        sky = cube_avg_median(cube) ;
    }

    if (sky == NULL) {
        e_error("cannot compute the sky") ;
        return NULL ;
    }


    /* Correct the input frames from sky */
    for (i=0 ; i<cube->np ; i++) {
        compute_status("subtracting sky frame...", i, cube->np, 2);
        if (sky_flags[i] == 0) image_sub_local(cube->plane[i], sky) ;
    }

    /* Normalize each object plane */
    for (i=0 ; i<cube->np ; i++) {
        if (sky_flags[i] == 0) {
            median_pix = image_getmedian(cube->plane[i]);
            for (j=0 ; j<cube->lx * cube->ly ; j++) {
                cube->plane[i]->data[j] -= median_pix ;
            }
        }
    }

    /* Return the sky */
    return sky ;
}


