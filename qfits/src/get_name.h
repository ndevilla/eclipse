/*----------------------------------------------------------------------------*/
/**
  @file     get_name.h
  @author   Y. Jung
  @date     Jan 2004
  @version  $Revision: 1.4 $
  @brief    Get various names (filenames, dir names, login name, etc...)
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: get_name.h,v 1.4 2004/01/20 15:05:55 yjung Exp $
	$Author: yjung $
	$Date: 2004/01/20 15:05:55 $
	$Revision: 1.4 $

*/

#ifndef GET_NAME_H
#define GET_NAME_H

#ifdef __cplusplus
extern "C" {
#endif


/* <dox> */
/*-----------------------------------------------------------------------------
						Function ANSI C prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the directory name in the given string.
  @param    filename    Full path name to scan.
  @return   Pointer to statically allocated string.

  Provide a full path name and you get in return a pointer to a statically 
  allocated string containing the name of the directory only, without trailing 
  slash. If the input string does not contain a slash (i.e. it is not a full 
  path), the returned string is '.', corresponding to the current working 
  directory. Since the returned string is statically allocated, do not try to 
  free it or modify it.

  This function does not check for the existence of the path or the file.

  Examples:
  @verbatim
    get_dir_name("/cdrom/data/image.fits") returns "/cdrom/data"
    get_dir_name("filename.fits") returns "."
  @endverbatim
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_dir_name(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out the base name of a file (i.e. without prefix path)
  @param    filename    Full path name to scan.
  @return   Pointer to char within the input string.

  Provide a full path name and you get in return a pointer to a statically 
  allocated string containing the name of the file only, without prefixing 
  directory names. If the input string does not contain a slash (i.e. it is 
  not a full path), the returned string is a copy of the input string.

  This function does not check for the existence of the path or the file.

  Examples:
  @verbatim
    qfits_get_base_name("/cdrom/data/image.fits") returns "image.fits"
    qfits_get_base_name("filename.fits") returns "filename.fits"
  @endverbatim
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_base_name(const char *filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out the root part of a basename (name without extension).
  @param    filename    File name to scan.
  @return   Pointer to statically allocated string.

  Find out the root part of a file name, i.e. the file name without extension. 
  Since in Unix a file name can have several dots, only a number of extensions 
  are supported. This includes:

  - .fits and .FITS
  - .tfits and .TFITS
  - .paf and .PAF
  - .ascii and .ASCII
  - .dat and .DAT

  This function does not check for the existence of the path or the file.

  Examples:
  @verbatim
    get_root_name("/cdrom/filename.fits") returns "/cdrom/filename"
    get_root_name("filename.paf") returns "filename"
    get_root_name("filename") returns "filename"
    get_root_name("filename.ext") returns "filename.ext"
  @endverbatim

  Since the returned string is statically allocated in this module, do not try 
  to free it or modify its contents.
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_root_name(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out the extension of a file name
  @param    filename    File name without path prefix.
  @return   Pointer to char within the input string.
  
  Find out the extension of a given file name. Notice that the input character 
  string must not contain a path prefix (typically, you feed in the output of 
  @c qfits_get_base_name).

  Works with all kinds of extensions: returns the part of the string after the 
  last dot. If no dot is found in the input string, NULL is returned.

  Examples:
  @verbatim
    get_ext_name("filename.fits") returns "fits"
    get_ext_name("hello.c") returns "c"
    get_ext_name("readme") returns NULL
  @endverbatim
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_ext_name(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Returns the user login name.
  @return   Pointer to statically allocated character string.

  Finds out what the login name of the current user is. The result is
  placed in a static character string inside this module and a pointer
  to the first character in this string is returned. Do not modify or
  free the returned string!

  If the user name cannot be determined, the returned pointer will
  point to a string which first character is a null character.
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_login_name(void) ;

/* </dox> */
#ifdef __cplusplus
}
#endif

#endif
/* vim: set ts=4 et sw=4 tw=75 */
