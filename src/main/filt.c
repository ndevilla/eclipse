/*----------------------------------------------------------------------------*/
/**
   @file    filt.c
   @author  Nicolas Devillard
   @date    August 29, 1995
   @version	$Revision: 1.25 $
   @brief   Filter a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: filt.c,v 1.25 2002/11/20 09:47:42 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 09:47:42 $
	$Revision: 1.25 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define OPT_FILTER			1001
#define OPT_FILTVAL			1002
#define OPT_FILTKERNHSIZE	1003

/*-----------------------------------------------------------------------------
   								Functions prototypes
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "apply a digital filter in spatial domain" ;

/*-----------------------------------------------------------------------------
  								    Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	cube_t	*	cube_in ;
	int			status ;
	double		filtval[9] ;	
	char		filt_name[FILENAMESZ+1] ;
	char		inname[FILENAMESZ+1],
				outname[FILENAMESZ+1] ;
	history	*	hs ;
	int			c ;
	int			i ;

    /* Initialize */
	filt_name[0] = (char)0 ;
	filtval[0] = 4.0 ;
	for (i=1 ; i<9 ; i++) filtval[i] = 0.00 ;
	inname[0] = outname[0] = (char)0 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"filter",  1, 0, OPT_FILTER},
            {"val",   	1, 0, OPT_FILTVAL},
            {"khsize", 	1, 0, OPT_FILTKERNHSIZE},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhf:k:p:",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c) {
            /* Standard option: display license undocumented option */
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
            /* Local options */
			case OPT_FILTER:
            case 'f':
	    		strncpy(filt_name, optarg, FILENAMESZ) ;
                break ;
			case OPT_FILTVAL:
            case 'p':
                sscanf(optarg,
	    				"%lg %lg %lg %lg %lg %lg %lg %lg %lg",
		    			filtval,
			    		filtval+1,
				    	filtval+2,
					    filtval+3,
    					filtval+4,
	    				filtval+5,
		    			filtval+6,
			    		filtval+7,
				    	filtval+8) ;
    			break ;
			case OPT_FILTKERNHSIZE:
			case 'k':
	    		filtval[0] = (double)atoi(optarg);
		    	break ;
            default:
                usage(argv[0]) ;
             break ;
        }
    }

	/* Initialize eclipse environment */
	eclipse_init();

    /* Get arguments    */
    if ((argc - optind) < 1) usage(argv[0]);
	strncpy(inname, argv[optind++], FILENAMESZ) ;
	if ((argc-optind) != 0) strncpy(outname, argv[optind], FILENAMESZ) ;
	else sprintf(outname, "%s_%s.fits", get_rootname(inname), filt_name);

	if ((cube_in = cube_load(inname)) == NULL) {
		e_error("loading [%s]", inname) ;
		return -1 ;
	}

	if ((status = cube_filter(cube_in, filt_name, filtval)) != 0) {
		e_error("applying filter: operation aborted\n") ;
		cube_del(cube_in);
		return -1 ;
	}

	hs = history_new();
	history_add(hs, "--- eclipse filt") ;
	history_add(hs, "input file:") ;
	history_add(hs, inname) ;
	history_add(hs, "filter: %s", filt_name) ;

	if (!strcmp(filt_name, "user-linear") || !strcmp(filt_name, "user-morpho")){
		history_add(hs, "using the following kernel:") ;
		history_add(hs, "%g %g %g", filtval[0], filtval[1], filtval[2]) ;
		history_add(hs, "%g %g %g", filtval[3], filtval[4], filtval[5]) ;
		history_add(hs, "%g %g %g", filtval[6], filtval[7], filtval[8]) ;
	} else if (!strcmp(filt_name, "flat")) {
		history_add(hs, "using a flat kernel of size %dx%d",
					2 * (int)filtval[0]+1,
					2 * (int)filtval[0]+1);
	} else if (!strcmp(filt_name, "3x1")) {
		history_add(hs,  "using the following 3x1 kernel:");
		history_add(hs, "%g %g %g", filtval[0], filtval[1], filtval[2]) ;
	}

	cube_save_fits_hdrcopy_wh(cube_in, outname, inname, hs) ;
	history_del(hs);
	cube_del(cube_in) ;
	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s [parameters] <in> [out]\n", pname);
	printf(
		"parameters are:\n"
		"\t[-f <name>] or [--filter <name>] to specify the filter\n"
		"\t[-p 'f1 ... f9'] or [--val 'f1 ... f9'] for user-defined filters\n"
		"\n");
	printf(
		"provided filters are:\n"
		"\n"
		"\tdx           dy           d2x          d2y\n"
		"\tcontour1     contour2     contour3     contrast1\n"
		"\tmin          max          median       max-min\n"
		"\tmean3        mean5        user-linear  user-morpho\n"
		"\t3x1          flat\n"
		"\n");
	printf(
		"the user-defined filters user-linear and user-morpho\n"
		"require filter values provided through -p or --val option\n"
		"followed by 9 values enclosed in quotes\n"
		"\n");
	printf(
		"the 3x1 filter expects only 3 values in quotes through the\n"
		"-p or --val option\n"
		"\n");
	printf(
		"the flat filter expects a single (integer) value through the -p\n"
		"or --val option, setting the kernel half-size.\n"
		"\n\n");
	exit(0) ;
}
