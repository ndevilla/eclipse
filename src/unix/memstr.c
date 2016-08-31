
/*-------------------------------------------------------------------------*/
/**
   @file	memstr.c
   @author	N. Devillard
   @date	Aug 1999
   @version	$Revision: 1.9 $
   @brief	A version of strstr supporting NULL characters.

   This module offers a single function: memstr(), which acts just like
   strstr(), except that it also supports NULL characters in the string
   to search.

   This is useful e.g. when searching a binary file for an ASCII key.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: memstr.c,v 1.9 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.9 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "memstr.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	find a char string in a char memory block
  @param	block	Char block to be searched
  @param	bsize	Size of the char block to be searched
  @param	pattern	Character string to look for
  @return	char pointer to the pattern in block, or NULL.

  Find a char pattern in a char memory block. This does not support
  regular expressions. The functionality is strictly similar to
  strstr(), except that the searched block is allowed to contain NULL
  characters.
 */
/*--------------------------------------------------------------------------*/

char * memstr(char * block, int bsize, char * pattern)
{
    int     found ;
    char *  where ;
    char *  start ;

    found = 0 ;
    start = block ;
    while (!found) {
        where = (char*)memchr(start,
							 (int)pattern[0],
							 (size_t)bsize - (size_t)(start - block));
        if (where==NULL) {
            found++ ;
        } else {
            if (memcmp(where, pattern, strlen(pattern))==0) {
                found++;
            }
        }
        start=where+1 ;
    }
    return where ;
}


/* vim: set ts=4 et sw=4 tw=75 */
