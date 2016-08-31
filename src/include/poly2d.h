/*-------------------------------------------------------------------------*/
/**
   @file    poly2d.h
   @author  N. Devillard
   @date    22 Jun 1999
   @version $Revision: 1.11 $
   @brief   2D polynomial handling
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: poly2d.h,v 1.11 2001/11/07 14:55:14 yjung Exp $
    $Author: yjung $
    $Date: 2001/11/07 14:55:14 $
    $Revision: 1.11 $
*/

#ifndef _POLY2D_H_
#define _POLY2D_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipow.h"
#include "xmemory.h"

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	2d Polynomial object.

  The following structure defines a 2d polynomial. 'deg' is the
  highest degree in the polynomial.
  
  \begin{itemize}
  \item 'nc' contains the number of coefficients in px, py, and c.
  \item 'px' and 'py' contain the powers of resp. x and y.
  \item 'c' contains the coefficients themselves.
  \end{itemize}

  For example, if you want to store the following polynomial:
  \begin{verbatim}
  p(x,y) = p0 + p1.x + p2.y + p3.x.y + p4.x^2 + p5.y^2
  \end{verbatim}

  You would have:

  \begin{verbatim}
  nc  = 6 (from 0 to 5 incl.)
  px contains:  0  1  0  1  2  0
  py contains:  0  0  1  1  0  2
  c  contains: p0 p1 p2 p3 p4 p5
  So that given x0 and y0, computing the polynomial is done with:
 
  poly2d    p ;
  int       i ;
  double    x0, y0, poly ;
 
  poly = 0.00 ;
  for (i=0 ; i<p.nc ; i++) {
      poly += p.c[i] * ipow(x0, p.px[i]) * ipow(y0, p.py[i]) ;
  }

  or simply:
  poly = poly2d_compute(&p, x0, y0);
  \end{verbatim}
 */
/*--------------------------------------------------------------------------*/
typedef struct _2D_POLY_ {
	int			nc ;		/* number of coefficients in px, py, c */
	int		*	px ;		/* powers of x                         */
	int		*	py ;		/* powers of y                         */
	double	*	c ;			/* polynomial coefficients             */
} poly2d ;

/*---------------------------------------------------------------------------
   							Function codes	
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the value of a poly2d at a given point.
  @param    p   Poly2d object.
  @param    x   x coordinate.
  @param    y   y coordinate.
  @return   The value of the 2d polynomial at (x,y) as a double.

  This computes the value of a poly2d in a single point. To
  compute many values in a row, see poly2d_compute_array().
 */
/*--------------------------------------------------------------------------*/
double poly2d_compute(
        poly2d  *   p,
        double      x,
        double      y) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the value of a poly2d at a list of points.
  @param    p   Poly2d to compute
  @param    x   Array of x values.
  @param    y   Array of y values.
  @param    z   Returned array of 2d polynomial values.
  @param    n   Number of values in each array: x, y, z.
  @return   int 0 if Ok, -1 if error.

  This function computes many poly2d values in a row. Provide two
  arrays of x and y positions (x[i] being associated to y[i]) and an
  allocated array z to store the results. The computation is:

  \begin{verbatim}
  z[i] = p(x[i], y[i])
  \end{verbatim}

  If anything goes wrong during the computation, this function returns
  -1.
 */
/*--------------------------------------------------------------------------*/
int poly2d_compute_array(
        poly2d  *   p,
        double  *   x,
        double  *   y,
        double  *   z,
        int         n) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Build a poly2d from a character string definition.
  @param    s   Poly2d definition as a character string.
  @return   A newly allocated poly2d definition.

  The format of the definition string is the following:

   "dx dy c0 dx dy c1 ... dx dy cn"
   "int int double ... int int double"

   Poly2d coefficients are given through triplets. First and
   second indicate respectively the exponents of x and y. The third
   number is the polynomial coefficient associated to this (x,y)
   couple.

   Example:

   to input the following poly2d in x and y:
   \begin{verbatim}
   z = 12.0 + 24.0*x + 36.0*y + 10.0*x*y - 3.0*x^2 - 5.0*y^2
   \end{verbatim}
   you would provide the following string:

   \begin{verbatim}
   "0 0 12.0 1 0 24.0 0 1 36.0 1 1 10.0 2 0 -3.0 0 2 -5.0"
   \end{verbatim}

   Notice that the returned object is a newly allocated poly2d,
   that must be freed by the caller using poly2d_free().
 */
/*--------------------------------------------------------------------------*/
poly2d * poly2d_build_from_string(char * s) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocated space to store a poly2d definition.
  @param    nc  Number of coefficients to store.
  @return   Newly allocated poly2d object.

  This function allocates three arrays in the poly2d object,
  containing respectively the degree of x, the degree of y, and the
  associated coefficient (as a double).

  The returned object must be freed using poly2d_free().
 */
/*--------------------------------------------------------------------------*/
poly2d * poly2d_allocate(int nc) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Frees space allocated for a poly2d object.
  @param    p   Poly2d object to free.
  @return   void

  This destructor is mandatorily called to destroy poly2d objects.
 */
/*--------------------------------------------------------------------------*/
void poly2d_free(poly2d * p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Print out a poly2d object on console (stderr).
  @param    p       Poly2d to print.
  @param    name    (Optional) name of the poly2d.
  @return   void

  This is for debugging purposes mostly. Giving a name to the
  poly2d is optional, to help discriminate information on the
  console. Provide NULL if you do not want to give a name.
 */
/*--------------------------------------------------------------------------*/
void poly2d_print(
        poly2d  *   p,
        char    *   name) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Read a 2d polynomial from a FITS table.
  @param    filename    Name of the FITS table file.
  @return   Newly allocated poly2d object.

  This function is somewhat specific. It requires that a poly2d object
  has been saved into a FITS table respecting the following format:

  \begin{itemize}
  \item The first table column contains the degrees for x.
  \item The second table column contains the degrees for y.
  \item The third table column contains the polynomial coefficients.
  \end{itemize}

  Example:

  \begin{verbatim}
  P(x,y) = 15 + 34*x + 79*y - 94*x*y + 61*x^2
  This polynomial is stored in the table as:

  degree_x      degree_y        coefficient
  -----------------------------------------
  0             0                15
  1             0                34
  0             1                79
  1             1               -94
  2             0                61
  \end{verbatim}

  As usual, the returned poly2d object is newly allocated, it must be
  freed using poly2d_free().
 */
/*--------------------------------------------------------------------------*/
poly2d * read_poly2d_from_table(char * filename) ;

#endif
