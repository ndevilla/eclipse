
/*-------------------------------------------------------------------------*/
/**
  @file     chebyshev.h
  @author   N. Devillard
  @date     Sep 2001
  @version  $Revision: 1.2 $
  @brief    Chebyshev polynomials

  This module implements various utilities related to chebyshev
  polynomials. It is stand-alone, i.e. it does not rely on any
  special kind of struct to work.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: chebyshev.h,v 1.2 2001/10/23 12:02:35 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/23 12:02:35 $
	$Revision: 1.2 $
*/

#ifndef _CHEBYSHEV_H_
#define _CHEBYSHEV_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** Hardcoded Chebyshev polynomial of order 0 */
#define chebyshev_0(x)	(1.0)
/** Hardcoded Chebyshev polynomial of order 1 */
#define chebyshev_1(x)	(x)
/** Hardcoded Chebyshev polynomial of order 2 */
#define chebyshev_2(x)	(2.0*(x)*(x)-1.00)
/** Hardcoded Chebyshev polynomial of order 3 */
#define chebyshev_3(x)	(4.0*(x)*(x)*(x)-3.0*(x))
/** Hardcoded Chebyshev polynomial of order 4 */
#define chebyshev_4(x)	(8.0*(x)*(x)*(x)*(x)-8.0*(x)*(x)+1)
/** Hardcoded Chebyshev polynomial of order 5 */
#define chebyshev_5(x)	(16.0*(x)*(x)*(x)*(x)*(x)-20.0*(x)*(x)*(x)+5.0*(x))

/*---------------------------------------------------------------------------
						Function ANSI prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Computes the value of the Chebyshev polynomial of degree n.
  @param    order   Chebyshev polynomial order.
  @param    x       Where to compute the polynomial.
  @return   1 double

  Computes the value of a Chebyshev polynomial of given order at
  a given point. The first 5 polynomials are hardcoded into #define
  for efficiency reasons and ease of use. The following ones are
  computed through the following recurrence relation:

  T[n+1](x) = 2xT[n](x) - T[n-1](x)
 */
/*--------------------------------------------------------------------------*/
double chebyshev(int order, double x);

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute Chebyshev polynomial values for a list of doubles.
  @param    order       Chebyshev polynomial order.
  @param    n           Number of points in the list.
  @param    x           List of points where to compute values.
  @return   1 newly allocated array of n doubles.

  Allocates a new array of as many doubles as in the input array, and
  fills it up with Chebyshev polynomial values. The returned array must
  be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * chebyshev_vector(int order, int n, double * x);

/*-------------------------------------------------------------------------*/
/**
  @brief    Build a list of Chebyshev polynomial coefficients
  @param    order   Order of the Chebyshev polynomial
  @return   1 pointer to an array of (order+1) ints.

  This function builds a list of Chebyshev coefficients for the
  requested order. Low-orders have been pre-computed and are
  statically declared in this module, further orders are derived
  recursively.
  The number of coefficients is always (order+1).

  The returned pointer points to a statically allocated array inside
  this module. Do not free it or modify it!
 */
/*--------------------------------------------------------------------------*/
int * chebyshev_coefs(int order);

#endif
