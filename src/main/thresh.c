/*----------------------------------------------------------------------------*/
/**
   @file    thresh.c
   @author  Nicolas Devillard
   @date    October 13, 1995
   @version	$Revision: 1.26 $
   @brief   Thresholds pixel values in a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: thresh.c,v 1.26 2002/11/20 16:31:50 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 16:31:50 $
	$Revision: 1.26 $
 */

/*-----------------------------------------------------------------------------
  							    Include
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
  							    Define
 -----------------------------------------------------------------------------*/

#define THRESHOLD_BINARY			0
#define THRESHOLD_PIXELS			1

#define OPT_LOWCUT			        1001
#define OPT_HIGHCUT			        1002
#define OPT_ASSIGNLOW		        1003
#define OPT_ASSIGNHIGH		        1004
#define OPT_BINARY			        1005

/*-----------------------------------------------------------------------------
  							    Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "cube threshold" ;

/*-----------------------------------------------------------------------------
  								    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	cube_t		*	cube_in ;
	int				status ;
	pixelmap	*	pixmap_out ;
	pixelvalue		lo_cut = MIN_PIX_VALUE, 
					hi_cut = MAX_PIX_VALUE ;
	pixelvalue		assigned_locut = MIN_PIX_VALUE,
					assigned_hicut = MAX_PIX_VALUE ; 
	int				mode = THRESHOLD_PIXELS ;
    char        *   name_i ;
    char            name_o[FILENAMESZ+1] ;
	int				assignLow=0,
					assignHigh=0 ;
	int				c ;
	history		*	hs ;

    if (argc<2) usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"lowcut",      1, 0, OPT_LOWCUT},
            {"highcut",     1, 0, OPT_HIGHCUT},
            {"assignlow",   1, 0, OPT_ASSIGNLOW},
            {"assignhigh",  1, 0, OPT_ASSIGNHIGH},
            {"binary",      0, 0, OPT_BINARY},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
						"Ll:h:bc:C:",
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
                usage(argv[0]) ;
                break ;
            /* Standard option: version */
            case OPT_VERSION:
                print_eclipse_version() ;
                return 0 ;
            /* thresh specific options */
			case OPT_BINARY:
            case 'b':
                mode = THRESHOLD_BINARY ;
                break ;
			case OPT_LOWCUT:
            case 'l':
                lo_cut = (pixelvalue)atof(optarg) ;
                break ;
			case OPT_HIGHCUT:
            case 'h':
                hi_cut = (pixelvalue)atof(optarg) ;
                break ;
			case OPT_ASSIGNLOW:
			case 'c':
                assigned_locut = (pixelvalue)atof(optarg) ;
                assignLow = 1 ;
                break ;
			case OPT_ASSIGNHIGH:
			case 'C':
                assigned_hicut = (pixelvalue)atof(optarg) ;
                assignHigh = 1 ;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
	}
	/* Initialize eclipse environment */
	eclipse_init();
 
    /* Get arguments    */
    if ((argc - optind) <1) {
        e_error("please provide an input file name") ;
        return -1 ;
    }
 
    name_i = argv[optind++] ;
	if (optind<argc) {
		strncpy(name_o, argv[optind], FILENAMESZ) ;
	} else {
		sprintf(name_o, "%s_thr.fits", get_rootname(name_i)) ;
	}

	if ((cube_in = cube_load(name_i)) == NULL) {
		e_error("loading file [%s]: aborting", name_i) ;
		return -1 ;
	}
	if ((cube_in->np > 1) && (mode == THRESHOLD_BINARY)) {
		e_error("NAXIS3 > 1: can only threshold an image to binary") ;
		cube_del(cube_in) ;
		return -1 ;
	}

	if (!assignLow) assigned_locut = lo_cut ; 
	if (!assignHigh) assigned_hicut = hi_cut ; 

	e_comment(0, "thresholding %s --> %s", name_i, name_o) ; 
	e_comment(0, "cut interval: [%g %g]", lo_cut, hi_cut) ;
	e_comment(0, "cut values: [%g %g]", assigned_locut, assigned_hicut) ;

	switch(mode) {
		case THRESHOLD_BINARY:
    		pixmap_out = image_threshold2pixelmap(cube_in->plane[0], lo_cut,
                    hi_cut);
            cube_del(cube_in) ;
            if (pixmap_out == NULL) {
                e_error("thresholding failed : aborting") ;
                return -1 ;
            }
            pixelmap_dump(pixmap_out, name_o) ;
            pixelmap_del(pixmap_out) ;
            break ;

		case THRESHOLD_PIXELS:
            status = cube_threshold(cube_in, lo_cut, hi_cut, assigned_locut,
                                 assigned_hicut) ;
            if (status!=0) {
                cube_del(cube_in);
                e_error("thresholding failed : aborting") ;
                return -1 ;
            }

            hs = history_new();
            history_add(hs, "--- eclipse thresh") ;
            history_add(hs, "initial file:") ;
            history_add(hs, name_i) ;
            if (fabs(lo_cut-MIN_PIX_VALUE)<(pixelvalue)1) {
                if (assignLow) {
                    history_add(hs, "lo: %g -> %g", lo_cut, assigned_locut);
                } else {
                    history_add(hs, "lo: %g -> %g", lo_cut, lo_cut);
                }
            }
            if (fabs(hi_cut-MAX_PIX_VALUE)<(pixelvalue)1) {
                if (assignHigh) {
                    history_add(hs, "hi: %g -> %g", hi_cut, assigned_hicut);
                } else {
                    history_add(hs, "hi: %g -> %g", hi_cut, hi_cut);
                }
            }
            cube_save_fits_hdrcopy_wh(cube_in, name_o, name_i, hs) ;
            history_del(hs);
            cube_del(cube_in) ;
            break ;

		default:
            e_error("undefined threshold mode: aborting") ;
            return -1 ;
	}

	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
hello_world(pname, prog_desc) ; 
	printf("use : %s [options] <in> [out]\n", pname) ;
	printf(
"options are:\n"
"\t-l or --lowcut value     : defines low cut\n"
"\t-h or --highcut value    : defines high cut.\n"
"\t-c or --assignlow value  : to assign a value to low cut pixels\n"
"\t-C or --assignhigh value : to assign a value to high cut pixels\n"
"\t-b or --binary           : outputs a pixel map\n"
"\n\n") ;
    exit(0) ;
}


