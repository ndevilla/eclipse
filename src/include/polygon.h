/*-------------------------------------------------------------------------*/
/**
   @file    polygon.h
   @author  N.Devillard
   @date    February 2000
   @version $Revision: 1.9 $
   @brief   Polygon handling routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: polygon.h,v 1.9 2001/11/07 15:00:01 yjung Exp $
    $Author: yjung $
    $Date4
    $Revision: 1.9 $
*/

#ifndef _POLYGON_H_
#define _POLYGON_H_

/*---------------------------------------------------------------------------
							Includes	
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include "doubles.h"

/*---------------------------------------------------------------------------
						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Find out if a point is inside a polygon.
  @param    poly    Polygon as a double3.
  @param    px      Candidate point X.
  @param    py      Candidate point Y.
  @return   int 1 if the point is inside the polygon, 0 else.

  The input polygon is given as an allocated double3. The
  candidate point is checked to be inside or outside the polygon
  (borders are inclusive).

  No check is done for impossible cases: polygons with less than 3
  vertices, all vertices aligned, etc.
 */
/*--------------------------------------------------------------------------*/
int polygon_contains_point(
        double3 *   poly,
        double      px,
        double      py) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Read one polygon definition from an ASCII file.
  @param    polygon_file    Opened pointer to a valid polygon file.
  @return   List of vertices as a double3.

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
double3 * polygon_load_from_file(FILE * polygon_file) ;

#endif
