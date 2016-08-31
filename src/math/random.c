/*----------------------------------------------------------------------------*/
/**
   @file	random.c
   @author	Nicolas Devillard
   @date	Tue, Apr 29th, 1997
   @version	$Revision: 1.8 $
   @brief	Random number/point generation routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: random.c,v 1.8 2002/10/18 14:52:59 yjung Exp $
	$Author: yjung $
	$Date: 2002/10/18 14:52:59 $
	$Revision: 1.8 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "random.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

/* To avoid missing definitions in the C library, define own math constants */

#ifndef M_SQRT1_2
/** 1 / sqrt(2) */
#define M_SQRT1_2       0.70710678118654752440  /* 1/sqrt(2) */
#endif

#ifndef M_PI_2
/** pi/2 */
#define M_PI_2          1.57079632679489661923  /* pi/2 */
#endif

/** Precision of the inverse function computation for bisection  */
#define GAUSS_RND_LIMIT     				1e-8

/** Default sigma for gaussian distribution  */
#define GAUSS_DEFAULT_SIGMA               	M_SQRT1_2

/**
   Default value for lower bound of the erf() function, used
   by bisection only. Given in terms of sigma.
 */  
#define LOWER_GAUSS_BOUND		-5.0
/**
   Default value for upper bound of the erf() function, used
   by bisection only. Given in terms of sigma.
 */  
#define UPPER_GAUSS_BOUND		 5.0

/*-----------------------------------------------------------------------------
								Macros
 -----------------------------------------------------------------------------*/
/** SQ(x) is x * x */
#define SQ(x) ((x)*(x))
/** Computes the square of an euclidean distance bet. 2 points */
#define pdist(x1,y1,x2,y2) (SQ(x1-x2)+SQ(y1-y2))
/** Computes the square of an euclidean distance bet. 2 points (polar coord) */
#define qdist(r1,t1,r2,t2) (SQ(r1)+SQ(r2)-2*r1*r2*cos((t1-t2)*M_PI_2/90))

/*-----------------------------------------------------------------------------
					Globals (private to this module)
 -----------------------------------------------------------------------------*/
static int random_initialized=0 ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Return a random value with a gaussian deviate
  @param	sigma		Sigma of the gaussian distribution
  @return	double random value

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
double random_gauss(double sigma)
{
    double  uniform ;
    double  x1, x2 ;
    double  err ;
    double  x ;
    double  val ;
    double  inv_sigma ;
    int     n_iterations ;

	if (random_initialized==0) {
		srand48((long)getpid()) ;
		random_initialized=1 ;
	}

	/* Compute the mean number of iterations to ensure we get */
	/* out of the loop one day, whether it converged or not. */
    n_iterations = (int)(0.5 + log((UPPER_GAUSS_BOUND - LOWER_GAUSS_BOUND) *
				  sigma/(double)GAUSS_RND_LIMIT) / log(2.0));
	/* Generate a uniform random number in [0 ; 1[ */
    uniform = 2.0 * drand48() - 1.0 ;

	/* Small or negative sigmas: use the default instead */
    if (sigma < 1e-10) sigma = GAUSS_DEFAULT_SIGMA ;

    inv_sigma = 1.0 / sigma ;
    x1 = LOWER_GAUSS_BOUND * sigma ;
    x2 = UPPER_GAUSS_BOUND * sigma ;
	x = (x1+x2)*0.5 ;

	/* Bisection search of the inverse value of erf(x) */
    err = 1.0 ;
    while ((fabs(err)>GAUSS_RND_LIMIT) && (fabs(x2-x1)>GAUSS_RND_LIMIT) &&
            (n_iterations)) {
        x = (x1+x2) * 0.5 ;
        val = erf(x * M_SQRT1_2 * inv_sigma) ;
        if (val < uniform)  x1 = x ; 
        else                x2 = x ; 
        err = uniform - val ;
        n_iterations -- ;
    }
    return x ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Outputs random numbers with a lorentzian distribution
  @param	dispersion	Lorentzian dispersion
  @return	double random value

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
double random_lorentz(double dispersion)
{
    double  uniform ;
    double  a ;

	if (random_initialized==0) {
		srand48((long)getpid()) ;
		random_initialized=1 ;
	}

	/* Small or negative dispersions: use the default instead (1.0)	*/
    if (fabs(dispersion)<1e-8) { a=1.0; } else { a=1.0/sqrt(dispersion); }

    /* Generate a uniform number in ]0.005 ; 0.995[ */
    do { uniform = drand48() ; } while ((uniform>0.995) && (uniform<0.005));

	/* 2x - 1 to get a uniform random number in ]-0.99 ; 0.99[ */
	/* This returns a number having the desired probability distribution */
    return (a * tan(M_PI_2 * (2.0 * uniform - 1.0) / a)) ;
}


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
        int	* 	r, 
		int 	np, 
		int 	homog)
{
    double      min_dist ;
    int         i ;
    int         gnp ;
    double3  *  list ;
	double		cand_x, cand_y ;
    int         ok ;
	int			start_ndx ;
	int			xmin, xmax, ymin, ymax ;

    /* error handling: test arguments are correct */
    if ((r==NULL) || (np<1)) return NULL ;
	if (random_initialized==0) {
		srand48((long)getpid()) ;
		random_initialized = 1 ;
	}
    if ((homog<1) || (homog>np)) homog = np ;

    list = double3_new(np);
	xmin = r[0] ;
	xmax = r[1] ;
	ymin = r[2] ;
	ymax = r[3] ;

    min_dist   = M_SQRT1_2*((xmax-xmin)*(ymax-ymin) / (double)(homog+1)) ;
    gnp        = 1 ;
    list->x[0] = 0 ;
    list->y[0] = 0 ;

    /* First: generate <homog> points */
    while (gnp < homog) {
		/* Pick a random point within requested range */
        cand_x = drand48() * (xmax - xmin) + xmin ;
        cand_y = drand48() * (ymax - ymin) + ymin ;

        /* Check the candidate obeys the minimal Poisson distance */
        ok = 1 ;
        for (i=0 ; i<gnp ; i++) {
			if (pdist(cand_x, cand_y, list->x[i], list->y[i])<min_dist) {
                /* does not check Poisson law: reject point */
                ok = 0 ;
                break ;
            }
        }
        if (ok) {
            /* obeys Poisson law: register the point as valid */
            list->x[gnp] = cand_x ;
            list->y[gnp] = cand_y ;
            gnp ++ ;
        }
    }

    /* Iterative process: */
	/* Pick points out of Poisson distance of the last <homog-1> points. */
	start_ndx=0 ;
    while (gnp < np) {
        /* Pick a random point within requested range */
        cand_x = drand48() * (xmax - xmin) + xmin ;
        cand_y = drand48() * (ymax - ymin) + ymin ;

        /* Check the candidate obeys the minimal Poisson distance */
        ok = 1 ;
        for (i=0 ; i<homog ; i++) {
            if (pdist(cand_x,
					  cand_y,
					  list->x[start_ndx+i],
					  list->y[start_ndx+i]) < min_dist) {
                /* does not check Poisson law: reject point */
                ok = 0 ;
                break ;
            }
        }
        if (ok) {
            /* obeys Poisson law: register the point as valid */
            list->x[gnp] = cand_x ;
            list->y[gnp] = cand_y ;
            gnp ++ ;
			start_ndx ++ ;
        }
    }
    return list ;
}

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
        int	* 	r, 
		int 	np, 
		int 	homog)
{
    double      min_dist ;
    int         i ;
    int         gnp ;
    double3  *  list ;
	double		cand_r, cand_t ;
    int         ok ;
	int			start_ndx ;
	int			r1, r2 ;

    /* error handling: test arguments are correct */
    if ((r==NULL) || (np<1)) return NULL ;
	if (random_initialized==0) {
		srand48((long)getpid()) ;
		random_initialized = 1 ;
	}
    if ((homog<1) || (homog>np)) homog = np ;

    list = double3_new(np);
	r1 = r[2] ;
	r2 = r[3] ;

    min_dist   = (M_PI_2/M_SQRT1_2)*(SQ(r2)-SQ(r1)) / (double)(homog+1) ;
    gnp        = 1 ;
    list->x[0] = r1 ;
    list->y[0] = 0 ;

    /* First: generate <homog> points */
    while (gnp < homog) {
		/* Pick a random point within requested range */
        cand_r = drand48() * (r2 - r1) + r1 ;
        cand_t = drand48() * 360 ;

        /* Check the candidate obeys the minimal Poisson distance */
        ok = 1 ;
        for (i=0 ; i<gnp ; i++) {
			if (qdist(cand_r, cand_t, list->x[i], list->y[i])<min_dist) {
                /* does not check Poisson law: reject point */
                ok = 0 ;
                break ;
            }
        }
        if (ok) {
            /* obeys Poisson law: register the point as valid */
            list->x[gnp] = cand_r ;
            list->y[gnp] = cand_t ;
            gnp ++ ;
        }
    }

    /* Iterative process: */
	/* Pick points out of Poisson distance of the last <homog-1> points. */
	start_ndx=0 ;
    while (gnp < np) {
        /* Pick a random point within requested range */
        cand_r = drand48() * (r2 - r1) + r1 ;
        cand_t = drand48() * 360 ;

        /* Check the candidate obeys the minimal Poisson distance */
        ok = 1 ;
        for (i=0 ; i<homog ; i++) {
            if (qdist(cand_r,
					  cand_t,
					  list->x[start_ndx+i],
					  list->y[start_ndx+i]) < min_dist) {
                /* does not check Poisson law: reject point */
                ok = 0 ;
                break ;
            }
        }
        if (ok) {
            /* obeys Poisson law: register the point as valid */
            list->x[gnp] = cand_r ;
            list->y[gnp] = cand_t ;
            gnp ++ ;
			start_ndx ++ ;
        }
    }
    return list ;
}

