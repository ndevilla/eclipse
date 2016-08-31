/*-------------------------------------------------------------------------*/
/**
   @file    fit_curve.h
   @author  N. Devillard
   @date    July 1998
   @version $Revision: 1.15 $
   @brief   1d and 2d fit related routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: fit_curve.h,v 1.15 2003/02/28 08:37:08 yjung Exp $
    $Author: yjung $
    $Date: 2003/02/28 08:37:08 $
    $Revision: 1.15 $
*/

#ifndef _FIT_CURVE_H_
#define _FIT_CURVE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "xmemory.h"
#include "matrix.h"
#include "median.h"
#include "ipow.h"
#include "doubles.h"

/* Following definitions are need if compiled out of eclipse */
#ifndef _ECLIPSE_TYPES_H_
#define e_error printf
#include "types.h"

static int Debug = 0 ;

#endif

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Fit a polynomial to a list of points.
  @param    poly_deg    Degree of the polynomial to fit.
  @param    list        List of points as a double3.
  @param    mse         Output mean squared error.
  @return   Array of (np+1) polynomial coefficients.

  The fitted polynomial is such that:
  \[
    P(x) = c_0 + c_1 x + c_2 x^2 + ... + c_n x^n
  \]
  So requesting a polynomial of degree n will return n+1 coefficients.
  Beware that with such polynomials, two input points shall never be
  on the same vertical!

  If you are not interested in getting the mean squared error back,
  feed in NULL instead of a pointer to a double for mse.

  The returned pointer must be freed using free().
 */
/*--------------------------------------------------------------------------*/
double * fit_1d_poly(
        int         poly_deg,
        double3 *   list,
        double  *   mse) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Fit a 2d surface with a polynomial in x and y.
  @param    surface         List of pixel on the surface.
  @param    control_string  String defining the polynomial to compute.
  @param    poly_deg        Maximal polynomial degree.
  @param    ncoeffs         Output number of computed coefficients.
  @param    mse             Output mean squared error.
  @return   A double array of computed coefficients.
 
  This function fits a 2d polynomial to a surface. The input points
  are given as double3, which include 3 coordinates per pixel.
 
  There are two ways of specifying the type of polynomial you want to
  fit. Either specify a maximal polynomial degree with poly_deg, and
  feed NULL for control_string, or fill up a control string and feed 0
  for poly_deg.
 
  The maximal polynomial degree indicates the highest sum for X and Y
  degrees. Example: for poly_deg=3, the following terms will be
  computed:
 
  \begin{verbatim}
  1     x       x^2      x^3
  y     x.y     x^2.y
  y^2   x.y^2
  y^3
  \end{verbatim}
 
  The control string contains (int,int) couples. The first integer
  specifies the degree for X, the second one the degree for Y. Couples
  are given in parentheses, integers separated by a comma, with no
  blanks within the parentheses. Couples are separated from other
  couples by one blank character.
 
  Example: to compute the fit for an equation of type:
 
  \begin{verbatim}
  P(x,y) = c[0] + c[1].x + c[2].x^2 + c[3].x.y
  \end{verbatim}
 
  You would provide the following control string:
 
  \begin{verbatim}
                "(0,0) (1,0) (2,0) (1,1)"
  \end{verbatim}
 
  \begin{itemize}
  \item (0,0) is degx=0 and degy=0 -> constant term, c[0]
  \item (1,0) is degx=1 and degy=0 -> term in x, c[1]
  \item (2,0) is degx=2 and degy=0 -> term in x^2, c[2]
  \item (1,1) is degx=1 and degy=1 -> term in x.y, c[3]
  \end{itemize}
 
  Be very cautious about the way the control string is formatted,
  because the parser is fairly basic. Be sure to respect the syntax
  rules!
 
  This control string is meant to be written by a programmer who wants
  to fit a surface. If you ever had to write a user-interface to feed
  in that information, you will probably have to make sure by yourself
  that the control string is properly formatted before calling this
  function.
 
 */
/*--------------------------------------------------------------------------*/
double * fit_surface_polynomial(
        double3 *   surface,
        char    *   control_string,
        int         poly_deg,
        int     *   ncoeffs,
        double  *   mse) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Translates a control string into a list of X and Y degrees.
  @param    s           Control string.
  @param    poly_deg    Maximal polynomial degree.
  @param    degx_tab    Output list of X degrees.
  @param    degy_tab    Output list of Y degrees.
  @return   Number of coefficients found, -1 in case of error.

  A control string is given as:

  \begin{verbatim}
  "(int,int) (int,int) [...] (int,int)"
  \end{verbatim}

  Each couple (int,int) represents the degree in x and y to be
  computed for the fit. Couples are given in parentheses and separated
  by commas, without any space between the parentheses.

  Couples are separated from each other by any number of blank
  characters (at least one is required).

  The following is a valid control string:
  \begin{verbatim}
  "(0,0) (1,2) (2,1) (1,1)"
  \end{verbatim}

  The following are invalid control strings:
  \begin{itemize}
  \item "(0, 0)", blanks in parentheses
  \item "( 0 , 0 )", blanks in parentheses
  \item "(0,0)(1,2)", no blank between couples
  \end{itemize}

  This is a very weak parser, because control strings are expected to
  be written by programmers and not fed by user input. Syntax errors
  are not returned in a very clean way.

  If you ever wanted to feed into this function something received
  from a user interface, you would probably want to implement some
  checks on the received string before feeding it to this function.

  If it becomes a requirement that users are requested to give control
  strings, we might want to enhance the parser in this function but it
  is currently outside of its scope.

 */
/*--------------------------------------------------------------------------*/
int buildup_polytab_from_string(
        char    *   s,
        int         poly_deg,
        int     *   degx_tab,
        int     *   degy_tab) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Fit a slope to a list of points (robust fit).
  @param    list    List of points.
  @param    np      Number of points in the list.
  @return   Pointer to newly allocated array of 3 doubles.

  The slope to fit has the following kind of equation:

  \begin{verbatim}
  y = c[0] + c[1] * x
  \end{verbatim}

  The returned coefficients are defined as:
  \begin{itemize}
  \item c[0] is the y-intercept.
  \item c[1] is the slope.
  \item c[2] is the median squared error of the fit.
  \end{itemize}

  The returned array must be freed by the caller using free().

  This is a very robust slope fit. It tolerates up to 50% of outliers
  in input.

 */
/*--------------------------------------------------------------------------*/
double * fit_slope_robust(double3 * list) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Fit a slope to a list of points
  @param    pts     Input list of points.
  @return   Pointer to newly allocated array of 3 doubles.

  The slope to fit has the following kind of equation:

  \begin{verbatim}
  y = c[0] + c[1] * x
  \end{verbatim}

  The returned coefficients are defined as:
  \begin{itemize}
  \item c[0] is the y-intercept.
  \item c[1] is the slope.
  \item c[2] is the median squared error of the fit.
  \end{itemize}

  The returned array must be freed by the caller using free().

  The fit method is a linear least-squares without any refinement,
  i.e. it is {\em very} sensitive to outliers. To robustify the fit,
  you probably want to call this function in an iterator, with
  rejection criteria.
 */
/*--------------------------------------------------------------------------*/
double * fit_slope(double3 * pts) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute a=y/x for all given points, returns the median a.
  @param    pts     Input list of points.
  @return   Pointer to newly allocated array of two doubles.

  This function takes in input a list of points supposed all aligned
  on a slope going through the origin (of equation y=ax). It computes
  the slope a = y/x for all points, and returns a pointer to two
  doubles:

  \begin{itemize}
  \item The median slope.
  \item The mean squared error.
  \end{itemize}

  Returning the median of all slopes makes it very robust to outliers.
  A more precise method would be to make a histogram of all slopes and
  take the maximum (i.e. the mode of the distribution). It can be
  shown that the median approximates the mode quite well for a large
  number of points.
  
 */
/*--------------------------------------------------------------------------*/
double * fit_proportional(double3 * pts) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Fit Legendre polynomials to a curve.
  @param    x       List of known x values.
  @param    y       List of known y values.
  @param    n       Number of values in the lists (same in x and y).
  @param    order   Maximal (inclusive) polynomial order to fit.
  @param    mse     Output mean squared error.
  @return   1 newly allocated array of order+1 doubles. 

  This function fits a linear combination of Legendre polynomials to the
  provided list of points. Beware that the input list of points is provided
  as two arrays: one for x, one for y, which must contain the same number
  of values (given by n).

  The maximal polynomial order to be fitted is set by 'order'. This
  function will do its best to fit, and return 'order+1' coefficients in a
  newly allocated array of doubles. The mean squared error can also be
  returned. If mse is non-NULL, the value pointed to by mse will receive
  the mean squared error.

  The returned coefficients can be checked against the input signal by
  computing for each x the value:

  \begin{verbatim}
  y = c[0]legendre(0,x) + c[1]legendre(1,x) + ...  +c[order]legendre(order,x)
  \end{verbatim}

  The returned array must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * fit_legendre(
        double  *   x,
        double  *   y,
        int         n,
        int         order,
        double  *   mse) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Fit a Gaussian to a list of points.
  @param    list        List of points as a double3.
  @return   Array of 3 Gaussian coefficients (amp, mu and sigma), or NULL.
 
  The fitted fonction is such that:
  \[ 
    G(x) = amp. exp[-0.5((x-mu)/sigma)^2]
  \] 
  The returned pointer must be freed using free().
 */ 
/*----------------------------------------------------------------------------*/
double * fit_1d_gauss(double3 * list) ;
double fun_gauss_rms(
        double  *   var,
        double3 *   pts);



#endif
