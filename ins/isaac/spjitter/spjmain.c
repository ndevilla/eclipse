/*----------------------------------------------------------------------------*/
/**
   @file    spjmain.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.3 $
   @brief   ISAAC spectroscopic jitter data reduction
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjmain.c,v 1.3 2003/01/09 12:40:41 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/09 12:40:41 $
	$Revision: 1.3 $
*/

static char cvsId[]=
"@(#) $Id: spjmain.c,v 1.3 2003/01/09 12:40:41 yjung Exp $";

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eclipse.h"
#include "spjgui.h"
#include "spjini.h"
#include "spjengine.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define OPT_FILE		1001
#define OPT_GENERATE	1002
#define OPT_TIMING		1003
#define OPT_GUI			1004

#define OPT_IN			2000
#define OPT_OUT			2001
#define OPT_CALIB		2002
#define OPT_ALGORITHM	2003

/*-----------------------------------------------------------------------------
                               Function prototypes
 -----------------------------------------------------------------------------*/
 
static void usage(char *pname) ;
static char prog_desc[] = "ISAAC spectroscopic jitter data reduction" ;

/*-----------------------------------------------------------------------------
                                     Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char        ini_name[FILENAMESZ] ;
    int         c ;
	int			gen_flag    = 0 ;
	int			gui_flag    = 0 ;
	int			timing_flag = 0 ;
	long		total_inpix ;
    int         sta ;

	char		name_i[FILENAMESZ];
	char		name_o[FILENAMESZ];
	char		name_c[FILENAMESZ];
    char        algo  [FILENAMESZ] ;

    /* Initialize */
	strcpy(ini_name, "spjitter.ini") ;

	strcpy(name_i, "framelist.ascii");
	strcpy(name_o, "spjitter_result");
	strcpy(name_c, "calib.ascii");
    algo[0] = 0 ;

    /* Command-line parsing */
    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license",   0, 0, OPT_LICENSE},
            {"version",   0, 0, OPT_VERSION},

			{"file",      1, 0, OPT_FILE},
			{"help",      0, 0, OPT_HELP},
			{"generate",  0, 0, OPT_GENERATE},
			{"time",      0, 0, OPT_TIMING},
			{"gui",  	  0, 0, OPT_GUI},

			{"in",  	  1, 0, OPT_IN},
			{"out",  	  1, 0, OPT_OUT},
			{"calib",  	  1, 0, OPT_CALIB},
			{"algorithm", 1, 0, OPT_ALGORITHM},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
						"A:LTc:f:ghi:o:tw",
                        long_options,
                        &option_index) ;
        if (c==-1)
            break ;

        switch(c)
        {

        /* Standard option: display license undocumented option */
            case OPT_LICENSE:
            case 'L':
            eclipse_display_license() ;
            return 0 ;

		/* Standard option: give version number */
			case OPT_VERSION:
			print_eclipse_version() ;
			printf("%s\n", cvsId);
			return 0 ;

			case OPT_HELP:
			case 'h':
			usage(argv[0]) ;
			break ;

		/* Local options */

			/* name of the .ini file */
			case OPT_FILE:
			case 'f':
			strcpy(ini_name, optarg) ;
			break ;

			/* generate a default .ini file */
			case OPT_GENERATE:
			case 'g':
			gen_flag = 1 ;
			break ;

			/* use the GUI */
			case OPT_GUI:
			case 'w':
			gui_flag = 1 ;
			break ;

			/* timing flag: use it to get an evaluation of CPU time */
			case OPT_TIMING:
			case 't':
			timing_flag = 1 ;
			break ;

			/* Input list name */
			case OPT_IN:
			case 'i':
			strcpy(name_i, optarg);
			break ;

			/* Output base name */
			case OPT_OUT:
			case 'o':
			strcpy(name_o, optarg);
			break ;

			/* Calibration file list name */
			case OPT_CALIB:
			case 'c':
			strcpy(name_c, optarg);
			break ;

			/* Algorithm */
			case OPT_ALGORITHM:
			case 'A':
			strcpy(algo, optarg);
			break ;

            default:
            usage(argv[0]) ;
            break ;
        }
	}

    /* Say hello */
    hello_world(argv[0],  prog_desc) ;

	/* Initialize eclipse environment */
	eclipse_init() ;

	if (gui_flag) {
        sta = spjitter_gui() ;
	} else if (gen_flag) {
		/* Generate a default ini file */
		sta = spjitter_ini_generate(ini_name, name_i, name_o, name_c, algo);
		if (sta== 0) {
			fprintf(stderr, "ini file [%s] has been generated\n", ini_name) ;
		}
	} else {
	/* Launch the spjitter engine, timing is not mandatory */
		if (timing_flag)
			eclipse_cpu_timing(START_CLOCK, -1) ;
		/* 
		 * MAIN CALL TO THE SPJITTER ENGINE
		 */
		total_inpix = spjitter_engine(ini_name) ;
		if (timing_flag) {
			printf("\n");
			printf("performance:\n");
			printf("\t      (s)\t      (us)\t(kpix/s)\n") ;
			eclipse_cpu_timing(STOP_CLOCK, total_inpix) ;
		} else {
			if (total_inpix>0)
				e_comment(0, "%ld pixels processed in input", total_inpix);
		}
        if (total_inpix<1) {
            sta=-1 ;
        } else {
            sta=0 ;
        }
	}
    if (debug_active()) xmemory_status() ;
    return sta ;
}

static void usage(char *pname)
{
    hello_world(pname, prog_desc) ;
	printf("\n") ;
    printf("use : %s [flags] [options]\n", pname) ;
    printf("flags are :\n\n") ;

	printf("\t-g or --generate : generate a .ini file\n") ;
	printf("\t-t or --time     : estimate used CPU time\n") ;
	printf("\t-h or --help     : get this help\n") ;

    printf("\noptions are :\n\n") ;
	printf("\t-f <filename> or --file <filename>\n") ;
	printf("\tto specify which .ini file to work on (default: spjitter.ini)\n");
	printf("\n") ;
    printf("\t-w or --gui\n") ;
    printf("\tto launch the GUI\n") ;
    printf("\n");

	printf("following options are only valid with -g or --generate:\n");
	printf("\t-i or --in <filename>     provide input file name\n");
	printf("\t-o or --out <filename>    provide output file name\n");
	printf("\t-c or --calib <filename>  provide calibration file name\n");
    printf("\t-A or --algorithm <name> provide algorithm name\n");
    printf("\n\n") ;
    exit(1) ;
}
