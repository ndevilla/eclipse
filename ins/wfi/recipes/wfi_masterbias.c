
/*---------------------------------------------------------------------------
   
   File name 	:	wfi_masterbias.c
   Author 		:	N. Devillard
   Created on	:	June 2000
   Description	:	WFI master bias creation

 *--------------------------------------------------------------------------*/

/*
	$Id: wfi_masterbias.c,v 1.15 2001/10/22 11:57:29 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/22 11:57:29 $
	$Revision: 1.15 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "eclipse.h"
#include "wfip_lib.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define OPT_XTNUM			1001

#define OPT_KAPPA1			1011
#define OPT_MINVALID1		1012

#define OPT_PRESCANX        1021
#define OPT_OVRSCANX        1022
#define OPT_CROP            1023
#define OPT_REJ             1024


static char recipe_version[] = "$Revision: 1.15 $";

static void usage(char *pname) ;
static char prog_desc[] = "WFI master bias creation" ;


static int wfi_create_master_bias(
	char	*	name_i,
	char	*	name_o,
	int			xtnum,
	double		kappa1,
	int			minvalid1,
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
	int		xtnum ;
	int		check ;
	char	name_i[FILENAMESZ];
	char	name_o[FILENAMESZ];

	double	kappa1 ;
	int		minvalid1 ;

    int     prescan_x[2];
    int     overscan_x[2];
    int     rej_int[2];
 
    int     crop_reg[4] ;

    /* Set default values for all parameters */
 
    /* Default prescan region for WFI */
    prescan_x[0]  = WFI_PRESCAN_X_MIN ;
    prescan_x[1]  = WFI_PRESCAN_X_MAX ;
 
    /* Default overscan region for WFI */
    overscan_x[0] = WFI_OVERSCAN_X_MIN ;
    overscan_x[1] = WFI_OVERSCAN_X_MAX ;
 
    /* Default rejection interval */
    rej_int[0]  = 10 ;
    rej_int[1]  = 10 ;
 
    /* Crop region: defines the region of interest in the image */
    crop_reg[0] = WFI_CROP_X_MIN ;
    crop_reg[1] = WFI_CROP_X_MAX ;
    crop_reg[2] = WFI_CROP_Y_MIN ;
    crop_reg[3] = WFI_CROP_Y_MAX ;

	xtnum = -1 ;
	kappa1 = 2.0 ;
	minvalid1 = 3 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},
 
            {"xtnum", 	1, 0, OPT_XTNUM},

            {"kappa1", 	1, 0, OPT_KAPPA1},
            {"min1", 	1, 0, OPT_MINVALID1},
 
            {"x-prescan",   1, 0, OPT_PRESCANX},
            {"x-overscan",  1, 0, OPT_OVRSCANX},
            {"reject",      1, 0, OPT_REJ},
 
            {"crop",        1, 0, OPT_CROP},

            {0, 0, 0, 0}
 
        } ;
        c = getopt_long(argc,
                        argv,
                        "Lc:hk:m:r:x:",
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
			printf("recipe version: %s\n", recipe_version);
            return 0 ;

		/* Local option: extension number */
			case OPT_XTNUM:
			case 'x':
			xtnum = (int)atoi(optarg);
			break;

		/* Local option: first kappa number */
			case OPT_KAPPA1:
			case 'k':
			kappa1 = (double)atof(optarg);
			break;

		/* Local option: first min valid */
			case OPT_MINVALID1:
			case 'm':
			minvalid1 = (int)atoi(optarg);
			break;

        /* Local option: Prescan x */
            case OPT_PRESCANX:
            check = sscanf(optarg, "%d %d", &prescan_x[0], &prescan_x[1]);
			if (check!=2) {
				e_error("expecting 2 values for --x-prescan, got %d", check);
				return -1 ;
			}
            break ;
 
        /* Local option: Overscan x */
            case OPT_OVRSCANX:
            check = sscanf(optarg, "%d %d", &overscan_x[0], &overscan_x[1]);
			if (check!=2) {
				e_error("expecting 2 values for --x-overscan, got %d", check);
				return -1 ;
			}
            break ;
 
        /* Local option: crop */
            case OPT_CROP:
            case 'c':
            check = sscanf(optarg, "%d %d %d %d",
							&crop_reg[0],
							&crop_reg[1],
							&crop_reg[2],
							&crop_reg[3]);
			if (check!=4) {
				e_error("expecting 4 values for --crop, got %d", check);
				return -1 ;
			}
            break ;
 
        /* Local option: rejection interval */
            case OPT_REJ:
            case 'r':
            check = sscanf(optarg, "%d %d", &rej_int[0], &rej_int[1]);
			if (check!=2) {
				e_error("expecting 2 values for --reject, got %d", check);
				return -1 ;
			}
            break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }
 
    /*
	 * Initialize eclipse environment.
     */
    eclipse_init() ;

    /*
     * If no argument, display help message
     */
    if ((argc-optind)<1) usage(argv[0]);

	/* Get in/out names */
	strcpy(name_i, argv[optind]);
	optind++;
	if ((argc-optind)<1) {
		sprintf(name_o, "%s_mb.fits", get_rootname(name_i));
	} else {
		strcpy(name_o, argv[optind]);
	}

	wfi_create_master_bias(	name_i,
						  	name_o,
						  	xtnum,
						  	kappa1,
						 	minvalid1,
						  	prescan_x,
							overscan_x,
							rej_int,
							crop_reg);

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
    printf("use : %s [options] framelist [output]\n", pname) ;
    printf(
"options are:\n"
"\t-x or --xtnum <n> to process extension <n>\n"
"\n"
"*** overscan options\n"
"\t--x-prescan  'beg end'              sets x prescan region\n"
"\t--x-overscan 'beg end'              sets x overscan region\n"
"\t-r or --reject 'min max'            sets rejection interval\n"
"\t-c or --crop 'xmin xmax ymin ymax'  sets cropping region\n"
"\n"
"*** frame rejection options\n"
"\t--kappa1 <val>      Kappa for sigma rejection of frames\n"
"\t--min1 <n>          Min number of frames to proceed after rejection\n"
"\n\n") ;
    exit(1) ;
} 


#define WFI_MIN_NUM_FRAMES		3

static int wfi_create_master_bias(
	char	*	name_i,
	char	*	name_o,
	int			xtnum,
	double		kappa1,
	int			minvalid1,
	int		*	prescan_x,
	int		*	overscan_x,
	int		*	rej_int,
	int		*	crop_reg
)
{
	cube_t		*	i_cube ;
	image_t		*	corrected ;
	image_t		*	stacked ;
	int				i ;
	int			*	valid ;
	int				nval ;
	double		*	all_means ;
	double			avg1, sig1 ;
	double			lo_b, hi_b ;
	char			cmt[5];

	qfits_header*	fh ;
	qfits_header*	fh_ref ;
	char			cval[80];
	char		*	getval ;
	framelist	*	flist ;
	char		*	first_name ;

	int				nchips ;


	i_cube = wfi_cube_load(name_i, xtnum);
	if (i_cube==NULL) {
		e_error("loading input list [%s]", name_i);
		return -1 ;
	}
	if (i_cube->np < WFI_MIN_NUM_FRAMES) {
		e_error("found only [%d] frames - need at least %d",
				i_cube->np,
				WFI_MIN_NUM_FRAMES);
		cube_del(i_cube);
		return -1 ;
	}

	/* Compute average value for each frame */
	all_means = malloc(i_cube->np * sizeof(double));
	avg1 = 0.00 ;
	for (i=0 ; i<i_cube->np ; i++) {
		compute_status("computing averages", i, i_cube->np, 1);
		all_means[i] = image_getmean(i_cube->plane[i]);
		avg1 += all_means[i];
	}
	avg1 /= (double)i_cube->np ;

	/* Compute average and sigma for rejection */
	sig1 = 0.00 ;
	for (i=0 ; i<i_cube->np ; i++) {
		sig1 += (all_means[i]-avg1) * (all_means[i]-avg1);
	}
	sig1 /= (double)(i_cube->np-1) ;
	sig1 = sqrt(sig1) ;

	/* Print out current results, reject frames */
	nval  = 0 ;
	valid = malloc(i_cube->np * sizeof(int));

	lo_b = avg1 - kappa1 * sig1 ;
	hi_b = avg1 + kappa1 * sig1 ;

	e_comment(1, "frame rejection setup:");
	e_comment(1, "average: %g", avg1);
	e_comment(1, "sigma  : %g", sig1);
	e_comment(1, "kappa  : %g", kappa1);
	e_comment(1, "low    : %g", lo_b);
	e_comment(1, "high   : %g", hi_b);

	for (i=0 ; i<i_cube->np ; i++) {
		if ((all_means[i]>lo_b) && (all_means[i]<hi_b)) {
			strcpy(cmt, "[X]");
			valid[i] = 1 ;
			nval++ ;
		} else {
			strcpy(cmt, "[ ]");
			valid[i] = 0 ;
		}
		e_comment(0, "%s frame %02d: %g", cmt, i+1, all_means[i]);
	}
	free(all_means);

	e_comment(0, "%d valid frames found", nval);
	if (nval < minvalid1) {
		e_error("not enough frames to continue (min set to %d)", minvalid1);
		cube_del(i_cube);
		free(valid);
		return -1 ;
	}

	/* Reduce cube to valid frames only */
	if (nval < i_cube->np) {
		if (cube_reject_planes(&i_cube, valid)!=0) {
			e_error("rejecting planes from cube: aborting");
			cube_del(i_cube);
			free(valid);
			return -1 ;
		}
	}
	free(valid);

	/* Overscan correction */
	e_comment(0, "applying overscan correction to all frames");
	for (i=0 ; i<i_cube->np ; i++) {
		corrected = wfi_overscan_correction(i_cube->plane[i],
											prescan_x,
											overscan_x,
											rej_int,
											crop_reg);
		image_del(i_cube->plane[i]);
		i_cube->plane[i] = corrected ;

		if (corrected == NULL) {
			e_error("overscan correction failed for frame %d", i+1);
			cube_del(i_cube);
			return -1 ;
		}
	}
	/* Reset global cube size */
	i_cube->lx = i_cube->plane[0]->lx ;
	i_cube->ly = i_cube->plane[0]->ly ;

	/* Frame combination */
	e_comment(0, "frame stacking");
	stacked = cube_avg_median(i_cube);
	cube_del(i_cube);

	if (stacked==NULL) {
		e_error("in final combination: aborting");
		return -1 ;
	}

	e_comment(0, "saving master bias as [%s]", name_o);

	/* Load header from reference frame */

	if (is_fits_file(name_i)) {
		fh_ref = qfits_header_read(name_i);
	} else {
		first_name = framelist_firstname(name_i);
		if (first_name==NULL) {
			fh_ref=NULL ;
		} else {
			fh_ref = qfits_header_read(first_name);
		}
	}
	if (fh_ref==NULL) {
		e_error("cannot get FITS header from file [%s]", name_i);
	}

	/* Create default header */
	fh = qfits_header_default();
	/* BITPIX */
	sprintf(cval, "%d", BPP_DEFAULT);
	qfits_header_add(fh, "BITPIX", cval, "Bits per pixel", NULL);
	/* NAXIS* */
	qfits_header_add(fh, "NAXIS", "2", "Number of axes", NULL);
	sprintf(cval, "%d", stacked->lx);
	qfits_header_add(fh, "NAXIS1", cval, "size in X", NULL);
	sprintf(cval, "%d", stacked->ly);
	qfits_header_add(fh, "NAXIS2", cval, "size in Y", NULL);
	/* ORIGIN */
	qfits_header_add(fh, "ORIGIN", "ESO", "File originator", NULL);
	/* INSTRUME */
	qfits_header_add(fh, "INSTRUME", "WFI", "Instrument", NULL) ;
	/* TELESCOP */
	qfits_header_add(fh, "TELESCOP", "MPI-2.2", "Telescope", NULL);
	/* DATE-OBS */
	getval=NULL ;
	if (fh_ref!=NULL) {
		getval = qfits_header_getstr(fh_ref, "DATE-OBS");
	}
	if (getval==NULL) {
		qfits_header_add(fh, "DATE-OBS", "Unknown", "Date of observation", NULL);
	} else {
		qfits_header_add(fh, "DATE-OBS", getval, "Date of observation", NULL);
	}
	/* MJD-OBS */
	getval=NULL ;
	if (fh_ref!=NULL) {
		getval = qfits_header_getstr(fh_ref, "MJD-OBS");
	}
	if (getval==NULL) {
		qfits_header_add(fh, "MJD-OBS", "Unknown", "MJD start of 1st frame", NULL);
	} else {
		qfits_header_add(fh, "MJD-OBS", getval, "MJD start of 1st frame", NULL);
	}
	/* ECLIPSE */
	qfits_header_add(fh, "ECLIPSE",
					get_eclipse_version(), "Eclipse version", NULL);
	/* PRO.CATG */
	qfits_header_add(fh, "HIERARCH ESO PRO CATG", "MASTER_BIAS",
					"product frame type", NULL);
	/* REC.BIAS.KFRAME */
	sprintf(cval, "%g", kappa1);
	qfits_header_add(fh, "HIERARCH ESO REC BIAS KFRAME", cval,
					"frame rejection kappa", NULL);
	/* REC.BIAS.BIASMIN */
	sprintf(cval, "%d", minvalid1);
	qfits_header_add(fh, "HIERARCH ESO REC BIAS BIASMIN", cval,
					"Min number of frames", NULL);
	/* REC.BIAS.BIAS<n> */
	flist = framelist_load(name_i);
	if (flist==NULL) {
		qfits_header_add(fh, "HIERARCH ESO REC BIAS BIAS000",
						"Unknown", "Input frame", NULL);
	} else {
		for (i=0 ; i<flist->n ; i++) {
			sprintf(cval, "HIERARCH ESO REC BIAS BIAS%03d", i+1);
			qfits_header_add(fh,
							cval,
							get_basename(flist->name[i]),
							"Input frame",
							NULL);
		}
		framelist_del(flist);
	}
	/* REC.BIAS.PRSCX */
	sprintf(cval, "'%d %d'", prescan_x[0], prescan_x[1]);
	qfits_header_add(fh, "HIERARCH ESO REC BIAS PRSCX", cval,
					"Prescan xmin xmax", NULL);
	/* REC.BIAS.OVSCX */
	sprintf(cval, "'%d %d'", overscan_x[0], overscan_x[1]);
	qfits_header_add(fh, "HIERARCH ESO REC BIAS OVSCX", cval,
					"Overscan xmin xmax", NULL);
	/* REC.BIAS.RJOVSC */
	sprintf(cval, "'%d %d'", rej_int[0], rej_int[1]);
	qfits_header_add(fh, "HIERARCH ESO REC BIAS RJOVSC", cval,
					"Rejection min max", NULL);
	/* REC.BIAS.TRIM */
	sprintf(cval, "'%d %d %d %d'",
			crop_reg[0],
			crop_reg[1],
			crop_reg[2],
			crop_reg[3]) ;
	qfits_header_add(fh, "HIERARCH ESO REC BIAS TRIM", cval,
					"xmin xmax ymin ymax", NULL);
	/* REC.BIAS.RECVERS */
	qfits_header_add(fh, "HIERARCH ESO REC BIAS RECVERS",
					recipe_version, "Recipe version", NULL);

	/* Find out how many CCD chips */
	nchips = qfits_header_getint(fh_ref, "HIERARCH ESO DET CHIPS", -1);
	if (nchips<0) {
		nchips = WFI_NCHIPS ;
	}
	getval = NULL ;
	for (i=0 ; i<nchips ; i++) {
		sprintf(cval, "HIERARCH ESO DET CHIP%d ID", i+1);
		getval = qfits_header_getstr(fh_ref, cval);
		if (getval!=NULL)
			break ;
	}
	if (getval!=NULL) {
		qfits_header_add(fh, "HIERARCH ESO DET CHIP ID", getval,
						"Chip ID", NULL);
	} else {
		qfits_header_add(fh, "HIERARCH ESO DET CHIP ID", "Unknown",
						"Chip ID", NULL);
	}
	qfits_header_destroy(fh_ref);

	image_save_fits_hdrdump(stacked, name_o, fh, BPP_DEFAULT);
	qfits_header_destroy(fh);
	image_del(stacked);
	return 0 ;

}
