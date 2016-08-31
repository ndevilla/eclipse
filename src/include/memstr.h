
/*-------------------------------------------------------------------------*/
/**
  @file		memstr.h
  @author	N. Devillard
  @date		Aug 1999
  @version	$Revision: 1.8 $
  @brief	A version of strstr supporting NULL characters.

  This module offers a single function: memstr(), which acts just like
  strstr(), except that it also supports NULL characters in the string
  to search.

  This is useful e.g. when searching a binary file for an ASCII key.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: memstr.h,v 1.8 2001/10/19 10:49:09 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 10:49:09 $
	$Revision: 1.8 $
*/

#ifndef _MEMSTR_H_
#define _MEMSTR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <string.h>

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    find a char string in a char memory block
  @param    block   Char block to be searched
  @param    bsize   Size of the char block to be searched
  @param    pattern Character string to look for
  @return   char pointer to the pattern in block, or NULL.

  Find a char pattern in a char memory block. This does not support
  regular expressions. The functionality is strictly similar to
  strstr(), except that the searched block is allowed to contain NULL
  characters.
 */
/*--------------------------------------------------------------------------*/

char * memstr(char * block, int	bsize, char * pattern) ;

#ifdef __cplusplus
}
#endif

#endif
