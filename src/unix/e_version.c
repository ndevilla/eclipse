

/*-------------------------------------------------------------------------*/
/**
   @file	e_version.c
   @author	N. Devillard
   @date	May 2000
   @version	$Revision: 1.9 $
   @brief	Get/display eclipse version and license.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: e_version.c,v 1.9 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.9 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_version.h"
#include "version.h"

/*---------------------------------------------------------------------------
							Static variables
 ---------------------------------------------------------------------------*/

/** eclipse logo in ASCII art */
static char eclipse_logo1[] =
"\n          _mm######mm_                        _ _\n"
"       _=~#############m_            ___  ___| (_)_ __  ___  ___\n"
"     _/  |###############W_         / _ \\/ __| | | '_ \\/ __|/ _ \\\n"
"    /    |#################W       |  __/ (__| | | |_) \\__ \\  __/\n"
"  ./     |##################W,      \\___|\\___|_|_| .__/|___/\\___|\n"
"  /      !###################W                   |_|\n"
" /        ####################b    eclipse is copyright (c) 1995-2001\n"
":'        V####################;   European Southern Observatory\n";

static char eclipse_logo2[] =
"/         `####################b\n"
"|          |####################   This program is free software, you can\n"
"|           !###################   redistribute it under the terms of the\n"
"|            `M#################   BSD license. See the file eclipse/LICENSE\n"
"t              Y###############@   for more details\n"
"|               `##############|\n"
" t                `Y*#########@\n"
" `;                  `~~Y****M'\n";

static char eclipse_logo3[] =
"  !,                        .!     For all questions concerning eclipse,\n"
"   `;                      :'      contact: <eclipse-team@eso.org>\n"
"    `=,                  .='       or check out the eclipse WWW pages at:\n"
"      `=_              _='         http://www.eso.org/eclipse/\n"
"         ~=__      __=~\n"
"             ~~~~~~\n\n" ;

/*---------------------------------------------------------------------------
  							Function codes
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
void print_eclipse_version(void)
{
    (void)fprintf(stdout, "eclipse version: %s\n", get_eclipse_version());
    return ;
}
 
 
/*-------------------------------------------------------------------------*/
/**
  @brief    Returns a string giving the current eclipse version number.
  @return   Pointer to statically allocated string.
 
  The eclipse version number is located in a single source file, as a
  statically allocated string. You do not have to free the returned
  pointer, or try to modify it.
 */
/*--------------------------------------------------------------------------*/
 
char * get_eclipse_version(void)
{
    return EclipseVersionNumber ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Displays a distribution logo on stderr.
  @return   void
 
  eclipse is copyright ESO, distributed under the BSD license.
  This function should be called whenever an eclipse command
  is called with the --license option.
 */
/*--------------------------------------------------------------------------*/
 
void eclipse_display_license(void)
{
    fprintf(stderr, "%s", eclipse_logo1) ;
    fprintf(stderr, "%s", eclipse_logo2) ;
    fprintf(stderr, "%s", eclipse_logo3) ;
}

/* vim: set ts=4 et sw=4 tw=75 */
