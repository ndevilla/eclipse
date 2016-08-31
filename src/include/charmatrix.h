
/*-------------------------------------------------------------------------*/
/**
   @file    charmatrix.h
   @author  N. Devillard
   @date    Jul 2000
   @version $Revision: 1.5 $
   @brief   Read tables in ASCII files to a 2d char matrix.

   This modules handles tables stored in ASCII files, which have the
   following format: data are separated by blanks (spaces or tabs)
   and lines by <CR>. The object used for storage is a 2d character
   matrix. This object is low-level enough to accomodate any kind
   of higher-level data structure.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: charmatrix.h,v 1.5 2001/10/17 10:29:01 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 10:29:01 $
	$Revision: 1.5 $
*/

#ifndef _CHARMATRIX_H_
#define _CHARMATRIX_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	2d character matrix object.

  This object stores a 2d character matrix, i.e. 2-dimensional array
  of character strings. It is useful to read an ASCII file containing
  tables in.
 */
/*-------------------------------------------------------------------------*/
typedef struct _charmatrix_ {
	int		lx ;
	int		ly ;
	char **	c ;
} charmatrix ;


/*---------------------------------------------------------------------------
   								Macros
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Returns the element (i,j) in a charmatrix.
  @param	m	Charmatrix to consider.
  @param	i	Column i (i from 0 to lx-1)
  @param	j	Line j (j from 0 to ly-1)
  @return	1 pointer to char

  This macro returns the element (i,j) in the requested matrix. The
  returned element is a pointer to char as expected from a charmatrix, that
  can be modified, freed, allocated, etc. Deallocating this pointer will be
  done by charmatrix_del().
 */
/*--------------------------------------------------------------------------*/

#define charmatrix_elem(m,i,j)	((m)->c[i+j*(m)->lx])

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocate a new charmatrix object.
  @param    lx      Max number of columns in the matrix.
  @param    ly      Max number of lines in the matrix.
  @return   1 newly allocated charmatrix object.

  This is the constructor for the charmatrix object. It requires a maximal
  number of lines and columns for creation. Once such an object is created,
  its size is fixed and should not be changed in any way.

  The returned object must be deallocated using charmatrix_del().
 */
/*--------------------------------------------------------------------------*/
charmatrix * charmatrix_new(int lx, int ly);

/*-------------------------------------------------------------------------*/
/**
  @brief    Deallocate a charmatrix object.
  @param    m   Charmatrix to free.
  @return   void

  Deallocates all memory associated to a charmatrix. This includes the word
  pointers and the structure pointers.
 */
/*--------------------------------------------------------------------------*/
void charmatrix_del(charmatrix * m);

/*-------------------------------------------------------------------------*/
/**
  @brief    Copy a charmatrix into a new object.
  @param    c   Charmatrix to copy.
  @return   1 newly allocated charmatrix object.

  Allocates a new charmatrix object and initializes its contents with a
  copy of the provided charmatrix. The returned object must be deallocated
  using charmatrix_del().
 */
/*--------------------------------------------------------------------------*/
charmatrix * charmatrix_copy(charmatrix * c);

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a charmatrix object to an opened file pointer.
  @param    m       Charmatrix to dump.
  @param    out     Output file.
  @return   void

  The following function dumps a charmatrix object to an opened file
  pointer. It is Ok to dump to standard streams (stdout or stderr).

  For debugging purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void charmatrix_dump(charmatrix * m, FILE * out);

/*-------------------------------------------------------------------------*/
/**
  @brief    Read a charmatrix from an ASCII file.
  @param    filename    Name of the ASCII file to parse.
  @return   1 newly allocated charmatrix object.

  This function reads a charmatrix from an ASCII file. The number of
  columns is determined by the column in the file that has the maximal
  number of tokens. The number of lines is determined by the number of
  lines containing valid tokens.

  The input ASCII file has the following syntax:

  - Blank lines are ignored
  - Lines starting with a hash are ignored (comments)
  - Tokens are separated by any number of whitespaces (tabs, blanks)
  - Whitespace is not allowed within one token
 */
/*--------------------------------------------------------------------------*/
charmatrix * charmatrix_read(char * filename);

#endif
