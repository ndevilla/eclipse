/*----------------------------------------------------------------------------*/
/**
   @file    fft.c   
   @author  Nicolas Devillard
   @date    August 23, 1995August 23, 1995
   @version	$Revision: 1.24 $
   @brief   FFT an image
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: fft.c,v 1.24 2002/11/19 16:21:01 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 16:21:01 $
	$Revision: 1.24 $
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

#define COORD_CARTESIAN		1
#define COORD_POLAR			2

#define OPT_INVERSE		    1001
#define OPT_NOSWAP		    1002
#define OPT_NOCONV		    1003
#define OPT_SWAPONLY	    1004

/*-----------------------------------------------------------------------------
   					    	Function prototype
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "fft 2d on an image" ;

/*-----------------------------------------------------------------------------
 *								Main 
 *----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    cube_t		*	cube_in ;	
	cube_t		*	complex ;
	cube_t		*	cube_out ;
	int				mode = FFT_FORWARD ;
	int				swapping = 1 ;
	int				swap_only = 0 ;
	int				coordinates = COORD_POLAR ; 
	char			inname[FILENAMESZ+1],
					outname[FILENAMESZ+1] ;
	history		*	hs ;
	int				c ;	
	int				i ;
	
    if (argc<2) usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"inverse", 0, 0, OPT_INVERSE},
            {"noswap",  0, 0, OPT_NOSWAP},
            {"noconv",  0, 0, OPT_NOCONV},
            {"swaponly",0, 0, OPT_SWAPONLY},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhnics",
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
			case OPT_NOSWAP:
			case 'n':
	    		swapping = 0 ;
		    	break ;
			case OPT_INVERSE:
			case 'i':
			    mode = FFT_INVERSE ;
    			break ;
			case OPT_NOCONV:
			case 'c':
	    		coordinates = COORD_CARTESIAN ;
		    	break ;
			case OPT_SWAPONLY:
			case 's':
    			swap_only = 1 ;
	    		break ;
            default:
                usage(argv[0]) ;
                break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

	if ((argc-optind)<1) {
		e_error("missing arguments: input file name") ;
		return -1 ;
	}

	strncpy(inname, argv[optind++], FILENAMESZ) ;
	if ((argc-optind)>0) {
		strncpy(outname, argv[optind], FILENAMESZ) ;
	} else {
		sprintf(outname, "%s_fft.fits", get_rootname(inname)) ;
	}
	
	if ((cube_in = cube_load(inname)) == NULL) return -1 ;

	/* Error handling */
	if (cube_in->np>2) {
		e_error("%d planes detected in %s\n"
				"can only fft single or double plane cubes",
				cube_in->np, inname) ;
		cube_del(cube_in) ;
		return -1 ;
	}

	if ((is_power_of_2(cube_in->lx)==-1) ||
		(is_power_of_2(cube_in->ly)==-1)) {
		e_error("can only apply FFT to images with a power of 2 dimension!");
		cube_del(cube_in) ;
		return -1 ;
	}
	if ((cube_in->np != 2) && (mode == FFT_INVERSE)) {
		e_error("cannot do inverse FFT on single plane cubes") ;
		cube_del(cube_in) ;
		return -1 ;
	}

	/* Forward FFT cube	*/
	if (swap_only) {
		for (i=0 ; i<cube_in->np ; i++) {
            compute_status("swapping quadrants", i, cube_in->np, 1);
			image_swapquad(cube_in->plane[i]) ;
		}
        e_comment(1, "saving result as %s", outname);
		hs = history_new() ;
		history_add(hs, "--- eclipse fft");
		history_add(hs, "input file:") ;
		history_add(hs, inname) ;
		history_add(hs, "only quadrant swapping applied") ;
		cube_save_fits_hdrcopy_wh(cube_in, outname, inname, hs) ;
		history_del(hs);
		cube_del(cube_in) ;
		return 0 ;
	}

	if (mode == FFT_FORWARD) {
        e_comment(1, "computing forward FFT...");
		complex = image_fft(cube_in->plane[0], NULL, FFT_FORWARD) ;
		cube_del(cube_in) ;
        if (complex==NULL) {
            e_error("computing FFT");
            return -1 ;
        }
		if (coordinates == COORD_POLAR) {
            e_comment(2, "converting (x,y)->(rho,theta)");
			cube_out = cube_conv_xy_rtheta(complex) ;
		} else {
			cube_out = cube_copy(complex) ;
		}
		cube_del(complex) ;

        if (cube_out==NULL) {
            e_error("converting coordinates");
            return -1 ;
        }
		hs = history_new();
		history_add(hs, "--- eclipse fft") ;
		history_add(hs, "input file:") ;
		history_add(hs, inname) ;
		history_add(hs, "fft applied") ;
		if (coordinates == COORD_POLAR) {
			history_add(hs, "conversion to polar coord. applied") ;
		}
		if (swapping) {
            e_comment(2, "swapping quadrants");
			image_swapquad(cube_out->plane[0]) ;
			image_swapquad(cube_out->plane[1]) ;
			history_add(hs, "quadrant swapping applied") ;
		}
        e_comment(1, "saving result as %s", outname);
		cube_save_fits_hdrcopy_wh(cube_out, outname, inname, hs) ;
		history_del(hs);
		cube_del(cube_out) ;
	} else {

	    /* Inverse FFT cube	*/
        e_comment(1, "computing inverse FFT...");
		if (swapping) {
            e_comment(2, "swapping quadrants");
			image_swapquad(cube_in->plane[0]) ;
			image_swapquad(cube_in->plane[1]) ;
		}

		if (coordinates == COORD_POLAR) {
            e_comment(2, "converting (rho,theta)->(x,y)");
			complex = cube_conv_rtheta_xy(cube_in) ;
		} else {
			complex = cube_copy(cube_in) ;
		}
		cube_del(cube_in) ;
        if (complex==NULL) {
            e_error("converting coordinates");
            return -1 ;
        }

        e_comment(2, "applying FFT...");
		cube_out = image_fft(complex->plane[0],complex->plane[1],FFT_INVERSE);
		cube_del(complex) ;
        if (cube_out==NULL) {
            e_error("computing FFT");
            return -1 ;
        }
		hs = history_new();
		history_add(hs, "--- eclipse fft") ;
		history_add(hs, "input file:") ;
		history_add(hs, inname) ;
		history_add(hs, "inverse fft applied") ;
		if (swapping) {
			history_add(hs,"quadrant swapping before ifft applied");
		}
		if (coordinates == COORD_POLAR) {
			history_add(hs,"conv to cartesian before ifft applied");
		}
        e_comment(1, "saving result image as %s", outname);
		image_save_fits_hdrcopy_wh(	cube_out->plane[0], 
									outname, 
									inname, 
									BPP_DEFAULT,
									hs) ;
		cube_del(cube_out) ;
	}

	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
    printf("use : %s [options] <in> [out]\n", pname) ;
	printf(
"default behaviour is:\n"
"\t--> forward FFT\n"
"\t\tFFT the input image (single plane)\n"
"\t\tconvert the 2 result planes from (x,y) to (amp,phase)\n"
"\t\tswap quadrants in (amp,phase)\n"
"\t--> inverse FFT\n"
"\t\tfrom a couple of images:\n"
"\t\tswap quadrants for each image in the couple\n"
"\t\tconvert from (amp,phase) to (x,y)\n"
"\t\tapply inverse FFT\n"
"\n"
"options are:\n"
"\t[-i] or [--inverse] inverse FFT\n"
"\t[-n] or [--noswap] no swapping\n"
"\t[-c] or [--noconv] does not convert (x,y) <=> (amp,phase)\n"
"\t[-s] or [--swaponly] only do quadrant swapping\n"
"\n\n") ;
    exit(0) ;
}


