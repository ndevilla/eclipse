/*-------------------------------------------------------------------------*/
/**
   @file	poly2d.c
   @author	N. Devillard
   @date	22 Jun 1999
   @version	$Revision: 1.14 $
   @brief	2D polynomial handling
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: poly2d.c,v 1.14 2002/06/19 12:27:45 yjung Exp $
	$Author: yjung $
	$Date: 2002/06/19 12:27:45 $
	$Revision: 1.14 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "poly2d.h"
#include "qfits.h"
#include "comm.h"

/*---------------------------------------------------------------------------
   								Functions code
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the value of a poly2d at a given point.
  @param	p	Poly2d object.
  @param	x	x coordinate.
  @param	y	y coordinate.
  @return	The value of the 2d polynomial at (x,y) as a double.

  This computes the value of a poly2d in a single point. To
  compute many values in a row, see poly2d_compute_array().
 */
/*--------------------------------------------------------------------------*/
double poly2d_compute(
		poly2d	*	p,
		double		x,
		double		y)
{
	double	z ;
	int		i ;

	z = 0.00 ;
	for (i=0 ; i<p->nc ; i++) {
		z += p->c[i] * ipow(x, p->px[i]) * ipow(y, p->py[i]) ;
	}
	return z ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the value of a poly2d at a list of points.
  @param	p	Poly2d to compute
  @param	x	Array of x values.
  @param	y	Array of y values.
  @param	z	Returned array of 2d polynomial values.
  @param	n	Number of values in each array: x, y, z.
  @return	int 0 if Ok, -1 if error.

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
		poly2d	*	p,
		double	*	x,
		double	*	y,
		double	*	z,
		int			n)
{
	register int	i, j ;
	register double	x0, y0, z0;

	if ((x==NULL) || (y==NULL) || (z==NULL) || (n<1)) return -1 ;

	for (j=0 ; j<n ; j++) {
		x0 = x[j] ;
		y0 = y[j] ;
		z0 = 0.00 ;
		for (i=0 ; i<p->nc ; i++) {
			z0 += p->c[i] * ipow(x0, p->px[i]) * ipow(y0, p->py[i]) ;
		}
		z[i] = z0 ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Build a poly2d from a character string definition.
  @param	s	Poly2d definition as a character string.
  @return	A newly allocated poly2d definition.

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
poly2d * poly2d_build_from_string(char * s)
{
	poly2d	*	p ;
	int			i ;
	int			loop ;
	char	*	tok ;
	char	*	wstring ;

	if (s==NULL) return NULL ;

	/* Count number of values in given string */
	i=0 ;
	wstring = strdup(s);
	tok = strtok(wstring, " ");
	while (tok!=NULL) {
		i ++ ;
		tok = strtok(NULL, " ");
	}
	free(wstring);
	if (i%3 != 0) {
		e_error("in polynomial syntax\n"
				"the provided string is not made of triplets:\n"
				"[%s]", s);
		return NULL ;
	}
	i /= 3 ;

	/* Allocate space for values */
	/* Parse again the string and assign values into the structure */
	p = poly2d_allocate(i);
	wstring = strdup(s);
	tok = strtok(wstring, " ");
	i=0 ;
	loop=0 ;
	while (tok!=NULL) {
		switch (loop) {
			case 0:
			p->px[i/3] = atoi(tok) ;
			loop++ ;
			break;

			case 1:
			p->py[i/3] = atoi(tok);
			loop++ ;
			break ;

			case 2:
			p->c[i/3] = atof(tok);
			loop=0 ;
			break ;
		}
		i++ ;
		tok = strtok(NULL, " ");
	}
	free(wstring);
	return p ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Allocated space to store a poly2d definition.
  @param	nc	Number of coefficients to store.
  @return	Newly allocated poly2d object.

  This function allocates three arrays in the poly2d object,
  containing respectively the degree of x, the degree of y, and the
  associated coefficient (as a double).

  The returned object must be freed using poly2d_free().
 */
/*--------------------------------------------------------------------------*/
poly2d * poly2d_allocate(int nc)
{
	poly2d	*	p ;

	p = malloc(sizeof(poly2d));
	p->nc = nc ;
	p->px = malloc(nc * sizeof(int));
	p->py = malloc(nc * sizeof(int));
	p->c  = malloc(nc * sizeof(double));

	return p ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Frees space allocated for a poly2d object.
  @param	p	Poly2d object to free.
  @return	void

  This destructor is mandatorily called to destroy poly2d objects.
 */
/*--------------------------------------------------------------------------*/
void poly2d_free(poly2d * p)
{
	if (p==NULL) return ;

	if (p->px != NULL) free(p->px);
	if (p->py != NULL) free(p->py);
	if (p->c  != NULL) free(p->c);

	free(p);
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Print out a poly2d object on console (stderr).
  @param	p		Poly2d to print.
  @param	name	(Optional) name of the poly2d.
  @return	void

  This is for debugging purposes mostly. Giving a name to the
  poly2d is optional, to help discriminate information on the
  console. Provide NULL if you do not want to give a name.
 */
/*--------------------------------------------------------------------------*/
void poly2d_print(
		poly2d	*	p, 
		char	* 	name)
{
	int		i ;

	if (p==NULL) return ;
	if ((p->px==NULL) || (p->py==NULL) || (p->c==NULL)) return ;
	if (p->nc < 1) return ;

	if (name!=NULL) {
		fprintf(stderr, "poly2d: [%s]\n", name);
	}
	fprintf(stderr, "poly2d: %d coefficients\n", p->nc);
	for (i=0 ; i<p->nc ; i++) {
		fprintf(stderr, "+ %g ", p->c[i]);
		if (p->px[i]!=0) {
			fprintf(stderr, "* x^%d", p->px[i]);
		}
		if (p->py[i]!=0) {
			fprintf(stderr, "* y^%d", p->py[i]);
		}
		fprintf(stderr, "\n");
	}
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Read a 2d polynomial from a FITS table.
  @param	filename	Name of the FITS table file.
  @return	Newly allocated poly2d object.

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
poly2d * read_poly2d_from_table(char * filename)
{
    qfits_table	    *   table ;
    poly2d          *   correct_poly ;
    double          *   degree ;
    double          *   coeffs ;
    int                 i ;
	double				null_val ;
	
	null_val = 0.0 ;
	
    /* Open the table */
    if ((table=qfits_table_open(filename, 1)) == NULL) {
        e_error("cannot open the table") ;
        return NULL ;
    }

    /* Allocate memory to the poly2d */
    if ((correct_poly=poly2d_allocate(table->nr)) == NULL) {
        e_error("cannot allocate memory for poly2d") ;
        qfits_table_close(table) ;
        return NULL ;
    }

    /* Read the first column of the table */
    degree = (double*)qfits_query_column_data(table, 0, NULL, (void*)&null_val);
    if (degree == NULL) {
        e_error("cannot query the 1st column of the table") ;
        qfits_table_close(table) ;
        poly2d_free(correct_poly) ;
        return NULL ;
    }
    for (i=0 ; i<correct_poly->nc ; i++) {
        correct_poly->px[i] = (int)degree[i] ;
    }
    free(degree) ;

    /* Read the second column of the table */
    degree = (double*)qfits_query_column_data(table, 1, NULL, (void*)&null_val);
    if (degree == NULL) {
        e_error("cannot query the 2nd column of the table") ;
        qfits_table_close(table) ;
        poly2d_free(correct_poly) ;
        return NULL ;
    }
    for (i=0 ; i<correct_poly->nc ; i++) {
        correct_poly->py[i] = (int)degree[i] ;
    }
    free(degree) ;

    /* Read the third column of the table */
    coeffs = (double*)qfits_query_column_data(table, 2, NULL, (void*)&null_val);
    if (coeffs == NULL) {
        e_error("cannot query the 3rd column of the table") ;
        qfits_table_close(table) ;
        poly2d_free(correct_poly) ;
        return NULL ;
    }
    for (i=0 ; i<correct_poly->nc ; i++) {
        correct_poly->c[i] = coeffs[i] ;
    }
    free(coeffs) ;
	qfits_table_close(table) ;

    return correct_poly ;
}



