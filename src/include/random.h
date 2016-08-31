/*----------------------------------------------------------------------------*/
/**
   @file    random.h
   @author  Nicolas Devillard
   @date    Tue, Apr 29th, 1997
   @version $Revision: 1.5 $
   @brief   Random number/point generation routines
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: random.h,v 1.5 2002/10/18 14:52:59 yjung Exp $
    $Author: yjung $
    $Date: 2002/10/18 14:52:59 $
    $Revision: 1.5 $
*/

#ifndef _RANDOM_H_
#define _RANDOM_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdlib.h>
#include "doubles.h"

/*-----------------------------------------------------------------------------
						Function ANSI prototypes
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @brief    Return a random value with a gaussian deviate
  @param    sigma       Sigma of the gaussian distribution
  @return   double random value

  If the provided value for sigma is smaller than 1e-10, the value 1 is taken 
  instead.

  How to generate random values with a gaussian deviate ?
  Let us assume a gaussian probability distribution:

  @f$ \frac{1}{\sigma \sqrt{2\pi}} e^{\frac{-x^2}{2 \sigma^2}} @f$
  
  the general integration of p(x) yields:
  
  @f$ F(x) = \int p(x) dx = erf(\frac{x}{\sigma \sqrt{2}}) @f$
  
  erf(x) being the following function (part of the standard math lib):

  @f$ erf(x) = \frac{2}{\sqrt{\pi}} \int e^{-t^2} dt @f$
  
  Let @f$ F^{-1} @f$ be the inverse of F.

  To get a gaussian-distributed random value, if x is a uniformly distributed 
  random value, then @f$ F^{-1}(x) @f$ is a gaussian-distributed random value.

  Instead of inverting F, we find t such as F(t)=x, which is equivalent to 
  searching a zero-crossing of F(t)-x, using a bisection (for example).
  
  The number of iterations required for the bisection to converge is: 
  log2(e0/e) where e0 is the search interval size, and e the required precision.

  In our case: e0 = 10 * sigma and e=GAUSS_RND_LIMIT
  (10 is UPPER_GAUSS_BOUND - LOWER_GAUSS_BOUND)
  for sigma=1 and GAUSS_RND_LIMIT of 1e-8, we need about 30 iterations.
 */
/*----------------------------------------------------------------------------*/
double random_gauss(double sigma) ;


/*----------------------------------------------------------------------------*/
/**
  @brief    Outputs random numbers with a lorentzian distribution
  @param    dispersion  Lorentzian dispersion
  @return   double random value

  See above, function gauss_random(), to know how to generate random values 
  having a given probability density.

  In this case, @f$ p(x) = \frac{1}{1+kx^2} @f$
  
  If @f$ a = \frac{1}{\sqrt{k}} @f$, we have:

  @f$ F(x) = a \arctan{\frac{x}{a}} @f$

  and
  
  @f$ F^{-1}(x) = a \tan{\frac{x}{a}} @f$

  The input range of the tangent function is:
  @f$ ] -\pi/2 ; \pi/2 [ @f$

  The output range of this function is -Inf to +Inf, clipped to -64a to +64a.
 */
/*----------------------------------------------------------------------------*/
double random_lorentz(double dispersion) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate points with a Poisson scattering property in a rectangle.
  @param    r       Array of 4 ints as [xmin,xmax,ymin,ymax]
  @param    np      Number of points to generate.
  @param    homog   Homogeneity factor.
  @return   Newly allocated double3 object.

  <b>POISSON POINT GENERATION</b>

  Without homogeneity factor, the idea is to generate a set of np points within
  a given rectangle defined by (xmin xmax ymin ymax).
  All these points obey a Poisson law, i.e. no couple of points is closer to 
  each other than a minimal distance. This minimal distance is defined as a 
  function of the input requested rectangle and the requested number of points 
  to generate. We apply the following formula:

  @f$ d_{min} = \sqrt{\frac{W \times H}{\sqrt{2}}} @f$

  Where W and H stand for the rectangle width and height. Notice that the 
  system in which the rectangle vertices are given is completely left 
  unspecified, generated points will have coordinates in the specified x and y 
  ranges.

  With a specified homogeneity factor h (2 < h <= np), the generation algorithm
  is different. The definition of h is:

  the Poisson law applies for any h consecutive points in the final output, but
  not for the whole point set. This enables us to generate groups of points 
  which statisfy the Poisson law, without constraining the whole set. This 
  actually is equivalent to dividing the rectangle in h regions of equal 
  surface, and generate points randomly in each of these regions, changing 
  region at each point.
 */
/*----------------------------------------------------------------------------*/
double3 * generate_rect_poisson_points(
        int *   r,
        int     np,
        int     homog) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate points with a Poisson scattering property in a ring.
  @param    r       Array of 4 ints as [x,y,r1,r2]
  @param    np      Number of points to generate.
  @param    homog   Homogeneity factor.
  @return   Newly allocated double3 object.
  See generate_rect_poisson_points().
 */
/*----------------------------------------------------------------------------*/
double3 * generate_ring_poisson_points(
        int *   r,
        int     np,
        int     homog) ;

#endif
