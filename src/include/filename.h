
/*-------------------------------------------------------------------------*/
/**
   @file    filename.h
   @author  N. Devillard
   @date    Nov 1999
   @version $Revision
   @brief   File name handling routines

   The following functions are useful to cut out a filename into its
   components. All functions work with statically allocated memory,
   i.e. the pointers they return are not permanent but likely to be
   overwritten at each function call. If you need a returned value
   later on, you should store it into a local variable.
   Example:

   @code
   char * s ;
   s = get_dirname("/mnt/cdrom/data/image.fits")
   @endcode

   s contains now "/mnt/cdrom/data" but will loose these contents at
   the next function call. To retain its value, you can either do:

   @code
   char s[1024];
   strcpy(s, get_dirname("/mnt/cdrom/data/image.fits"));
   @endcode

   or:

   @code
   char * s;
   s = strdup(get_dirname("/mnt/cdrom/data/image.fits"));
   ...
   free(s);
   @endcode
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: filename.h,v 1.8 2001/10/19 10:49:09 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 10:49:09 $
	$Revision: 1.8 $
*/

#ifndef _FILENAME_H_
#define _FILENAME_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Find the directory name in the given string.
  @param    filename    Full path name to scan.
  @return   Pointer to statically allocated string.

  Provide a full path name and you get in return a pointer to a
  statically allocated string containing the name of the directory
  only, without trailing slash. If the input string does not contain a
  slash (i.e. it is not a full path), the returned string is '.',
  corresponding to the current working directory. Since the returned
  string is statically allocated, do not try to free it or modify it.

  This function does not check for the existence of the path or the
  file.

  Examples:
  @verbatim
    get_dirname("/cdrom/data/image.fits") returns "/cdrom/data"
    get_dirname("filename.fits") returns "."
  @endverbatim
 */
/*--------------------------------------------------------------------------*/
char * get_dirname(char * filename);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out the base name of a file (i.e. without prefix path)
  @param    filename    Full path name to scan.
  @return   Pointer to char within the input string.

  Provide a full path name and you get in return a pointer to a
  statically allocated string containing the name of the file
  only, without prefixing directory names. If the input string does
  not contain a slash (i.e. it is not a full path), the returned
  string is a copy of the input string.

  This function does not check for the existence of the path or the
  file.

  Examples:
  @verbatim
    get_basename("/cdrom/data/image.fits") returns "image.fits"
    get_basename("filename.fits") returns "filename.fits"
  @endverbatim
 */
/*--------------------------------------------------------------------------*/
char * get_basename(const char *filename);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out the root part of a basename (name without extension).
  @param    filename    File name to scan.
  @return   Pointer to statically allocated string.

  Find out the root part of a file name, i.e. the file name without
  extension. Since in Unix a file name can have several dots, only a
  number of extensions are supported. This includes:

  - .fits and .FITS
  - .tfits and .TFITS
  - .paf and .PAF
  - .ascii and .ASCII
  - .dat and .DAT

  This function does not check for the existence of the path or the
  file.

  Examples:
  @verbatim
    get_rootname("/cdrom/filename.fits") returns "/cdrom/filename"
    get_rootname("filename.paf") returns "filename"
    get_rootname("filename") returns "filename"
    get_rootname("filename.ext") returns "filename.ext"
  @endverbatim

  Since the returned string is statically allocated in this module, do
  not try to free it or modify its contents.
 */
/*--------------------------------------------------------------------------*/
char * get_rootname(char * filename);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out the extension of a file name
  @param    filename    File name without path prefix.
  @return   Pointer to char within the input string.
  
  Find out the extension of a given file name. Notice that the input
  character string must not contain a path prefix (typically, you feed
  in the output of @c get_basename).

  Works with all kinds of extensions: returns the part of the string
  after the last dot. If no dot is found in the input string, NULL is
  returned.

  Examples:
  @verbatim
    get_extname("filename.fits") returns "fits"
    get_extname("hello.c") returns "c"
    get_extname("readme") returns NULL
  @endverbatim
 */
/*--------------------------------------------------------------------------*/
char * get_extname(char * filename);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out in which directory a command lives.
  @param    pname   Name of the program to be found.
  @return   Pointer to statically allocated string.

  The input character string must be the name of a command without
  prefixing path of any kind, i.e. only the command name. The returned
  character string is the path (issued from the PATH environment
  variable) in which a command matching the same name was found.

  Examples:
  @verbatim
    (assuming there is a prog called 'hello' in the cwd)
    get_program_path("hello") returns "."
    get_program_path("ls")    returns "/bin"
    get_program_path("csh")   returns "/usr/bin"
    get_program_path("/bin/ls") returns NULL
  @endverbatim
 */
/*--------------------------------------------------------------------------*/
char * get_program_path(char * pname);

#endif
