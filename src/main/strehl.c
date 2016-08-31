/*----------------------------------------------------------------------------*/
/**
   @file    strehl.c
   @author  Nicolas Devillard
   @date    July 30, 1996
   @version	$Revision: 1.33 $
   @brief   Strehl computation over a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: strehl.c,v 1.33 2002/11/20 16:24:22 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 16:24:22 $
	$Revision: 1.33 $
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

/* Settings for Adonis */
#define PRIMARY_3_60        (3.47)
#define SECONDARY_3_60      (1.66)
#define LAMBDA_0_3_60       (2.20)
#define D_LAMBDA_3_60       (0.30)
#define PIXSCALE_3_60       (0.05)

/* Settings for NACO */
#define PRIMARY_UT4         (8)
#define SECONDARY_UT4       (1.1)
#define LAMBDA_0_UT4        (5)
#define D_LAMBDA_UT4        (0.1)
#define PIXSCALE_UT4        (0.05)

/* Common settings */
#define STREHL_BOX_SIZE     (64)
#define STREHL_STAR_RADIUS  (2.0)

/*-----------------------------------------------------------------------------
   							   Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "Strehl ratio computation" ;
 
/*-----------------------------------------------------------------------------
 							    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int			c ;
	strehl_parm	spar ;

	cube_t	*	in ;
	int			p ;
	int			cn ;

	memset(&spar, 0, sizeof(strehl_parm));

    /* Set default parameters to UT4 (NACO) */
	spar.m1 = PRIMARY_UT4 ;
	spar.m2 = SECONDARY_UT4 ;
	spar.l0 = LAMBDA_0_UT4 ;
	spar.dl = D_LAMBDA_UT4 ;
	spar.pscale = PIXSCALE_UT4 ;

    spar.size = STREHL_BOX_SIZE ;
	spar.psf_save = 0 ;
	spar.psf_filename = "psf1.fits" ;
    spar.pos_x = spar.pos_y = -1 ;
	spar.estim_bg = 1 ;
    spar.star_bg = 0.0 ;
    spar.star_radius = STREHL_STAR_RADIUS ;
    spar.bg_radius1 = -1.0 ;
    spar.bg_radius2 = -1.0 ;
    
	if (argc<2) usage(argv[0]) ;

    /* 
	 * Command line parsing by getopt()
	 * See man page for getopt(3c) and implementation in ./pipes
	 */
    while ((c = getopt(argc, argv, "Lp:d:l:s:S:P:gr:R:b:T:")) != EOF)
        switch(c) {
            /* Standard option: display license (not documented in usage)	*/
			case 'L':
                eclipse_display_license() ;
                return 0 ;

            /* Strehl computation relative options	*/
			/* primary mirror diameter	*/
			case 'p':
                spar.m1 = (double)atof(optarg) ;
                break ;

			/* secondary mirror diameter	*/
			case 'd':
                spar.m2 = (double)atof(optarg) ;
                break ;

			/* Wavelength and filter width	*/
			case 'l':
                sscanf(optarg, "%lg %lg", &spar.l0, &spar.dl) ;
                break ;

			/* Pixel Scale	*/
			case 's':
                spar.pscale = (double)atof(optarg) ;
                break ;

		    /* Optional: output the ideal PSF to a FITS file */	
			case 'g':
                spar.psf_save = 1 ;
                break ;

			/* Star position */
			case 'P':
                sscanf(optarg, "%d %d", &spar.pos_x, &spar.pos_y) ;
                break ;

            /* Star radius in arcsec */
            case 'r':
                spar.star_radius = (double)atof(optarg) ;
                break ;

			/* Radii for background ring	*/
			case 'R':
                sscanf(optarg, "%lg %lg", &spar.bg_radius1, &spar.bg_radius2) ;
                break ;

			/* Provide a background value	*/
			case 'b':
                spar.star_bg = (double)atof(optarg) ;
                spar.estim_bg = 0 ;
                break ;

            /* Box size */
            case 'S':
                spar.size = (int)atoi(optarg) ;
                break ;

            /* Telescope settings */
            case 'T':
                if (!strcmp(optarg, "3.60")) {
                    /* 3.60 settings (Adonis) */
                    spar.m1 = PRIMARY_3_60 ;
                    spar.m2 = SECONDARY_3_60 ;
                    spar.l0 = LAMBDA_0_3_60 ;
                    spar.dl = D_LAMBDA_3_60 ;
                    spar.pscale = PIXSCALE_3_60 ;
                } else if (!strcmp(optarg, "ut4")) {
                    /* UT4 settings (NACO) */
                    spar.m1 = PRIMARY_UT4 ;
                    spar.m2 = SECONDARY_UT4 ;
                    spar.l0 = LAMBDA_0_UT4 ;
                    spar.dl = D_LAMBDA_UT4 ;
                    spar.pscale = PIXSCALE_UT4 ;
                } else {
                    e_error("unknown telescope: %s\n" "should be 3.60 or ut4\n",
                        optarg);
                    return -1 ;
                }
                break ;

            default:
                usage(argv[0]) ;
                break ;
        }

    /* Get input file name */
    if ((argc-optind) < 1) {
        e_error("missing arguments") ;
        return -1 ;
    }

	/* Initialize eclipse environment */
	eclipse_init();

    /* Display used parameters */
    e_comment(0, "parameters used for computation\n");
    e_comment(0,
           "m1 (m)              %g\n"
           "m2 (m)              %g\n"
           "l0 (um)             %g\n"
           "dl (um)             %g\n"
           "pscale (arcsec/pix) %g\n",
           spar.m1,
           spar.m2,
           spar.l0,
           spar.dl,
           spar.pscale);
    e_comment(0,
           "box size (pix)      %d\n"
           "psf save            %s\n"
           "\n"
           "bg provided         %g\n"
           "bg estimation       %s\n"
           "bg settings         %g %g\n"
           "\n"
           "star x              %d\n"
           "star y              %d\n",
           spar.size,
           spar.psf_save ? spar.psf_filename : "no",
           spar.star_bg,
           spar.estim_bg ? "yes" : "no",
           spar.bg_radius1, spar.bg_radius2,
           spar.pos_x,
           spar.pos_y);

    /* Specified size has to  be a power of 2 */
    if (is_power_of_2(spar.size)<0) {
        e_error("Specified box size must be a power of 2: aborting");
        return -1 ;
    }
    
	for (cn=optind ; cn<argc ; cn++) {
		/* Load cube */
		in = cube_load(argv[cn]);
		if (in==NULL) {
			return -1 ;
		}
		if (in->lx!=in->ly) {
			e_error("can only compute strehl on square images");
			cube_del(in);
			return -1 ;
		}
		if (is_power_of_2(in->lx)<0) {
			e_error("input image size must be a power of 2");
			cube_del(in);
			return -1 ;
		}
		/* Loop on all planes	*/
		printf("file: %s\n", argv[cn]) ;
		for (p=0 ; p<in->np; p++) {
			if (image_compute_strehl(in->plane[p], &spar) == -1) {
                e_warning("cannot compute strehl for plane %d", p+1) ;
            } else printf("plane: %04d\tstrehl %g (bg: %g)\terr: %g\n", 
                    p+1, spar.strehl, spar.star_bg, spar.strehl_err) ;
		}
		cube_del(in);
	}
	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc);
	printf(	"use: %s [options] <cubes...>\n", pname) ;
	printf(
    "options are:\n"
    "\t-p size      M1 size in meters\n"
    "\t-d size      M2 size in meters (incl. obs. ratio)\n"
    "\t-l 'l0 dl'   Central wavelength and width in microns\n"
    "\t-s scale     Pixel scale in arcsec/pix\n"
    "\t-r radius    Star radius in arcsec\n"
    "\t-R 'r1 r2'   Background radii in arcsec\n"
    "\t-b value     Background known value\n"
    "\t-S size      Size of generated PSF image\n"
    "\t-P 'x y'     Specify star position in pixels\n"
    "\t-g           Save PSF as 'psf_strehl.fits'\n"
    "\t-T name      Get default settings for a telescope\n"
    "\t             name can be '3.60' or 'ut4'\n"
    "\n\n");
	exit(0) ;
}
