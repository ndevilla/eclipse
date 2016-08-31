/*-------------------------------------------------------------------------*/
/**
   @file    legendre.h
   @author  N. Devillard
   @date    July 2000
   @version $Revision: 1.4 $
   @brief   Legendre polynomials
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: legendre.h,v 1.4 2001/11/07 13:53:24 yjung Exp $
    $Author: yjung $
    $Date: 2001/11/07 13:53:24 $
    $Revision: 1.4 $
*/

#ifndef _LEGENDRE_H_
#define _LEGENDRE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define legendre_0(x)	(1.00)
#define legendre_1(x)	(x)
#define legendre_2(x)	(-0.5+1.5*(x)*(x))
#define legendre_3(x)	(-1.5*(x)+2.5*(x)*(x)*(x))
#define legendre_4(x)	(0.375-3.75*(x)*(x)+4.375*(x)*(x)*(x)*(x))
#define legendre_5(x)	(1.875*(x)-8.75*(x)*(x)*(x)+7.875*(x)*(x)*(x)*(x)*(x))

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Computes the value of the Legendre polynomial of degree n.
  @param    order   Legendre polynomial order.
  @param    x       Where to compute the polynomial.
  @return   1 double

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
        int     order, 
        double  x) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute Legendre polynomial values for a list of numbers.
  @param    order   Legendre polynomial order
  @param    n       Number of points in the list
  @param    x       List of points where to compute the polynomial values.
  @return   1 newly allocated array of n doubles.

  Allocates a new array of as many doubles as in the input array, and fills
  it up with Legendre polynomial values. The returned array must be
  deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * legendre_vector(
        int         order,
        int         n,
        double  *   x) ;

#endif
