
/*---------------------------------------------------------------------------
   
   File name 	:	wfi_overscan.c
   Author 		:	N. Devillard
   Created on	:	18 May 2000
   Description	:	WFI overscan correction

 *--------------------------------------------------------------------------*/

/*
	$Id: wfi_overscan.c,v 1.8 2001/10/22 11:57:29 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/22 11:57:29 $
	$Revision: 1.8 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <string.h>

#include "eclipse.h"
#include "wfip_lib.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define OPT_PRESCANX		1001
#define OPT_OVRSCANX		1002
#define OPT_CROP			1005
#define OPT_REJ				1006


static void usage(char *pname) ;
static char prog_desc[] = "WFI overscan correction" ;
static char recipe_version[] = "$Revision: 1.8 $" ;

static int
wfi_overscan_correct(
	char	*	name_i,
	char	*	name_o,
	int		*	prescan_x,
	int		*	overscan_x,
	int		*	rej_int,
	int		*	crop_reg
);

/*---------------------------------------------------------------------------
									Main	
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int     c ;

	int		prescan_x[2];
	int		overscan_x[2];
	int		rej_int[2];

	int		crop_reg[4] ;

	char	name_i[FILENAMESZ] ;
	char	name_o[FILENAMESZ] ;

	int		dump_values ;

	/* Set default values for all parameters */

	dump_values = 0 ;
	/* Default prescan region for WFI */
	prescan_x[0]  = 5 ;
	prescan_x[1]  = 48 ;

	/* Default overscan region for WFI */
	overscan_x[0] = 2100 ;
	overscan_x[1] = 2142 ;

	/* Default rejection interval */
	rej_int[0]	= 10 ;
	rej_int[1]	= 10 ;

	/* Crop region: defines the region of interest in the image */
	crop_reg[0] = 55 ;		/* x min */
	crop_reg[1] = 2090 ;	/* x max */
	crop_reg[2] = 35 ;		/* y min */
	crop_reg[3] = 4120 ;	/* y max */

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},
 
            {"x-prescan",	1, 0, OPT_PRESCANX},
            {"x-overscan",	1, 0, OPT_OVRSCANX},
            {"reject",		1, 0, OPT_REJ},

            {"crop",		1, 0, OPT_CROP},
 
            {0, 0, 0, 0}
 
        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhc:dr:",
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
 
        /* Standard option : help */
            case OPT_HELP:
            case 'h':
            usage(argv[0]) ;
            break ;
 
        /* Standard option: version */
            case OPT_VERSION:
            print_eclipse_version() ;
            return 0 ;

		/* Local option: Prescan x */	
			case OPT_PRESCANX:
			sscanf(optarg, "%d %d", &prescan_x[0], &prescan_x[1]);
			break ;

		/* Local option: Overscan x */	
			case OPT_OVRSCANX:
			sscanf(optarg, "%d %d", &overscan_x[0], &overscan_x[1]);
			break ;

		/* Local option: crop */	
			case OPT_CROP:
			case 'c':
			sscanf(optarg, "%d %d %d %d",
					&crop_reg[0],
					&crop_reg[1],
					&crop_reg[2],
					&crop_reg[3]);
			break ;

		/* Local option: rejection interval */	
			case OPT_REJ:
			case 'r':
			sscanf(optarg, "%d %d",
					&rej_int[0],
					&rej_int[1]);
			break ;

		/* Local option: dump input params */
			case 'd':
			dump_values ++ ;
			break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }
 
    /* Initialize eclipse environment */
    eclipse_init() ;

    /*
     * If no argument, display help message
     */
    if ((argc-optind)<1) usage(argv[0]);

	/* Set in/out file names */
	strcpy(name_i, argv[optind++]);
	if ((argc-optind)!=0) {
		strcpy(name_o, argv[optind]);
	} else {
		sprintf(name_o, "%s_scan.fits", get_rootname(name_i));
	}

	if (dump_values) {
		printf(
		"\n"
		"Parameters for this command:\n"
		"\n"
		"[FileNames]\n"
		"Input     = %s\n"
		"Output    = %s\n"
		"\n"
		"[Prescan]\n"
		"xmin      = %d\n"
		"xmax      = %d\n"
		"\n"
		"[Overscan]\n"
		"xmin        = %d\n"
		"xmax        = %d\n"
		"\n"
		"[Scan rejection]\n"
		"min         = %d\n"
		"max         = %d\n"
		"\n"
		"[Trimming]\n"
		"xmin        = %d\n"
		"xmax        = %d\n"
		"ymin        = %d\n"
		"ymax        = %d\n"
		"\n\n",
		name_i,
		name_o,
		prescan_x[0],
		prescan_x[1],
		overscan_x[0],
		overscan_x[1],
		rej_int[0],
		rej_int[1],
		crop_reg[0],
		crop_reg[1],
		crop_reg[2],
		crop_reg[3]);
	}

	wfi_overscan_correct(name_i,
						 name_o,
						 prescan_x,
						 overscan_x,
						 rej_int,
						 crop_reg) ;

    if (debug_active())
		xmemory_status() ;
    return 0 ;
} 



/*
 * This function only gives the usage for the program
 */
 
static void
usage(char *pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] <WFI extension file>\n", pname) ;
    printf("options are:\n");
    printf("\t--x-prescan  'beg end'              sets x prescan region\n");
    printf("\t--x-overscan 'beg end'              sets x overscan region\n");
	printf("\t--reject or -r 'min max'            sets rejection interval\n");
    printf("\t-c or --crop 'xmin xmax ymin ymax'  sets cropping region\n");
    printf("\n\n") ;
    exit(1) ;
} 


static int
wfi_overscan_correct(
	char	*	name_i,
	char	*	name_o,
	int		*	prescan_x,
	int		*	overscan_x,
	int		*	rej_int,
	int		*	crop_reg
)
{
	image_t		*	wfi_frame ;
	image_t		*	cropped_frame ;
	qfits_header*	fh ;
	char			sval[80] ;

	/* Load input image */
	e_comment(0, "loading input [%s]", name_i);
	if ((wfi_frame = image_load(name_i))==NULL) {
		e_error("cannot load frame [%s]", name_i);
		return -1 ;
	}

	e_comment(0, "overscan correction");
	cropped_frame = wfi_overscan_correction(wfi_frame,
											prescan_x,
											overscan_x,
											rej_int,
											crop_reg);
	image_del(wfi_frame);
	if (cropped_frame==NULL) {
		e_error("correcting overscan for frame [%s]", name_i);
		return -1 ;
	}

	/* Build FITS header for output */
	fh = qfits_header_read(name_i);
	if (fh==NULL) {
		e_error("reading FITS header from [%s]", name_i);
		return -1 ;
	}

	/* Add eclipse version */
	qfits_header_add(fh,
					"ECLIPSE",
					get_eclipse_version(),
					"Eclipse version",
					NULL);
	/* Add recipe version */
	qfits_header_add(fh,
					"HIERARCH ESO REC OVSCAN RECVERS",
					recipe_version,
					"Recipe version",
					NULL);	
	/* Add REC.OVSCAN.PRSCX */
	sprintf(sval, "'%d %d'", prescan_x[0], prescan_x[1]);
	qfits_header_add(fh,
					"HIERARCH ESO REC OVSCAN PRSCX",
					sval,
					"Prescan xmin xmax",
					NULL);
	/* Add REC.OVSCAN.OVSCX */
	sprintf(sval, "'%d %d'", overscan_x[0], overscan_x[1]);
	qfits_header_add(fh,
					"HIERARCH ESO REC OVSCAN OVSCX",
					sval,
					"Overscan xmin xmax",
					NULL);
	/* Add REC.OVSCAN.RJOVSC */
	sprintf(sval, "'%d %d'", rej_int[0], rej_int[1]);
	qfits_header_add(fh,
					"HIERARCH ESO REC OVSCAN RJOVSC",
					sval,
					"Rejection min max",
					NULL);
	/* Add REC.OVSCAN.TRIM */
	sprintf(sval, "'%d %d %d %d'",
			crop_reg[0],
			crop_reg[1],
			crop_reg[2],
			crop_reg[3]) ;
	qfits_header_add(fh,
					"HIERARCH ESO REC OVSCAN TRIM",
					sval,
					"xmin xmax ymin ymax",
					NULL);

	e_comment(0, "saving output [%s]", name_o);
	image_save_fits_hdrdump(cropped_frame,
								name_o,
								fh,
								BPP_DEFAULT);
	qfits_header_destroy(fh);
	image_del(cropped_frame);
	return 0 ;
}
