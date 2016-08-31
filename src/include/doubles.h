
/*-------------------------------------------------------------------------*/
/**
  @file     doubles.h
  @author   N. Devillard
  @date     Jul 2000
  @version  $Revision: 1.8 $
  @brief    double3 object definition and methods.

  This module offers methods to handle a double3 object. Such an
  object is simply a container for three lists of doubles of identical
  sizes, called resp. x, y, and z. It is useful to carry around e.g.
  point coordinates or offset measurements.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: doubles.h,v 1.8 2002/07/31 14:37:34 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/07/31 14:37:34 $
	$Revision: 1.8 $
*/

#ifndef _DOUBLES_H_
#define _DOUBLES_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    A structure that holds 3 doubles: x, y and z.

  This simple structure is meant to receive three double numbers. Nothing
  is specified about the meaning of these values, it is left to users to
  clearly document the use they make of it.

  An extra field: n, holds the number of values contained in the struct.
 */
/*-------------------------------------------------------------------------*/
typedef struct _double3_ {
    double  *   x ;
    double  *   y ;
    double  *   z ;
    int         n ;
} double3 ;


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Allocate a new double3 array.
  @param    size    Number of elements to allocate in the array.
  @return   1 newly allocated double3 array.

  The associated double arrays are allocated. The returned object must be
  deallocated using double3_del().
 */
/*--------------------------------------------------------------------------*/
double3 * double3_new(int size);

/*-------------------------------------------------------------------------*/
/**
  @brief    Deallocate a double3 array.
  @param    d   double3 object to deallocate.
  @return   void

  Deallocates the arrays contained in the variable and the main pointer
  itself. If passed a NULL pointer, does nothing.
 */
/*--------------------------------------------------------------------------*/
void double3_del(double3 * d);

/*-------------------------------------------------------------------------*/
/**
  @brief    Sort a list of numbers in a double3 object.
  @param    p   List of numbers to sort.
  @param    c   Sorting criterion to use.
  @return   void

  The input list is modified to sort out all values following the sorting
  criterion. Possible sorting criteria are:

  - +1 to sort by increasing z
  - -1 to sort by decreasing z
 */
/*--------------------------------------------------------------------------*/
void double3_sort(double3 * p, int c);

/*-------------------------------------------------------------------------*/
/**
  @brief    Read a list of double3 entries from an ASCII file
  @param    filename    Name of the input ASCII file.
  @return   1 newly allocated double3, or NULL if error.

  Parse an input ASCII file for coordinates. The returned double3 contains
  the identified numbers. If only two columns were found in the input
  file, only the x and y fields are filled, z is left initialized to zero.

  Lines beginning with a hash are ignored, blank lines also.
 */
/*--------------------------------------------------------------------------*/
double3 * double3_read(char * filename);

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a double3 object to an opened file pointer.
  @param    d   Double3 object to dump
  @param    f   Opened file pointer to dump to.
  @return   void

  Dump a double3 struct to the requested file. The passed file pointer must
  be opened before use. It is Ok to provide @c stdout or @c stderr as file
  pointers.
 */
/*--------------------------------------------------------------------------*/
void double3_dump(double3 * d, FILE * f);

#endif
