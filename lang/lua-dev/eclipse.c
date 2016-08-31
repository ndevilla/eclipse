
/*----------------------------------------------------------------------------
   
   File name    :   eclipse.c
   Author       :	N. Devillard
   Created on   :	Dec 2000
   Description  :	eclipse/Lua command interpreter.

 ---------------------------------------------------------------------------*/

/*
	$Id: eclipse.c,v 1.9 2001/05/02 13:17:01 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/05/02 13:17:01 $
	$Revision: 1.9 $
*/

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eclipse.h"
#include "eclipse-lua.h"
#include "lualib.h"

/*----------------------------------------------------------------------------
                           		Defines 
 ---------------------------------------------------------------------------*/

#define MAXCMDLINESZ	10240


static char prog_desc[] = "eclipse command interpreter" ;
static char prog_vers[] = "$Revision: 1.9 $";

static void usage(char *pname) ;
static int  eclipse_interpreter(int    argc,
								char * argv[],
								char * command_file,
								char * command_string);
 
/*----------------------------------------------------------------------------
                            Main code
 ---------------------------------------------------------------------------*/


int main(int argc, char *argv[])
{
	char	*	command_string ;
	char	*	command_file ;

	command_string = NULL ;
	command_file   = NULL ;

	/* No option: get help */
	if (argc<2) {
		usage(argv[0]);
	}

	/* Parse command-line arguments */
	if (!strcmp(argv[1], "-L") || !strcmp(argv[1], "--license")) {
		/* License */
		eclipse_display_license() ;
		return 1 ;
	} else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		/* Help */
		usage(argv[0]);
	} else if (!strcmp(argv[1], "--version")) {
		/* Version */
		print_eclipse_version() ;
		printf("eclipse interpreter version: %s\n", prog_vers);
		return 1 ;
	} else if (!strcmp(argv[1], "-c")) {
		if (argc>2) {
			command_string = argv[2];
		} else {
			e_error("no command string passed to -c option");
			return -1 ;
		}
	} else if (!strcmp(argv[1], "--")) {
		command_string = "STDIN" ;
	} else if (argv[1][0]=='-') {
		e_error("unsupported option: %s", argv[1]);
		return -1 ;
	} else {
		command_file = argv[1];
	}
	return eclipse_interpreter(argc, argv, command_string, command_file);
}


static int eclipse_interpreter(
	int    argc,
	char * argv[],
	char * command_string,
	char * command_file)
{
	int			sta ;
	lua_State * L ;
	char		cmdline[MAXCMDLINESZ+1];

	/* Initialize eclipse */
	eclipse_init();

	/* Create a new Lua VM and initialize it */
	L = lua_open(0);

	/* Open various libraries */
	lua_baselibopen(L);
	lua_iolibopen(L);
	lua_strlibopen(L);
	lua_mathlibopen(L);
	lua_dblibopen(L);

	/* Open eclipse library */
	lua_eclipselibopen(L);

	/* Declare command-line arguments as global array 'args' in Lua */
	lua_parseargs(L, argc, argv);

	/* Handle different script sources */
	sta=-1;
	if (command_file!=NULL) {
		/* Execute given script */
		sta = lua_dofile(L, command_file);
		if (sta!=0) {
			e_error("processing file [%s]", command_file);
		}
	} else if (command_string!=NULL) {
		if (!strcmp(command_string, "STDIN")) {
			/* Loop over stdin as buffer */
			while (fgets(cmdline, MAXCMDLINESZ, stdin)!=NULL) {
				sta = lua_dostring(L, cmdline);
				if (sta!=0) {
					e_error("processing stdin");
					break ;
				}
			}
		} else {
			/* Command passed on command-line */
			sta = lua_dostring(L, command_string);
			if (sta!=0) {
				e_error("processing command");
			}
		}

	} else {
		/* Should never reach here */
		sta = -1 ;
	}

	/* Close Lua virtual machine */
	lua_close(L);
	if (debug_active())
		xmemory_status() ;
	return sta ;
}

/*
 * This function only gives the usage for the program
 */

static void usage(char *pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] [-c cmd | filename] [args]\n", pname) ;
	printf(
"\n"
"\t-c \"commands\"           executes commands in the string\n"
"\t--                      receives commands from stdin\n"
"\tfilename                executes script 'filename'\n"
"\n"
"options are:\n"
"\t-h or --help            prints this message and exits\n"
"\t-L or --license         prints out the license and exits\n"
"\t--version               print out eclipse version\n"
"\n\n"
	);
    exit(1) ;
}


