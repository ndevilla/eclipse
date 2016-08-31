/*-------------------------------------------------------------------------*/
/**
   @file	function_1d.c
   @author	Nicolas Devillard
   @date	Tue, Sept 23 1997
   @version	$Revision: 1.30 $
   @brief	1d signal processing related routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: function_1d.c,v 1.30 2004/01/09 13:34:30 llundin Exp $
	$Author: llundin $
	$Date: 2004/01/09 13:34:30 $
	$Revision: 1.30 $
*/

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>
#include <float.h>

#include "function_1d.h"
#include "fit_curve.h"
#include "image_stats.h"
#include "comm.h"

/*----------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define HALF_CENTROID_DOMAIN	5

/*----------------------------------------------------------------------------
						Private function prototypes
 ---------------------------------------------------------------------------*/

static double * function1d_generate_smooth_kernel(int filt_type, int hw);
static int function1d_search_value(pixelvalue * x, int len, pixelvalue key, 
		int * found_ptr) ;

/*----------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Allocates a new array of pixelvalues.
  @param	nsamples	Number of values to store in the array.
  @return	Pointer to newly allocated array of pixelvalues.

  The returned array must be freed using function1d_del(), not free().
  This is in case some further housekeeping attributes are allocated
  together with the object in the future.

  Returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_new(int nsamples)
{
	if (nsamples<1) return NULL ;
	return calloc(nsamples, sizeof(pixelvalue)) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Deallocate an array of pixelvalues.
  @param	s	Array to deallocate.
  @return	void

  Deallocates an array allocated by function1d_new().
 */
/*--------------------------------------------------------------------------*/
void function1d_del(pixelvalue * s)
{
	if (s) free(s);
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Copy an array of pixelvalues to a new array.
  @param	arr		Array to copy.
  @param	ns		Number of samples in the array.
  @return	Pointer to newly allocated array of pixelvalues.

  Creates a new array using function1d_new(), with the same number of
  samples as the input signal, then copies over all values from source
  to destination array using memcpy().

  The returned array must be freed using function1d_del(), not free().
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_dup(pixelvalue * arr, int ns)
{
	pixelvalue	*	n_arr ;

	n_arr = function1d_new(ns);
	memcpy(n_arr, arr, ns * sizeof(pixelvalue));
	return n_arr ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Find out a line centroid to subpixel precision.
  @param    line    Array of pixels.
  @param    npix    Number of pixels in the array.
  @return    Centroid position as a double, or -1 on error.

  The input signal is assumed to be flat almost everywhere, with a
  single peak somewhere around the middle. Other kinds of signals are
  not handled correctly.

  There must be at least HALF_CENTROID_DOMAIN pixels on either side of the
  maximum pixelvalue.

  The position of the peak is located to subpixel precision by
  simply weighting positions with pixelvalues.
 */
/*--------------------------------------------------------------------------*/
double function1d_find_centroid(
        pixelvalue    *    line,
        int                npix)
{
    pixelvalue    min = 0;
    pixelvalue    max ;
    double        centroid ;
    double        weights ;
    int           i, maxpos ;

    if (line==NULL) return -1.0 ;

    /* Search for the maximum pixel value on the line */
    max = line[0] ;
    maxpos = 0;
    for (i=1 ; i<npix ; i++) {
        if (line[i]>max) {
            max = line[i] ;
            maxpos = i ;
        }
    }

    if (maxpos < HALF_CENTROID_DOMAIN ||
             maxpos >= npix - HALF_CENTROID_DOMAIN) return -1.0 ;

    /* Centroiding is only defined for non-negative intensities. If the
       centroiding region has negative intensities then find the minimum
       and offset the signal by this minimum */
    for (i=maxpos-HALF_CENTROID_DOMAIN; i<=maxpos+HALF_CENTROID_DOMAIN; i++)
        if (line[i] < min) min = line[i];

    /* The centroid pos is the weighted average over the max pixel neighborhood */
    centroid = 0.0 ;
    weights  = 0.0 ;
    for (i=maxpos-HALF_CENTROID_DOMAIN; i<=maxpos+HALF_CENTROID_DOMAIN; i++) {
        centroid += (double)(line[i]-min) * (double)i ;
        weights  += (double)(line[i]-min) ;
    }

    if (fabs(weights)<fabs(centroid)*FLT_EPSILON ) centroid = -1.0 ;
    else centroid /= weights ;
    
    return centroid ;    
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find out a local maximum in a 1d signal around a position.
  @param	line	Array of pixels.
  @param	npix	Number of pixels in the array.
  @param	where	Where to look around.
  @param	hs		Half-size of the search domain.
  @return	Local maximum position as a double.

  The closest local maximum to the given position is located to subpixel
  precision. This precision is achieved by simply weighting positions with
  pixelvalues.

  The 'where' parameter indicates where to look for a maximum as an index
  in the array, i.e. it must lie between 0 and npix-1 (inclusive). The 'hs'
  parameter indicates the half-size of the search domain, i.e. if hs=5 a
  local maximum will be searched +/-5 pixels around the requested position.

  Returns a negative value if an error occurred.
 */
/*--------------------------------------------------------------------------*/
double function1d_find_locmax(
		pixelvalue	*	line,
		int				npix,
		int				where,
		int				hs)
{
	pixelvalue	max ;
	double		centroid ;
	double		weights ;
	int			i, maxpos ;

	if ((where<hs) || (where>(npix-hs-1))) return (double)-1.0 ;
	
	/* Search for the closest local maximal around the requested range. */
	max = line[where] ;
	maxpos = where ;
	for (i=-hs ; i<=hs ; i++) {
		if (line[where+i]>max) {
			max = line[where+i] ;
			maxpos = where+i ;
		}
	}

	/* The centroid pos is the weighted average over the max pixel neighborhood */
	centroid = 0.0 ;
	weights  = 0.0 ;
	for (i=maxpos-hs; i<=maxpos+hs; i++) {
		centroid += (double)line[i] * (double)i ;
		weights  += (double)line[i] ;
	}
	if (fabs(weights)>fabs(centroid)*FLT_EPSILON) centroid /= weights ;
	else centroid = -1.0 ;
	
	return centroid ;	
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a low-pass filter to a 1d signal.
  @param	input_sig	Input signal
  @param	samples		Number of samples in the signal
  @param	filter_type	Type of filter to use.
  @param	hw			Filter half-width.
  @return	Pointer to newly allocated array of pixels.

  This kind of low-pass filtering consists in a convolution with a
  given kernel. The chosen filter type determines the kind of kernel
  to apply for convolution. Possible kernels and associated symbols
  can be found in function_1d.h.

  Smoothing the signal is done by applying this kind of low-pass
  filter several times.

  The returned smooth signal has been allocated using
  function1d_new(), it must be freed using function1d_del(). The
  returned signal has exactly as many samples as the input signal.
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_filter_lowpass(
		pixelvalue	*	input_sig,
		int				samples,
		int				filter_type,
		int				hw)
{
	pixelvalue	*	out_sig ;
	int				i, j ;
	double			replace ;
	double		*	kernel ;

	/* allocate output signal */
    out_sig = function1d_new(samples);

	/* generate low-pass filter kernel */
    kernel = function1d_generate_smooth_kernel(filter_type, hw) ;

    /* compute edge effects for the first hw elements */
    for (i=0 ; i<hw ; i++) {
        replace = 0.0 ;
        for (j=-hw ; j<=hw ; j++) {
            if (i+j<0) {
                replace += kernel[hw+j] * (double)input_sig[0] ;
            } else {
                replace += kernel[hw+j] * (double)input_sig[i+j] ;
            }
        }
        out_sig[i] = (pixelvalue)replace ;
    }

    /* compute edge effects for the last hw elements */
    for (i=samples-hw ; i<samples ; i++) {
        replace = 0.0 ;
        for (j=-hw ; j<=hw ; j++) {
            if (i+j>samples-1) {
                replace += kernel[hw+j] * (double)input_sig[samples-1] ;
            } else {
                replace += kernel[hw+j] * (double)input_sig[i+j] ;
            }
        }
        out_sig[i] = (pixelvalue)replace ;
    }

    /* compute all other elements */
    for (i=hw ; i<samples-hw ; i++) {
        replace = 0.0 ;
        for (j=-hw ; j<=hw ; j++) {
            replace += kernel[hw+j] * (double)input_sig[i+j] ;
        }
        out_sig[i] = (pixelvalue)replace ;
    }

    free(kernel) ;
    return out_sig ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Generate a kernel for smoothing filters (low-pass).
  @param	filt_type	Type of kernel to generate.
  @param	hw			Kernel half-width.
  @return	Pointer to newly allocated kernel.

  Supported kernels can be found in function_1d.h

  The returned array has been allocated with calloc(), it must be
  freed using free(). The returned array contains 2h+1 doubles, where
  h is the kernel half-width.
 */
/*--------------------------------------------------------------------------*/
static double * function1d_generate_smooth_kernel(int filt_type, int hw)
{
    double  *   kernel ;
    double      norm ;
    int         i ;

    kernel = (double*)calloc(2*hw+1, sizeof(double)) ;

	switch(filt_type) {

		case LOW_PASS_LINEAR:
		for (i=-hw ; i<=hw ; i++) {
			/* flat kernel */
			kernel[hw+i] = 1.0 / (double)(2*hw+1) ;
		}
		break ;

		case LOW_PASS_GAUSSIAN:
		norm = 0.00 ;
		for (i=-hw ; i<=hw ; i++) {
			/* gaussian kernel */
			kernel[hw+i] = exp(-(double)(i*i)) ;
			norm += kernel[hw+i] ;
		}
		for (i=0 ; i<2*hw+1 ; i++) {
			kernel[i] /= norm ;
		}
		break ;

		default:
		e_error("unrecognized low pass filter: cannot generate kernel") ;
		return (double*)NULL ;
		break ;
	}

    return kernel ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a 1d median filter of given half-width.
  @param	list		List of input pixelvalues.
  @param	np			Number of points in the list.
  @param	hw			Filter half-width.
  @return	Pointer to newly allocated array of pixelvalues.

  This function applies a median smoothing to a given signal and
  returns a newly allocated signal containing a median-smoothed
  version of the input. The returned array has exactly as many samples
  as the input array. It has been allocated using function1d_new() and
  must be deallocated using function1d_del().

  For half-widths of 1,2,3,4, the filtering is optimized for speed.
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_median_smooth(
		pixelvalue * list,
		int 		 np,
		int			 hw)
{
    int     		i,j ;
    pixelvalue	*	row ;
    pixelvalue	* 	smoothed ;

	/* simply copy first 3 and last 3 items */
    smoothed = function1d_new(np);
    for (i=0 ; i<hw ; i++) {
        smoothed[i] = list[i] ;
    }
    for (i=np-hw ; i<np ; i++) {
        smoothed[i] = list[i] ;
    }

	/* median filter on all central items */
	row = function1d_new(2*hw+1);
    for (i=hw ; i<np-hw ; i++) {
        for (j=-hw ; j<=hw ; j++) {
            row[j+hw] = list[i+j] ;
        }
		smoothed[i] = median_pixelvalue(row, 2*hw+1) ; 
    }
	function1d_del(row) ;
    return smoothed ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Subtract low-frequency components from a signal.
  @param	signal	Input signal.
  @param	ns		Number of samples.
  @return	Pointer to newly allocated array of pixelvalues.

  The returned signal is such as: out = in - smooth(in).

  The returned array has been allocated using function1d_dup(), it
  must be deallocated using function1d_del(). The returned array has
  exactly as many elements as the input array.
 */
/*--------------------------------------------------------------------------*/
#define LOWFREQ_PASSES		5
pixelvalue * function1d_remove_lowfreq(
		pixelvalue * signal,
		int			 ns)
{
	pixelvalue	*	sig_in ;
	pixelvalue	*	smooth ;
	int				i ;
	
	/* Apply severe low-pass filter several times */
	sig_in = function1d_dup(signal, ns);
	for (i=0 ; i<LOWFREQ_PASSES ; i++) {
		smooth = function1d_filter_lowpass( sig_in,
											ns,
											LOW_PASS_LINEAR,
											5);
		free(sig_in);
		sig_in = smooth ;
	}

	/* Subtract smoothed signal from input signal */
	for (i=0 ; i<ns ; i++) {
		smooth[i] = signal[i] - smooth[i];
	}
	return smooth ;
}
#undef LOWFREQ_PASSES


/*-------------------------------------------------------------------------*/
/**
  @brief	Remove thermal background from a signal.
  @param	signal	Input signal.
  @param	ns		Number of samples in the input signal.
  @return	Pointer to newly allocated array of pixelvalues.

  Many assumptions are made about the input signal. What is expected
  is typically the a collapsed image taken in K band, where the
  thermal background is rising as an exponential of the wavelength.

  This function tries to remove the thermal background contribution by
  first estimating it, then interpolating missing background values,
  and finally subtracting it from the input signal.

  The returned array has been allocated using function1d_new(), it
  must be freed using function1d_del(). The returned array has exactly
  as many samples as the input array.
 */
/*--------------------------------------------------------------------------*/
#define SAMPLE_BORDER	10
pixelvalue * function1d_remove_thermalbg(
		pixelvalue * signal,
		int			 ns)
{
	pixelvalue	*	smooth ;
	int				nmin ;
	pixelvalue		lef[2], rig[2];
	pixelvalue	*	x,
				*	y,
				*	spl_x,
				*	spl_y ;
	double			med_y ;
	double			avg2med ;
	double			dist ;
	FILE		*	tmp ;
	int				i ;
	
	/* Detect all local minima */
	nmin = 0 ;
	x = function1d_new(ns);
	y = function1d_new(ns);

	for (i=SAMPLE_BORDER ; i<(ns-SAMPLE_BORDER) ; i++) {
		lef[0] = signal[i-2];
		lef[1] = signal[i-1];
		rig[0] = signal[i+1];
		rig[1] = signal[i+2];

		if ( (signal[i] < lef[0]) &&
			 (signal[i] < lef[1]) &&
			 (signal[i] < rig[0]) &&
			 (signal[i] < rig[1])) {
			x[nmin] = (pixelvalue)i ;
			y[nmin] = signal[i];
			nmin ++ ;
		}
	}

	if (debug_active()>1) {
		tmp = fopen("minima", "w");
		for (i=0 ; i<nmin ; i++) {
			fprintf(tmp, "%g %g\n", x[i], y[i]);
		}
		fclose(tmp);
	}

	/* Interpolate linearly missing values */
	spl_x = function1d_new(ns);
	spl_y = function1d_new(ns);
	for (i=0 ; i<ns ; i++) {
		spl_x[i] = (pixelvalue)i ;
	}
	function1d_interpolate_linear(x, y, nmin, spl_x, spl_y, ns);

	function1d_del(x) ;
	function1d_del(y) ;
	function1d_del(spl_x);

	/* Compute median and average distance to the median */
	med_y = (double)median_pixelvalue(signal, ns);
	avg2med = 0.0 ;
	for (i=0 ; i<ns ; i++) {
		avg2med += fabs((double)signal[i] - med_y) ;
	}
	avg2med /= (double)ns ;

	/* Reset all pixels out of median + 2 * avg2med to zero. */
	for (i=0 ; i<ns ; i++) {
		dist = fabs((double)signal[i] - med_y);
		if (dist > (2.0*avg2med)) {
			spl_y[i] = (pixelvalue)0 ;
		}
	}

	if (debug_active()>1) {
		tmp = fopen("linear", "w");
		for (i=0 ; i<ns ; i++) {
			fprintf(tmp, "%d %g\n", i, spl_y[i]);
		}
		fclose(tmp);
	}

	smooth = function1d_new(ns);
	for (i=0 ; i<ns ; i++) {
		if (spl_y[i]>1e-4) {
			smooth[i] = signal[i] - spl_y[i];
		} else {
			smooth[i] = 0.0 ;
		}
	}
	function1d_del(spl_y);
	return smooth ;
}
#undef LOWFREQ_PASSES


/*-------------------------------------------------------------------------*/
/**
  @brief	Linear signal interpolation.
  @param	x		Input list of x positions.
  @param	y		Input list of y positions.
  @param	len		Number of samples in x and y.
  @param	spl_x	List of abscissas where the signal must be computed.
  @param	spl_y	Output list of computed signal values.
  @param	spl_len	Number of samples in spl_x and spl_y.
  @return	void

  To apply this interpolation, you need to provide a list of x and y
  positions, and a list of x positions where you want y to be computed
  (with linear interpolation).

  The returned signal has spl_len samples. It has been allocated using
  function1d_new() and must be deallocated using function1d_del().
 */
/*--------------------------------------------------------------------------*/
void function1d_interpolate_linear(
		pixelvalue	*	x,
		pixelvalue	*	y,
		int				len,
		pixelvalue	*	spl_x,
		pixelvalue	*	spl_y,
		int				spl_len)
{
	double		a, b ;
	int			i, j ;
	int			found ;

	for (i=0 ; i<spl_len ; i++) {
		/* Find (x1,y1) on the left of the current point */
		found = 0 ;
		for (j=0 ; j<(len-1) ; j++) {
			if ((spl_x[i]>=x[j]) && (spl_x[i]<=x[j+1])) {
				found++ ;
				break ;
			}
		}
		if (!found) {
			spl_y[i] = 0.0;
		} else {
			a = ((double)y[j+1]-(double)y[j]) /
				((double)x[j+1]-(double)x[j]);
			b = (double)y[j] - a * (double)x[j] ;
			spl_y[i] = (pixelvalue)(a * (double)spl_x[i] + b) ;
		}
	}
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Conducts a binary search for a value.
  @param	x			Contains the abscissas of interpolation.
  @param	len			Length of the x array.
  @param	key			The value to locate in x.
  @param	found_ptr	Output flag, 1 if value was found, else 0.
  @return	The index of the largest value in x for which x[i]<key.

  This function does a binary search for a value in an array. This
  routine is to be called only if key is in the interval between x[0]
  and x[len-1]. The input x array is supposed sorted.

 */
/*--------------------------------------------------------------------------*/
static int function1d_search_value(
    	pixelvalue	*	x,
    	int 			len,
    	pixelvalue 		key,
    	int 		*	found_ptr)
{
    int	high,
		low,
		middle;

    low  = 0;
    high = len - 1;

    while (high >= low) {
		middle = (high + low) / 2;
		if (key > x[middle]) {
			low = middle + 1;
		} else if (key < x[middle]) {
			high = middle - 1;
		} else {
			*found_ptr = 1;
			return (middle);
		}
    }
    *found_ptr = 0;
    return (low);
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Interpolate a vector along new abscissas.
  @param	x		List of x positions.
  @param	y		List of y positions.
  @param	len		Number of samples in x and y.
  @param	splx	Input new list of x positions.
  @param	sply	Output list of interpolated y positions.
  @param	spllen	Number of samples in splx and sply.
  @return	Int 0 if Ok, -1 if error.

  Reference:

  \begin{verbatim}
  	Numerical Analysis, R. Burden, J. Faires and A. Reynolds.
  	Prindle, Weber & Schmidt 1981 pp 112
  \end{verbatim}

  Provide in input a known list of x and y values, and a list where
  you want the signal to be interpolated. The returned signal is
  written into sply.
 */
/*--------------------------------------------------------------------------*/
int function1d_natural_spline(
		pixelvalue	* 	x,
    	pixelvalue	* 	y,
    	int 			len,
    	pixelvalue	* 	splx,
    	pixelvalue	* 	sply,
    	int 			spllen)
{
    int 			end;
    int 			loc,
					found;
    register int 	i,
					j,
					n;
    double 		*	h;			/* vector of deltas in x */
    double 		*	alpha;
    double 		*	l,
				*	mu,
				*	z,
				*	a,
				*	b,
				*	c,
				*	d,
					v;

    end = len - 1;

	a = malloc(sizeof(double) * spllen * 9) ;
    b = a + len;
    c = b + len;
    d = c + len;
    h = d + len;
    l = h + len;
    z = l + len;
    mu = z + len;
    alpha = mu + len;

    for (i = 0; i < len; i++) {
		a[i] = (double)y[i];
    }

    /* Calculate vector of differences */
    for (i = 0; i < end; i++) {
		h[i] = (double)x[i + 1] - (double)x[i];
		if (h[i] < 0.0) {
			free(a) ;
			return -1;
		}
    }

    /* Calculate alpha vector */
    for (n = 0, i = 1; i < end; i++, n++) {
		/* n = i - 1 */
		alpha[i] = 3.0 * ((a[i+1] / h[i]) - (a[i] / h[n]) - (a[i] / h[i]) +
				  (a[n] / h[n]));
    }

    /* Vectors to solve the tridiagonal matrix */
    l[0] = l[end] = 1.0;
    mu[0] = mu[end] = 0.0;
    z[0] = z[end] = 0.0;
    c[0] = c[end] = 0.0;

    /* Calculate the intermediate results */
    for (n = 0, i = 1; i < end; i++, n++) {
		/* n = i-1 */
		l[i] = 2 * (h[i] + h[n]) - h[n] * mu[n];
		mu[i] = h[i] / l[i];
		z[i] = (alpha[i] - h[n] * z[n]) / l[i];
    }
    for (n = end, j = end - 1; j >= 0; j--, n--) {
		/* n = j + 1 */
		c[j] = z[j] - mu[j] * c[n];
		b[j] = (a[n] - a[j]) / h[j] - h[j] * (c[n] + 2.0 * c[j]) / 3.0;
		d[j] = (c[n] - c[j]) / (3.0 * h[j]);
    }

    /* Now calculate the new values */
    for (j = 0; j < spllen; j++) {
		v = (double)splx[j];
		sply[j] = (pixelvalue)0;

		/* Is it outside the interval? */
		if ((v < (double)x[0]) || (v > (double)x[end])) {
			continue;
		}
		/* Search for the interval containing v in the x vector */
		loc = function1d_search_value(x, len, (pixelvalue)v, &found);
		if (found) {
			sply[j] = y[loc];
		} else {
			loc--;
			v -= (double)x[loc];
			sply[j] = (pixelvalue)(	a[loc] +
									v * (b[loc] +
									v * (c[loc] +
									v * d[loc])));
		}
    }
	free(a) ;
    return 0;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Sorts the input signal, takes out highest and lowest
  			values, and returns the average of the remaining pixels.
  @param	line		Input signal.
  @param	npix		Number of samples in the input signal.
  @param	pix_low		Number of lowest pixels to reject.
  @param	pix_high	Number of highest pixels to reject.
  @return	The filtered average of the input signal.

  No input parameter is modified.

  The input signal is first copied. This copy is then sorted, and the
  highest and lowest pixels are taken out of the list. Remaining
  pixelvalues are averaged and the result is returned.
 */
/*--------------------------------------------------------------------------*/
pixelvalue function1d_average_reject(
		pixelvalue	*	line,
		int				npix,
		int				pix_low,
		int				pix_high)
{
	pixelvalue	*	sorted ;
	int				i ;
	double			avg ;

	/* Sanity tests */
	if ((line==NULL) || (npix<1)) return (pixelvalue)0 ;
	if ((pix_low+pix_high)>=npix) return (pixelvalue)0 ;

	/* Copy input line and sort it */
	sorted = malloc(npix * sizeof(pixelvalue)) ;
	memcpy(sorted, line, npix * sizeof(pixelvalue)) ;
	pixel_qsort(sorted, npix);

	/* Find out average of remaining values */
	avg = 0.00 ;
	for (i=pix_low+1 ; i<(npix-pix_high) ; i++) {
		avg += (double)sorted[i] ;
	}
	free(sorted);
	avg /= (double)(npix - pix_high - pix_low) ;

	return (pixelvalue)avg ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute full width half Max
  @param    line    Array of pixelvalues.
  @param    npix	Size of the array.
  @param	max_pos	Position of the max. (NULL if not availlable)
  @param	Y		Threshold- half value (NULL if not availlable)	
  @return 	-1 in error case, otherwise the FwHM
 */
/*--------------------------------------------------------------------------*/
double function1d_get_fwhm(
		pixelvalue	*	line,
		int				npix,
		int			*	max_pos,
		double		*	Y)
{
	int			maxp ;
	pixelvalue	max ;
	pixelvalue	threshold ;
	pixelvalue	noise ;
    double      x_left, x_right ;
    int         x1, x2 ;
    pixelvalue  y1, y2 ;
    int         i ;

	/* Find out the maximum and its position if not provided */
	if (max_pos == NULL) {
		maxp = 0 ;
		max = line[maxp] ; 
		for (i=1 ; i<npix ; i++)
			if (line[i] > max) {
				max = line[i] ;
				maxp = i ;
			}
	} else {
		maxp = *max_pos ;
		max = line[maxp] ;
	}

	/* Find out the threshold if not provided */
	if (Y == NULL) {
		noise = find_noise_level_around_peak(line, npix, maxp) ;
		if (noise > line[maxp]*9/10) return -1.0 ;
		threshold = noise + (max - noise)/2.0 ;	
	} else threshold = *Y ;
	
    /* Find first value lower than Y on the left of the maximum */
    i = maxp ;
    while ((i>0) && (line[i]>threshold)) i-- ;
	
	if (i==0) return -1.0 ;
    x1 = i ;    y1 = line[x1] ;
    x2 = i+1 ;  y2 = line[x2] ;
	if (imstat_x_for_y_between_2_points(x1,y1,x2,y2,threshold,&x_left) != 0){
        return -1.0 ;
    }
    if (x_left<i) return -1.0;
	
	/* Find first value lower than Y on the right of the maximum */
    i = maxp ;
    while ((i<npix-1) && (line[i]>threshold) ) i++ ;
    if (i==(npix-1)) return -1.0 ;
    x1 = i-1 ;  y1 = line[x1] ;
    x2 = i ;    y2 = line[x2] ;
    if (imstat_x_for_y_between_2_points(x1,y1,x2,y2,threshold,&x_right) != 0){
        return -1.0 ;
    }
    if (x_right>=i) return -1.0;

    return x_right - x_left ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Cross-correlation of two 1d signals.
  @param    v1          The first signal.
  @param    width_1     Number of samples in first signal.
  @param    v2          The second signal.
  @param    width_2     Number of samples in second signal.
  @param    half_search Half-size of the search range.
  @param    delta       Output cross-correlation offset.
  @return   Maximum cross-correlation value as a double.

  The length of the two signals must currently be equal.

  The cross-correlation is computed with shifts ranging from -half_search
  to half_search.

  The cross-correlation number is in fact the dot-product of two unit-vectors
  and is therefore commutative (with a sign-change for delta) and ranging
  from -1 to 1.

  The returned delta is that which gives the maximimum cross-correlation.
  If, in absence of rounding erors, more than one shift would give the maximimum
  cross-correlation, rounding errors may cause any one of those shifts to be
  returned. If rounding errors have no effect delta is the shift with smallest
  absolute value (if two shifts with opposite sign and same absolute value both
  give the maximimum cross-correlation delta is negative).

  Cross-correlation with half_search == 0 requires about 8n FLOPs, where
  n is the number of samples.
  Each increase of half_search by 1 requires about 4m FLOPs more, where
  m = n - half_search is the number of elements that are cross-correlated.

  Returns -100.0 in case of error.
 */
/*--------------------------------------------------------------------------*/
double function1d_xcorrelate(
        pixelvalue  *   v1,
        int             width_1,
        pixelvalue  *   v2,
        int             width_2,
        int             half_search,
        double      *   delta)
{


    double   xc  = 0;
    double   mean2 = 0;
    double   mean1 = 0;
    double   var2  = 0;
    double   var1  = 0; 
    double   rwidth;
    const int width = width_1;
    int      i;


    if (width <= 0 || width_1 != width_2) return -100;

    rwidth = (double) 1 / width; /* less than maximal precision OK here */

    /* Compute mean, normalization factors and cross-correlation 
       - with zero delta */
    for (i=0 ; i<width ; i++) {
        mean1 += v1[i] ;
        mean2 += v2[i] ;
        var1  += v1[i] * v1[i];
        var2  += v2[i] * v2[i];
        xc    += v1[i] * v2[i];
    }
    mean1 *= rwidth;
    mean2 *= rwidth ;

    /* Correct for the mean */
    var1 -= mean1 * mean1 * width;
    var2 -= mean2 * mean2 * width;
    xc   -= mean1 * mean2 * width;

    /* var can only be zero with a constant vector
       - in which case xc is zero */ 
    if ( var1 > 0 && var2 > 0 )
        xc /= sqrt(var1 * var2);
    else {
        /* Remove some rounding errors */
        if (var1 < 0) var1 = 0;
        if (var2 < 0) var2 = 0;
        xc = 0;
    }

    *delta = 0; /* Why a double ? */

    if (half_search > 0 && xc < 1) {
        const double dwidth = 1 + rwidth;
        double   mean1_p = mean1;
        double   mean1_n = mean1;
        double   mean2_p = mean2;
        double   mean2_n = mean2;
        double   var1_p = var1;
        double   var1_n = var1;
        double   var2_p = var2;
        double   var2_n = var2;
        int      step ;

        /* No use to iterate further than width - 2 */
        if (half_search > width - 2) half_search = width - 2;

        for (step=1 ; step<=half_search ; step++) {
            double     xc_p = 0;
            double     xc_n = 0;
            const int istop = width-step;

            /* Correct means and normalization factors
               - define sample out of range to be zero */

            /* mean & var are in fact only changed
               when the dropped element is non-zero */
            var1_p  -= v1[step - 1] * (v1[step - 1] * dwidth - 2 * mean1_p);
            var1_n  -= v1[istop   ] * (v1[istop   ] * dwidth - 2 * mean1_n);
            var2_p  -= v2[istop   ] * (v2[istop   ] * dwidth - 2 * mean2_p);
            var2_n  -= v2[step - 1] * (v2[step - 1] * dwidth - 2 * mean2_n);

            mean1_p -= v1[step - 1] * rwidth;
            mean1_n -= v1[istop   ] * rwidth;
            mean2_p -= v2[istop   ] * rwidth;
            mean2_n -= v2[step - 1] * rwidth;

            for (i=0 ; i<istop ; i++) {
                xc_p += v1[i+step] * v2[i];
                xc_n += v2[i+step] * v1[i];
            }

            if (var1_n * var2_n > 0) {
                /* Subtract the mean-term */
                xc_n -= mean1_n * mean2_n * (2*width - istop);
                /* - and divide by the norm of the mean-corrected vectors */
                xc_n /= sqrt(var1_n * var2_n);
                if (xc_n > xc) {
                    xc = xc_n;
                    *delta = (double) -step; /* Why a double ? */
                }
            }
            if (var1_p * var2_p > 0) {
                /* Subtract the mean-term */
                xc_p -= mean1_p * mean2_p * (2*width - istop);
                /* - and divide by the norm of the mean-corrected vectors */
                xc_p /= sqrt(var1_p * var2_p);
                if (xc_p > xc) {
                    xc = xc_p;
                    *delta = (double) step; /* Why a double ? */
                }
            }
        }
    }

    /* xc can only be outside [-1;1] due to rounding errors */
    if (xc < -1) {
        xc = -1;
    } else if (xc > 1) {
        xc =  1;
    }
    return xc ;
}
