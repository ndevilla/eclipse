/*----------------------------------------------------------------------------*/
/**
   @file    deadpix.c
   @author  Nicolas Devillard
   @date    13 Oct 1995
   @version	$Revision: 1.29 $
   @brief   create a dead pixel map from a sky cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: deadpix.c,v 1.29 2002/11/19 12:05:14 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 12:05:14 $
	$Revision: 1.29 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define DEADPIX_JOB_NOJOB			0
#define DEADPIX_JOB_DETECT			1
#define DEADPIX_JOB_CLEAN			2

#define MIN_PLANES_SKY_PROCESSING	10

#define METHOD_MEDIAN				0
#define METHOD_SKYVAR				1
#define METHOD_UNKNOWN				2

#define MEDIAN_THRESHOLD			10.0 
#define SIGMA_WIDTH					3.0 

#define OPT_DETECT					1001
#define OPT_CLEAN					1002
#define OPT_DETECT_SKYVAR			1004
#define OPT_DETECT_MEDIAN			1005

#define OPT_SIGMA					2001
#define OPT_THRESHOLD				3001
#define OPT_PIXMAP					4001
#define OPT_SKY						4002

#define OPT_IN						5001
#define OPT_OUT						5002

/*-----------------------------------------------------------------------------
   							Private declarations	
 -----------------------------------------------------------------------------*/

static int determine_detect_method(char * cubename) ;
static void usage(char *pname) ;
static char prog_desc[] = "bad pixel map handling" ;

/*-----------------------------------------------------------------------------
  								Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	pixelmap	*	bad_pixelmap ;
	char		*	skyname ;
	char		*	pixmapname ;
	int				detect_method ;
	pixelvalue		median_threshold ;
	double			sigma_width ;
	int				c ;
	int				deadpix_job ;
	char		*	name_in ;
	char			name_out[FILENAMESZ+1] ;
	cube_t		*	cube_in ;
	int				status ;
	history		*	hs ;

    /* Initialize */
	pixmapname       = "badpixmap" ;
	skyname          = NULL ;
	detect_method    = METHOD_UNKNOWN ;
	median_threshold = MEDIAN_THRESHOLD ;
	sigma_width      = SIGMA_WIDTH ;
	deadpix_job		 = DEADPIX_JOB_NOJOB ;
	name_in			 = NULL ;
	name_out[0]		 = 0 ;

    if (argc<2) usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
			/* Standard options */
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

			/* What job to do? */
			{"detect",  0, 0, OPT_DETECT},
			{"clean",   0, 0, OPT_CLEAN},

			/* For 'detect' only */
			{"skyvar",  0, 0, OPT_DETECT_SKYVAR},
			{"median",  0, 0, OPT_DETECT_MEDIAN},

			{"sigma",     1, 0, OPT_SIGMA},
			{"threshold", 1, 0, OPT_THRESHOLD},

			{"pixmap",  1, 0, OPT_PIXMAP},
			{"sky",     1, 0, OPT_SKY},

			{"in",      1, 0, OPT_IN},
			{"out",     1, 0, OPT_OUT},

            {0, 0, 0, 0}
        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhdcm:s:t:p:S:i:o:",
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
			case OPT_DETECT:
			case 'd':
	    		if (deadpix_job != DEADPIX_JOB_NOJOB) {
		    		e_error("can only do detect or clean, not both") ;
			    	return -1 ;
			    }
			    deadpix_job = DEADPIX_JOB_DETECT ;
			    break ;
			case OPT_CLEAN:
			case 'c':
			    if (deadpix_job != DEADPIX_JOB_NOJOB) {
				    e_error("can only do detect or clean, not both") ;
				    return -1 ;
			    }
			    deadpix_job = DEADPIX_JOB_CLEAN ;
			    break ;
			case 'm':
			    if (!strcmp(optarg, "median")) {
                    detect_method = METHOD_MEDIAN ;
                } else if (!strcmp(optarg, "skyvar")) {
                    detect_method = METHOD_SKYVAR ;
                } else {
                    e_error("unknown detect method: [%s] use median or skyvar",
                            optarg) ;
                    return -1 ;
                }
                break ;
			case OPT_DETECT_SKYVAR:
			    detect_method = METHOD_SKYVAR ;
			    break ;
			case OPT_DETECT_MEDIAN:
			    detect_method = METHOD_MEDIAN ;
			    break ;
			case OPT_SIGMA:
			case 's':
			    sigma_width = (double)atof(optarg) ;
			    break ;
			case OPT_THRESHOLD:
			case 't':
			    median_threshold = (pixelvalue)atof(optarg) ;
			    break ;
			case OPT_PIXMAP:
			case 'p':
			    pixmapname = optarg ;
			    break ;
			case OPT_SKY:
			case 'S':
			    skyname = optarg ;
			    break ;
			case OPT_IN:
			case 'i':
			    name_in = optarg;
			    break ;
			case OPT_OUT:
			case 'o':
			    strncpy(name_out, optarg, FILENAMESZ) ;
			    break ;
			default:
			    usage(argv[0]) ;
		    	break ;
		}
	}

	/* Initialize eclipse environment */
	eclipse_init();

	if (deadpix_job == DEADPIX_JOB_NOJOB) {
		e_error("no job requested: specify [--detect | --clean]") ;
		return -1 ;
	}

	if (deadpix_job == DEADPIX_JOB_DETECT) {
		/* This part takes care of bad pixel detection */
		if (skyname==NULL) {
			e_error("no provided sky file name: use -S/--sky option");
			return -1 ;
		}
		if (detect_method == METHOD_UNKNOWN)
			detect_method = determine_detect_method(skyname) ;

		switch(detect_method) {
			case METHOD_MEDIAN:
			    bad_pixelmap = cube_detect_deadpix_median(skyname, 
                        median_threshold) ;
			    break ;

			case METHOD_SKYVAR:
			    bad_pixelmap = cube_detect_deadpix_z(skyname, sigma_width) ;
			    break ;

			case METHOD_UNKNOWN:
			default:
			    bad_pixelmap = NULL ;
			    e_error("unable to detect: no specified method") ;
			    break ;
		}		

		if (bad_pixelmap == NULL) {
			e_error("in computing pixel map: no output") ;
			return -1 ;
		}

		e_comment(1, "dumping dead pixel map\n") ;
		if (file_exists(pixmapname)) {
			e_warning("overwriting file [%s]", pixmapname) ;
		}
		pixelmap_dump(bad_pixelmap, pixmapname) ;
		pixelmap_del(bad_pixelmap) ;
	} else {
		/* This part takes care of bad pixel cleaning */
		if (name_in==NULL) {
			e_error("missing input file name, use the -i/--in option");
			return -1 ;
		}

		if (name_out[0] == (char)0) {
			sprintf(name_out, "%s_cln.fits", get_rootname(name_in)) ;
		}

		cube_in = cube_load(name_in) ;
		if (cube_in == NULL) {
			e_error("cannot load input cube [%s]: aborting", name_in) ;
			return -1 ;
		}

		bad_pixelmap = pixelmap_load(pixmapname) ;
		if (bad_pixelmap == NULL) {
			e_error("cannot load pixel map [%s]: aborting", pixmapname);
			cube_del(cube_in) ;
			return -1 ;
		}

		if ((bad_pixelmap->lx != cube_in->lx) ||
			(bad_pixelmap->ly != cube_in->ly)) {
			e_error("input cube and pixel map have different sizes") ;
			cube_del(cube_in) ;
			pixelmap_del(bad_pixelmap) ;
			return -1 ;
		}

		e_comment(0, "replacing bad pixels...") ;
		status = cube_clean_deadpix(cube_in, bad_pixelmap) ;
		pixelmap_del(bad_pixelmap) ;
		if (status != 0) {
			e_error("during cleaning: aborting") ;
			cube_del(cube_in);
			return -1 ;
		}

		hs = history_new();
		history_add(hs, "--- eclipse deadpix [clean]") ;
		history_add(hs, "input file:") ;
		history_add(hs, name_in) ;
		history_add(hs, "bad pixel map:") ;
		history_add(hs, pixmapname) ;
		cube_save_fits_hdrcopy_wh(cube_in, name_out, name_in, hs) ;
		history_del(hs);
		cube_del(cube_in) ;
	}

	if (debug_active()) xmemory_status() ;
	return 0 ;
}

/*-----------------------------------------------------------------------------
  							Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Choose which method is best to apply
  @param    cubename    sky cube name
  @return   int corresponding to the chosen method
  Depending only on number of images in cube
 */
/*----------------------------------------------------------------------------*/
static int determine_detect_method(char * cubename)
{
	int				detect_method ;
	cube_info	*	fileinfo ;

	if ((fileinfo = cube_getinfo(cubename)) == NULL) {
		e_error("in reading file [%s]: aborting", cubename) ;
		return METHOD_UNKNOWN ;
	}

	if (fileinfo->n_im < MIN_PLANES_SKY_PROCESSING) {
		detect_method = METHOD_MEDIAN ;
		e_comment(0, "Using median method") ;
	} else {
		detect_method = METHOD_SKYVAR ;
		e_comment(0, "Using sky variations method") ;
	}

	free(fileinfo) ;
	return detect_method ;
}

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ; 
	printf(
	"\n"
	"use : %s [parameters]\n"
	"parameters are:\n"
	"\n", pname);
	printf(
	"-> specify a mode (one is mandatory)\n"
	"\n"
	"\t--detect or -d to switch to detection mode\n"
	"\t--clean  or -c to switch to cleaning mode\n"
	"\n");
	printf(
	"\t-> detect mode options\n"
	"\n"
	"\t\t--skyvar or -m skyvar to use sky variation method\n"
	"\t\t--sigma <S> or -s <S> to specify sigma\n"
	"\t\t--sky <name> or -S <name> to provide input sky name\n"
	"\n");
	printf(
	"\t\t--median or -m median to use median method\n"
	"\t\t--threshold <T> or -t <T> to specify median threshold\n"
	"\n\n");
	printf(
	"\t-> cleaning mode options\n"
	"\n"
	"\t\t--in <file> or -i <file> to specify input file name\n"
	"\t\t--out <file> or -o <file> to specify output file name\n"
	"\t\t(default output name for *.fits is *.cln.fits)\n"
	"\n\n");
	printf(
	"-> common to both modes:\n"
	"\n"
	"\t--pixmap <name> or -p <name> specifies the pixel map name\n"
	"\t\tin detect mode, this is an output\n"
	"\t\tin cleaning mode, this is an input\n"
	"\n\n");
	exit(0) ;
}
