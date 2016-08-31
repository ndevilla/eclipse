
/*-------------------------------------------------------------------------*/
/**
   @file    e_version.h
   @author  N. Devillard
   @date    May 2000
   @version $Revision: 1.6 $
   @brief   Get/display eclipse version and license.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: e_version.h,v 1.6 2002/02/15 09:41:04 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/02/15 09:41:04 $
	$Revision: 1.6 $
*/

#ifndef _E_VERSION_H_
#define _E_VERSION_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------
						Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Prints out the current eclipse version on stdout.
  @return   void
 
  Prints out a one-liner giving the current eclipse version. This
  function should be called whenever an eclipse command is called with
  the --version option, possibly with additional messages relative to
  the command version number.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
void print_eclipse_version(void);
/* </python> */
 
/*-------------------------------------------------------------------------*/
/**
  @brief    Returns a string giving the current eclipse version number.
  @return   Pointer to statically allocated string.
 
  The eclipse version number is located in a single source file, as a
  statically allocated string. You do not have to free the returned
  pointer, or try to modify it.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
char * get_eclipse_version(void);
/* </python> */

/*-------------------------------------------------------------------------*/
/**
  @brief    Displays a distribution logo on stderr.
  @return   void
 
  eclipse is copyright ESO, distributed under the BSD license.
  This function should be called whenever an eclipse command
  is called with the --license option.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
void eclipse_display_license(void); 
/* </python> */

#endif
