/*----------------------------------------------------------------------------*/
/**
   @file    stcube.c
   @author  Nicolas Devillard
   @date    August 23, 1995
   @version	$Revision: 1.26 $
   @brief   Give minimum statistics about a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: stcube.c,v 1.26 2002/11/20 15:12:58 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 15:12:58 $
	$Revision: 1.26 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define OPT_BADPIXMAP		1001
#define OPT_ZONE			1002
#define OPT_STDEV			1003

/*-----------------------------------------------------------------------------
   								Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "cube statistics" ;
static int produce_time_stdev_image(char * name_i, char * name_o);

/*-----------------------------------------------------------------------------
   									Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	cube_t		*	current ;
	image_stats	*	stats ;
	int				i ;
	int				cn ;
	int				c ;
	char	    *   badpix_filename ;
	int		    *	zone ;
	double	    *	val_range ;

	pixelmap	*	bp_map ;
	int				stdev_image ;
	char		*	stdev_name;

    /* Initialize */
    stdev_image     = 0 ;
	badpix_filename = NULL ;
	zone			= NULL ;
	val_range		= NULL ;
	stdev_name		= NULL ;

    if (argc<2) usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

			{"badpixmap",	1, 0, OPT_BADPIXMAP},
			{"zone",		1, 0, OPT_ZONE},
			{"stdev",       1, 0, OPT_STDEV},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
						"Lhb:z:s:",
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
            /* stcube specific options */
			case OPT_BADPIXMAP:
			case 'b':
                badpix_filename = optarg ;
                break ;
			case OPT_ZONE:
			case 'z':
                zone = malloc(4 * sizeof(int));
                if (sscanf(optarg, "%d %d %d %d",
                                    &zone[0],
                                    &zone[2],
                                    &zone[1],
                                    &zone[3])!=4) {
                    e_error("-z/--zone option expects 4 values");
                    free(zone);
                    return -1 ;
                }
                break ;
			case OPT_STDEV:
			case 's':
                stdev_image = 1 ;
                stdev_name = optarg;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
    }

	if ((argc-optind)<1) {
		e_error("missing argument: input file name(s)") ;
		return -1 ;
	}

	/* Initialize eclipse environment */
	eclipse_init();

	/* Time standard deviation mode */
	if (stdev_image) {
		if (zone!=NULL) {
			e_warning("-z/--zone option useless with -s");
			free(zone);
		}
		produce_time_stdev_image(argv[optind], stdev_name);
		return 0 ;
	}

	/* In case a bad pixel map name was provided, try to load it	*/
	if (badpix_filename!=NULL) {
	    e_comment(0, "loading pixel map...") ;
	    if ((bp_map = pixelmap_load(badpix_filename)) == NULL) {
			if (zone!=NULL)
				free(zone);
		    return -1 ;
	    }
	} else {
		bp_map = NULL ;
	}

	for (cn=optind ; cn<argc ; cn++) {
		printf("\n") ;
		printf("#----------------------------------------------------------\n");
		printf("# FILE %s\n", argv[cn]) ;

		current = cube_load(argv[cn]) ;
		if (current==NULL) {
			if (zone!=NULL) free(zone);
			return -1 ;
		}
		printf(
		"# PLANE     MIN       MAX       MEAN      MEDIAN    STDEV    FLUX\n"
		);
		if (zone!=NULL) {
			printf("# in zone xmin=%d xmax=%d ymin=%d ymax=%d (incl)\n",
					zone[0],
					zone[1],
					zone[2],
					zone[3]);
		}
		for (i=0 ; i<current->np ; i++) {
			stats = image_getstats_opts(current->plane[i], bp_map, NULL, zone);
		   if (stats != NULL) {
				printf("  % 4d      %-9g %-9g %-9g %-9g %-9g %-9g\n",
						i+1,
						(double)stats->min_pix,
						(double)stats->max_pix,
						(double)stats->avg_pix,
						(double)stats->median_pix,
						(double)stats->stdev,
						(double)stats->flux);
				free(stats) ;
				fflush(stdout) ;
			}
		}
		cube_del(current) ;

		if (verbose_active())
printf("#----------------------------------------------------------\n");
		printf("\n") ;
	}

	if (zone!=NULL) free(zone);
	if (bp_map!=NULL) pixelmap_del(bp_map);
	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ; 
	printf("use : %s [options] <cubes...>\n", pname);
	printf(
"options are :\n"
"\t-b or --badpixmap name to take into account bad pixels\n"
"\t-z or --zone 'llx lly urx ury' to compute in a rectangle zone only\n"
"\t-s or --stdev <name> standard deviation frame on a cube\n"
"\n\n") ;
    exit(0) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    produce a standard deviation image from an input cube
  @param    name_i  input file name 
  @param    name_o  output file name
  @return   int 0 if Ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int produce_time_stdev_image(
        char    *   name_i,
        char    *   name_o)
{
	cube_t	*	in ;
	image_t *	stdev ;

	/* Load input cube */
	if ((in=cube_load(name_i))==NULL) {
		return -1 ;
	}
	/* Compute standard deviation frame */
	e_comment(0, "computing standard deviation frame");
	stdev = cube_stdev_z(in);
	cube_del(in);
	if (stdev==NULL) {
		e_error("computing standard deviation frame");
		return -1 ;
	}
	/* Save standard deviation frame */
	e_comment(0, "saving frame [%s]", name_o);
	image_save_fits_hdrcopy(stdev, name_o, name_i, BPP_DEFAULT);
	image_del(stdev);

	return 0 ;
}


