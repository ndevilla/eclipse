/*----------------------------------------------------------------------------*/
/**
   @file    average.c
   @author  Nicolas Devillard
   @date    August 23, 1995
   @version	$Revision: 1.16 $
   @brief   Average a list of frames or a cube to a single frame
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: average.c,v 1.16 2002/11/19 09:21:58 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 09:21:58 $
	$Revision: 1.16 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define OPT_CUT				1001
#define OPT_MET				1002

#define OPT_FILT_LOW		2001
#define OPT_FILT_HIGH		2002
#define OPT_CYCLE_STEP		2003
#define OPT_RUN_HW			2004

/*-----------------------------------------------------------------------------
   						    Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[]="average a list of frames or a cube to a single frame";

/*-----------------------------------------------------------------------------
  								Main	
 -----------------------------------------------------------------------------*/

int main(int argc, char *argv[]) 
{
	int				c ;
	average_method	amethod ;
	cut_method		cmethod ;
	int				lo_rej, hi_rej ;
	int				cycle_step ;
	int				run_hw ;
	char			inputname[FILENAMESZ+1] ;
	char			outputname[FILENAMESZ+1] ;
	int				ret ;

    /* Test inputs */
    if (argc<2) usage(argv[0]);

    /* Initialize */
	inputname[0] = (char)0 ;
	outputname[0] = (char)0 ;
	cmethod = cut_whole ;
	amethod = avg_linear ;
	lo_rej = -1.0 ;
	hi_rej = -1.0 ;
	cycle_step = -1 ;
	run_hw     = -1 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"cut",     1, 0, OPT_CUT},
            {"method",  1, 0, OPT_MET},

            {"filt-low",  1, 0, OPT_FILT_LOW},
            {"filt-high", 1, 0, OPT_FILT_HIGH},
            {"step",  	  1, 0, OPT_CYCLE_STEP},
            {"halfwidth", 1, 0, OPT_RUN_HW},

            {"in",  	1, 0, OPT_INPUT},
            {"out", 	1, 0, OPT_OUTPUT},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhi:o:",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c)
        {

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
			case OPT_INPUT:
			case 'i':
			strncpy(inputname, optarg, FILENAMESZ) ;
			break ;

			case OPT_OUTPUT:
			case 'o':
			strncpy(outputname, optarg, FILENAMESZ) ;
			break ;

			case OPT_CUT:
			if (!strcmp(optarg, "whole")) {
				cmethod = cut_whole ;
			} else if (!strcmp(optarg, "cycle")) {
				cmethod = cut_cycle ;
			} else if (!strcmp(optarg, "running")) {
				cmethod = cut_running ;
			} else {
				e_error("unsupported cut style: [%s]", optarg) ;
				return -1 ;
			}
			break ;

			case OPT_MET:
			if(!strcmp(optarg, "linear")) {
				amethod = avg_linear ;
			} else if (!strcmp(optarg, "median")) {
				amethod = avg_median ;
			} else if (!strcmp(optarg, "sum")) {
				amethod = avg_sum ;
			} else if (!strcmp(optarg, "filtered")) {
				amethod = avg_filtered ;
			} else {
				e_error("unsupported average method: [%s]", optarg) ;
			}
			break ;
                                             
            case OPT_FILT_LOW:
			lo_rej = atoi(optarg) ;
            break ;

            case OPT_FILT_HIGH:
			hi_rej = atoi(optarg) ;
            break ;

            case OPT_CYCLE_STEP:
			cycle_step = (int)atoi(optarg) ;
            break ;

            case OPT_RUN_HW:
			run_hw = (int)atoi(optarg) ;
            break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }

	/* Initialize eclipse environment */
	eclipse_init();

	/* Real processing starts here */
	ret = average_engine(inputname, 
                            outputname, 
                            cmethod, 
                            amethod, 
                            cycle_step,
					        run_hw, 
                            lo_rej, 
                            hi_rej) ;

	if (debug_active()) xmemory_status() ;

	return ret ;
}

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ;
	printf(
		"\n"
		"use: %s [options] [parameters]\n"
		"parameters are:\n"
		"\t--in  or -i <incube> to give input cube name\n"
		"\t--out or -o <outcube> to give output cube name (optional)\n"
		"\n", pname);
	printf(
		"options are:\n"
		"\t--cut whole (default) to average a cube to an image\n"
		"\t--cut cycle --step <n> to use cycle average\n"
		"\t--cut running --halfwidth <n> to use running average\n"
		"\n");
	printf(
		"\t--method linear (default) normal average\n"
		"\t--method sum to do a sum only\n"
		"\t--method median to do a median average\n"
		"\t--method filtered to do a filtered average, with parameters:\n"
		"\t\t--filt-low <n>  where <n> is a number of low pixels\n"
		"\t\t--filt-high <n> where <n> is a number of high pixels\n"
		"\n\n");
	exit(0) ;
}
	

