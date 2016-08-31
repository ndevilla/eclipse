
/*-------------------------------------------------------------------------*/
/**
  @file		chebyshev.c
  @author	N. Devillard
  @date		Sep 2001
  @version	$Revision: 1.2 $
  @brief	Chebyshev polynomials

  This module implements various utilities related to chebyshev
  polynomials. It is stand-alone, i.e. it does not rely on any
  special kind of struct to work.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: chebyshev.c,v 1.2 2001/10/23 12:02:34 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/23 12:02:34 $
	$Revision: 1.2 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chebyshev.h"


/*
 * Static storage of the first 21 Chebyshev polynomial coefficients.
 * Coefficients are stored in increasing degrees: from 0 to n.
 */

/** Max order for hardcoded Chebyshev polynomial */
#define CHEBYSHEV_MAXORDER	20

static int cheb_0[]={1};
static int cheb_1[]={0,1};
static int cheb_2[]={-1,0,2};
static int cheb_3[]={0,-3,0,4};
static int cheb_4[]={1,0,-8,0,8};
static int cheb_5[]={0,5,0,-20,0,16};
static int cheb_6[]={-1,0,18,0,-48,0,32};
static int cheb_7[]={0,-7,0,56,0,-112,0,64};
static int cheb_8[]={1,0,-32,0,160,0,-256,0,128};
static int cheb_9[]={0,9,0,-120,0,432,0,-576,0,256};
static int cheb_10[]={-1,0,50,0,-400,0,1120,0,-1280,0,512};
static int cheb_11[]={0,-11,0,220,0,-1232,0,2816,0,-2816,0,1024};
static int cheb_12[]={1,0,-72,0,840,0,-3584,0,6912,0,-6144,0,2048};
static int cheb_13[]={0,13,0,-364,0,2912,0,-9984,0,16640,0,-13312,0,4096};
static int cheb_14[]={-1,0,98,0,-1568,0,9408,0,-26880,0,39424,0,-28672,0,8192};
static int cheb_15[]={0,-15,0,560,0,-6048,0,28800,0,-70400,0,92160,0,-61440,0,16384};
static int cheb_16[]={1,0,-128,0,2688,0,-21504,0,84480,0,-180224,0,212992,0,-131072,0,32768};
static int cheb_17[]={0,17,0,-816,0,11424,0,-71808,0,239360,0,-452608,0,487424,0,-278528,0,65536};
static int cheb_18[]={-1,0,162,0,-4320,0,44352,0,-228096,0,658944,0,-1118208,0,1105920,0,-589824,0,131072};
static int cheb_19[]={0,-19,0,1140,0,-20064,0,160512,0,-695552,0,1770496,0,-2723840,0,2490368,0,-1245184,0,262144};
static int cheb_20[]={1,0,-200,0,6600,0,-84480,0,549120,0,-2050048,0,4659200,0,-6553600,0,5570560,0,-2621440,0,524288};

static int * chebyshev_c[CHEBYSHEV_MAXORDER+1] = {
	cheb_0, cheb_1, cheb_2, cheb_3, cheb_4, cheb_5, cheb_6,
	cheb_7, cheb_8, cheb_9, cheb_10, cheb_11, cheb_12, cheb_13,
	cheb_14, cheb_15, cheb_16, cheb_17, cheb_18, cheb_19, cheb_20,
};



/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Computes the value of the Chebyshev polynomial of degree n.
  @param	order	Chebyshev polynomial order.
  @param	x		Where to compute the polynomial.
  @return	1 double

  Computes the value of a Chebyshev polynomial of given order at
  a given point. The first 5 polynomials are hardcoded into #define
  for efficiency reasons and ease of use. The following ones are
  computed through the following recurrence relation:

  T[n+1](x) = 2xT[n](x) - T[n-1](x)
 */
/*--------------------------------------------------------------------------*/

double chebyshev(int order, double x)
{
	double	p ;

	if (order<0) return 0 ;
	switch(order) {

		case 0: p = chebyshev_0(x); break ;
		case 1: p = chebyshev_1(x); break ;
		case 2: p = chebyshev_2(x); break ;
		case 3: p = chebyshev_3(x); break ;
		case 4: p = chebyshev_4(x); break ;
		case 5: p = chebyshev_5(x); break ;
		default:
		p = 2.0*x*chebyshev(order-1, x)-chebyshev(order-2, x);
		break ;
	}
	return p ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute Chebyshev polynomial values for a list of doubles.
  @param	order		Chebyshev polynomial order.
  @param	n			Number of points in the list.
  @param	x			List of points where to compute values.
  @return	1 newly allocated array of n doubles.

  Allocates a new array of as many doubles as in the input array, and
  fills it up with Chebyshev polynomial values. The returned array must
  be deallocated using free().
 */
/*--------------------------------------------------------------------------*/

double * chebyshev_vector(int order, int n, double * x)
{
	double * y ;
	int		 i ;

	if (x==NULL || n<1 || order<0) return NULL ;

	y = malloc(n * sizeof(double));
	for (i=0 ; i<n ; i++) {
		y[i] = chebyshev(order, x[i]);
	}
	return y ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Build a list of Chebyshev polynomial coefficients
  @param	order	Order of the Chebyshev polynomial
  @return	1 pointer to an array of (order+1) ints.

  This function builds a list of Chebyshev coefficients for the
  requested order. Low-orders have been pre-computed and are
  statically declared in this module, further orders are derived
  recursively.
  The number of coefficients is always (order+1).

  The returned pointer points to a statically allocated array inside
  this module. Do not free it or modify it!
 */
/*--------------------------------------------------------------------------*/

int * chebyshev_coefs(int order)
{
	if (order<0 || order>CHEBYSHEV_MAXORDER) return NULL ;
	return chebyshev_c[order];
}
