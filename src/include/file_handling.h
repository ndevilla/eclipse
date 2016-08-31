
/*-------------------------------------------------------------------------*/
/**
   @file    file_handling.h
   @author  N. Devillard
   @date    Mar 1995
   @version $Revision: 1.12 $
   @brief   File handling routines.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: file_handling.h,v 1.12 2001/10/17 10:24:13 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2001/10/17 10:24:13 $
    $Revision: 1.12 $
 */

#ifndef _FILE_HANDLING_H_
#define _FILE_HANDLING_H_

/*---------------------------------------------------------------------------
						Function ANSI C prototypes
 --------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Find if a given file name corresponds to an existing file.
  @param    filename    Name of the file to look up.
  @return   int 1 if file exists, 0 if not
 
  Find out if the given character string corresponds to a file that
  can be stat()'ed.
 */
/*--------------------------------------------------------------------------*/
int file_exists(char * filename);

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
int filesize(char *filename) ;

#endif
