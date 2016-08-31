/*-------------------------------------------------------------------------*/
/**
   @file    function_1d.c
   @author  Nicolas Devillard
   @date    Tue, Sept 23 1997
   @version $Revision: 1.17 $
   @brief   1d signal processing related routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: function_1d.h,v 1.17 2004/01/08 15:19:00 llundin Exp $
    $Author: llundin $
    $Date: 2004/01/08 15:19:00 $
    $Revision: 1.17 $
*/

#ifndef _FUNCTION_1D_H_
#define _FUNCTION_1D_H_

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

#include "xmemory.h"
#include "local_types.h"
#include "pixel_handling.h"
#include "median.h"

/*----------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define LOW_PASS_LINEAR			100
#define LOW_PASS_GAUSSIAN		101

/*----------------------------------------------------------------------------
  						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocates a new array of pixelvalues.
  @param    nsamples    Number of values to store in the array.
  @return   Pointer to newly allocated array of pixelvalues.

  The returned array must be freed using function1d_del(), not free().
  This is in case some further housekeeping attributes are allocated
  together with the object in the future.

  Returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_new(int nsamples) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Deallocate an array of pixelvalues.
  @param    s   Array to deallocate.
  @return   void

  Deallocates an array allocated by function1d_new().
 */
/*--------------------------------------------------------------------------*/
void function1d_del(pixelvalue * s) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Copy an array of pixelvalues to a new array.
  @param    arr     Array to copy.
  @param    ns      Number of samples in the array.
  @return   Pointer to newly allocated array of pixelvalues.

  Creates a new array using function1d_new(), with the same number of
  samples as the input signal, then copies over all values from source
  to destination array using memcpy().

  The returned array must be freed using function1d_del(), not free().
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_dup(pixelvalue * arr, int ns) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find out a line centroid to subpixel precision.
  @param    line    Array of pixels.
  @param    npix    Number of pixels in the array.
  @return   Centroid position as a double.

  The input signal is assumed to be flat almost everywhere, with a
  single peak somewhere around the middle. Other kinds of signals are
  not handled correctly.

  The position of the peak is located to subpixel precision by
  simply weighting positions with pixelvalues.
 */
/*--------------------------------------------------------------------------*/
double function1d_find_centroid(
        pixelvalue  *   line,
        int             npix) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find out a local maximum in a 1d signal around a position.
  @param    line    Array of pixels.
  @param    npix    Number of pixels in the array.
  @param    where   Where to look around.
  @param    hs      Half-size of the search domain.
  @return   Local maximum position as a double.

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
        pixelvalue  *   line,
        int             npix,
        int             where,
        int             hs) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a low-pass filter to a 1d signal.
  @param    input_sig   Input signal
  @param    samples     Number of samples in the signal
  @param    filter_type Type of filter to use.
  @param    hw          Filter half-width.
  @return   Pointer to newly allocated array of pixels.

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
        pixelvalue  *   input_sig,
        int             samples,
        int             filter_type,
        int             hw) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Apply a 1d median filter of given half-width.
  @param    list        List of input pixelvalues.
  @param    np          Number of points in the list.
  @param    hw          Filter half-width.
  @return   Pointer to newly allocated array of pixelvalues.

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
        int          np,
        int          hw) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract low-frequency components from a signal.
  @param    signal  Input signal.
  @param    ns      Number of samples.
  @return   Pointer to newly allocated array of pixelvalues.

  The returned signal is such as: out = in - smooth(in).

  The returned array has been allocated using function1d_dup(), it
  must be deallocated using function1d_del(). The returned array has
  exactly as many elements as the input array.
 */
/*--------------------------------------------------------------------------*/
pixelvalue * function1d_remove_lowfreq(
        pixelvalue * signal,
        int          ns) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Remove thermal background from a signal.
  @param    signal  Input signal.
  @param    ns      Number of samples in the input signal.
  @return   Pointer to newly allocated array of pixelvalues.

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
pixelvalue * function1d_remove_thermalbg(
        pixelvalue * signal,
        int          ns) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Linear signal interpolation.
  @param    x       Input list of x positions.
  @param    y       Input list of y positions.
  @param    len     Number of samples in x and y.
  @param    spl_x   List of abscissas where the signal must be computed.
  @param    spl_y   Output list of computed signal values.
  @param    spl_len Number of samples in spl_x and spl_y.
  @return   void

  To apply this interpolation, you need to provide a list of x and y
  positions, and a list of x positions where you want y to be computed
  (with linear interpolation).

  The returned signal has spl_len samples. It has been allocated using
  function1d_new() and must be deallocated using function1d_del().
 */
/*--------------------------------------------------------------------------*/
void function1d_interpolate_linear(
        pixelvalue  *   x,
        pixelvalue  *   y,
        int             len,
        pixelvalue  *   spl_x,
        pixelvalue  *   spl_y,
        int             spl_len) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Interpolate a vector along new abscissas.
  @param    x       List of x positions.
  @param    y       List of y positions.
  @param    len     Number of samples in x and y.
  @param    splx    Input new list of x positions.
  @param    sply    Output list of interpolated y positions.
  @param    spllen  Number of samples in splx and sply.
  @return   Int 0 if Ok, -1 if error.

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
        pixelvalue  *   x,
        pixelvalue  *   y,
        int             len,
        pixelvalue  *   splx,
        pixelvalue  *   sply,
        int             spllen) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Sorts the input signal, takes out highest and lowest
            values, and returns the average of the remaining pixels.
  @param    line        Input signal.
  @param    npix        Number of samples in the input signal.
  @param    pix_low     Number of lowest pixels to reject.
  @param    pix_high    Number of highest pixels to reject.
  @return   The filtered average of the input signal.

  No input parameter is modified.

  The input signal is first copied. This copy is then sorted, and the
  highest and lowest pixels are taken out of the list. Remaining
  pixelvalues are averaged and the result is returned.
 */
/*--------------------------------------------------------------------------*/
pixelvalue function1d_average_reject(
        pixelvalue  *   line,
        int             npix,
        int             pix_low,
        int             pix_high) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute full width half Max
  @param    line    Array of pixelvalues.
  @param    npix    Size of the array.
  @param    max_pos Position of the max. (NULL if not availlable)
  @param    Y       Threshold- half value (NULL if not availlable)  
  @return   -1 in error case, otherwise the FwHM
 */
/*--------------------------------------------------------------------------*/
double function1d_get_fwhm(
        pixelvalue  *   line,
        int             npix,
        int         *   max_pos,
        double      *   Y) ;


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
        pixelvalue *    v1,
        int             width_1,
        pixelvalue *    v2,
        int             width_2,
        int             half_search,
        double     *    delta) ;

#endif
