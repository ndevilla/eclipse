/*----------------------------------------------------------------------------*/
/**
  @file		get_name.c
  @author	Y. Jung
  @date		Jan 2004
  @version	$Revision: 1.6 $
  @brief    Get various names (filenames, dir names, login name, etc...)

  The following functions are useful to cut out a filename into its components.
  All functions work with statically allocated memory, i.e. the pointers they 
  return are not permanent but likely to be overwritten at each function call. 
  If you need a returned value later on, you should store it into a local 
  variable.
  
  Example:
  
  @code
  char * s ;
  s = qfits_get_dir_name("/mnt/cdrom/data/image.fits")
  @endcode
  
  s contains now "/mnt/cdrom/data" but will loose these contents at the next 
  function call. To retain its value, you can either do:
  
  @code
  char s[1024];
  strcpy(s, qfits_get_dir_name("/mnt/cdrom/data/image.fits"));
  @endcode
  
  or:

  @code
  char * s;
  s = strdup(qfits_get_dir_name("/mnt/cdrom/data/image.fits"));
  ...
  free(s);
  @endcode
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: get_name.c,v 1.6 2004/05/26 09:10:49 yjung Exp $
	$Author: yjung $
	$Date: 2004/05/26 09:10:49 $
	$Revision: 1.6 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include "get_name.h"

/*-----------------------------------------------------------------------------
  							    Define
 -----------------------------------------------------------------------------*/

/** Maximum size of a filename buffer */
#define MAXNAMESZ       4096

/*-----------------------------------------------------------------------------
  							Function codes
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
    qfits_get_dir_name("/cdrom/data/image.fits") returns "/cdrom/data"
    qfits_get_dir_name("filename.fits") returns "."
  @endverbatim
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_dir_name(char * filename)
{
    static char path[MAXNAMESZ];
    char *last_slash;

    if (strlen(filename)>MAXNAMESZ) return NULL ;
    strcpy(path, filename);
    /* Find last '/'.  */
    last_slash = path != NULL ? strrchr (path, '/') : NULL;

    if (last_slash == path)
    /* The last slash is the first character in the string.  We have to
    return "/".  */
        ++last_slash;
    else if (last_slash != NULL && last_slash[1] == '\0')
        /* The '/' is the last character, we have to look further.  */
        last_slash = memchr (path, last_slash - path, '/');

    if (last_slash != NULL)
        /* Terminate the path.  */
        last_slash[0] = '\0';
    else
        /* This assignment is ill-designed but the XPG specs require to
        return a string containing "." in any case no directory part is
        found and so a static and constant string is required.  */
        strcpy(path, ".");
    return path;
}

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
char * qfits_get_base_name(const char *filename)
{
    char *p ;
    p = strrchr (filename, '/');
    return p ? p + 1 : (char *) filename;
}

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
  - .txt and .TXT

  This function does not check for the existence of the path or the file.

  Examples:
  @verbatim
    qfits_get_root_name("/cdrom/filename.fits") returns "/cdrom/filename"
    qfits_get_root_name("filename.paf") returns "filename"
    qfits_get_root_name("filename") returns "filename"
    qfits_get_root_name("filename.ext") returns "filename.ext"
  @endverbatim

  Since the returned string is statically allocated in this module, do not try 
  to free it or modify its contents.
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_root_name(char * filename)
{
    static char path[MAXNAMESZ+1];
    char * lastdot ;

    if (strlen(filename)>MAXNAMESZ) return NULL ;
    memset(path, MAXNAMESZ, 0);
    strcpy(path, filename);
    lastdot = strrchr(path, '.');
    if (lastdot == NULL) return path ;
    if ((!strcmp(lastdot, ".fits")) || (!strcmp(lastdot, ".FITS")) ||
        (!strcmp(lastdot, ".paf")) || (!strcmp(lastdot, ".PAF")) ||
        (!strcmp(lastdot, ".dat")) || (!strcmp(lastdot, ".DAT")) ||
        (!strcmp(lastdot, ".txt")) || (!strcmp(lastdot, ".TXT")) ||
        (!strcmp(lastdot, ".tfits")) || (!strcmp(lastdot, ".TFITS")) ||
        (!strcmp(lastdot, ".ascii")) || (!strcmp(lastdot, ".ASCII")))
    {
        lastdot[0] = (char)0;
    }
    return path ;
}

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
    qfits_get_ext_name("filename.fits") returns "fits"
    qfits_get_ext_name("hello.c") returns "c"
    qfits_get_ext_name("readme") returns NULL
  @endverbatim
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_ext_name(char * filename)
{
    char * p;
    p = strrchr(filename, '.');
    return p ? p+1 : NULL ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Returns the user login name.
  @return   Pointer to statically allocated character string.

  Finds out what the login name of the current user is. The result is placed in
  a static character string inside this module and a pointer to the first 
  character in this string is returned. Do not modify or free the returned 
  string!

  If the user name cannot be determined, the returned pointer will point to a 
  string which first character is a null character.
 */
/*----------------------------------------------------------------------------*/
char * qfits_get_login_name(void)
{
    struct passwd * pw ;
    static char name[32];

    pw = getpwuid(getuid());
    if (pw!=NULL) {
        strcpy(name, pw->pw_name);
    } else {
        name[0]=0 ;
    }
    return name ;
}

