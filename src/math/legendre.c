/*-------------------------------------------------------------------------*/
/**
   @file	legendre.c
   @author	N. Devillard
   @date	July 2000
   @version	$Revision: 1.5 $
   @brief	Legendre polynomials
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: legendre.c,v 1.5 2001/11/07 13:53:22 yjung Exp $
	$Author: yjung $
	$Date: 2001/11/07 13:53:22 $
	$Revision: 1.5 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "legendre.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Computes the value of the Legendre polynomial of degree n.
  @param	order	Legendre polynomial order.
  @param	x		Where to compute the polynomial.
  @return	1 double

  Computes the value of a Legendre polynomial of given order at a given
  point. The first 6 polynomials are hardcoded into #define for efficiency
  reasons and ease of use. The following ones are computed through the
  recurrence relation:

  \begin{verbatim}
  P(i+1) = [(2i + 1) x P(i) - i P(i-1)] / (i + 1)
  P(i) = [(2i - 1) x P(i-1) - (i-1) P(i-2) / i
       { oddx  }        prev_di          di
  \end{verbatim}

  Notice that the polynomial computation itself is not optimized, to favour
  readability.
 */
/*--------------------------------------------------------------------------*/
double legendre(
		int		order, 
		double 	x)
{
	double	p0 ;
	double	p1 ;
	double	di ;
	double  twox ;
	double	oddx ;
	double	prev_di, pnow ;
	int		j ;

	pnow = 0 ;
	if (order<0) return pnow ;

	switch (order) {
		case 0: pnow = legendre_0(x); break ;
		case 1: pnow = legendre_1(x); break ;
		case 2: pnow = legendre_2(x); break ;
		case 3: pnow = legendre_3(x); break ;
		case 4: pnow = legendre_4(x); break ;
		case 5: pnow = legendre_5(x); break ;
		default:
			p0   = 1.00 ;
			p1   = x ;
			di   = 1.0 ;
			twox = 2.0 * x ;
			oddx = x ;

			for (j=2 ; j<=order ; j++) {
				prev_di = di ;
				di += 1.0 ;
				oddx += twox ;
				/* recurrence relation */
				pnow = (oddx * p1 - prev_di * p0) / di ;
				p0 = p1 ;
				p1 = pnow ;
			}
		break ;
	}
	return pnow ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute Legendre polynomial values for a list of numbers.
  @param	order	Legendre polynomial order
  @param	n		Number of points in the list
  @param	x		List of points where to compute the polynomial values.
  @return	1 newly allocated array of n doubles.

  Allocates a new array of as many doubles as in the input array, and fills
  it up with Legendre polynomial values. The returned array must be
  deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * legendre_vector(
		int			order, 
		int 		n, 
		double 	* 	x)
{
	double	*	y ;
	int			i ;

	y = malloc(n * sizeof(double));
	for (i=0 ; i<n ; i++) {
		y[i] = legendre(order, x[i]);
	}
	return y ;
}


