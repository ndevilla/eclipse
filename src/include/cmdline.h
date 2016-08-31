
/*-------------------------------------------------------------------------*/
/**
   @file    cmdline.h
   @author  N. Devillard
   @date    Jan 2001
   @version $Revision: 1.3 $
   @brief   Command-line parsing routines.

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
	$Id: cmdline.h,v 1.3 2001/10/17 08:18:55 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 08:18:55 $
	$Revision: 1.3 $
*/

#ifndef _CMDLINE_H_
#define _CMDLINE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Option with no expected argument */
#define OPT_EXPNOARG	0
/** Option with an expected argument */
#define OPT_EXPARG		1


/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Command-line specification.

  This structure allows to hold a specification for one given command-line
  option. It currently allows to declare a short name, a long name,
  a short description, a flag indicating if the option expects an
  argument or not (usually set with @c OPT_EXPNOARG or @c OPT_EXPARG),
  and the formal name of the argument if there is one.

  Example:

  @verbatim
  -a/--alpha <value> would be declared as:

  opt_short   = 'a'
  opt_long    = "alpha"
  opt_desc    = "The alpha parameter"
  opt_exparg  = OPT_EXPARG
  opt_argname = "value"
  @endverbatim

  To define a set of constraints for a number of command-line options,
  build up a (usually static) array of such structs.
 */
/*-------------------------------------------------------------------------*/

typedef struct _cmdline_spec_ {
    int     opt_short ;     /** Name of short option         */
    char *  opt_long ;      /** Name of long option          */
    char *  opt_desc ;      /** Description                  */
    int     opt_exparg ;    /** Expects an argument?         */
    char *  opt_argname ;   /** Formal name of the argument  */
} cmdline_spec ;





/*---------------------------------------------------------------------------
						Function ANSI prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a command-line spec as a short text help on stdout.
  @param    spec    Spec to dump
  @return   void

  This function takes as argument a valid command-line specification array
  and dumps it in a (hopefully) readable way on stdout. This should be
  used to print out short help messages associated to --help option.

  If spec is NULL, the function returns immediately.
 */
/*--------------------------------------------------------------------------*/
void cmdline_dump(cmdline_spec * spec);


/*-------------------------------------------------------------------------*/
/**
  @brief    Parse a command-line into a dictionary.
  @param    argc    Number of arguments on the command-line.
  @param    argv    Array of arguments.
  @param    spec    Command-line constraints.
  @return   1 newly allocated dictionary.

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
dictionary * cmdline_parse(int argc, char ** argv, cmdline_spec * spec);


#endif
