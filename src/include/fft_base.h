
/*-------------------------------------------------------------------------*/
/**
  @file     fft_base.h
  @author   N. Devillard
  @date     Oct 1999
  @version  $Revision: 1.9 $
  @brief    Base FFT routines

  This module offers very low-level FFT operators to work on arrays
  of doubles in N dimensions.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: fft_base.h,v 1.9 2001/10/23 12:25:32 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/23 12:25:32 $
	$Revision: 1.9 $
*/

#ifndef _FFT_BASE_H_
#define _FFT_BASE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

/**
 * Since a Fourier transform involves complex numbers, this type stores a
 * complex number in memory as a couple of doubles.
 */

typedef struct _DOUBLE_COMPLEX_ {
	    double x, y ;
} dcomplex ;



/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Request a forward FFT */
#define FFT_FORWARD		 1
/** Request an inverse FFT */
#define FFT_INVERSE		-1


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    N-dimensional FFT.
  @param    data        N-dimensional data set stored in 1d.
  @param    nn          Dimensions of the set.
  @param    ndim        How many dimensions this set has.
  @param    isign       Transform direction.
  @return   void

  This routine is a public domain FFT. See extract of Usenet article
  below. Found on http://www.tu-chemnitz.de/~arndt/joerg.html

  @verbatim
  From: alee@tybalt.caltech.edu (Andrew Lee)
  Newsgroups: comp.sources.misc
  Subject: N-dimensional, Radix 2 FFT Routine
  Date: 17 Jul 87 22:26:29 GMT
  Approved: allbery@ncoast.UUCP
  X-Archive: comp.sources.misc/8707/48

  [..]
  Now for the usage (finally):
  data[] is the array of complex numbers to be transformed,
  nn[] is the array giving the dimensions (I mean size) of the array,
  ndim is the number of dimensions of the array, and
  isign is +1 for a forward transform, and -1 for an inverse transform.

  data[] and nn[] are stored in the "natural" order for C:
  nn[0] gives the number of elements along the leftmost index,
  nn[ndim - 1] gives the number of elements along the rightmost index, and
  data should be declared along the lines of
  struct (f)complex data[nn[0], nn[1], ..., nn[ndim - 1]]

  Additional notes: The routine does NO NORMALIZATION, so if you do a
  forward, and then an inverse transform on an array, the result will
  be identical to the original array MULTIPLIED BY THE NUMBER OF
  ELEMENTS IN THE ARRAY.  Also, of course, the dimensions of data[]
  must all be powers of 2.
  @endverbatim

 */
/*--------------------------------------------------------------------------*/
void fftn(dcomplex data[], unsigned nn[], int ndim, int isign);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find if a given integer is a power of 2.
  @param    p   Integer to check.
  @return   The corresponding power of 2, or -1.

  If the given number is an integer power of 2, the power is returned.
  Otherwise -1 is returned.

  Examples:

  is_power_of_2(1024) returns 10
  is_power_of_2(1023) returns -1

 */
/*--------------------------------------------------------------------------*/
int is_power_of_2(int p);


#endif
