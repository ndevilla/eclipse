/*----------------------------------------------------------------------------*/
/**
   @file    norm.c  
   @author  Nicolas Devillard
   @date    October 13, 1995
   @version	$Revision: 1.20 $
   @brief   Normalize a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: norm.c,v 1.20 2002/11/20 12:27:47 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 12:27:47 $
	$Revision: 1.20 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define SCALE_FLUX		10

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "cube normalisation" ;

/*-----------------------------------------------------------------------------
  								Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	cube_t	*	cube_in ;
	int			status ;
    char    *   name_i ;
	char		name_o[FILENAMESZ+1] ;
	int			c ;
	int			mode = NORM_MEAN ;
	double		flux_scaling = 0.0 ;
	history	*	hs ;

    if (argc<2) usage(argv[0]);
	
	/* Command line parsing by getopt() */
    while ((c = getopt(argc, argv, "f:m:L")) != EOF) {
        switch(c) {
			case 'L':
    			eclipse_display_license() ;
	    		return 0 ;

			case 'm':
    			if (strcmp(optarg, "scale") == 0)           mode = NORM_SCALE ;
		    	else if (strcmp(optarg, "mean") == 0)       mode = NORM_MEAN ;
                else if (strcmp(optarg, "flux") == 0)       mode = NORM_FLUX ;
                else if (strcmp(optarg, "aflux") == 0)      mode = NORM_AFLUX ;
                else if (strcmp(optarg, "scaleflux") == 0)  mode = SCALE_FLUX ;
                else {
                    e_error("illegal normalization mode") ;
                    return -1 ;
                }
                break ;
			case 'f':
                flux_scaling = (double)atof(optarg) ;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

    /* Get arguments    */
    if ((argc - optind) < 1) {
        e_error("missing arguments") ;
        return -1 ;
    }
 
    name_i = argv[optind++] ;
	if (optind<argc)    strncpy(name_o, argv[optind], FILENAMESZ) ;
	else                sprintf(name_o, "%s_norm.fits", get_rootname(name_i)) ;
 
	if ((cube_in = cube_load(name_i)) == NULL) {
		e_error("in loading cube : aborting", name_i) ;
		return -1 ;
	}

	if (mode == SCALE_FLUX) status = cube_scale_flux(cube_in, flux_scaling) ;
	else                    status = cube_normalize(cube_in, mode) ;

	if (status!=0) {
		e_error("during normalization: aborting") ;
		return -1 ;
	}

	hs = history_new();
	history_add(hs, "--- eclipse norm") ;
	history_add(hs, "initial input frame is:") ;
	history_add(hs, name_i) ;
	cube_save_fits_hdrcopy_wh(cube_in, name_o, name_i, hs) ;
	history_del(hs);
	cube_del(cube_in) ;

	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s [-m <type>] [options] <incube> [outcube]\n", pname) ;
	printf(
"\ndefault output name for in.fits is in.norm.fits\n"
"\nIndicate what shall be normalized in the output"
"\nwith the -m <type> option:\n"
"\t[-m scale] to normalize pixel values to the [0..1] interval\n"
"\t[-m mean] to normalize to a mean pixel value of 1.0 (default)\n"
"\t[-m flux] to normalize to unity flux\n"
"\t[-m aflux] to normalize to unity absolute flux\n"
"\t[-m scaleflux [-f value]] to scale all fluxes to a value\n"
"\n\n");
	exit(0) ;
}

