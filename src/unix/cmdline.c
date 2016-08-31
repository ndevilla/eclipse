

/*-------------------------------------------------------------------------*/
/**
   @file	cmdline.c
   @author	N. Devillard
   @date	Jan 2001
   @version	$Revision: 1.5 $
   @brief	Command-line parsing routines.

   This module offers generic command-line parsing. For a program
   to use this module, a command-line specification must be built
   to indicate what are the possible options, their names and arguments,
   a short help, etc. An engine then takes up the command-line
   specifications and command-line arguments and matches them to return
   a newly allocated dictionary, from which command-line info can easily
   be retrieved.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: cmdline.c,v 1.5 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.5 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"

#ifdef _ECLIPSE_
#include "comm.h"
#else
#include "e_error.h"
#endif


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*
 * Returns:
 * 0 if not an option
 * 1 for a short option
 * 2 for a long option
 */
static int identify_option_typ(char * arg)
{
	int	len ;
	int	val ;

	if (arg==NULL) return 0 ;

	len = strlen(arg);
	switch (len) {
		case 0:
		case 1:
		val = 0 ;
		break ;

		case 2:
		if (arg[0]=='-') {
			val = 1 ;
		} else {
			val = 0 ;
		}
		break ;

		default:
		if (arg[0]=='-') {
			if (arg[1]=='-') {
				val = 2 ;
			} else {
				val = -1 ;
			}
		} else {
			val = 0 ;
		}
		break ;
	}
	return val ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Dump a command-line spec as a short text help on stdout.
  @param	spec	Spec to dump
  @return	void

  This function takes as argument a valid command-line specification array
  and dumps it in a (hopefully) readable way on stdout. This should be
  used to print out short help messages associated to --help option.

  If spec is NULL, the function returns immediately.
 */
/*--------------------------------------------------------------------------*/
void cmdline_dump(cmdline_spec * spec)
{
	int		i ;

	if (spec==NULL) return ;
	i=0 ;
	while (spec[i].opt_desc!=NULL) {
		printf("[-%c | --%s]", spec[i].opt_short, spec[i].opt_long);
		if (spec[i].opt_exparg) {
			printf(" <%s>", spec[i].opt_argname);
		}
		printf("\n");
		printf("\t%s\n", spec[i].opt_desc);
		i++ ;
	}
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Parse a command-line into a dictionary.
  @param	argc	Number of arguments on the command-line.
  @param	argv	Array of arguments.
  @param	spec	Command-line constraints.
  @return	1 newly allocated dictionary.

  This function takes the (argc,argv) couple as received by main()
  and a specification describing the constraints expected for the
  command-line. It checks out that constraints are satisfied by the
  given arguments. If everything goes fine, it creates a new
  dictionary structure following these rules:

  - All informations about the arguments are named "arg.*"
  - The number of received arguments (not options) is "arg.n"
  - The arguments themselves are "arg.0", "arg.1", ... "arg.(n-1)"
  - "arg.0" contains the name of the program (= argv[0])
  - Options are inserted in the dictionary according to their
  long option name, e.g. an option called 'alpha' will end up in the
  dictionary as "arg.alpha".

  Example: Assuming the command-line specification declared three
  options -a/--alpha, -b/--beta, -g/--gamma, all expecting a value, 
  an option -d/--delta expecting no value (a flag), and the following
  command line:

  @verbatim 
  % prog file1.fits file2.fits -a 21 -b 42 --gamma 35 file3.fits -d
  @endverbatim

  The returned dictionary will contain the following entries:

  @verbatim
  key             value          comment

  arg.0           prog           The program name
  arg.1           file1.fits     First argument
  arg.2           file2.fits     Second argument
  arg.3           file3.fits     Third argument
  arg.n           4              Number of arguments: 3 + 1 (arg.0)
  arg.alpha       21             Value of alpha option
  arg.beta        42             Value of beta option
  arg.gamma       35             Value of gamma option
  arg.delta       1              Delta is present (set to "1")
  @endverbatim

  The command-line specification is a structure, described in
  cmdline.h. It is usually static (declared once for each program).
  See the header file for more information about how to fill up
  a command-line specification. It is Ok to provide NULL for spec,
  in that case no argument is expected on the command-line.
 */
/*--------------------------------------------------------------------------*/

dictionary * cmdline_parse(int argc, char ** argv, cmdline_spec * spec)
{
	dictionary *	d ;
	int				i, j ;
	char	   *	arg ;
	int				len ;
	int				found ;
	char			key[1024] ;
	int				npar ;
	
	/* Bulletproof entries */
	if ((argc<1) || argv==NULL) return NULL ;

	/* Allocate a new dictionary */
	d = dictionary_new(0);

	/* Store argv[0] as opt.progname */
	dictionary_set(d, "arg.0", argv[0]);

	/* If no further argument, stop here */
	if (argc<2) {
		dictionary_set(d, "arg.n", "1");
		return d ;
	}

	/* Counter for non-option parameters */
	npar = 1 ;
	i=1 ;
	while (i<argc) {

		/* Define two aliases to the current argv for easier reading */
		arg = argv[i] ;
		len = strlen(arg);
		found = 0 ;

		switch (identify_option_typ(arg)) {
			case -1:
			/* Error in option syntax */
			e_error("syntax error in argument: %s", arg);
			dictionary_del(d);
			return NULL ;

			case 0:
			/* Normal parameter */
			sprintf(key, "arg.%d", npar);
			dictionary_set(d, key, arg);
			npar ++ ;
			break ;

			case 1:
			/* Short option */
			/* Check that this is a valid option */
			if (spec==NULL) {
				e_error("illegal argument: %s", arg);
				dictionary_del(d);
				return NULL ;
			}
			j=0 ;
			while (spec[j].opt_desc) {
				if (spec[j].opt_short == arg[1]) {
					found++ ;
					break ;
				}
				j++ ;
			}
			/* Exit if illegal option */
			if (!found) {
				e_error("illegal argument: %s", arg);
				dictionary_del(d);
				return NULL ;
			}
			/* Create a new key for this option */
			sprintf(key, "arg.%s", spec[j].opt_long);
			/* Check if an argument was expected for this option */
			if (spec[j].opt_exparg) {
				/* An argument is expected */
				i++ ;
				if (i==argc) {
					/* Last parameter on cmd-line: error */
					e_error("option -%c (--%s) requires a parameter",
							spec[j].opt_short,
							spec[j].opt_long);
					dictionary_del(d);
					return NULL ;
				}
				/* Set key in dictionary as arg.long_opt */
				dictionary_set(d, key, argv[i]);
			} else {
				/* Key without value: set as "1" */
				dictionary_set(d, key, "1");
			}
			break ;

			case 2:
			/* Long option */
			if (spec==NULL) {
				e_error("illegal argument: %s", arg);
				dictionary_del(d);
				return NULL ;
			}
			/* Check that this is a valid option */
			j=0 ;
			while (spec[j].opt_desc) {
				if (!strcmp(spec[j].opt_long, arg+2)) {
					found++ ;
					break ;
				}
				j++ ;
			}
			/* Exit if illegal option */
			if (!found) {
				e_error("illegal argument: %s", arg);
				dictionary_del(d);
				return NULL ;
			}
			/* Check if an argument was expected for this option */
			sprintf(key, "arg.%s", spec[j].opt_long);
			if (spec[j].opt_exparg) {
				/* An argument is expected */
				i++ ;
				if (i==argc) {
					/* Last parameter on cmd-line: error */
					e_error("option -%c (--%s) requires an argument",
							spec[j].opt_short,
							spec[j].opt_long);
					dictionary_del(d);
					return NULL ;
				}
				/* Set key in dictionary as arg.long_opt */
				dictionary_set(d, key, argv[i]);
			} else {
				/* Key without value: set as "1" */
				dictionary_set(d, key, "1");
			}
			break ;
		}
		i++ ;
	}
	/* Set number of normal parameters */
	dictionary_setint(d, "arg.n", npar);

	return d ;
}

/* vim: set ts=4 et sw=4 tw=75 */
