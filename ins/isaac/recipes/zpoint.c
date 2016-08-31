/*----------------------------------------------------------------------------*/
/**
   @file    zpoint.c
   @author  N. Devillard
   @date    February 2001
   @version	$Revision: 1.85 $
   @brief   ISAAC night zero points
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: zpoint.c,v 1.85 2005/09/15 12:14:35 yjung Exp $
	$Author: yjung $
	$Date: 2005/09/15 12:14:35 $
	$Revision: 1.85 $
*/

/*-----------------------------------------------------------------------------
							Includes
 -----------------------------------------------------------------------------*/

#include <ctype.h>
#include "eclipse.h"
#include "isaacp_lib.h"
#include "irstd.h"

/*-----------------------------------------------------------------------------
							Defines
 -----------------------------------------------------------------------------*/

#define OFFS_HEADER				1
#define OFFS_FILE				2

#define DEF_RADIUS_STAR			30.0
#define DEF_RADIUS_BGI			40.0
#define DEF_RADIUS_BGO			60.0

#define DEF_LOCATE_SX			20
#define DEF_LOCATE_SY			20

#define DEF_OUTPUTNAME			"stdstar"

#define ZP_NOCHOP               0
#define ZP_CHOP                 1

#define ZP_SW                   1
#define ZP_LW                   2

#define MAX_DET_VAL             32000

/*-----------------------------------------------------------------------------
                            New types
 -----------------------------------------------------------------------------*/

typedef struct _ZEROP_BB_ {

	/* Name of input frame list */
	char * name_i;

	/* Number of input frames */
	int		nframes ;

	/* Filtered list of input frames */
	char	**	input_list ;

	/* Number of planes to process (nframes for LW, 2(nframes-1) for SW) */
	int		np ;

    /* Calibration files are stored in the calib ASCII list */
    char    *   calib ;

    /* Calibration files */
	char        flatfield[FILENAMESZ] ;
    char        detlin_a[FILENAMESZ] ;
    char        detlin_b[FILENAMESZ] ;
    char        detlin_c[FILENAMESZ] ;

	/* Flag for chopped data */
	int		chopped ;

    /* Arm used for acquisition: ZP_SW or ZP_LW */
    int     acq_arm ;

	/* Filter name */
	char    *	filter_name ;
	/* ID of the filter used for observation */
	isaac_filter_id	filter_obs ;
	/* ID of the filter used for computation */
	isaac_filter_id	filter_comp ;

	double	dit ;

	/* Standard star definition */
	int		provided_star_pos ;
	double	star_ra ;
	double	star_dec ;

	double	star_mag ;
	char	star_name[ASCIILINESZ];
	char	star_sptype[ASCIILINESZ];
	int		star_temperature ;
	int     star_source ;


	/* Offset handling */
	/* Source of offsets: header or file */
	int		offset_source ;
	char  *	offset_file;

	/* Star location parameters */
	int		locate_sx ;
	int		locate_sy ;

	/* Frame offsets */
	double * dx ;
	double * dy ;

	/* List of star position in all frames */
	int	   * star_x ;
	int	   * star_y ;

	/* Optional check image output */
	int		 check_img ;

	/* Airmass parameter */
	char  ** airmass_start ;
	char  ** airmass_end ;

	/* MJD-OBS */
	int		 mjd_found ;
	char   ** mjd_obs ;

	/* Pixel scale */
	int		pixscale_found ;
	char	pixscale[ASCIILINESZ];

	/* Humidity level */
	int		 humidity_found ;
	double	 humidity_level ;

	/* Photometry computation radii */
	double	phot_obj_radius;
	double	phot_bgi_radius;
	double	phot_bgo_radius;

	/* Flux and background in all frames */
	double	*	flux ;
    double      flux_median ;
	double	*	background ;

	/* Computed FWHM in all frames */
	double	*	fwhm_x ;
	double	*	fwhm_y ;
	
	/* Output base name */
	char	* name_o;

} zeropoint_bb ;


/*-----------------------------------------------------------------------------
						Function prototypes
 -----------------------------------------------------------------------------*/

static zeropoint_bb * zeropoint_bb_new(void) ;
static void zeropoint_bb_del(zeropoint_bb *) ;
static int zp_engine(zeropoint_bb *) ;
static int zp_get_input(zeropoint_bb *) ;
static cube_t * zp_load(zeropoint_bb *) ;
static int zp_locate_star(zeropoint_bb *, cube_t *) ;
static int zp_compute(zeropoint_bb *, cube_t *) ;
static int zp_get_filter_and_starmag(zeropoint_bb *) ;
static int zp_output_results(zeropoint_bb *) ;

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
				Functions private to this module
 -----------------------------------------------------------------------------*/

/* ZP blackboard constructor */
static zeropoint_bb * zeropoint_bb_new(void)
{
	return calloc(1, sizeof(zeropoint_bb));
}

/* ZP blackboard destructor */
static void zeropoint_bb_del(zeropoint_bb * zpc)
{
	int		i ;
	
	if (zpc==NULL) return ;

	if (zpc->nframes>0) {
		for (i=0 ; i<zpc->nframes ; i++) {
			free(zpc->input_list[i]);
		}
		free(zpc->input_list);
	}

	if (zpc->dx != NULL) free(zpc->dx);
	if (zpc->dy != NULL) free(zpc->dy);

	if (zpc->star_x != NULL) free(zpc->star_x);
	if (zpc->star_y != NULL) free(zpc->star_y);

	if (zpc->airmass_start != NULL) {
		for (i=0 ; i<zpc->nframes ; i++) {
			if (zpc->airmass_start[i]!=NULL) {
				free(zpc->airmass_start[i]) ;
			}
		}
		free(zpc->airmass_start);
	}
	if (zpc->airmass_end != NULL) {
		for (i=0 ; i<zpc->nframes ; i++) {
			if (zpc->airmass_end[i]!=NULL) {
				free(zpc->airmass_end[i]) ;
			}
		}
		free(zpc->airmass_end);
	}

	if (zpc->mjd_obs!=NULL) {
		for (i=0 ; i<zpc->nframes ; i++) {
			if (zpc->mjd_obs[i] != NULL)
				free(zpc->mjd_obs[i]);
		}
		free(zpc->mjd_obs);
	}

    if (zpc->filter_name!=NULL) free(zpc->filter_name);
	if (zpc->flux != NULL) free(zpc->flux);
	if (zpc->background != NULL) free(zpc->background);

	if (zpc->fwhm_x != NULL) free(zpc->fwhm_x) ;
	if (zpc->fwhm_y != NULL) free(zpc->fwhm_y) ;

	free(zpc);
	return ;
}


/*-----------------------------------------------------------------------------
						    	Main 
 -----------------------------------------------------------------------------*/
int isaac_zpoint_main(void * dict)
{
	dictionary		*	d ;
	char			*	sval ;
	zeropoint_bb	*	zpc ;
	int					status ;

	d = (dictionary*)dict ;

	/* Initialize a blackboard structure */
	zpc = zeropoint_bb_new() ;

	/* Get input/output names */
	zpc->name_i = dictionary_get(d, "arg.1", NULL);
	if (zpc->name_i==NULL) {
		e_error("missing input file name");
        zeropoint_bb_del(zpc);
		return -1 ;
	}
	zpc->name_o = dictionary_get(d, "arg.output", NULL);
	if (zpc->name_o==NULL) {
		zpc->name_o = DEF_OUTPUTNAME ;
	}
    
    /* Get calibration files list */
    zpc->calib = dictionary_get(d, "arg.calib", NULL);
    
	/* Get ref star position */
	sval = dictionary_get(d, "arg.star", NULL);
	if (sval==NULL) {
		zpc->provided_star_pos = 0 ;
	} else {
		zpc->provided_star_pos = 1 ;
		if (sscanf(sval, "%lg %lg", &(zpc->star_ra), &(zpc->star_dec))!=2) {
			e_error("in -s/--star: expected two values");
			zeropoint_bb_del(zpc);
			return -1 ;
		}
	}

	/* Get filter name */
	sval = dictionary_get(d, "arg.filter", NULL);
    if (sval!=NULL) {
        zpc->filter_name = strdup(sval) ;
    } else {
        zpc->filter_name = NULL ;
    }

	/* Get magnitude */
	zpc->star_mag = dictionary_getdouble(d, "arg.mag", 99.0);
	/* Get search size */
	sval = dictionary_get(d, "arg.locate", NULL);
	if (sval!=NULL) {
		if (sscanf(sval, "%d %d", &(zpc->locate_sx), &(zpc->locate_sy))!=2) {
			e_error("in -l/--locate: expected two values");
			zeropoint_bb_del(zpc);
			return -1 ;
		}
	} else {
		zpc->locate_sx = DEF_LOCATE_SX ;
		zpc->locate_sy = DEF_LOCATE_SY ;
	}
	/* Get photometry radiuses */
	sval = dictionary_get(d, "arg.radius", NULL);
	if (sval!=NULL) {
		if (sscanf(sval, "%lg %lg %lg",
					&(zpc->phot_obj_radius),
					&(zpc->phot_bgi_radius),
					&(zpc->phot_bgo_radius)) != 3) {
			e_error("in -r/--radius: expected three values");
			zeropoint_bb_del(zpc);
			return -1 ;
		}
	} else {
		zpc->phot_obj_radius = DEF_RADIUS_STAR ;
		zpc->phot_bgi_radius = DEF_RADIUS_BGI ;
		zpc->phot_bgo_radius = DEF_RADIUS_BGO ;
	}
	/* Get check image flag */
	zpc->check_img = dictionary_getint(d, "arg.check", 0);

	/* Get DIT value */
	zpc->dit = dictionary_getdouble(d, "arg.dit", -1.0);

	/* Get offset file name */
	zpc->offset_file = dictionary_get(d, "arg.offset", NULL);
	if (zpc->offset_file!=NULL) {
		zpc->offset_source = OFFS_FILE ;
	} else {
		zpc->offset_source = OFFS_HEADER ;
	}

    /* Get chop/nochop flag */
    sval = dictionary_get(d, "arg.type", NULL);
    if (sval==NULL) {
        zpc->chopped = -1 ;
    } else if (!strcmp(sval, "nochop")) {
        zpc->chopped = ZP_NOCHOP ;
    } else if (!strcmp(sval, "chop")) {
        zpc->chopped = ZP_CHOP ;
    } else {
        e_error("in -t/--type: expected chop or nochop");
        zeropoint_bb_del(zpc);
        return -1 ;
    }

    /* Get instrument data type */
    INSID = pfits_identify_insstr("isaac");
    
	status = zp_engine(zpc);

	/* Deallocate blackboard */
	zeropoint_bb_del(zpc);

    return status ;
}


/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/
static int zp_engine(zeropoint_bb * zpc)
{
	cube_t			*	zp_cube ;
	int					p ;
	int					np=6 ;

    e_comment(0, "--> START zero point engine");
	p=0 ;

    /* Load program configuration file */
	p++ ;
    e_comment(1, "part %d of %d: getting input information", p, np);
	if (zp_get_input(zpc)!=0) {
        e_error("getting input information: aborting");
        return -1 ;
    }

	/* Getting standard star information from database */
	p++ ;
	e_comment(1, "part %d of %d: getting star info from database",p,np);
	if (zp_get_filter_and_starmag(zpc)!=0) {
		e_error("gathering standard star information: aborting");
		return -1 ;
	}

	/* Load input cube */
	p++ ;
	e_comment(1, "part %d of %d: loading frames", p, np);
	zp_cube = zp_load(zpc);
	if (zp_cube==NULL) {
		e_error("loading input list of frames: [%s]", zpc->name_i);
		return -1 ;
	}

	/* Locate standard star in all frames */
	p++ ;
	e_comment(1, "part %d of %d: locating standard star", p, np);
	if (zp_locate_star(zpc, zp_cube)!=0) {
		e_error("in standard star location: aborting");
		cube_del(zp_cube);
		return -1 ;
	}

    /* Compute photometry */
	p++ ;
    e_comment(1, "part %d of %d: photometry computation", p, np) ;
    if (zp_compute(zpc, zp_cube)!=0) {
        e_error("during photometry computation");
        return -1 ;
    }

	/* Free cube: not needed anymore */
	cube_del(zp_cube);

	/* Output data */
	p++ ;
	e_comment(1, "part %d of %d: building data output", p, np);
	if (zp_output_results(zpc)!=0) {
		e_error("during data output");
		return -1;
	}
	e_comment(0, "--> STOP zero point engine");
	return 0 ;
}

static int zp_get_input(zeropoint_bb * zpc)
{
	framelist	*	flist ;
	int				i ;
	char		*	sval ;
	double			dval ;
	double3		*	offsets ;
	int				nrhum ;
	double			rhum ;
    char        *   flat_type ;
    char        *   detlina_type ;
    char        *   detlinb_type ;
    char        *   detlinc_type ;

	/* Check input values */
	if (zpc==NULL) return -1 ;

	/* Load list of frame names */
	flist = framelist_load(zpc->name_i);
	if (flist==NULL) {
		e_error("loading list [%s]", zpc->name_i);
		return -1 ;
	}

    /* Identify chopped/non-chopped data if needed */
    if (zpc->chopped < 0) {
        /* Get template ID */
        sval = pfits_get(INSID, flist->name[0], "templateid");
        if (sval!=NULL) {
            /* Switch on known template IDs */
            if (!strcmp(sval, "ISAACSW_img_tec_Zp")) {
                zpc->chopped = ZP_NOCHOP ;
            } else if (!strcmp(sval, "ISAACLW_img_tec_Zp")) {
                zpc->chopped = ZP_CHOP ;
            } else if (!strcmp(sval, "ISAACLW_img_tec_ZpNoChop")) {
                zpc->chopped = ZP_NOCHOP ;
            } else if (!strcmp(sval, "ISAACSW_img_cal_GenericOffset")) {
                zpc->chopped = ZP_NOCHOP ;
            }
        }
        if (zpc->chopped < 0) {
            /* No known template ID: try out chopping status */
            sval = pfits_get(INSID, flist->name[0], "chopping_status");
            if (sval!=NULL) {
                if (sval[0]=='T') {
                    zpc->chopped = ZP_CHOP ;
                } else if (sval[0]=='F') {
                    zpc->chopped = ZP_NOCHOP ;
                }
            }
        }
    }
    /* Check out what was found */
    if (zpc->chopped<0) {
        e_error("cannot find chopping mode: use -t/--type option");
        framelist_del(flist);
        return -1 ;
    }
	/* Get acquisition arm */
    sval = pfits_get(INSID, flist->name[0], "arm");
    if (sval==NULL) {
        e_error("cannot determine SW/LW arm for %s", flist->name[0]);
        return -1 ;
    }
	switch (toupper(sval[0])) {
		case 'S': zpc->acq_arm = ZP_SW ; break ;
		case 'L': zpc->acq_arm = ZP_LW ; break ;
		default: break ;
	}

	/* Filter list for LW mode */
	isaac_lw_filter_halfcycle(&flist);
	if (flist->n<1) {
		e_error("only half-cycle frames in input list: aborting");
		return -1 ;
	}

	zpc->nframes = flist->n ;

	/* Store this filtered framelist into blackboard */
	zpc->input_list = malloc(flist->n * sizeof(char*)) ;
	for (i=0 ; i<flist->n ; i++) {
		zpc->input_list[i] = strdup(flist->name[i]);
	}

	/* Load plane by plane relevant FITS informations */

	/* DIT */
	if (zpc->dit<0.0) {
		e_comment(2, "getting DIT from first frame");
		sval = pfits_get(INSID, flist->name[0], "dit");
		if (sval==NULL) {
			e_error("cannot get DIT from first frame");
			e_error("please provide a value through -d/--dit option");
			framelist_del(flist);
			return -1 ;
		}
		dval = (double)atof(sval);
		if (dval<0.0) {
			e_error("getting DIT value from first frame");
			e_error("retrieved [%g] expected positive value", dval);
			framelist_del(flist);
			return -1 ;
		}
		zpc->dit = dval ;
	}

	/* Star position */
	if (zpc->provided_star_pos) {
		e_comment(2, "using provided star position:");
		e_comment(2, "RA  = %g (degrees)", zpc->star_ra);
		e_comment(2, "Dec = %g (degrees)", zpc->star_dec);
	} else {
		sval = pfits_get(INSID, flist->name[0], "ra");
		if (sval==NULL) {
			e_error("no value found in header for RA: aborting");
			e_error("please provide a value through -s/--star option");
			framelist_del(flist);
			return -1 ;
		}
		zpc->star_ra = (double)atof(sval);
		sval = pfits_get(INSID, flist->name[0], "dec") ;
		if (sval==NULL) {
			e_error("no value found in header for DEC: aborting");
			e_error("please provide a value through -s/--star option");
			framelist_del(flist);
			return -1 ;
		}
		zpc->star_dec = (double)atof(sval);
		e_comment(2, "using star position found in header:");
		e_comment(2, "RA  = %g (degrees)", zpc->star_ra);
		e_comment(2, "Dec = %g (degrees)", zpc->star_dec);
	}
	
	/* Offset information */
	zpc->dx = malloc(flist->n * sizeof(double));
	zpc->dy = malloc(flist->n * sizeof(double));

	switch (zpc->offset_source) {

		case OFFS_HEADER:
		e_comment(2, "acquiring offsets from FITS headers");
		for (i=0 ; i<flist->n ; i++) {
			sval = pfits_get(INSID, flist->name[i], "cumoffsetx");
			if (sval==NULL) {
				e_error("cannot retrieve X offset from file [%s]",
						flist->name[i]);
				e_error("provide offset input using -o/--offset option");
				framelist_del(flist);
				return -1 ;
			}
			zpc->dx[i] = (double)atof(sval);

			sval = pfits_get(INSID, flist->name[i], "cumoffsety") ;
			if (sval==NULL) {
				e_error("cannot retrieve Y offset from file [%s]",
						flist->name[i]);
				e_error("provide offset input using -o/--offset option");
				framelist_del(flist);
				return -1 ;
			}
			zpc->dy[i] = (double)atof(sval);
		}

		/* Make offsets relative to first frame */
		for (i=1 ; i<flist->n ; i++) {
			zpc->dx[i] -= zpc->dx[0];
			zpc->dy[i] -= zpc->dy[0];
		}
		zpc->dx[0] = 0.0 ;
		zpc->dy[0] = 0.0 ;
		break;

		case OFFS_FILE:
		e_comment(2, "acquiring offsets from file [%s]", zpc->offset_file);
		offsets = load_offsets_from_txtfile(zpc->offset_file);
		if (offsets==NULL) {
			e_error("reading offset file [%s]: aborting", zpc->offset_file);
			framelist_del(flist);
			return -1 ;
		}
		for (i=0 ; i<flist->n ; i++) {
			zpc->dx[i] = offsets->x[i] ;
			zpc->dy[i] = offsets->y[i] ;
		}
		double3_del(offsets);
		break ;

		default:
		break ;
	}

	/* Airmass, MJD-OBS, humidity level */
	zpc->airmass_start = malloc(flist->n * sizeof(char*));
	zpc->airmass_end   = malloc(flist->n * sizeof(char*));
	zpc->mjd_obs       = malloc(flist->n * sizeof(char*));

	nrhum = 0 ;
	rhum  = 0.0 ;

	for (i=0 ; i<flist->n ; i++) {
        zpc->airmass_start[i]=
			strdup(pfits_get(INSID, flist->name[i], "airmass_start"));
        zpc->airmass_end[i]=
			strdup(pfits_get(INSID, flist->name[i], "airmass_end"));
 
        sval = pfits_get(INSID, flist->name[i], "mjdobs");
        if (sval!=NULL) {
            zpc->mjd_found = 1 ;
            zpc->mjd_obs[i] = strdup(sval);
        } else {
            zpc->mjd_obs[i] = strdup("--");
        }
 
        sval = pfits_get(INSID, flist->name[i], "humidity_level");
        if (sval!=NULL) {
			zpc->humidity_found = 1 ;
            rhum += (double)atof(sval) ;
            nrhum ++ ;
        }
    }
 
    if (nrhum>0) {
        rhum /= (double)nrhum ;
        zpc->humidity_level = rhum ;
        e_comment(2, "found average humidity level: %g", rhum);
    }

	/* Get observation filter */
	if (zpc->filter_name==NULL) {
		sval = pfits_get(INSID, flist->name[0], "filter");
    } else {
        sval = zpc->filter_name ;
    }
    /* Associate to filter label */
    zpc->filter_obs = isaac_get_filterid(sval) ;
    if (zpc->filter_obs==isaac_filter_invalid) {
        e_error("invalid filter: %s", sval);
        framelist_del(flist);
        return -1 ;
    }
	e_comment(2, "observation filter: [%s]",
				isaac_get_filtername(zpc->filter_obs));

	/* Get pixel scale */
	sval = pfits_get(INSID, flist->name[0], "pixscale") ;
	if (sval != NULL) {
		zpc->pixscale_found = 1 ;
		strcpy(zpc->pixscale, sval);
	} else {
		zpc->pixscale_found = 0 ;
	}
	framelist_del(flist);

    /* Get calibration files */
    strcpy(zpc->flatfield, "none") ;
    strcpy(zpc->detlin_a, "none") ;
    strcpy(zpc->detlin_b, "none") ;
    strcpy(zpc->detlin_c, "none") ;
    
    if (zpc->calib != NULL) {
        if (strcmp(zpc->calib, "?")) {
            /* Load the ascii list */
            flist = framelist_load(zpc->calib) ;
            if (flist==NULL) {
                e_error("cannot load calib file %s: aborting", zpc->calib);
                return -1 ;
            }
            /* Are there defined file types in second column? */
            if (flist->type == NULL) {
                e_error("no frame type defined in list %s", zpc->calib);
                framelist_del(flist);
                return -1 ;
            }
            /* Get the calibration frames expected types (from DO_CATG) */
            flat_type = pfits_getdocat_value(INSID, docat_imag_flat) ;
            detlina_type = pfits_getdocat_value(INSID, 
                    docat_imag_detlin_coeff_A);
            detlinb_type = pfits_getdocat_value(INSID, 
                    docat_imag_detlin_coeff_B);
            detlinc_type = pfits_getdocat_value(INSID, 
                    docat_imag_detlin_coeff_C);
            
            for (i=0 ; i<flist->n ; i++) {
                /* Retrieve the flatfield */
                if (flat_type != NULL) {
                    if (!strcmp(flist->type[i], strlwc(flat_type))) {
                        strcpy(zpc->flatfield, flist->name[i]) ;
                    } else if (strstr(flist->type[i], "flat") != NULL) {
                        e_warning("%s should be used instead of %s in 2nd col.",
                                flat_type, flist->type[i]) ;
                        strcpy(zpc->flatfield, flist->name[i]) ;
                    }
                }
                /* Retrieve the detlin A coefficients */
                if (detlina_type != NULL) {
                    if (!strcmp(flist->type[i], strlwc(detlina_type))) {
                        strcpy(zpc->detlin_a, flist->name[i]) ;
                    } else if (strstr(flist->type[i], "detlin_a") != NULL) {
                        e_warning("%s should be used instead of %s in 2nd col.",
                                detlina_type, flist->type[i]) ;
                        strcpy(zpc->detlin_a, flist->name[i]) ;
                    }
                }
                /* Retrieve the detlin B coefficients */
                if (detlinb_type != NULL) {
                    if (!strcmp(flist->type[i], strlwc(detlinb_type))) {
                        strcpy(zpc->detlin_b, flist->name[i]) ;
                    } else if (strstr(flist->type[i], "detlin_b") != NULL) {
                        e_warning("%s should be used instead of %s in 2nd col.",
                                detlinb_type, flist->type[i]) ;
                        strcpy(zpc->detlin_b, flist->name[i]) ;
                    }
                }
                /* Retrieve the detlin C coefficients */
                if (detlinc_type != NULL) {
                    if (!strcmp(flist->type[i], strlwc(detlinc_type))) {
                        strcpy(zpc->detlin_c, flist->name[i]) ;
                    } else if (strstr(flist->type[i], "detlin_c") != NULL) {
                        e_warning("%s should be used instead of %s in 2nd col.",
                                detlinc_type, flist->type[i]) ;
                        strcpy(zpc->detlin_c, flist->name[i]) ;
                    }
                }
            }
            framelist_del(flist);
        }
    }
    
	return 0 ;
}

static cube_t * zp_load(zeropoint_bb * zpc)
{
    cube_t      *   zp_cube ;
	cube_t	    *	i_cube ;
	image_t     *	p1,
			    *	p2 ;
    image_t     *   detlin_a ;
    image_t     *   detlin_b ;
    image_t     *   detlin_c ;
	image_t     *	flat ;
	double	    *	ofx, 
			    *	ofy ;
	framelist   *	flist ;
	char	    *	lwc ;
	int			    i, np, ip ;

	/* Load flat-field */
	flat=NULL ;
	if (strcmp(zpc->flatfield, "none")) {
        /* Input flat field is a FITS frame: load it */
        flat = image_load(zpc->flatfield);
        if (flat==NULL) {
            e_error("cannot load flat-field frame %s", zpc->flatfield);
            return NULL ;
        }
        e_comment(2, "loaded flat-field [%s]", zpc->flatfield);
    }

    /* Create framelist with zpc->input_list */
    flist = calloc(1, sizeof(framelist));
    flist->name = zpc->input_list ;
    flist->n    = zpc->nframes ;
    
	/* Load cube from input_list */
    if ((i_cube = isaac_loadcube(flist)) == NULL) {
		e_error("cannot load cube from [%s]", zpc->name_i);
		if (flat!=NULL) image_del(flat);
        free(flist);
		return NULL ;
	}
    free(flist);

    /* Correct the non-linearity if coefficients provided */
    if ((strcmp(zpc->detlin_a, "none")) 
            && (strcmp(zpc->detlin_b, "none")) 
            && (strcmp(zpc->detlin_c, "none"))) {
        detlin_a = image_loadext(zpc->detlin_a, 0, 0, 1) ;
        detlin_b = image_loadext(zpc->detlin_b, 0, 0, 1) ;
        detlin_c = image_loadext(zpc->detlin_c, 0, 0, 1) ;
        
        if ((detlin_a != NULL) && (detlin_b != NULL) && (detlin_c != NULL)) {
            e_comment(2, "correct the non-linearity on input frames") ;
            if (cube_correct_detlin(i_cube,detlin_a,detlin_b,detlin_c) == -1) {
                e_warning("cannot correct non-linearity on input frames") ;
            }
        }
        if (detlin_a != NULL) image_del(detlin_a) ;
        if (detlin_b != NULL) image_del(detlin_b) ;
        if (detlin_c != NULL) image_del(detlin_c) ;
    }
        
	/* Divide by flat-field if present */
	if (flat!=NULL) {
		e_comment(2, "dividing input cube by flatfield...");
		if (cube_div_im(i_cube, flat)!=0) {
			e_error("dividing input cube by flatfield");
			image_del(flat);
			cube_del(i_cube);
			return NULL ;
		}
		image_del(flat);
		flat = NULL ;
	}

	/* If data are chopped, nothing else needs to be done */
	if (zpc->chopped == ZP_CHOP) {
		e_comment(2, "data acquired in chop mode");
		/* As many measurements as there are input frames */
		zpc->np = zpc->nframes ;
		return i_cube ;
	}

	e_comment(2, "data acquired in nochop mode");
	/* Nochop mode: need to subtract frames by pairs */
	ip = i_cube->np ;
	np = 2*ip - 2 ;
	/* 2n-2 measurements in total */
	zpc->np = np ;
	zp_cube = cube_new(i_cube->lx, i_cube->ly, np);

	for (i=0 ; i<ip ; i++) {
		compute_status("computing difference frames...", i, i_cube->np, 2);

		if (i==0) {
			/* First plane has only one difference pair */
			p1 = i_cube->plane[0] ;
			p2 = i_cube->plane[1] ;
			zp_cube->plane[0] = image_sub(p1, p2);
			continue ;
		}
		
		if (i==(ip-1)) {
			/* Last plane has only one difference pair */
			p1 = i_cube->plane[ip-1];
			p2 = i_cube->plane[ip-2];
			zp_cube->plane[np-1] = image_sub(p1, p2);
			continue ;
		}
		
		/* General case: two pairs are generated per input frame */
		/* First pair */
		p1 = i_cube->plane[i];
		p2 = i_cube->plane[i-1];
		zp_cube->plane[2*i-1] = image_sub(p1, p2);

		/* Second pair */
		p1 = i_cube->plane[i] ;
		p2 = i_cube->plane[i+1] ;
		zp_cube->plane[2*i] = image_sub(p1, p2);
	}
	cube_del(i_cube);

	/* Re-compute offsets */
	ofx = malloc(np * sizeof(double));
	ofy = malloc(np * sizeof(double));
	
	/* First and last plane offsets */
	ofx[0] = zpc->dx[0];
	ofy[0] = zpc->dy[0];

	ofx[np-1] = zpc->dx[ip-1];
	ofy[np-1] = zpc->dy[ip-1];

	for (i=1 ; i<(ip-1) ; i++) {
		/* First pair */
		ofx[2*i-1] = zpc->dx[i];
		ofy[2*i-1] = zpc->dy[i];

		/* Second pair */
		ofx[2*i] = zpc->dx[i];
		ofy[2*i] = zpc->dy[i];
	}

	free(zpc->dx);
	free(zpc->dy);
	zpc->dx = ofx ;
	zpc->dy = ofy ;

	return zp_cube ;
}

static int zp_locate_star(zeropoint_bb * zpc, cube_t * zp_cube)
{
	detected	*	det ;
	int				i, j ;
	int				pos[2] ;
	int				edge_x, edge_y ;
	int				dx, dy ;
	double3		*	peaks ;
	double3 	*	tmpeaks ;
	int			*	valid_pk ;
	int				nvalid ;
	image_t 	*	check ;
    image_t     *   tmp_im ;
	char			check_name[FILENAMESZ];
	int				check_vigsz ;
	double			dist, 
					min_dist ;
	qfits_header*	fh ;
	framelist	*	raw ;
    int             init_x,
                    init_y ;
    pixelvalue      colour ;
	
	/* Run through all offsets to compute the edge constraints */
	edge_x = 0 ;
	edge_y = 0 ;
	for (i=0 ; i<zpc->np ; i++) {
		dx = (int)fabs(zpc->dx[i]+0.5);
		dy = (int)fabs(zpc->dy[i]+0.5);
		if (dx>edge_x) edge_x = dx ;
		if (dy>edge_y) edge_y = dy ;
	}
   

	edge_x = 0 ;
	edge_y = 0 ;
	/* Run an object detection on the first frame above 5 sigmas */
    det = detected_ks_engine(zp_cube->plane[0], 5.0, 0) ;
	
    /* If no star is found, try an alternative method */
    if ((det != NULL && det->nbobj==0) || (det == NULL)) {
        e_warning("cannot find any star - try to first remove outliers") ;
        /* Remove the negative values and outliers for object detection */
        if ((tmp_im = image_threshold(zp_cube->plane[0], 0.0, MAX_DET_VAL, 
                        0.0, 0.0)) == NULL) {
            e_error("cannot threshold the image") ;
            return -1 ;
        }
        if ((det = detected_ks_engine(tmp_im, 1.0, 0)) == NULL) {
            e_error("cannot find any star in first plane");
            image_del(tmp_im) ;
            return -1 ;
        }
        image_del(tmp_im) ;
	}
	if (det->nbobj==0) {
		e_error("cannot find any star in first plane");
		return -1 ;
	}
	peaks = detected2double3(det) ;
	detected_del(det) ;

	/* Identify candidates */
	valid_pk = malloc(peaks->n * sizeof(int));
	localize_xcorr_centers(peaks,
						   zp_cube->lx,
						   zp_cube->ly,
						   edge_x,
						   edge_y,
						   &nvalid,
						   valid_pk);
	if (nvalid<1) {
		e_error("no valid star found in input frames");
		e_error("there are indeed star objects in the frames");
		e_error("but none of them is seen in ALL frames");
		free(valid_pk);
		double3_del(peaks);
		return -1 ;
	}

	/* Filter out invalid stars */
	tmpeaks = double3_new(nvalid);
	j=0 ;
	for (i=0 ; i<peaks->n ; i++) {
		if (valid_pk[i]) {
			tmpeaks->x[j] = peaks->x[i] ;
			tmpeaks->y[j] = peaks->y[i] ;
			tmpeaks->z[j] = peaks->z[i] ;
			j++ ;
		}
	}
	double3_del(peaks);
	peaks = tmpeaks ;
	tmpeaks = NULL ;
	free(valid_pk);

    /* The assumed star center is just above (10 pix) the image center */
    init_x = zp_cube->lx/2 ;
    init_y = zp_cube->ly/2 + 10 ;

	min_dist = zp_cube->lx*zp_cube->lx + zp_cube->ly*zp_cube->ly ;
	for (i=0 ; i<peaks->n ; i++) {
		dist = (peaks->x[i]-init_x)*(peaks->x[i]-init_x) +
			   (peaks->y[i]-init_y)*(peaks->y[i]-init_y) ;
		if (dist<min_dist) {
			min_dist = dist ;
			pos[0] = (int)(peaks->x[i]+0.5);
			pos[1] = (int)(peaks->y[i]+0.5);
		}
	}
	double3_del(peaks);

	/* Store star position in all frames */
	zpc->star_x = malloc(zpc->np * sizeof(int));
	zpc->star_y = malloc(zpc->np * sizeof(int));
	for (i=0 ; i<zpc->np ; i++) {
		zpc->star_x[i] = pos[0] + (int)(zpc->dx[i]+0.5) ;
		zpc->star_y[i] = pos[1] + (int)(zpc->dy[i]+0.5) ;
	}
	
	/* Refine star positions */
	for (i=0 ; i<zpc->np ; i++) {
		image_locate_peak(zp_cube->plane[i],
						  zpc->star_x[i],
						  zpc->star_y[i],
						  zpc->locate_sx,
						  zpc->locate_sy,
						  &pos[0]);
		e_comment(2, "star[%02d] located [%03d %03d]",
					i+1, pos[0], pos[1]);
		zpc->star_x[i] = pos[0];
		zpc->star_y[i] = pos[1];
	}

	if (zpc->check_img) {
		check_vigsz = 2 * (int)zpc->phot_bgo_radius + 1 ;
		check = image_new(zpc->np * (2+check_vigsz), 2+check_vigsz) ;
        colour=100 ;
		for (i=0 ; i<zpc->np ; i++) {
			image_paste_vig_local( check,
								   zp_cube->plane[i],
								   i*(check_vigsz+2)+2,
								   2,
								   zpc->star_x[i] - (check_vigsz/2),
								   zpc->star_y[i] - (check_vigsz/2),
								   zpc->star_x[i] + (check_vigsz/2),
								   zpc->star_y[i] + (check_vigsz/2));
		}
        image_draw_circle(check,
                          2+check_vigsz/2,
                          check_vigsz/2,
                          zpc->phot_obj_radius,
                          colour);
        image_draw_circle(check,
                          2+check_vigsz/2,
                          check_vigsz/2,
                          zpc->phot_bgi_radius,
                          colour);
        image_draw_circle(check,
                          2+check_vigsz/2,
                          check_vigsz/2,
                          zpc->phot_bgo_radius,
                          colour);
		sprintf(check_name, "%s_check.fits", zpc->name_o);
		e_comment(1, "saving check image [%s]", check_name);
		fh = qfits_header_read(zpc->input_list[0]) ;
		isaac_header_for_image(fh) ;
		raw = framelist_load(zpc->name_i) ;
		if (isaac_pro_fits(fh,
						check_name,
                        "REDUCED",
                        NULL,
                        procat_imag_zpoint_result,
                        "OK",
                        "cal_zp",
                        zpc->np,
						raw,
						NULL) == -1) {
			e_error("unable to write the PRO keyword in the fits header") ;
			framelist_del(raw) ;
			return -1 ;
		}
		framelist_del(raw) ;
		/* Modify the target name */
		qfits_header_mod(fh, "HIERARCH ESO OBS TARG NAME",zpc->star_name, NULL);
		image_save_fits_hdrdump(check, check_name, fh, BPP_DEFAULT) ; 
		image_del(check);
		qfits_header_destroy(fh) ;
	}
	return 0 ;
}

static int zp_compute(zeropoint_bb * zpc, cube_t * zp_cube)
{
    int             i, np ;
	double		*	fwhm_point ;
	double			flux_lo ;
	double			flux_hi ;
	double			flux_mean ;
	double			flux_rms ;
	
    /* Compute photometry for all input planes */
	np = zpc->np ;
    zpc->flux = malloc(np * sizeof(double)) ;
    zpc->flux_median = -1.0 ;
    zpc->background = malloc(np * sizeof(double)) ;
	zpc->fwhm_x = malloc(np * sizeof(double)) ;
	zpc->fwhm_y = malloc(np * sizeof(double)) ;

    for (i=0 ; i<np ; i++) {
        compute_status("computing FWHM and photometry", i, np, 2);

		/* Get FWHM */
		fwhm_point = image_getfwhm(zp_cube->plane[i],
									   0,	/* threshold flag */
									   0,	/* threshold value */
									   zpc->star_x[i],
									   zpc->star_y[i],
									   1, 1	/* search domain */) ;
		zpc->fwhm_x[i] = fwhm_point[0] ;
		zpc->fwhm_y[i] = fwhm_point[1] ;
		free(fwhm_point) ;
		
        /* Compute background and flux in the first pair */
        zpc->background[i] =
			image_get_disk_background(zp_cube->plane[i],
									 zpc->star_x[i],
									 zpc->star_y[i],
									 zpc->phot_bgi_radius,
									 zpc->phot_bgo_radius,
									 BG_METHOD_MEDIAN) ;
        zpc->flux[i] =
			image_get_disk_flux(zp_cube->plane[i],
								zpc->star_x[i],
								zpc->star_y[i],
								zpc->phot_obj_radius,
								zpc->background[i]) ;
    }

	/* Get low, high, mean and rms for all fluxes */
	flux_lo = zpc->flux[0] ;
	flux_hi = zpc->flux[0] ;
	flux_mean = 0.0 ;
	for (i=0 ; i<np ; i++) {
		if (zpc->flux[i]<flux_lo) {
			flux_lo = zpc->flux[i] ;
		} else if (zpc->flux[i]>flux_hi) {
			flux_hi = zpc->flux[i] ;
		}
		flux_mean += zpc->flux[i];
	}
	flux_mean /= (double)np ;
	flux_rms  = 0.0 ;
	for (i=0 ; i<np ; i++) {
		flux_rms += (zpc->flux[i]-flux_mean)*(zpc->flux[i]-flux_mean);
	}
	flux_rms /= (double)np ;
	flux_rms = sqrt(flux_rms);
    zpc->flux_median = double_median(zpc->flux, np) ;

	/* Show results */
	e_comment(2, "flux measurements");
	e_comment(2, "low     : %g", flux_lo);
	e_comment(2, "high    : %g", flux_hi);
	e_comment(2, "average : %g", flux_mean);
	e_comment(2, "rms     : %g", flux_rms);
	e_comment(2, "median  : %g", zpc->flux_median);

    return 0 ;
}

static int double_sort(const void * d1, const void * d2)
{ if (*(double*)d1 > *(double*)d2) return 1 ; else return -1 ; }


static int zp_get_filter_and_starmag(zeropoint_bb * zpc)
{
    irstd       *   refstar ;
	double		    star_mag ;
    ir_waveband     band ;
	char	    *	cat ;
    int             i ;

	strcpy(zpc->star_name, "unknown");
	strcpy(zpc->star_sptype, "unknown");
	zpc->star_temperature = -1 ;

	/* The star magnitude was user-provided */
	if (zpc->star_mag<(double)98.0) {
		e_comment(1, "using provided magnitude [%g] in band %s",
				zpc->star_mag,
				isaac_get_filtername(zpc->filter_obs));
		zpc->filter_comp = zpc->filter_obs ;
		return 0 ;
	}
	
	/* Getting the standard star means using a catalog name */
	e_comment(2, "getting standard star from database...");

	/* Get associated filter. */
	/* Locate a suitable broad-band filter */
	switch (isaac_associate_filter(zpc->filter_obs)) {
		case isaac_filter_z:
		case isaac_filter_sz:
		case isaac_filter_js:
		case isaac_filter_j:
		case isaac_filter_jblock:
            band = WAVEBAND_J ;
            zpc->filter_comp = isaac_filter_j;
            break ;
		case isaac_filter_sh:
		case isaac_filter_h:
            band = WAVEBAND_H ;
            zpc->filter_comp = isaac_filter_h;
            break ;
		case isaac_filter_ks:
            band = WAVEBAND_KS ;
            zpc->filter_comp = isaac_filter_ks;
            break ;
		case isaac_filter_sk:
		case isaac_filter_k:
            band = WAVEBAND_K ;
            zpc->filter_comp = isaac_filter_k;
            break ;
		case isaac_filter_sl:
		case isaac_filter_l:
            band = WAVEBAND_L ;
            zpc->filter_comp = isaac_filter_l;
            break ;
		case isaac_filter_mnb:
		case isaac_filter_m:
            band = WAVEBAND_M ;
            zpc->filter_comp = isaac_filter_m;
            break ;
		default:
            e_error("cannot determine associated broadband filter: aborting");
            zpc->filter_comp = isaac_filter_invalid ;
            return -1 ;
	}

    /* Get the star */
    switch (zpc->acq_arm) {
        /* SW mode */
        case ZP_SW:
            e_comment(3, "Try in LCO-Palomar") ;
            refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra, 
                    zpc->star_dec, band, "LCO-Palomar", &star_mag) ;
            if (refstar==NULL) {
                e_comment(3, "Try in LCO-Palomar-NICMOS-Red-Stars") ;
                refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra, 
                        zpc->star_dec, band, "LCO-Palomar-NICMOS-Red-Stars", 
                        &star_mag) ;
                if (refstar==NULL) {
                    e_comment(3, "Try in all catalogs") ;
                    refstar = irstd_get_star_magnitude(zpc->star_ra, 
                            zpc->star_dec, band, &star_mag) ;
                }
            }
            break ;
        /* LW mode  */
        case ZP_LW:
            e_comment(3, "Try in ESO-VanDerBliek") ;
            refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra, 
                    zpc->star_dec, band, "ESO-VanDerBliek", &star_mag) ;
            if (refstar==NULL) {
                e_comment(3, "Try in MSSSO-Photometric") ;
                refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra, 
                        zpc->star_dec, band, "MSSSO-Photometric", &star_mag) ;

                if (refstar==NULL) {
                    e_comment(3, "Try in MSSSO-Spectroscopic") ;
                    refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                            zpc->star_dec, band, "MSSSO-Spectroscopic",
                            &star_mag) ;
                    if (refstar==NULL) {
                        e_comment(3, "Try in all catalogs") ;
                        refstar = irstd_get_star_magnitude(zpc->star_ra, 
                                zpc->star_dec, band, &star_mag) ;
                    }
                }
            }
            break ;
        default:
            return -1 ;
    }
    
    /* Special case: swap K and Ks if needed */
    if ((refstar==NULL) && (band==WAVEBAND_K)) {
        refstar = irstd_get_star_magnitude(zpc->star_ra, zpc->star_dec,
                WAVEBAND_KS, &star_mag) ;
        if (refstar != NULL) zpc->filter_comp = isaac_filter_ks ;
    } else if ((refstar==NULL) && (band==WAVEBAND_KS)) {
        refstar = irstd_get_star_magnitude(zpc->star_ra, zpc->star_dec,
                WAVEBAND_K, &star_mag) ;
        if (refstar != NULL) zpc->filter_comp = isaac_filter_k ;
    }

	/* If the magnitude still is not know, abort */
	if (refstar == NULL) {
		e_error("star magnitude not found in database: aborting");
		return -1 ;
	}

	/* Store reference star in blackboard */
	strcpy(zpc->star_name, refstar->name);
	strcpy(zpc->star_sptype, refstar->sptype);
	zpc->star_temperature = irstd_get_star_temperature((char*)refstar->sptype);
	zpc->star_mag = star_mag ;
	zpc->star_source = refstar->source ;

	return 0 ;
}

static int zp_output_results(zeropoint_bb * zpc)
{
    int         i ;
    double  *   comp_mag ;
    double      avg_mag, rms_mag ;
    double      sqsum ;
	int			nfluxes_ok ;
	FILE	*	paf ;
	int			ra1, ra2, ra3 ;
	int			de1, de2, de3 ;
	char		sign ;
	char		out_name[FILENAMESZ];
	char	*	sval ;
	double		avg_airmass ;

    /* Create output PAF file */
	sprintf(out_name, "%s.paf", zpc->name_o);

	e_comment(2, "creating output PAF [%s]", out_name);
	paf = qfits_paf_print_header(out_name, "ISAAC/zero_point",
						   "Zero point computation results",
                           get_login_name(),
                           get_datetime_iso8601());
	if (paf==NULL) return -1 ;
	fprintf(paf, "\n");

    /* Add ARCFILE */
    sval = pfits_get(INSID, zpc->input_list[0], "arcfile") ;
    if (sval != NULL) {
        fprintf(paf, "ARCFILE   \"%s\"  \n", sval) ;
    }
    /* TPL.ID  */
    sval = pfits_get(INSID, zpc->input_list[0], "templateid") ;
    if (sval != NULL) {
        fprintf(paf, "TPL.ID  \"%s\" \n", sval) ;
    }
	/* Add PRO.CATG */
	fprintf(paf, "PRO.CATG              \"%s\" ;# Product category\n",
			pfits_getprokey(INSID, procat_imag_zpoint_qc)) ;

	/* Add the date */
	fprintf(paf, "DATE-OBS                  \"%s\" ;# Date\n",
			pfits_get(INSID, zpc->input_list[0], "date_obs")) ;

	/* Add the Airmass */
	fprintf(paf, "TEL.AIRM.START        \"%s\" ;# Airmass at start\n",
			pfits_get(INSID, zpc->input_list[0], "airmass_start")) ;

	/* MJD-OBS */
	if (zpc->mjd_found) {
		fprintf(paf, "MJD-OBS               %s ;# Obs start\n",
			zpc->mjd_obs[0]);
	} else {
		fprintf(paf, "MJD-OBS               0.0 ;# Obs start unknown\n");
	}

	/* INS.MODE */
	sval = pfits_get(INSID, zpc->input_list[0], "mode");
	fprintf(paf, "INS.MODE              \"%s\"\n", sval ? sval : "unknown");
	
	/* OBS.ID */
	sval = pfits_get(INSID, zpc->input_list[0], "obs_id");
	fprintf(paf, "OBS.ID                \"%s\"\n", sval ? sval : "unknown");
	
	fprintf(paf, "\n# Detector section\n");
	if (zpc->pixscale_found) {
		fprintf(paf, "INS.PIXSCALE          %s ;#"
					 "pixel scale in arcsec/pix\n",
				zpc->pixscale);
	} else {
		fprintf(paf, "INS.PIXSCALE          -1 #"
					 "pixel scale (not found)\n");
	}
	fprintf(paf, "DET.DIT               %g ;# DIT in seconds\n",zpc->dit);
	fprintf(paf, "\n");

	/* RA and Dec */
	fprintf(paf, "# Position as given in input\n");
	fprintf(paf, "RA                    %g ;# in degrees\n", zpc->star_ra);
	fprintf(paf, "DEC                   %g ;# in degrees\n", zpc->star_dec);
	fprintf(paf, "\n");

	/* List of input frames */
	fprintf(paf, "# Frame section\n");
	fprintf(paf, "# path: %s\n", get_dirname(zpc->input_list[0]));
	fprintf(paf, "# Name / Airmass Start / Airmass End / MJD-OBS\n");
	fprintf(paf, "# FRAMELIST.START\n");
    for (i=0 ; i < zpc->nframes ; i++) {
		fprintf(paf, "# %s\t%s\t%s\t%s\n",
				get_basename(zpc->input_list[i]),
				zpc->airmass_start[i],
				zpc->airmass_end[i],
				zpc->mjd_obs[i]);
    }
	fprintf(paf, "# FRAMELIST.END\n");

	ra_conv(zpc->star_ra, &ra1, &ra2, &ra3);
	dec_conv(zpc->star_dec, &sign, &de1, &de2, &de3);

	e_comment(2, "-> Standard star used");
	e_comment(2, "-> Name      : %s", zpc->star_name);
	e_comment(2, "-> RA        : %g (deg) / %02d:%02d:%02d",
			zpc->star_ra, ra1, ra2, ra3) ;
	e_comment(2, "-> DEC       : %g (deg) / %c%02d:%02d:%02d",
			zpc->star_dec, sign, de1, de2, de3);
	e_comment(2, "-> SpType    : %s", zpc->star_sptype);
	e_comment(2, "-> Temp (K)  : %d", zpc->star_temperature);
	e_comment(2, "-> Filter    : %s", isaac_get_filtername(zpc->filter_obs));
	if (zpc->filter_obs != zpc->filter_comp) {
		e_warning("different filter used for computation");
		e_warning("acquired in filter [%s]",
					isaac_get_filtername(zpc->filter_obs)) ;
		e_warning("computed with filter [%s]",
					isaac_get_filtername(zpc->filter_comp)) ;
	}
	e_comment(2, "-> Magnitude : %g", zpc->star_mag);

	fprintf(paf, "\n");
	fprintf(paf, "# Standard star section\n");
	fprintf(paf,
			"# Name         : %s\n"
			"# RA           :  %02d:%02d:%02d (%g)\n"
			"# Dec          : %c%02d:%02d:%02d (%g)\n"
			"# SpType       : %s\n"
			"# Magnitude    : %g\n"
			"# Band         : %s\n",
			zpc->star_name,
			ra1, ra2, ra3, zpc->star_ra,
			sign, de1, de2, de3, zpc->star_dec,
			zpc->star_sptype,
			zpc->star_mag,
			isaac_get_filtername(zpc->filter_comp));
	fprintf(paf, "\n") ;

	fprintf(paf, "# FLUX.DATA.START\n");
	fprintf(paf, "# flux\tbackground\tzeropoint\tfwhm_x\tfwhm_y\n");

	comp_mag = malloc(zpc->np * sizeof(double)) ;

    for (i=0 ; i<zpc->np ; i++) {
        if (zpc->flux[i]>0.0) {
			comp_mag[i] = 	zpc->star_mag +
							2.5 * log10(zpc->flux[i]) -
							2.5 * log10(zpc->dit);
			fprintf(paf, "# %8.1f\t%+4.1f\t%g\t\t%4.4g\t%4.4g\n",
					zpc->flux[i],
					zpc->background[i],
					comp_mag[i],
					zpc->fwhm_x[i],
					zpc->fwhm_y[i]) ;
        } else {
			comp_mag[i] = -1.0 ;
			fprintf(paf, "# %8.1f\t%+4.1f\t%g\t\t%4.4g\t%4.4g\n",
					zpc->flux[i],
					zpc->background[i],
					comp_mag[i],
					zpc->fwhm_x[i],
					zpc->fwhm_y[i]) ;
		}
    }
	fprintf(paf, "# FLUX.DATA.END\n");
    fprintf(paf, "\n\n");

	/*
	 * Give out average humidity level
	 */

	if (zpc->humidity_found) {
		fprintf(paf, "#\n");
		fprintf(paf, "# Average humidity level from ASM\n");
		fprintf(paf, "#\n");
		fprintf(paf, "\n");
		fprintf(paf, "QC.AMBI.RHUM.AVG      %g\n", zpc->humidity_level);
		fprintf(paf, "\n\n");
		e_comment(0, "Average humidity level: %g\n", zpc->humidity_level);
	}

	/* Compute average airmass during the observation */
	avg_airmass = (atof(zpc->airmass_start[0]) +
				   atof(zpc->airmass_end[zpc->nframes-1])) / 2.0 ;

    /*
     * Compute average computed magnitude and RMS
     * Forget about highest and lowest values
     */

	fprintf(paf, "# Zero point result section\n");
	fprintf(paf, "\n");

	avg_mag = 0.0 ;
	sqsum = 0.0 ;
	nfluxes_ok = 0 ;
	if (zpc->chopped == ZP_NOCHOP) {
		/* Reject highest and lowest value */
		qsort(comp_mag, zpc->np, sizeof(double), double_sort);
		for (i=1 ; i<(zpc->np-1) ; i++) {
			avg_mag += comp_mag[i] ;
			sqsum += comp_mag[i] * comp_mag[i] ;
			nfluxes_ok++ ;
		}
	} else {
		/* Keep all measurements */
		for (i=0 ; i<zpc->np ; i++) {
			avg_mag += comp_mag[i] ;
			sqsum += comp_mag[i] * comp_mag[i] ;
			nfluxes_ok++ ;
		}
	}
	if (nfluxes_ok<1) {
		e_error("no valid flux measurement: cannot compute ZP");
		fprintf(paf, "# Cannot compute zero point\n") ;
		fprintf(paf, "QC.ZPOINT             -1\n") ;
		fprintf(paf, "QC.ZPOINTRMS          -1\n") ;
		fprintf(paf, "QC.FILTER.OBS         \"%s\"\n",
				isaac_get_filtername(zpc->filter_obs)) ;
		fprintf(paf, "QC.STDNAME            \"unknown\"\n");
		fprintf(paf, "QC.CATNAME            \"unknown\"\n");
		fprintf(paf, "QC.AIRMASS            %g\n", avg_airmass);
	} else {
		avg_mag /= (double)(nfluxes_ok) ;
		sqsum   /= (double)(nfluxes_ok) ;
        /* Rounding errors can cause the variance to be negative */
        rms_mag = sqsum - avg_mag * avg_mag ;
        rms_mag = rms_mag > 0 ? sqrt(rms_mag) : 0 ;
		fprintf(paf, "QC.ZPOINT             %g\n", avg_mag) ;
		fprintf(paf, "QC.ZPOINTRMS          %g\n", rms_mag) ;
		fprintf(paf, "QC.FILTER.OBS         \"%s\"\n",
				isaac_get_filtername(zpc->filter_obs)) ;
		fprintf(paf, "QC.FILTER.REF         \"%s\"\n",
				isaac_get_filtername(zpc->filter_comp)) ;
		fprintf(paf, "QC.STDNAME            \"%s\"\n", zpc->star_name);
		fprintf(paf, "QC.CATNAME            \"%s\"\n", 
                irstd_catalog_name(zpc->star_source));
		fprintf(paf, "QC.AIRMASS            %g\n", avg_airmass);
        fprintf(paf, "QC.FLUX.MED           %g\n", zpc->flux_median) ;
		fprintf(paf, "\n\n# end of file\n");

		e_comment(0, "Computation results\n") ;
		e_comment(0, "ZeroPoint    = %g\n", avg_mag) ;
		e_comment(0, "ZeroPointRMS = %g\n", rms_mag) ;
	}
	free(comp_mag) ;

    fclose(paf) ;
    return 0 ;
}

