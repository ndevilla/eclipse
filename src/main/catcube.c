/*----------------------------------------------------------------------------*/
/**
   @file    catcube.c
   @author  Nicolas Devillard
   @date    October 05, 1995
   @version	$Revision: 1.23 $
   @brief   create one cube from a list of cubes
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: catcube.c,v 1.23 2003/04/14 14:00:28 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/14 14:00:28 $
	$Revision: 1.23 $
 */

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
  								Functions prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "concatenate data cubes" ;

/*-----------------------------------------------------------------------------
  									Main
 -----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	char			outname[FILENAMESZ+1] ;
	int				c ;
	int				i ;
	cube_t		*	in ;
	history		*	hs ;

    /* Test inputs */
	if (argc<2) usage(argv[0]) ;

    /* Initialize */
	strncpy(outname, "cat.fits", FILENAMESZ);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"out",     1, 0, OPT_OUTPUT},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lho:",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c) {
            /* Standard option: display license (not documented in usage)   */
			case OPT_LICENSE:
            case 'L':
                eclipse_display_license() ;
                return 0 ;
 
            /* Standard option : help */
            case OPT_HELP:
            case 'h':
                usage(argv[0]) ;
                break ;

            /* Standard option: version */
            case OPT_VERSION:
                print_eclipse_version() ;
                return 0 ;

            /* Local option: name of the output file */
			case OPT_OUTPUT:
			case 'o':
                strncpy(outname, optarg, FILENAMESZ) ;
                break ;

            default:
                usage(argv[0]) ;
                break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

    e_comment(0, "loading input data...");
    /* Check that the output file is not in the input list */
    for (i=optind ; i<argc ; i++) {
        if (!strcmp(outname, argv[i])) {
            e_error("the output cube is also in the input list") ;
            return -1 ;
        }
    }

    /* Load input list */
    if ((in = cube_load_strings(argv+optind, argc-optind)) == NULL) {
        e_error("loading cube: aborting");
        return -1 ;
    }

    hs = history_new() ;
    history_add(hs, "--- eclipse catcube");
    history_add(hs, "this cube is made from");
    for (i=optind ; i<argc ; i++) history_add(hs, get_basename(argv[i]));
    e_comment(0, "saving concatenated cube...");
    cube_save_fits_hdrcopy_wh(in, outname, argv[optind], hs);
    history_del(hs);
    cube_del(in);

	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ;
	printf(
"\n"
"use : %s [options] cube1 [cube2 ...]\n"
"merge the cubes 1,2..n into one out cube on the z-axis\n"
"\toutput name is given by\n"
"\t--out <outname> or -o <outname>\n"
"\tdefault output name is 'cat.fits'\n"
"\n\n", pname) ;
	exit(0) ;
}
