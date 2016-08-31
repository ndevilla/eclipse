/*-------------------------------------------------------------------------*/
/**
   @file	polygon.c
   @author	N.Devillard
   @date	February 2000
   @version	$Revision: 1.8 $
   @brief	Polygon handling routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: polygon.c,v 1.8 2001/11/07 15:00:00 yjung Exp $
	$Author: yjung $
	$Date4
	$Revision: 1.8 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "polygon.h"
#include "static_sz.h"
#include "parse_tok.h"

#ifdef _ECLIPSE_
#include "comm.h"
#else
#include "e_error.h"
#endif

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Find out if a point is inside a polygon.
  @param	poly	Polygon as a double3.
  @param	px		Candidate point X.
  @param	py		Candidate point Y.
  @return	int 1 if the point is inside the polygon, 0 else.

  The input polygon is given as an allocated double3. The
  candidate point is checked to be inside or outside the polygon
  (borders are inclusive).

  No check is done for impossible cases: polygons with less than 3
  vertices, all vertices aligned, etc.
 */
/*--------------------------------------------------------------------------*/
int polygon_contains_point(
		double3	*	poly,
		double		px,
		double		py)
{
	int		crossings ;
	int		i, j ;
	double	sx ;
	double	a, b ;

	/* Initialize */
	crossings=0 ;
	/* Loop over all edges */
	for (i=0 ; i<poly->n ; i++) {
		j=i+1 ;
		if (j==poly->n) j=0 ;
		/* Both vertices below horizontal */
		if ((poly->y[i]<py) && (poly->y[j]<py))
			continue ;
		/* Both vertices above horizontal */
		if ((poly->y[i]>py) && (poly->y[j]>py))
			continue ;
		/* Check where the crossing occurs */
		a = (poly->y[i]-poly->y[j])/(poly->x[i]-poly->x[j]) ;
		b = poly->y[i] - a * poly->x[i] ;
		sx = (py - b)/a ;
		if (sx>=px) crossings++ ;
	}
	return crossings & 1 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Read one polygon definition from an ASCII file.
  @param	polygon_file	Opened pointer to a valid polygon file.
  @return	List of vertices as a double3.

  A polygon definition file contains potentially several polygons, like:

  \begin{verbatim}
#
# Polygon definition file
#

10 20 11 23 43 128 78 29
11 98 76 56 12 27
  \end{verbatim}

  Blank lines and lines starting with '#' are ignored.
  Polygons are defined by couples of floating-point values, giving
  the vertices in the correct order.

  This routine expects an opened file pointer, pointing to the
  beginning of the next line to read. It returns a double3
  corresponding to the next valid line, or NULL if none can be
  found.

  To read several polygons from the same file, call the function as
  many times as you have polygons to read (if none can be found, this
  function will return NULL) on the same FILE pointer. After the last
  polygon has been read, you can fclose() the file.
 */
/*--------------------------------------------------------------------------*/
double3 * polygon_load_from_file(FILE * polygon_file)
{
    char    	line[ASCIILINESZ] ;
    char	** 	ltok ;
    int     	ntok ;
    double3	* 	pt ;
    int      	npt ;
    double  	r ;
    int     	i ;
    int     	status ;

    status = 0 ;
	ltok = NULL ;
    while (status==0) {
        /* Try to read one line from input file */
        if (fgets(line, ASCIILINESZ, polygon_file)==NULL) {
            status = -1 ;
        } else {
            /* Tokenize this line */
            ltok = tokenize_line(line, " \t\n", &ntok);
            if (ntok>1) {
                if (ltok[0][0]!='#') {
                    status = 1 ;
                } else {
                    free_tokens(ltok, ntok);
                }
            }
        }
    }
    if (status == -1) return NULL ;
    if (ntok%2) {
        e_error("in polygon definition: even number of coordinates\n"
				"current line is:\n"
				"%s", line);
        free_tokens(ltok, ntok);
        return NULL ;
    }
    npt = ntok / 2 ;
	pt = double3_new(npt);
    for (i=0 ; i<npt ; i++) {
        if (sscanf(ltok[2*i], "%lg", &r)!=1) {
            e_error("not a floating-point value: [%s]\n"
					"current line is:\n"
					"%s", ltok[2*i], line);
            double3_del(pt);
            free_tokens(ltok, ntok);
            return NULL ;
        }
        pt->x[i] = r ;

        if (sscanf(ltok[2*i+1], "%lg", &r)!=1) {
            e_error("not a floating-point value: [%s]\n"
					"current line is:\n"
					"%s", ltok[2*i+1], line);
            double3_del(pt);
            free_tokens(ltok, ntok);
            return NULL ;
        }
        pt->y[i] = r ;
    }
    free_tokens(ltok, ntok);
    return pt ;
}


