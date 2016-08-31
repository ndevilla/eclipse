
/*-------------------------------------------------------------------------*/
/**
   @file    manpage.h
   @author  N. Devillard
   @date    Jul 2001
   @version $Revision: 1.3 $
   @brief   Man page pretty-printing on console.

   This module is useful to print out manual pages stored into
   static character strings onto the console.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: manpage.h,v 1.3 2001/10/19 07:21:51 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 07:21:51 $
	$Revision: 1.3 $
*/

#ifndef _MANPAGE_H_
#define _MANPAGE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*---------------------------------------------------------------------------
						Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Pretty-print an RCS tag
  @param    rcsval      Value to be pretty-printed
  @return   1 pointer to statically allocated string, NULL if error occurs.

  This function takes a character string containing an RCS tag, makes
  it pretty inside a statically allocated character string and returns
  a pointer to the string. The returned value is weak, since it changes
  with each function call. Do not try to free or modify the contents
  of the returned pointer.
 */
/*--------------------------------------------------------------------------*/
char * rcs_value(char * rcsval);

/*-------------------------------------------------------------------------*/
/**
  @brief    Print out a man page string to an opened file pointer.
  @param    title       Man page title.
  @param    manpage     String containing the man page to dump.
  @param    version     (Optional) version string to dump.
  @param    lastmod     (Optional) 'last modified' string to dump.
  @param    fp          Opened file pointer to dump to.
  @param    format      Output file format (see below).
  @return   void

  Dump a given man page string to an opened file pointer.
  Supported formats are: standard Unix man page in pure ASCII form
  (no troff formatting commands), or HTML. The 'format' parameter
  is a character string containing either "man" or "html".

  version and lastmod are optional parameters to give more information
  about the version number and last modification date.
 */
/*--------------------------------------------------------------------------*/
void manpage_dump(
	char * title,
	char * manpage,
	char * version,
	char * lastmod,
	FILE * fp,
	char * format
);

#endif
