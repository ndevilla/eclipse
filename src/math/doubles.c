/*----------------------------------------------------------------------------*/
/**
  @file		doubles.c
  @author	N. Devillard
  @date		Jul 2000
  @version	$Revision: 1.14 $
  @brief	double3 object definition and methods.

  This module offers methods to handle a double3 object. Such an
  object is simply a container for three lists of doubles of identical
  sizes, called resp. x, y, and z. It is useful to carry around e.g.
  point coordinates or offset measurements.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: doubles.c,v 1.14 2003/04/23 15:27:27 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/23 15:27:27 $
	$Revision: 1.14 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doubles.h"
#include "static_sz.h"
#include "comm.h"
#include "xmemory.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Allocate a new double3 array.
  @param	size	Number of elements to allocate in the array.
  @return	1 newly allocated double3 array.

  The associated double arrays are allocated. The returned object must be
  deallocated using double3_del().
 */
/*----------------------------------------------------------------------------*/
double3 * double3_new(int size)
{
	double3	*	d ;

	d = malloc(sizeof(double3)) ;
	d->n = size ;
	d->x = calloc(size, sizeof(double));
	d->y = calloc(size, sizeof(double));
	d->z = calloc(size, sizeof(double));
	return d ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Deallocate a double3 array.
  @param	d	double3 object to deallocate.
  @return	void

  Deallocates the arrays contained in the variable and the main pointer
  itself. If passed a NULL pointer, does nothing.
 */
/*----------------------------------------------------------------------------*/
void double3_del(double3 * d)
{
	if (d==NULL) return ;
	if (d->n>0) {
		if (d->x!=NULL) free(d->x);
		if (d->y!=NULL) free(d->y);
		if (d->z!=NULL) free(d->z);
	}
	free(d);
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Sort a list of numbers in a double3 object.
  @param	p	List of numbers to sort.
  @param	c	Sorting criterion to use.
  @return	void

  The input list is modified to sort out all values following the sorting
  criterion. Possible sorting criteria are:

  - +1 to sort by increasing z
  - -1 to sort by decreasing z
 */
/*----------------------------------------------------------------------------*/
void double3_sort(double3 * p, int c)
{
    int     lo, up ;
    int     i, j ;
    double  x, y, z ;
 
	if (p==NULL) return ;
	if (p->n<2) return ;

	/* Linear insertion sort, probably not the fastest around... */
    lo=0 ;
    up=p->n-1 ;

	if (c<0) {
		/* Sorting by decreasing order */
		for (i=up-1 ; i>=lo ; i--) {
			x=p->x[i] ; y=p->y[i] ; z=p->z[i] ;
			for (j=i+1 ; j<=up && (z < p->z[j]) ; j++) {
				p->x[j-1]=p->x[j];
				p->y[j-1]=p->y[j];
				p->z[j-1]=p->z[j];
			}
			p->x[j-1]=x ; p->y[j-1]=y ; p->z[j-1]=z ;
		}
	} else {
		/* Sort by increasing order */
		for (i=up-1 ; i>=lo ; i--) {
			x=p->x[i] ; y=p->y[i] ; z=p->z[i] ;
			for (j=i+1 ; j<=up && (z > p->z[j]) ; j++) {
				p->x[j-1]=p->x[j];
				p->y[j-1]=p->y[j];
				p->z[j-1]=p->z[j];
			}
			p->x[j-1]=x ; p->y[j-1]=y ; p->z[j-1]=z ;
		}
	}
    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Read a list of double3 entries from an ASCII file
  @param	filename	Name of the input ASCII file.
  @return	1 newly allocated double3, or NULL if error.

  Parse an input ASCII file for coordinates. The returned double3 contains
  the identified numbers. If only one or two columns were found in the input
  file, only the x and y fields are filled, z is left initialized to zero.

  Lines beginning with a hash are ignored, blank lines also.
 */
/*----------------------------------------------------------------------------*/
double3 * double3_read(char * filename)
{
	FILE	*	in ;
	char		line[ASCIILINESZ] ;
	double		x, y, z ;
	int			np ;
	int			i ;
	double3	*	d ;
	int			ret ;

	if ((in=fopen(filename, "r"))==NULL) {
		e_error("cannot read [%s]", filename);
		return NULL ;
	}

	/* Count how many values are given in the file */
	np = 0 ;
	while (fgets(line, ASCIILINESZ, in)!=NULL) {
		if (line[0]!='#') {
			ret = sscanf(line, "%lg %lg %lg", &x, &y, &z);
			if (ret==1 || ret==2 || ret==3) {
				np++ ;
			}
		}
	}
	rewind(in);
	d = double3_new(np);
	i=0 ;
	while (fgets(line, ASCIILINESZ, in)!=NULL) {
		if (line[0]!='#') {
			ret = sscanf(line, "%lg%*c%lg%*c%lg", &x, &y, &z);
			if (ret==1) {
                d->x[i] = x ;
				d->y[i] = 0.00 ;
				d->z[i] = 0.00 ;
                i++ ;
            } else if (ret==2) {
				d->x[i] = x ;
				d->y[i] = y ;
				d->z[i] = 0.00 ;
				i++ ;
			} else if (ret==3) {
				d->x[i] = x ;
				d->y[i] = y ;
				d->z[i] = z ;
				i++ ;
			}
		}
	}
	fclose(in);
	return d ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Dump a double3 object to an opened file pointer.
  @param	d	Double3 object to dump
  @param	f	Opened file pointer to dump to.
  @return	void

  Dump a double3 struct to the requested file. The passed file pointer must
  be opened before use. It is Ok to provide @c stdout or @c stderr as file
  pointers.
 */
/*----------------------------------------------------------------------------*/
void double3_dump(double3 * d, FILE * f)
{
	int		i ;

	if (d==NULL || f==NULL) return ;
	if (d->n<1) return ;

	fprintf(f, "---------------------------------------------\n");
	fprintf(f, "points: %d\n", d->n);
	for (i=0 ; i<d->n ; i++) {
		fprintf(f, "%g\t%g\t%g\n", d->x[i], d->y[i], d->z[i]);
	}
	return ;
}
