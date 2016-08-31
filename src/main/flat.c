/*----------------------------------------------------------------------------*/
/**
   @file    flat.c
   @author  Nicolas Devillard
   @date    October 13, 1995
   @version	$Revision: 1.23 $
   @brief   create gain maps from flat fields
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: flat.c,v 1.23 2002/11/20 10:16:37 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 10:16:37 $
	$Revision: 1.23 $
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

#define LO_THRESH_BADPIX	0.5
#define HI_THRESH_BADPIX	2.0

#define OPT_PIXMAP			1001
#define OPT_INTERCEPTS		1002
#define OPT_ERRMAP			1003
#define OPT_PROPORTIONAL	1004
#define OPT_DARK			1005

/*-----------------------------------------------------------------------------
   								Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "create linear gain maps out of twilight data cubes";

/*-----------------------------------------------------------------------------
 								    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	cube_t		*	cube_in ;
	image_t		**	results ;
    image_t     *   norm_gain ;
	pixelmap	*	bp_map ;
	char			nm_in[FILENAMESZ+1],
					nm_out[FILENAMESZ+1],
					full_nm[FILENAMESZ+1];
	int				error_map_flag = 0 ;
	int				pixmap_flag = 0 ;
	int				intercepts_flag = 0 ;
	int				proportional_flag = 0 ;
	int				dark_flag = 0 ;
	char			dark_name[FILENAMESZ];
	cube_t		*	dark_frame ;
	cube_t		*	dark_sub ;
	int				c ;

    if (argc<2) usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"pixmap",     0, 0, OPT_PIXMAP},
            {"intercepts", 0, 0, OPT_INTERCEPTS},
            {"errmap",     0, 0, OPT_ERRMAP},
            {"errmap",     0, 0, OPT_ERRMAP},
            {"dark",       1, 0, OPT_DARK},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Ld:ehobp",
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
			case OPT_PIXMAP:
			case 'b':
	     		pixmap_flag = 1 ;
               break ;
			case OPT_INTERCEPTS:
            case 'o':
    			intercepts_flag = 1 ;
                break ;
			case OPT_ERRMAP:
			case 'e':
		    	error_map_flag = 1 ;
			    break ;
			case OPT_PROPORTIONAL:
			case 'p':
    			proportional_flag = 1 ;
	    		break ;
			case OPT_DARK:
			case 'd':
		    	dark_flag = 1 ;
			    strncpy(dark_name, optarg, FILENAMESZ);
    			break ;
            default:
                usage(argv[0]) ;
                break ;
        }
	}
 
	/* Initialize eclipse environment */
	eclipse_init();

    /* Get arguments    */
	if ((argc-optind)<1) {
        e_error("missing argument: twilight cube name") ;
        return -1 ;
    }
    strncpy(nm_in, argv[optind], FILENAMESZ) ;
	optind++ ;
	if ((argc-optind)<1) strncpy(nm_out, get_rootname(nm_in), FILENAMESZ);
	else strncpy(nm_out, get_rootname(argv[optind]), FILENAMESZ);

	if ((cube_in = cube_load(nm_in)) == NULL) {
		e_error("in loading cube [%s]: aborting", nm_in) ;
		return -1 ;
	}

	/* Dark frame handling */
	if (dark_flag) {
		e_comment(1, "loading dark frame...", dark_name);
		dark_frame = cube_load(dark_name);
		if (dark_frame==NULL) {
			e_error("cannot load dark frame [%s]", dark_name);
		} else {
			e_comment(1, "subtracting dark...");
			dark_sub = cube_copy(cube_in);
			cube_del(cube_in);
			cube_sub(dark_sub, dark_frame);
			cube_del(dark_frame);
			cube_in = dark_sub ;
			e_comment(1, "switching to proportional fit");
			proportional_flag = 1 ;
		}
	}

	if (proportional_flag) results = cube_create_gainmap_proportional(cube_in);
	else results = cube_create_gainmap_robust(cube_in) ;

	cube_del(cube_in) ;
	if ((results == NULL) || (results[0] == NULL) || (results[1] == NULL)) {
		e_error("creating regression maps: aborting") ;
		return -1 ;
	}

    /* Normalize the gain */
    norm_gain = image_normalize(results[0], NORM_MEAN) ;
    
	/* Save flat-field */
	e_comment(0, "saving flat-field");
	sprintf(full_nm, "%s_flat.fits", nm_out);
	image_save_fits_hdrcopy(norm_gain, full_nm, nm_in, BPP_DEFAULT) ;

	/* Create and save bad pixel map if requested */
	if (pixmap_flag) {
		if ((bp_map = image_threshold2pixelmap(norm_gain,
											LO_THRESH_BADPIX,
											HI_THRESH_BADPIX)) == NULL) {
			e_error("cannot create pixel map: not output") ;
		} else {
			e_comment(0, "saving bad pixel map");
			sprintf(full_nm, "%s_badpix.fits", nm_out);
			pixelmap_dump(bp_map, full_nm) ;
			pixelmap_del(bp_map) ;
		}
	}
	image_del(results[0]) ;
    image_del(norm_gain) ;

	if (!proportional_flag) {
	    /* LINEAR FIT: results[1] has intercept map, results[2] has error map */
		/* Save intercepts map if requested */
		if (intercepts_flag) {
			e_comment(0, "saving intercept map");
			sprintf(full_nm, "%s_intercept.fits", nm_out);
			if (results[1]!=NULL) {
				image_save_fits_hdrcopy(results[1], full_nm, nm_in,BPP_DEFAULT);
			} else {
				e_error("null intercept map: cannot save");
			}
		}
		if (results[1] != NULL) image_del(results[1]) ;

		/* Save error map if requested */
		if (error_map_flag) {
			e_comment(0, "saving error map");
			sprintf(full_nm, "%s_errmap.fits", nm_out);
			if (results[2]!=NULL) {
				image_save_fits_hdrcopy(results[2], full_nm, nm_in,BPP_DEFAULT);
			} else {
				e_error("null error map: cannot save");
			}
		}
		if (results[2] != NULL) image_del(results[2]);
	} else {
	    /* PROPORTIONAL FIT: results[1] has error map, no intercept map */
		if (intercepts_flag) e_warning("no intercept map for proportional fit");

		if (error_map_flag) {
			e_comment(0, "saving error map");
			sprintf(full_nm, "%s_errmap.fits", nm_out);
			if (results[1]!=NULL) {
				image_save_fits_hdrcopy(results[1], full_nm, nm_in,BPP_DEFAULT);
			} else {
				e_error("null error map: cannot save");
			}
		}
		if (results[1] != NULL) image_del(results[1]);
	}

	free(results) ;
	if (debug_active()) xmemory_status() ;
	return 0 ;
}
 
static void usage(char * pname)
{
	hello_world(pname, prog_desc);
    printf("use : %s [options] <in_twilight_cube> [basename]\n", pname) ;
	printf(
"additional outputs are specified by the following options:\n"
"\t-o or --intercepts outputs y-intercepts\n"
"\t-e or --errmap outputs error map\n"
"\t-b or --pixmap outputs a bad pixel map\n"
"options are:\n"
"\t-p or --prop indicates proportional fit only\n"
"\t-d or --dark <file> to request dark subtraction\n"
"\n\n");
    exit(0) ;
}

