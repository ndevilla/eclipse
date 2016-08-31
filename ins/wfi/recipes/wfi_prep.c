
/*---------------------------------------------------------------------------
   
   File name 	:	wfi_prep.c
   Author 		:	N. Devillard
   Created on	:	Dec 2000
   Description	:	WFI pre-processing routine

 *--------------------------------------------------------------------------*/

/*
	$Id: wfi_prep.c,v 1.12 2001/10/22 11:57:29 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/22 11:57:29 $
	$Revision: 1.12 $
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

#define OPT_SATCHECK		1001
#define OPT_SATLEVEL		1002
#define OPT_SATMAX  		1003

#define OPT_BIASNAME		1005
#define OPT_FFNAME			1006

#define OPT_PRESCAN			1030
#define OPT_OVERSCAN        1031
#define OPT_SCANREJ         1032
#define OPT_TRIMMING        1033


#define WFIPREP_DEFAULTNAME	"wfiprep.fits"

static char recipe_version[] = "$Revision: 1.12 $";
static void usage(char *pname) ;
static char prog_desc[] = "WFI pre-processing stage" ;


typedef struct _wfiprep_bb_ {
	/* Input/Output file names */
	char	*	name_i ;
	char	**	frame_name ;
	int			np ;

	char	*	name_bias ;
	char	*	name_ff ;

	/* Saturation level threshold and max acceptable %age */
	int			sat_check ;
	double		sat_level ;
	double		sat_max ;

	/* Pre/Overscan parameters */
    int         prescan_x[2] ;  /* xmin xmax */
    int         overscan_x[2] ; /* xmin xmax */
    int         scanrej[2] ;    /* min max */
    int         trimreg[4] ;    /* xmin xmax ymin ymax */

} wfiprep_bb ;

wfiprep_bb * wfiprep_bb_new(void)
{
	wfiprep_bb	*	bb ;

	bb = calloc(1, sizeof(wfiprep_bb));
	
	/* Set default parameters for all */

	bb->sat_check = 1 ;
	bb->sat_level = WFI_SATLEVEL ;
	bb->sat_max   = WFI_SATMAX ;

    bb->prescan_x[0]    = WFI_PRESCAN_X_MIN ;
    bb->prescan_x[1]    = WFI_PRESCAN_X_MAX ;
    bb->overscan_x[0]   = WFI_OVERSCAN_X_MIN ;
    bb->overscan_x[1]   = WFI_OVERSCAN_X_MAX ;
    bb->scanrej[0]      = 10 ;
    bb->scanrej[1]      = 10 ;
 
    bb->trimreg[0]  = WFI_CROP_X_MIN ;
    bb->trimreg[1]  = WFI_CROP_X_MAX ;
    bb->trimreg[2]  = WFI_CROP_Y_MIN ;
    bb->trimreg[3]  = WFI_CROP_Y_MAX ;

	return bb ;
}

void wfiprep_bb_del(wfiprep_bb * bb)
{
	int		i ;

	if (bb==NULL) return ;

	if (bb->name_i) free(bb->name_i);
	if (bb->np > 0) {
		if (bb->frame_name!=NULL) {
			for (i=0 ; i<bb->np ; i++) {
				free(bb->frame_name[i]);
			}
			free(bb->frame_name);
		}
	}
	if (bb->name_bias) free(bb->name_bias);
	if (bb->name_ff) free(bb->name_ff);

	free(bb);
	return ;
}

void wfiprep_bb_dump(wfiprep_bb * bb, FILE * out)
{
	fprintf(out, 
"\n"
"Parameters for this command:\n"
"\n"
"[FileNames]\n"
"Input       = %s\n"
"Bias        = %s\n"
"FlatField   = %s\n" 
"\n"
"[SaturationCheck]\n"
"Activated   = %s\n"
"MaxLevel    = %g\n"
"Percentage  = %g\n"
"\n"
"[Prescan]\n"
"xmin        = %d\n"
"xmax        = %d\n"
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
	bb->name_i,
	bb->name_bias ? bb->name_bias : "none",
	bb->name_ff ? bb->name_ff : "none",

	bb->sat_check ? "yes" : "no",
	bb->sat_level,
	bb->sat_max,

    bb->prescan_x[0],
    bb->prescan_x[1],
    bb->overscan_x[0],
    bb->overscan_x[1],
    bb->scanrej[0],
    bb->scanrej[1],
    bb->trimreg[0],
    bb->trimreg[1],
    bb->trimreg[2],
    bb->trimreg[3]
	);
}


int wfiprep_saturation(cube_t * prep, wfiprep_bb * bb)
{
	int			i ;
	int			p ;
	int			npix ;
	int			limit ;

	limit = (int)(bb->sat_max * (prep->lx * prep->ly));
	for (p=0 ; p<prep->np ; p++) {
		if (prep->np>1) {
			compute_status("saturation count", p, prep->np, 2);
		}
		npix = 0 ;
		for (i=0 ; i<(prep->lx * prep->ly) ; i++) {
			if (prep->plane[p]->data[i] > (pixelvalue)bb->sat_level) {
				npix++ ;
			}
		}
		if (npix > limit) {
			e_error("frame %s has %d pixels above saturation (%g)",
					bb->frame_name[i],
					npix,
					bb->sat_level);
			return -1 ;
		}
	}
	return 0 ;
}

int wfiprep_save(cube_t * prep, wfiprep_bb * bb)
{
	char			name_o[FILENAMESZ];
	int				p ;
	qfits_header*	fh ;
	char			sval[80];


	/* Loop over planes in the input cube */
	for (p=0 ; p<prep->np ; p++) {
		/* Build output name */
		sprintf(name_o, "%s_pre.fits",
				get_rootname(get_basename(bb->frame_name[p])));
		e_comment(1, "saving [%s]", name_o);

		/* Read FITS header from input file */
		fh = qfits_header_read(bb->frame_name[p]);
		if (fh==NULL) {
			e_error("reading FITS header from [%s]", bb->frame_name[p]);
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
						"HIERARCH ESO REC PRERED RECVERS",
						recipe_version,
						"Recipe version",
						NULL);
		/* Add REC.PRERED.THRSAT */
		sprintf(sval, "%g", bb->sat_level);
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED THRSAT",
						sval,
						"saturation threshold",
						NULL);
		/* Add REC.PRERED MAXSATPIX */
		sprintf(sval, "%g", bb->sat_max);
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED MAXSATPIX",
						sval,
						"max % of sat pix",
						NULL);
		/* Add REC.PRERED.PRSCX */
		sprintf(sval, "'%d %d'", bb->prescan_x[0], bb->prescan_x[1]);
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED PRSCX",
						sval,
						"Prescan xmin xmax",
						NULL);
		/* Add REC.PRERED.OVSCX */
		sprintf(sval, "'%d %d'", bb->overscan_x[0], bb->overscan_x[1]);
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED OVSCX",
						sval,
						"Overscan xmin xmax",
						NULL);
		/* Add REC.PRERED.RJOVSC */
		sprintf(sval, "'%d %d'", bb->scanrej[0], bb->scanrej[1]);
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED RJOVSC",
						sval,
						"Rejection min max",
						NULL);
		/* Add REC.PRERED.TRIM */
		sprintf(sval, "'%d %d %d %d'",
				bb->trimreg[0],
				bb->trimreg[1],
				bb->trimreg[2],
				bb->trimreg[3]) ;
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED TRIM",
						sval,
						"xmin xmax ymin ymax",
						NULL);		
		/* Add REC.PRERED.MBIAS */
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED MBIAS",
						get_basename(bb->name_bias),
						"Bias used",
						NULL);
		/* Add REC.PRERED.MFLAT */
		qfits_header_add(fh,
						"HIERARCH ESO REC PRERED MFLAT",
						get_basename(bb->name_ff),
						"Flatfield used",
						NULL);

		image_save_fits_hdrdump(prep->plane[p],
									name_o,
									fh,
									BPP_DEFAULT);
		qfits_header_destroy(fh);
	}
	return 0 ;
}

#define ALGPARTS 6

int wfiprep_engine(wfiprep_bb * bb)
{
	cube_t		*	prep ;
	image_t		*	im ;
	framelist	*	flist ;
	int				i ;
	int				part ;

	e_comment(0, "--> START WFI preprocessing engine");
	part=0 ;

	/* Load input cube */
	part++ ;
	e_comment(0, "-> Part %d of %d: loading input data", part, ALGPARTS);
	prep = cube_load(bb->name_i);
	if (prep==NULL) {
		e_error("loading input data [%s]", bb->name_i);
		return -1 ;
	}

	/* Load frame names for error messages */
	if (is_fits_file(bb->name_i)==1) {
		bb->np = 1 ;
		bb->frame_name = malloc(sizeof(char*));
		bb->frame_name[0] = strdup(bb->name_i);
	} else {
		flist = framelist_load(bb->name_i);
		bb->np = flist->n ;
		bb->frame_name = malloc(flist->n * sizeof(char*));
		for (i=0 ; i<flist->n ; i++) {
			bb->frame_name[i] = strdup(flist->name[i]);
		}
		framelist_del(flist);
	}

	part++;
	e_comment(0, "-> Part %d of %d: checking saturation level", part, ALGPARTS);
	/* Saturation check if requested */
	if (bb->sat_check) {
		if (wfiprep_saturation(prep, bb)!=0) {
			e_error("in saturation check: aborting");
			cube_del(prep);
			return -1 ;
		}
	} else {
		e_comment(1, "skipped (on request)");
	}


	/* Overscan correction */
	part++;
	e_comment(0, "-> Part %d of %d: applying overscan/prescan/trimming",
				part, ALGPARTS);
	for (i=0 ; i<prep->np ; i++) {
		if (prep->np>1) {
			compute_status("correcting", i, prep->np, 1);
		}
		im = wfi_overscan_correction(prep->plane[i],
									 bb->prescan_x,
									 bb->overscan_x,
									 bb->scanrej,
									 bb->trimreg);
		if (im==NULL) {
			e_error("during overscan/prescan/trimming: aborting");
			cube_del(prep);
			return -1 ;
		}
		image_del(prep->plane[i]);
		prep->plane[i] = im ;
	}
	/* Cube size has changed, recompute sizes */
	prep->lx = prep->plane[0]->lx ;
	prep->ly = prep->plane[0]->ly ;

	/* Bias correction */
	part++;
	e_comment(0, "-> Part %d of %d: bias subtraction", part, ALGPARTS);
	im = image_load(bb->name_bias);
	if (im == NULL) {
		e_error("cannot load bias frame [%s]", bb->name_bias);
		cube_del(prep);
		return -1 ;
	}
	if (cube_sub_im(prep, im)!=0) {
		e_error("in bias subtraction: aborting");
		image_del(im);
		cube_del(prep);
		return -1 ;
	}
	image_del(im);

	/* Flat-field correction */
	part++;
	e_comment(0, "-> Part %d of %d: flatfield division", part, ALGPARTS);
	im = image_load(bb->name_ff);
	if (im == NULL) {
		e_error("cannot load flat-field frame [%s]", bb->name_ff);
		cube_del(prep);
		return -1 ;
	}
	if (cube_div_im(prep, im)!=0) {
		e_error("in flat-field division: aborting");
		image_del(im);
		cube_del(prep);
		return -1 ;
	}
	image_del(im);

	/* Save results */
	part++;
	e_comment(0, "-> Part %d of %d: saving results", part, ALGPARTS);
	if (wfiprep_save(prep, bb)!=0) {
		e_error("saving results");
	}
	cube_del(prep);

	e_comment(0, "--> STOP WFI preprocessing engine");
	return 0;
}


/*---------------------------------------------------------------------------
									Main	
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int             c ;
	int				display_params ;
	wfiprep_bb	*	bb ;
	int				status ;

	display_params = 0 ;
	bb = wfiprep_bb_new() ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 	0, 0, OPT_LICENSE},
            {"help",    	0, 0, OPT_HELP},
            {"version", 	0, 0, OPT_VERSION},

            {"bias",        1, 0, OPT_BIASNAME},
            {"flat",        1, 0, OPT_FFNAME},

			{"nosat",		0, 0, OPT_SATCHECK},
			{"satlevel",	1, 0, OPT_SATLEVEL},
			{"satmax",		1, 0, OPT_SATMAX},

            {"prescan",     1, 0, OPT_PRESCAN},
            {"overscan",    1, 0, OPT_OVERSCAN},
            {"scanrej",     1, 0, OPT_SCANREJ},
            {"trim",        1, 0, OPT_TRIMMING},
 
            {0, 0, 0, 0}
 
        } ;
        c = getopt_long(argc,
                        argv,
                        "Ldhx:",
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

        /* Local option: dump default values and exit */
            case 'd':
            display_params = 1 ;
            break ;

		/* Bias name */
            case OPT_BIASNAME:
            bb->name_bias = strdup(optarg);
            break ;

		/* Flat-field name */
			case OPT_FFNAME:
			bb->name_ff = strdup(optarg);
			break ;

		/* Saturation check */
			case OPT_SATCHECK:
			bb->sat_check = 0 ;
			break ;

		/* Saturation level */
			case OPT_SATLEVEL:
			bb->sat_level = atof(optarg);
			break ;

		/* Saturation percentage */
			case OPT_SATMAX:
			bb->sat_max = atof(optarg);
			if ((bb->sat_max<1e-4) || (bb->sat_max>(1.0-1e04))) {
				e_error("in --satmax option");
				e_error("invalid percentage: should be in [0..1]");
				wfiprep_bb_del(bb);
				return -1 ;
			}
			break ;

		/* Pre/Overscan + trimming */
            case OPT_PRESCAN:
            sscanf(optarg, "%d %d",
                    &(bb->prescan_x[0]),
                    &(bb->prescan_x[1]));
            break ;
 
            case OPT_OVERSCAN:
            sscanf(optarg, "%d %d",
                    &(bb->overscan_x[0]),
                    &(bb->overscan_x[1]));
            break ;
 
            case OPT_SCANREJ:
            sscanf(optarg, "%d %d",
                    &(bb->scanrej[0]),
                    &(bb->scanrej[1]));
            break ;
 
            case OPT_TRIMMING:
            sscanf(optarg, "%d %d %d %d",
                    &(bb->trimreg[0]),
                    &(bb->trimreg[1]),
                    &(bb->trimreg[2]),
                    &(bb->trimreg[3]));
            break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }
 
    /* Initialize eclipse environment */
    eclipse_init() ;

	/* Get input file name */
	if ((argc-optind)<1) {
		e_error("missing input file name as first argument");
		wfiprep_bb_del(bb);
		return -1 ;
	}
	bb->name_i = strdup(argv[optind]);
	optind ++ ;

	if (display_params) {
		wfiprep_bb_dump(bb, stdout);
	}

	/* Check input names */
	if (bb->name_bias==NULL) {
		e_warning("no input bias name given");
	}
	if (bb->name_ff==NULL) {
		e_warning("no input flatfield name given");
	}

	/* Startup WFI preprocessor engine */
	status = wfiprep_engine(bb);
	wfiprep_bb_del(bb);

    if (debug_active())
		xmemory_status() ;
    return status ;
} 



/*
 * This function only gives the usage for the program
 */
 
static void
usage(char *pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] <input>\n", pname) ;
    printf(
"options are:\n"
"\n"
"\t--bias <name>      Name of a bias to subtract\n"
"\t--flat <name>      Name of a flatfield to divide by\n"
"\n"
"\t--nosat            Deactivates saturation checks\n"
"\t--satlevel <val>   Saturation level in ADUs\n"
"\t--satmax  <pcent>  High percentage of saturated pixels\n"
"\n"
"\t--prescan 'xmin xmax'              Prescan region definition\n"
"\t--overscan 'xmin xmax'             Overscan region definition\n"
"\t--scanrej 'min max'                Scan rejection definition\n"
"\t--trim 'xmin xmax ymin ymax'       Trimming region definition\n"
"\n"
"\t-d                 Print out configuration parameters and run\n"
"\n\n");
    exit(1) ;
} 

