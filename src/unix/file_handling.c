
/*-------------------------------------------------------------------------*/
/**
   @file	file_handling.c
   @author	N. Devillard
   @date	Mar 1995
   @version	$Revision: 1.13 $
   @brief	File handling routines.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: file_handling.c,v 1.13 2002/01/15 10:05:06 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/01/15 10:05:06 $
    $Revision: 1.13 $
 */


/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>

#include "file_handling.h"


/*----------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Find if a given file name corresponds to an existing file.
  @param    filename    Name of the file to look up.
  @return   int 1 if file exists, 0 if not
 
  Find out if the given character string corresponds to a file that
  can be stat()'ed.
 */
/*--------------------------------------------------------------------------*/

int file_exists(char * filename)
{
	struct stat buf ;
	int			exists ;
	
	if (stat(filename, &buf)==-1) {
		exists=0;
	} else {
		exists=1;
	}
	return exists;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Returns the size of a file in bytes.
  @param    filename    Name of the file to look up.
  @return   Size in bytes as an int.
 
  Finding out the size of a file in bytes is a highly non-portable
  trick. This function uses the stat POSIX system call to find out how
  big the size is. There are other tricks which are slightly more
  portable but less robust and slower.
 */
/*--------------------------------------------------------------------------*/

int filesize(char *filename)
{
	int	size ;
	struct stat	fileinfo ;
	/* POSIX compliant	*/
	if (stat(filename, &fileinfo) != 0) {
		size = (int)0 ;
	} else {
		size = (int)fileinfo.st_size ;
	}
	return size ;
}

/* vim: set ts=4 et sw=4 tw=75 */
