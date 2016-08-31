
/*-------------------------------------------------------------------------*/
/**
  @file		fft_base.c
  @author	N. Devillard
  @date		Oct 1999
  @version	$Revision: 1.5 $
  @brief	Base FFT routines

  This module offers very low-level FFT operators to work on arrays
  of doubles in N dimensions.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: fft_base.c,v 1.5 2001/10/23 12:25:31 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/23 12:25:31 $
	$Revision: 1.5 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "fft_base.h"
#include "pi.h"
#include "xmemory.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	N-dimensional FFT.
  @param	data		N-dimensional data set stored in 1d.
  @param	nn			Dimensions of the set.
  @param	ndim		How many dimensions this set has.
  @param	isign		Transform direction.
  @return	void

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

void
fftn(
	dcomplex data[],
	unsigned nn[],
	int ndim, 
	int isign)
{
	int 				idim;
	unsigned 			i1, i2rev, i3rev, ibit;
	unsigned 			ip2, ifp1, ifp2, k2, n;
	unsigned 			nprev = 1, ntot = 1;
	register unsigned 	i2, i3;
	double 				theta;
	dcomplex	 		w, wp;
	double 				wtemp;
	dcomplex	 		temp, wt;
	double				t1, t2 ;

	/*      Compute total number of complex values  */
	for (idim = 0; idim < ndim; ++idim)
		ntot *= nn[idim];

	for (idim = ndim - 1; idim >= 0; --idim) {
		n = nn[idim];

		ip2 = nprev * n;        /*  Unit step for next dimension */
		i2rev = 0;              /*  Bit reversed i2 */

		/*      This is the bit reversal section of the routine */
		/*      Loop over current dimension     */
		for (i2 = 0; i2 < ip2; i2 += nprev) {
			if (i2 < i2rev)
				/*      Loop over lower dimensions      */
				for (i1 = i2; i1 < i2 + nprev; ++i1)
					/*      Loop over higher dimensions  */
					for (i3 = i1; i3 < ntot; i3 += ip2) {
						i3rev = i3 + i2rev - i2;
						temp = data[i3];
						data[i3] = data[i3rev];
						data[i3rev] = temp;
					}

			ibit = ip2;
			/*      Increment from high end of i2rev to low */
			do {
				ibit >>= 1;
				i2rev ^= ibit;
			} while (ibit >= nprev && !(ibit & i2rev));
		}

		/*      Here begins the Danielson-Lanczos section of the routine */
		/*      Loop over step sizes    */
		for (ifp1 = nprev; ifp1 < ip2; ifp1 <<= 1) {
			ifp2 = ifp1 << 1;
			/*  Initialize for the trig. recurrence */
			theta = isign * 2.0 * PI_NUMB / (ifp2 / nprev);
			wp.x = sin(0.5 * theta);
			wp.x *= -2.0 * wp.x;
			wp.y = sin(theta);
			w.x = 1.0;
			w.y = 0.0;

			/*  Loop by unit step in current dimension  */
			for (i3 = 0; i3 < ifp1; i3 += nprev) {
				/*      Loop over lower dimensions      */
				for (i1 = i3; i1 < i3 + nprev; ++i1)
					/*  Loop over higher dimensions */
					for (i2 = i1; i2 < ntot; i2 += ifp2) {
						/*      Danielson-Lanczos formula */
						k2 = i2 + ifp1;
						wt = data[k2];

/*	Complex multiply using 3 real multiplies.  Should usually be faster.	*/
						data[k2].x = data[i2].x - (temp.x =
							(t1 = w.x * wt.x) - (t2 = w.y * wt.y));
						data[k2].y = data[i2].y - (temp.y =
							(w.x + w.y) * (wt.x + wt.y) - t1 - t2);
						data[i2].x += temp.x;
						data[i2].y += temp.y;
					}
				/*      Trigonometric recurrence        */
				wtemp = w.x;
/*	Complex multiply using 3 real multiplies.	*/
				w.x += (t1 = w.x * wp.x) - (t2 = w.y * wp.y);
				w.y += (wtemp + w.y) * (wp.x + wp.y) - t1 - t2;
			}
		}
		nprev *= n;
	}
	
	return ;
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Find if a given integer is a power of 2.
  @param	p	Integer to check.
  @return	The corresponding power of 2, or -1.

  If the given number is an integer power of 2, the power is returned.
  Otherwise -1 is returned.

  Examples:

  is_power_of_2(1024) returns 10
  is_power_of_2(1023) returns -1

 */
/*--------------------------------------------------------------------------*/

int
is_power_of_2(int p)
{
	float	c ;
	int		power2 ;

	if (p == 0) { /* Yes, 0 is a power of 2	*/
		power2 =  1 ;
	} else if (p<0) { /* No, negatives are no power of 2 (in R at least) */
		power2 = -1 ;
	} else { /* Compute log in base 2	*/
		c = (float)(log((double)p) / log(2.0)) ;
		if (c == (float)((int)c)) {
			power2 = (int)c ;
		} else {
			power2 = -1 ;
		}
	}
	
	return power2 ;
}

