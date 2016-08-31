/*----------------------------------------------------------------------------*/
/**
   @file    ghost.c
   @author  N. Devillard
   @date    February 2001
   @version	$Revision: 1.6 $
   @brief   ISAAC electrical ghost removal procedure
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: ghost.c,v 1.6 2002/12/10 09:43:17 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/10 09:43:17 $
	$Revision: 1.6 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int isaac_ghost_main(void * dict)
{
	dictionary	*	d ;

	int				force_flag ;

	char			argname[10];
	char		*	filename ;
	
	int				nfiles ;
	int				errors ;
	int				i ;

	d = (dictionary*)dict ;
	/* Get options */
	force_flag = dictionary_getint(d, "arg.force", 0);
	
	/* Get number of input files */
	nfiles = dictionary_getint(d, "arg.n", -1);
	if (nfiles<1) {
		e_error("missing input file name(s): aborting");
		return -1 ;
	}
	/* Loop on input file names */
	errors = 0 ;
	for (i=1 ; i<nfiles ; i++) {
		sprintf(argname, "arg.%d", i);
		filename = dictionary_get(d, argname, NULL);
		if (file_exists(filename)!=1) {
			e_error("file [%s] does not exist", filename);
		} else if (is_fits_file(filename)!=1) {
			e_error("file [%s] is not a FITS file", filename);
		} else {
			errors += isaac_ghost_removal(filename, force_flag);
		}
	}
	return errors ;
}

