/*----------------------------------------------------------------------------*/
/**
   @file	fit_curve.c
   @author	N. Devillard
   @date	July 1998
   @version	$Revision: 1.22 $
   @brief	1d and 2d fit related routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: fit_curve.c,v 1.22 2003/10/24 11:32:18 yjung Exp $
	$Author: yjung $
	$Date: 2003/10/24 11:32:18 $
	$Revision: 1.22 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include "dstats.h"
#include "fit_curve.h"
#include "legendre.h"
#include "comm.h"
#include "optimization.h"

/*-----------------------------------------------------------------------------
							Private functions
 -----------------------------------------------------------------------------*/

static void robust_linear_fit(double*,double*,int,double*,double*,double*) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Fit a polynomial to a list of points.
  @param	poly_deg	Degree of the polynomial to fit.
  @param	list		List of points as a double3.
  @param	mse			Output mean squared error.
  @return	Array of (np+1) polynomial coefficients.

  The fitted polynomial is such that:
  \[
	P(x) = c_0 + c_1 x + c_2 x^2 + ... + c_n x^n
  \]
  So requesting a polynomial of degree n will return n+1 coefficients. Beware 
  that with such polynomials, two input points shall never be on the same 
  vertical!
  If you are not interested in getting the mean squared error back, feed in 
  NULL instead of a pointer to a double for mse.
  The returned pointer must be freed using free().
 */
/*----------------------------------------------------------------------------*/
double * fit_1d_poly(
		int			poly_deg,
		double3	*	list,
		double	*	mse)
{
	int			i, k ;
	matrix	*	ma, 
			*	mb, 
			*	mx ;
	double	*	c ;
	double		err ;
	double		xp, y ;

	if (list->n < poly_deg+1) {
		e_error("cannot fit %dth degree polynomial with %d points",
				poly_deg, list->n);
		return NULL ;
	}

	ma = matrix_new(poly_deg+1, list->n) ;
	mb = matrix_new(1, list->n) ;

	for (i=0 ; i<list->n ; i++) {
		ma->m[i] = 1.0 ;
		for (k=1 ; k<=poly_deg ; k++) {
			ma->m[i+k*list->n] = ipow(list->x[i], k) ;
		}
		mb->m[i] = list->y[i] ;
	}

	/* Solve XA=B by a least-square solution (aka pseudo-inverse). */
	mx = matrix_leastsq(ma,mb) ;
	matrix_del(ma) ;
	matrix_del(mb) ;
	if (mx==NULL) {
		e_error("cannot fit: non-invertible matrix") ;
		return NULL ;
	}

	c = malloc((poly_deg+1)*sizeof(double)) ;
	for (i=0 ; i<(poly_deg+1) ; i++) c[i] = mx->m[i] ;
	matrix_del(mx) ;

	/* If requested, compute mean squared error */
	if (mse != NULL) {
		err = 0.00 ;
		for (i=0 ; i<list->n ; i++) {
			y = c[0] ;
			/* Compute the value obtained through the fit */
			for (k=1 ; k<=poly_deg ; k++) {
				xp = ipow(list->x[i], k) ;
				y += c[k] * xp ; 
			}
			/* Subtract from the true value, square, accumulate */
			xp   = ipow(list->y[i] - y, 2) ;
			err += xp ; 
		}
		/* Average the error term */
		err /= (double)list->n ;
		*mse = err ;
	}
	return c ;
}

/*----------------------------------------------------------------------------*/
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
/*----------------------------------------------------------------------------*/
double * fit_surface_polynomial(
    	double3 *   surface,
    	char    *   control_string,
    	int         poly_deg,
    	int     *   ncoeffs,
    	double  *   mse)
{
	int			np ;
	int			degx, degy ;
	matrix	*	ma, 
			*	mb, 
			*	mx ;
	double		x, y, z ;
	int			nc ;
	double	*	c ;
	double		err ;
	int		*	degx_tab ;
	int		*	degy_tab ;
	int			i, j ;

	if (surface==NULL || control_string==NULL || ncoeffs==NULL) return NULL ;

	/* Fill up look-up table for coefficients to compute */
	nc = (1+poly_deg)*(2+poly_deg) / 2 ;
	degx_tab = malloc(nc * sizeof(int)) ;
	degy_tab = malloc(nc * sizeof(int)) ;

	if (control_string == NULL) {
		i=0 ;
		for (degy=0 ; degy<=poly_deg ; degy++) {
			for (degx=0 ; degx<=poly_deg ; degx++) {
				if (degx+degy <= poly_deg) {
					degx_tab[i] = degx ;
					degy_tab[i] = degy ;
					i++ ;
				}
			}
		}
	} else {
		nc = buildup_polytab_from_string(control_string,
										 poly_deg,
										 degx_tab,
										 degy_tab) ;
	}

	/* Initialize matrices */
	np = surface->n ;
	/* ma contains the polynomial terms in the order described */
	/* above in each column, for each input point. */
	ma = matrix_new(nc, np) ;
	/* mb contains the intensity (z-axis) values in a single line */
	mb = matrix_new(1, np) ;

	/* Fill up matrices */
	for (i=0 ; i<np ; i++) { 
		/* Get x and y value for current point */
		x    = surface->x[i] ;
		y    = surface->y[i] ;

		for (j=0 ; j<nc ; j++) {
			ma->m[i+j*np] = ipow(x,degx_tab[j]) * ipow(y,degy_tab[j]) ; 
		}
		/* mb contains surface values (z-axis) */
		mb->m[i] = surface->z[i] ;
	}
	
	/* Solve XA=B by a least-square solution (aka pseudo-inverse). */
	mx = matrix_leastsq(ma,mb) ;
	matrix_del(ma) ;
	matrix_del(mb) ;
	if (mx==NULL) {
		e_error("cannot fit: non-invertible matrix") ;
		return NULL ;
	}
	/* Store coefficients for output in a single array */
	c = (double*)malloc(nc * sizeof(double)) ;
	for (i=0 ; i<nc ; i++) c[i] = mx->m[i] ;
	matrix_del(mx) ;
	*ncoeffs = nc ;

	/* If requested, compute mean squared error */
	if (mse != NULL) {
		err = 0.00 ;
		for (i=0 ; i<np ; i++) {
			z = 0.00 ;
			for (j=0 ; j<nc ; j++) {
				z += c[j] *
					 ipow(surface->x[i], degx_tab[j]) *
					 ipow(surface->y[i], degy_tab[j]);
			}
			/* Subtract from the true value, square, accumulate */
			err += ipow(surface->z[i]-z, 2) ;
		}
		/* Average the error term */
		err /= (double)np ;
		*mse = err ;
	}
	free(degx_tab) ;
	free(degy_tab) ;
	return c ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Translates a control string into a list of X and Y degrees.
  @param	s			Control string.
  @param	poly_deg	Maximal polynomial degree.
  @param	degx_tab	Output list of X degrees.
  @param	degy_tab	Output list of Y degrees.
  @return	Number of coefficients found, -1 in case of error.

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
/*----------------------------------------------------------------------------*/
int buildup_polytab_from_string(
	    char    *   s,
	    int         poly_deg,
	    int     *   degx_tab,
	    int     *   degy_tab)
{
    char    *   s2 ;
    char    *   token ;
    int         degx, degy ;
    int         nc ;
    int         i, j ;
    int         ret ;

	if (s==NULL || degx_tab==NULL || degy_tab==NULL) return -1 ;
	if (s==NULL) return -1 ;
	if (poly_deg<0) return -1 ;
	if (degx_tab==NULL || degy_tab==NULL) return -1 ;

	/* Count number of coefficients provided */
    nc = 0 ;
    for (i=0 ; i<(int)strlen(s) ; i++) if (s[i] == ',') nc++ ;

    /* Cut the string into tokens, get degrees for x and y */
    s2 = strdup(s) ;
    token = strtok(s2," ") ;
    if (token==NULL) {
        e_error("invalid control string: aborting") ;
        free(s2) ;
        return -1 ;
    }
    ret = sscanf(token, "(%d,%d)", &degx, &degy) ;
    if (ret!=2) {
        e_error("error in control string: [%s]", token) ;
        free(s2) ;
        return -1 ;
    }
    degx_tab[0] = degx ;
    degy_tab[0] = degy ;

    for (i=1 ; i<nc ; i++) {
        token = strtok(NULL, " ") ;
        sscanf(token, "(%d,%d)", &degx, &degy) ;
        if (ret!=2) {
            e_error("error in control string: [%s]", token) ;
            free(s2) ;
            return -1 ;
        }
        if (degx+degy > poly_deg) {
            e_error("error in control string: %s with poly_deg=%d",
                    token, poly_deg) ;
            free(s2) ;
            return -1 ;
        }
        /* check for duplicates */
        for (j=0 ; j<i ; j++) {
            if ((degx_tab[j] == degx) && (degy_tab[j] == degy)) {
                e_error("duplicate found in control string: %s aborting",
						token);
                free(s2) ;
                return -1 ;
            }
        }
        degx_tab[i] = degx ; 
        degy_tab[i] = degy ;
    }
    free(s2) ;
    return nc ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Fit a slope to a list of points (robust fit).
  @param	list	List of points.
  @param	np		Number of points in the list.
  @return	Pointer to newly allocated array of 3 doubles.

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
/*----------------------------------------------------------------------------*/
double * fit_slope_robust(double3 * list)
{
	double	*	c ;

	if (list==NULL) return NULL ;
	c = malloc(3 * sizeof(double)) ;
	robust_linear_fit(list->x, list->y, list->n, c, c+1, c+2) ; 
	return c ;
}

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define MAX_ITERATE		30
static void
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
robust_linear_fit(
		double * x,
		double * y,
		int      np,
		double * a,
		double * b,
		double * abdev)
{
	int 	i;
	double 	aa, bb, bcomp, b1, b2, del, abdevt,
			f, f1, f2,
			sigb, temp, d, sum ;
	double	sx, sy,
			sxy, sxx,
			chisq ;
	double* arr ;
	double	aa_ls, bb_ls ;
	int		iter ;

	sx = sy = sxx = sxy = 0.00 ;
	for (i=0 ; i<np ; i++) {
		sx  += x[i];
		sy  += y[i];
		sxy += x[i] * y[i];
		sxx += x[i] * x[i];
	}

	del = np * sxx - sx * sx;
	aa_ls = aa  = (sxx * sy - sx * sxy) / del;
	bb_ls = bb  = (np * sxy - sx * sy) / del;

	chisq = 0.00 ;
	for (i=0;i<np;i++) {
		temp = y[i] - (aa+bb*x[i]) ;
		temp *= temp ;
		chisq += temp ;
	}

	arr = malloc(np * sizeof(double)) ;
	sigb = sqrt(chisq/del);
	b1   = bb ;

	bcomp = b1 ;
	sum = 0.00 ;
	for (i=0 ; i<np ; i++) {
		arr[i] = y[i] - bcomp * x[i];
	}
	aa = double_median(arr, np) ;
	abdevt = 0.0;
	for (i=0 ; i<np ; i++) {
		d = y[i] - (bcomp * x[i] + aa);
		abdevt += fabs(d);
		if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
		if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
	}
	f1 = sum ;
	b2   = bb + SIGN(3.0 * sigb, f1);
	bcomp = b2 ;
	sum = 0.00 ;
	for (i=0 ; i<np ; i++) arr[i] = y[i] - bcomp * x[i];
	aa = double_median(arr, np) ;
	abdevt = 0.0;
	for (i=0 ; i<np ; i++) {
		d = y[i] - (bcomp * x[i] + aa);
		abdevt += fabs(d);
		if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
		if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
	}
	f2 = sum ;

	if (fabs(b2-b1)<1e-7) {
		*a = aa ;
		*b = bb ;
		*abdev = abdevt / (double)np;
		free(arr);
		return ;
	}

	iter = 0 ;
	while (f1*f2 > 0.0) {
		bb = 2.0*b2-b1;
		b1 = b2;
		f1 = f2;
		b2 = bb;

		bcomp = b2 ;
		sum = 0.00 ;
		for (i=0 ; i<np ; i++) arr[i] = y[i] - bcomp * x[i];
		aa = double_median(arr, np) ;
		abdevt = 0.0;
		for (i=0 ; i<np ; i++) {
			d = y[i] - (bcomp * x[i] + aa);
			abdevt += fabs(d);
			if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
			if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
		}
		f2 = sum ;
		iter++;
		if (iter>=MAX_ITERATE) break ;
	}
	if (iter>=MAX_ITERATE) {
		*a = aa_ls ;
		*b = bb_ls ;
		*abdev = -1.0 ;
		free(arr);
		return ;
	}

	sigb = 0.01 * sigb;
	while (fabs(b2-b1) > sigb) {
		bb = 0.5 * (b1 + b2) ;
		if ((fabs(bb-b1)<1e-7) || (fabs(bb-b2)<1e-7)) break;
		bcomp = bb ;
		sum = 0.00 ;
		for (i=0 ; i<np ; i++) arr[i] = y[i] - bcomp * x[i];
		aa = double_median(arr, np) ;
		abdevt = 0.0;
		for (i=0 ; i<np ; i++) {
			d = y[i] - (bcomp * x[i] + aa);
			abdevt += fabs(d);
			if (fabs(y[i]) > 1e-7) d /= fabs(y[i]);
			if (fabs(d) > 1e-7) sum += (d >= 0.0 ? x[i] : -x[i]);
		}
		f = sum ;

		if (f*f1 >= 0.0) {
			f1=f;
			b1=bb;
		} else {
			f2=f;
			b2=bb;
		}
	}
	free(arr) ;
	*a=aa;
	*b=bb;
	*abdev=abdevt/np;
}
#undef MAX_ITERATE
#undef SIGN

/*----------------------------------------------------------------------------*/
/**
  @brief	Fit a slope to a list of points
  @param	pts		Input list of points.
  @return	Pointer to newly allocated array of 3 doubles.

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
/*----------------------------------------------------------------------------*/
double * fit_slope(double3 * pts)
{
	double	*	slope ;
	int			i ;
	double		sum_x, sum_y, sum_xy, sum_x2, det ;
	double		y ;

	if (pts==NULL) return NULL ;
	
	sum_x = sum_y = sum_xy = sum_x2 = 0.00 ;
	for (i=0 ; i<pts->n ; i++) {
		sum_x  += pts->x[i] ;
		sum_y  += pts->y[i] ;
		sum_xy += pts->x[i] * pts->y[i] ;
		sum_x2 += pts->x[i] * pts->x[i] ;
	}
	slope = malloc(3 * sizeof(double));

	det      = ((double)pts->n * sum_x2) - (sum_x * sum_x) ;
	slope[0] = ((sum_y * sum_x2) - (sum_x * sum_xy)) / det ;
	slope[1] = (((double)pts->n * sum_xy) - (sum_x * sum_y)) / det ;

	for (i=0 ; i<pts->n ; i++) {
		y = slope[0] + slope[1] * pts->x[i] ;
		slope[2] += (pts->y[i] - y) * (pts->y[i] - y) ;
	}
	slope[2] /= (double)pts->n ;
	return slope ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute a=y/x for all given points, returns the median a.
  @param	pts		Input list of points.
  @return	Pointer to newly allocated array of two doubles.

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
/*----------------------------------------------------------------------------*/
double * fit_proportional(double3 * pts)
{
#define FITPROP_BIG_SLOPE	1e30
	double	*	slopes ;
	double	*	med_slope ;
	double		y ;
	double		sq_err ;
	int			i ;

	if (pts==NULL) return NULL ;

	slopes = malloc(pts->n * sizeof(double)) ;
	for (i=0 ; i<pts->n ; i++) {
		if (fabs(pts->x[i])>1e-30)  slopes[i] = pts->y[i] / pts->x[i] ;
		else                        slopes[i] = FITPROP_BIG_SLOPE ; 
	}
	med_slope = malloc(2 * sizeof(double));
	med_slope[0] = double_median(slopes, pts->n);
	free(slopes);

	sq_err = 0.00 ;
	for (i=0 ; i<pts->n ; i++) {
		y = med_slope[0] * pts->x[i] ;
		sq_err += (y-pts->y[i])*(y-pts->y[i]) ; 
	}
	sq_err /= (double)pts->n ;
	med_slope[1] = sq_err ;

	return med_slope ;
#undef FITPROP_BIG_SLOPE
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Fit Legendre polynomials to a curve.
  @param	x		List of known x values.
  @param	y		List of known y values.
  @param	n		Number of values in the lists (same in x and y).
  @param	order	Maximal (inclusive) polynomial order to fit.
  @param	mse		Output mean squared error.
  @return	1 newly allocated array of order+1 doubles.	

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
/*----------------------------------------------------------------------------*/
double * fit_legendre(
		double	*	x,
		double	*	y,
		int			n,
		int			order,
		double	*	mse)
{
    matrix  *	ma, 
			*	mb, 
			*	mx ;
    double  *   fit_c ;
    double      sq_err ;
    double      z ;
    int         i, j ;
 
    /* Check entries */
    if ((x==NULL) || (y==NULL) || (n<1) || (order<0)) return NULL ;
 
    /* Fill up matrices */
    ma = matrix_new(order+1, n) ;
    mb = matrix_new(1, n) ;
    for (i=0 ; i<n ; i++) {
        for (j=0 ; j<=order ; j++) ma->m[i+j*n] = legendre(j, x[i]);
        mb->m[i] = y[i] ;
    }
    /* Solve least-squares equation */
    mx = matrix_leastsq(ma, mb);
    matrix_del(ma);
    matrix_del(mb);
    if (mx==NULL) {
        e_error("cannot fit: non invertible matrix");
        return NULL ;
    }
    /* Store coefficients */
    fit_c = malloc((order+1) * sizeof(double));
    for (i=0 ; i<=order ; i++) fit_c[i] = mx->m[i] ;
    matrix_del(mx);
 
    /* Compute mean-squared error if needed */
    if (mse!=NULL) {
        sq_err = 0.00 ;
        for (i=0 ; i<n ; i++) {
            z = 0.00 ;
            for (j=0 ; j<=order ; j++) z += fit_c[j] * legendre(j, x[i]);
            z -= y[i] ;
            sq_err += (z*z);
        }
        sq_err /= (double)n;
        *mse = sq_err ;
    }
    return fit_c ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Fit a Gaussian to a list of points.
  @param	list		List of points as a double3.
  @return	Array of 3 Gaussian coefficients (amp, mu and sigma), or NULL.

  The fitted fonction is such that:
  \[
	G(x) = amp. exp[-0.5((x-mu)/sigma)^2]
  \]
  The returned pointer must be freed using free().
 */
/*----------------------------------------------------------------------------*/
double * fit_1d_gauss(double3 * list)
{
    double  **  estimates ;
    int         neval ;
    int         ndim ;
    double      max_val ;
    int         max_ind ;
    int         i ;
  
    /* Initialize */
    neval = 0 ;

    /* 3 dimensions: a, mu, sigma */
    ndim = 3 ;
    
    /* Allocate the estimates  */
    estimates = malloc((ndim+1)*sizeof(double*)) ;
    for (i=0 ; i<ndim+1 ; i++) estimates[i] = malloc(ndim*sizeof(double)) ; 
    
    /* Compute the estimates */
    max_val = list->y[0] ;
    max_ind = 0 ;
    for (i=1 ; i<list->n ; i++) {
        if (max_val < list->y[i]) {
            max_val = list->y[i] ;
            max_ind = i ;
        }
    }
    estimates[0][0] = list->y[max_ind] ;
    estimates[0][1] = list->x[max_ind] ;
    estimates[0][2] = 1 ;
    estimates[1][0] = 1.1 * list->y[max_ind] ;
    estimates[1][1] = 1.1 * list->x[max_ind] ;
    estimates[1][2] = 10 ;
    estimates[2][0] = 1.2 * list->y[max_ind] ;
    estimates[2][1] = 1.2 * list->x[max_ind] ;
    estimates[2][2] = 20 ;
    estimates[3][0] = 1.3 * list->y[max_ind] ;
    estimates[3][1] = 1.3 * list->x[max_ind] ;
    estimates[3][2] = 30 ;
    /* Call the optimization function */
    minimize(estimates, ndim, 1e-3, fun_gauss_rms, list, &neval) ;
    
    return estimates[0] ;
}

double fun_gauss_rms(
        double  *   var,
        double3 *   pts)
{
    double  amp, mu, sigma ;
    double  ret ;
    double  val ;
    int     i ;

    /* Initialize */
    ret = 0 ;
    amp = var[0] ;
    mu = var[1] ;
    sigma = var[2] ;

    /* Compute */
    for (i=0 ; i<pts->n ; i++) {
        val = -0.5*((pts->x[i]-mu)/sigma)*((pts->x[i]-mu)/sigma) ;
        ret += (pts->y[i]-amp*exp(val))*(pts->y[i]-amp*exp(val)) ; 
    }

    return ret ;
}

