/*----------------------------------------------------------------------------*/
/**
   @file    zpoint.c
   @author  Y. Jung
   @date    January 2002
   @version	$Revision: 1.58 $
   @brief   CONICA night zero points (adapted from ISAAC)
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: zpoint.c,v 1.58 2004/02/19 16:38:14 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/19 16:38:14 $
	$Revision: 1.58 $
*/

/*-----------------------------------------------------------------------------
							Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "conicap_lib.h"
#include "irstd.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
							Defines
 -----------------------------------------------------------------------------*/

#define PRIMARY_UT4                     (8)
#define SECONDARY_UT4                   (1.1)
#define STREHL_BOX_SIZE                 (64)
#define STREHL_STAR_RADIUS              (2.0)
#define STREHL_BACKGROUND_R1            (2.0)
#define STREHL_BACKGROUND_R2            (3.0)

#define OFFS_HEADER				        1
#define OFFS_FILE				        2

#define DEF_RADIUS_STAR_ARCSEC          2.0
#define DEF_RADIUS_BGI_ARCSEC			2.1
#define DEF_RADIUS_BGO_ARCSEC			3.0

/* Default search size is larger for NACO than ISAAC (smaller FOV) */
#define DEF_LOCATE_SX		        	50
#define DEF_LOCATE_SY			        50

#define DEF_OUTPUTNAME			        "stdstar"

/*-----------------------------------------------------------------------------
                            New types
 -----------------------------------------------------------------------------*/
typedef struct _ZEROP_BB_ {

	/* Name of input frame list */
	char 		*	name_i;

	/* Number of input frames */
	int				nframes ;

	/* Filtered list of input frames */
	char		**	input_list ;

    /* Number of frames to process: 2*(nframes-1) */
    int             np ;

	/* Flat field name, or NULL if not present */
	char		*	flatfield ;

	/* Filter name */
    char        *   filter_name ;
	/* ID of the filter used for observation */
	conica_filter_id	filter_obs ;
	/* ID of the filter used for computation */
	conica_filter_id	filter_comp ;

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
	int			offset_source ;
	char 	*	offset_file;

	/* Star location parameters */
	int			locate_sx ;
	int			locate_sy ;

	/* Frame offsets */
	double 	* 	dx ;
	double 	* 	dy ;

	/* List of star position in all frames */
	int		*	star_x ;
	int		* 	star_y ;

    /* Strehl values */
    double  *   strehl ;
    double  *   strehl_error ;
    double  *   star_bg ;
    double  *   star_peak ;
    double  *   star_flux ;
    double  *   psf_peak ;
    double  *   psf_flux ;
    double  *   bg_noise ;
    
	/* Optional check image output */
	int		    check_img ;
    int         check_circle ;

	/* Airmass parameter */
	char	**	airmass_start ;
	char	**	airmass_end ;

	/* MJD-OBS */
	int			mjd_found ;
	char	**	mjd_obs ;

	/* Pixel scale */
	int			pixscale_found ;
	double		pixscale;

	/* Humidity level */
	int		 	humidity_found ;
	double	 	humidity_level ;

	/* Photometry computation radii */
	double		phot_obj_radius;
	double		phot_bgi_radius;
	double		phot_bgo_radius;

	/* Flux and background in all frames */
	double	*	flux ;
	double	*	background ;

	/* Computed FWHM in all frames */
	double	*	fwhm_x ;
	double	*	fwhm_y ;
	
	/* Output base name */
	char	*	name_o;

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
static int zp_compute_strehl(zeropoint_bb *, cube_t *) ;
static int zp_compute(zeropoint_bb *, cube_t *) ;
static int zp_get_filter_and_starmag(zeropoint_bb *) ;
static int zp_output_results(zeropoint_bb *) ;
static double zp_median_keyword(char **, int, char *) ;

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

    if (zpc->strehl != NULL) free(zpc->strehl);
    if (zpc->strehl_error != NULL) free(zpc->strehl_error);
    if (zpc->star_bg != NULL) free(zpc->star_bg);
    if (zpc->star_peak != NULL) free(zpc->star_peak);
    if (zpc->star_flux != NULL) free(zpc->star_flux);
    if (zpc->psf_peak != NULL) free(zpc->psf_peak);
    if (zpc->psf_flux != NULL) free(zpc->psf_flux);
    if (zpc->bg_noise != NULL) free(zpc->bg_noise);

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

	if (zpc->flux != NULL) free(zpc->flux);
	if (zpc->background != NULL) free(zpc->background);

	if (zpc->fwhm_x != NULL) free(zpc->fwhm_x) ;
	if (zpc->fwhm_y != NULL) free(zpc->fwhm_y) ;

	free(zpc);
	return ;
}


/*-----------------------------------------------------------------------------
							Main code
 -----------------------------------------------------------------------------*/
int conica_zpoint_main(void * dict)
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

	/* Get flat-field name */
	zpc->flatfield = dictionary_get(d, "arg.flat", NULL);

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
        zpc->filter_name = strdup(sval);
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
		zpc->phot_obj_radius = DEF_RADIUS_STAR_ARCSEC ;
		zpc->phot_bgi_radius = DEF_RADIUS_BGI_ARCSEC ;
		zpc->phot_bgo_radius = DEF_RADIUS_BGO_ARCSEC ;
	}
	/* Get check image flag */
	zpc->check_img = dictionary_getint(d, "arg.check", 0);

    /* Get circle flag */
    zpc->check_circle = dictionary_getint(d, "arg.circle", 0);

	/* Get DIT value */
	zpc->dit = dictionary_getdouble(d, "arg.dit", -1.0);

	/* Get offset file name */
	zpc->offset_file = dictionary_get(d, "arg.offset", NULL);
	if (zpc->offset_file!=NULL) zpc->offset_source = OFFS_FILE ;
	else 						zpc->offset_source = OFFS_HEADER ;

    /* Get pixel scale */
    sval = dictionary_get(d, "arg.pscale", NULL);
    if (sval!=NULL) {
        zpc->pixscale_found = 1;
        zpc->pixscale = (double)atof(sval);
    } else {
        zpc->pixscale_found = 0 ;
        zpc->pixscale = -1 ;
    }

    INSID = pfits_identify_insstr("naco");
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
	int					np=7 ;

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

    /* Compute the STREHL on the std star in each image */
    p++ ;
    e_comment(1, "part %d of %d: STREHL computation", p, np) ;
     if (zp_compute_strehl(zpc, zp_cube)!=0) {
         e_error("in strehl ratio computation: aborting") ;
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

	/* Check input values */
	if (zpc==NULL) return -1 ;

	/* Load list of frame names */
	flist = framelist_load(zpc->name_i);
	if (flist==NULL) {
		e_error("loading list [%s]", zpc->name_i);
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
		sval = pfits_get(INSID, flist->name[0], "dit") ;
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
		sval = pfits_get(INSID, flist->name[0], "ra") ;
		if (sval==NULL) {
			e_error("no value found in header for RA: aborting");
			e_error("please provide a value through -s/--star option");
			framelist_del(flist);
			return -1 ;
		}
		zpc->star_ra = (double)atof(sval);
		sval = pfits_get(INSID, flist->name[0], "dec");
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
			sval = pfits_get(INSID, flist->name[i], "cumoffsetx") ;
			if (sval==NULL) {
				e_error("cannot retrieve X offset from file [%s]",
						flist->name[i]);
				e_error("provide offset input using -o/--offset option");
				framelist_del(flist);
				return -1 ;
			}
			zpc->dx[i] = (double)atof(sval);

			sval = pfits_get(INSID, flist->name[i], "cumoffsety");
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
        sval = pfits_get(INSID, flist->name[i], "airmass_start") ;
        if (sval!=NULL) {
            zpc->airmass_start[i] = strdup(sval);
        } else {
            zpc->airmass_start[i] = strdup("--") ;
        }
        sval = pfits_get(INSID, flist->name[i], "airmass_end") ;
        if (sval!=NULL) {
            zpc->airmass_end[i] = strdup(sval);
        } else {
            zpc->airmass_end[i] = strdup("--") ;
        }
 
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
        e_comment(1, "found average humidity level: %g", rhum);
    }

	/* Get observation filter */
	if (zpc->filter_name==NULL) {
		sval = pfits_get(INSID, flist->name[0], "filter");
    } else {
        sval = zpc->filter_name ;
    }
    zpc->filter_obs = conica_get_filterid(sval) ;
    if (zpc->filter_obs==conica_filter_invalid) {
        e_error("invalid filter: %s", sval);
        framelist_del(flist);
        return -1 ;
    }
	e_comment(1, "observation filter: [%s]",
				conica_get_filtername(zpc->filter_obs));

	/* Get pixel scale */
    if (zpc->pixscale_found==0) {
        sval = pfits_get(INSID, flist->name[0], "pixscale") ;
        if (sval != NULL) {
            zpc->pixscale_found = 1 ;
            zpc->pixscale = (double)atof(sval);
            if (zpc->pixscale<0) {
                e_error("wrong pixel scale in header: %g", zpc->pixscale);
                framelist_del(flist);
                return -1 ;
            }
        } else {
            e_error("unknown pixel scale - cannot set radius") ;
            framelist_del(flist);
            return -1 ;
        }
    }
	
	/* Set flux and background radius */
    zpc->phot_obj_radius /= zpc->pixscale ;
    zpc->phot_bgi_radius /= zpc->pixscale ;
    zpc->phot_bgo_radius /= zpc->pixscale ;

	framelist_del(flist);
	return 0 ;
}

static cube_t * zp_load(zeropoint_bb * zpc)
{
	cube_t	  *	zp_cube ;
	cube_t	  *	i_cube ;
	image_t   *	p1,
			  *	p2 ;
	image_t   *	flat ;
	int			i, np, ip ;
	double	  *	ofx, 
			  *	ofy ;
	framelist *	flist ;
	char	  *	lwc ;

	/* Load flat-field */
	flat=NULL ;
	if (zpc->flatfield!=NULL) {
		if (is_fits_file(zpc->flatfield)) {
			/* Input flat field is a FITS frame: load it */
			flat = image_load(zpc->flatfield);
			if (flat==NULL) {
				e_error("cannot load flat-field frame %s: aborting",
						zpc->flatfield);
				return NULL ;
			}
			e_comment(0, "loaded flat-field %s", zpc->flatfield);

		} else if (is_ascii_list(zpc->flatfield)) {
			/* Input flat-field in an ASCII list: load list */
			flist = framelist_load(zpc->flatfield);
			if (flist==NULL) {
				e_error("cannot load flat-field name from %s: aborting",
						zpc->flatfield);
				return NULL ;
			}
			/* Are there defined file types in second column? */
			if (flist->type==NULL) {
				e_error("no frame type defined in list %s: aborting",
						zpc->flatfield);
				framelist_del(flist);
				return NULL ;
			}
			/* Try to locate a frame type containing 'flat'. */
			for (i=0 ; i<flist->n ; i++) {
				if (flist->type[i]!=NULL) {
					lwc = strlwc(flist->type[i]);
					if (strstr(lwc, "flat")!=NULL) {
						flat = image_load(flist->name[i]);
						if (flat==NULL) {
							e_error("loading flat-field frame %s",
									flist->name[i]);
						} else {
							e_comment(0, "loaded flat-field %s",
									flist->name[i]);
						}
						break ;
					}
				}
			}
			framelist_del(flist);
			if (flat==NULL) {
				e_error("no flatfield found in list %s: aborting",
						zpc->flatfield);
				return NULL ;
			}
		}
	}

	/* Load cube from input_list */
	i_cube = cube_load_strings(zpc->input_list, zpc->nframes);
	if (i_cube==NULL) {
		e_error("cannot load cube from [%s]", zpc->name_i);
		if (flat!=NULL)
			image_del(flat);
		return NULL ;
	}

	/* Divide by flat-field if present */
	if (flat!=NULL) {
		e_comment(0, "dividing input cube by flatfield...");
		if (cube_div_im(i_cube, flat)!=0) {
			e_error("dividing input cube by flatfield");
			image_del(flat);
			cube_del(i_cube);
			return NULL ;
		}
		image_del(flat);
		flat = NULL ;
	}

	/* Need to subtract frames by pairs */
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
	char			check_name[FILENAMESZ];
	int				check_vigsz ;
	double			dist, 
					min_dist ;
	qfits_header*	fh ;
	framelist	*	raw ;
    pixelvalue      colour ;
	
	/* Run through all offsets to compute the edge constraints */
	edge_x = 0 ;
	edge_y = 0 ;
	for (i=0 ; i<zpc->np ; i++) {
		dx = (int)fabs(zpc->dx[i]+0.5);
		dy = (int)fabs(zpc->dy[i]+0.5);
		if (dx>edge_x)
			edge_x = dx ;
		if (dy>edge_y)
			edge_y = dy ;

	}
	/* Run an object detection on the first frame above 5 sigmas */
	if ((det = detected_ks_engine(zp_cube->plane[0], 5.0, 0)) == NULL) {
		e_error("cannot find any star in first plane");
		return -1 ;
	}
    if (det->nbobj < 1) {
        e_error("cannot find any star in first plane");
        detected_del(det) ;
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

	/* Assume the standard star position is the closest to the center */
	min_dist = zp_cube->lx*zp_cube->lx + zp_cube->ly*zp_cube->ly ;
	for (i=0 ; i<peaks->n ; i++) {
		dist = (peaks->x[i]-zp_cube->lx/2)*(peaks->x[i]-zp_cube->lx/2) +
			   (peaks->y[i]-zp_cube->ly/2)*(peaks->y[i]-zp_cube->ly/2) ;
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
		check = image_new(zpc->np*(2+check_vigsz), 2+check_vigsz) ;
        colour = 100;
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
        if (zpc->check_circle) {
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
        }
		sprintf(check_name, "%s_check.fits", zpc->name_o);
		e_comment(1, "saving check image [%s]", check_name);
		fh = qfits_header_read(zpc->input_list[0]) ;
		conica_header_for_image(fh) ;
		raw = framelist_load(zpc->name_i) ;
		if (conica_pro_fits(fh,
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
		qfits_header_mod(fh, "HIERARCH ESO OBS TARG NAME", zpc->star_name,NULL);
		image_save_fits_hdrdump(check, check_name, fh, BPP_DEFAULT) ; 
		image_del(check);
		qfits_header_destroy(fh) ;
	}
	return 0 ;
}

static int zp_compute_strehl(zeropoint_bb * zpc, cube_t * zp_cube)
{
    strehl_parm             spar ;
    int                     i ;
    
    memset(&spar, 0, sizeof(strehl_parm)) ;
    spar.m1 = PRIMARY_UT4 ;
    spar.m2 = SECONDARY_UT4 ;
    conica_get_filterdef(zpc->filter_obs, &spar.l0, &spar.dl);
    spar.pscale = zpc->pixscale ;
    spar.size = STREHL_BOX_SIZE ;
    spar.star_radius = STREHL_STAR_RADIUS ;
    spar.psf_save = 0 ;
    spar.estim_bg = 1 ;
    spar.bg_radius1 = STREHL_BACKGROUND_R1 ;
    spar.bg_radius2 = STREHL_BACKGROUND_R2 ;

    /* Display the used parameters */
    e_comment(2, "Primary miror size:   %g\n", spar.m1) ;
    e_comment(2, "Secondary miror size: %g\n", spar.m2) ;
    e_comment(2, "l0 : %g\n", spar.l0) ;
    e_comment(2, "dl : %g\n", spar.dl) ;
    e_comment(2, "Pixel scale : %g\n", spar.pscale) ;
    e_comment(2, "Extraction image size : %d\n", spar.size) ;
    e_comment(2, "Save or not psf : %d\n", spar.psf_save) ;
    e_comment(2, "psf file name : %s\n", spar.psf_filename) ;
    e_comment(2, "Star radius : %g arcsec.\n", spar.star_radius) ;
    e_comment(2, "Background radii : %g and %g arcsec.\n", spar.bg_radius1, 
            spar.bg_radius2) ;

    /* Allocate the strehl array */
    zpc->strehl       = malloc(zp_cube->np * sizeof(double)) ;
    zpc->strehl_error = malloc(zp_cube->np * sizeof(double)) ;
    zpc->star_bg      = malloc(zp_cube->np * sizeof(double)) ;
    zpc->star_peak    = malloc(zp_cube->np * sizeof(double)) ;
    zpc->star_flux    = malloc(zp_cube->np * sizeof(double)) ;
    zpc->psf_peak     = malloc(zp_cube->np * sizeof(double)) ;
    zpc->psf_flux     = malloc(zp_cube->np * sizeof(double)) ;
    zpc->bg_noise     = malloc(zp_cube->np * sizeof(double)) ;
    
    /* Compute the strehl for each image */
    for (i=0 ; i<zp_cube->np ; i++) {
        /* Current image star position */
        spar.pos_x = zpc->star_x[i] ;
        spar.pos_y = zpc->star_y[i] ;
        /* Compute the strehl */
        if (image_compute_strehl(zp_cube->plane[i], &spar) == -1) {
            e_warning("cannot compute strehl for plane %d", i+1) ;
            zpc->strehl[i] = -1.0 ;
            zpc->strehl_error[i] = -1.0 ;
            zpc->star_bg[i] = -1.0 ;
            zpc->star_peak[i] = -1.0 ;
            zpc->star_flux[i] = -1.0 ;
            zpc->psf_peak[i] = -1.0 ;
            zpc->psf_flux[i] = -1.0 ;
            zpc->bg_noise[i] = -1.0 ;
        } else {
            zpc->strehl[i] = spar.strehl ;
            zpc->strehl_error[i] = spar.strehl_err ;
            zpc->star_bg[i] = spar.star_bg ;
            zpc->star_peak[i] = spar.star_peak ;
            zpc->star_flux[i] = spar.star_flux ;
            zpc->psf_peak[i] = spar.psf_peak ;
            zpc->psf_flux[i] = spar.psf_flux ;
            zpc->bg_noise[i] = spar.bg_noise ;
        }
        e_comment(2, "Star nb %d [%d %d] (bg: %g): strehl = %g (+/- %g)", i+1, 
            spar.pos_x, spar.pos_y, spar.star_bg, spar.strehl, spar.strehl_err);
    }
   	return 0 ;
}

static int zp_compute(zeropoint_bb * zpc, cube_t * zp_cube)
{
	double		*	fwhm_point ;
	double			flux_lo ;
	double			flux_hi ;
	double			flux_mean ;
	double			flux_rms ;
    int             i, np ;
	
    /*
     * Compute photometry for all input planes
     */
	np = zpc->np ;
    zpc->flux = malloc(np * sizeof(double)) ;
    zpc->background = malloc(np * sizeof(double)) ;
	zpc->fwhm_x = malloc(np * sizeof(double)) ;
	zpc->fwhm_y = malloc(np * sizeof(double)) ;

	e_comment(2, "Star radius:           %g\n", zpc->phot_obj_radius) ;
	e_comment(2, "Background int radius: %g\n", zpc->phot_bgi_radius) ;
	e_comment(2, "Background ext radius: %g\n", zpc->phot_bgo_radius) ;

    for (i=0 ; i<np ; i++) {
        compute_status("computing FWHM and photometry", i, np, 1);

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
        zpc->background[i] = image_get_disk_background(zp_cube->plane[i],
									 zpc->star_x[i],
									 zpc->star_y[i],
									 zpc->phot_bgi_radius,
									 zpc->phot_bgo_radius,
									 BG_METHOD_MEDIAN) ;
        zpc->flux[i] = image_get_disk_flux(zp_cube->plane[i],
									zpc->star_x[i],
									zpc->star_y[i],
									zpc->phot_obj_radius,
									zpc->background[i]) ;
    }

	/*
	 * Get low, high, mean and rms for all fluxes
	 */
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

	/* Show results */

	e_comment(2, "flux measurements");
	e_comment(2, "low     : %g", flux_lo);
	e_comment(2, "high    : %g", flux_hi);
	e_comment(2, "average : %g", flux_mean);
	e_comment(2, "rms     : %g", flux_rms);

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
				conica_get_filtername(zpc->filter_obs));
		zpc->filter_comp = zpc->filter_obs ;
		return 0 ;
	}
	
	/* Getting the standard star means using a catalog name. */
	e_comment(1, "getting standard star from database...");
	
	/* Get associated filter. */
    zpc->filter_comp = conica_associate_filter(zpc->filter_obs) ;

    /* Locate a suitable broad-band filter */
	switch (zpc->filter_comp) {
		case conica_filter_j:
		case conica_filter_jc:
            band = WAVEBAND_J ;
            break ;
		case conica_filter_h:
            band = WAVEBAND_H ;
            break ;
		case conica_filter_k:
            band = WAVEBAND_K ;
            break ;
		case conica_filter_ks:
            band = WAVEBAND_KS ;
            break ;
		case conica_filter_l:
		case conica_filter_lprime:
            band = WAVEBAND_L ;
            break ;
		case conica_filter_mprime:
            band = WAVEBAND_M ;
            break ;
		default:
            e_error("cannot determine associated broadband filter: aborting");
            zpc->filter_comp = conica_filter_invalid ;
            return -1 ;
    }

    /* Get the star */
    switch (zpc->filter_comp) {
        /* SW mode */
        case conica_filter_j:
        case conica_filter_jc:
        case conica_filter_h:
        case conica_filter_k:
        case conica_filter_ks:
            e_comment(2, "Try in LCO-Palomar") ;
            refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                    zpc->star_dec, band, "LCO-Palomar", &star_mag) ;
            if (refstar==NULL) {
                e_comment(2, "Try in LCO-Palomar-NICMOS-Red-Stars") ;
                refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                        zpc->star_dec, band, "LCO-Palomar-NICMOS-Red-Stars",
                        &star_mag) ;
                if (refstar==NULL) {
                    e_comment(2, "Try in ESO-VanDerBliek") ;
                    refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                            zpc->star_dec, band, "ESO-VanDerBliek", &star_mag) ;
                    if (refstar==NULL) {
                        e_comment(2, "Try in UKIRT-Extended") ;
                        refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                                zpc->star_dec, band, "UKIRT-Extended", 
                                &star_mag) ;
                        if (refstar==NULL) {
                            e_comment(2, "Try in UKIRT-Fundamental") ;
                            refstar = irstd_get_star_magnitude_one_cat(
                                    zpc->star_ra, zpc->star_dec, band,
                                    "UKIRT-Fundamental",  &star_mag) ;
                            if (refstar==NULL) {
                                e_comment(2, "Try in SAAO-Carter") ;
                                refstar = irstd_get_star_magnitude_one_cat(
                                        zpc->star_ra, zpc->star_dec, band,
                                        "SAAO-Carter",  &star_mag) ;
                                if (refstar==NULL) {
                                    e_comment(2, "Try in all catalogs") ;
                                    refstar = irstd_get_star_magnitude(
                                            zpc->star_ra, zpc->star_dec, band, 
                                            &star_mag) ;
                                }
                            }
                        }
                    }
                }
            }
            break ;
        /* LW mode  */
        case conica_filter_l:
        case conica_filter_lprime:
        case conica_filter_mprime:
            e_comment(2, "Try in ESO-VanDerBliek") ;
            refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                    zpc->star_dec, band, "ESO-VanDerBliek", &star_mag) ;
            if (refstar==NULL) {
                e_comment(2, "Try in UKIRT-Standards") ;
                refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                        zpc->star_dec, band, "UKIRT-Standards", &star_mag) ;
                if (refstar==NULL) {
                    e_comment(2, "Try in UKIRT-LM") ;
                    refstar = irstd_get_star_magnitude_one_cat(zpc->star_ra,
                            zpc->star_dec, band, "UKIRT-LM",&star_mag);
                    if (refstar==NULL) {
                        e_comment(2, "Try in all catalogs") ;
                        refstar = irstd_get_star_magnitude(zpc->star_ra,
                                zpc->star_dec, band, &star_mag) ;
                    }
                }
            }
            break ;
        default:
            e_error("cannot determine associated broadband filter: aborting");
            zpc->filter_comp = conica_filter_invalid ;
            return -1 ;
    }

    /* Special case: swap K and Ks if needed */
    if ((refstar==NULL) && (band==WAVEBAND_K)) {
        refstar = irstd_get_star_magnitude(zpc->star_ra, zpc->star_dec, 
                WAVEBAND_KS, &star_mag) ;
        if (refstar != NULL) zpc->filter_comp = conica_filter_ks ;
    } else if ((refstar==NULL) && (band==WAVEBAND_KS)) {
        refstar = irstd_get_star_magnitude(zpc->star_ra, zpc->star_dec, 
                WAVEBAND_K, &star_mag) ;
        if (refstar != NULL) zpc->filter_comp = conica_filter_k ;
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

static double zp_median_keyword(
        char    **  input_list,
        int         nframes,
        char    *   keyword)
{
    char    *   sval ;
    double      val ;
    double  *   vals ;
    int         nval ;
    double      med ;
    int         i ;
  
    /* Test inputs  */
    if ((input_list==NULL) || (nframes<1) || (keyword==NULL)) return -1.0 ;

    /* Initialize */
    nval = 0 ; 
    
    vals = malloc(nframes * sizeof(double)) ;
    
    for (i=0 ; i<nframes ; i++) {
        sval = pfits_get(INSID, input_list[i], keyword) ;
        if (sval == NULL) return -1.0 ;
        val = (double)atof(sval) ;
        if (fabs(val) > 1e-3) {
            vals[nval] = val ;
            nval ++ ;
        }
    }
   if (nval==0) {
        free(vals) ;
        return 0.0 ;
    }
    med = double_median(vals, nval) ;

    /* Free and return */
    free(vals) ;
    return med ;
}

static int zp_output_results(zeropoint_bb * zpc)
{
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
    double      str_val, str_err, str_peak, str_flux, str_rms ;
    double      median ;
    int         i ;

    /* Create output PAF file */
	sprintf(out_name, "%s.paf", zpc->name_o);

	e_comment(1, "creating output PAF [%s]", out_name);
	paf = qfits_paf_print_header(out_name, "CONICA/zero_point",
						   "Zero point computation results",
                           get_login_name(),
                           get_datetime_iso8601());
	if (paf==NULL) return -1 ;
	fprintf(paf, "\n");

    /* Add ARCFILE */
    sval = pfits_get(INSID, zpc->input_list[0], "arcfile") ;
    if (sval != NULL) fprintf(paf, "ARCFILE   \"%s\"  \n", sval) ;

    /* TPL.ID  */
    sval = pfits_get(INSID, zpc->input_list[0], "templateid") ;
    if (sval != NULL) fprintf(paf, "TPL.ID  \"%s\" \n", sval) ;
    
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

    /* AOS.INS.DICH.POSNAM  */
    sval = pfits_get(INSID, zpc->input_list[0], "dich_posname") ;
    fprintf(paf, "AOS.INS.DICH.POSNAM     \"%s\"\n", sval ? sval : "unknown");

    /* AOS.OCS.WFS.MODE */
    sval = pfits_get(INSID, zpc->input_list[0], "wfs_mode") ;
    fprintf(paf, "AOS.OCS.WFS.MODE        \"%s\"\n", sval ? sval : "unknown");

    /* AOS.OCS.WFS.TYPE */
    sval = pfits_get(INSID, zpc->input_list[0], "wfs_type") ;
    fprintf(paf, "AOS.OCS.WFS.TYPE        \"%s\"\n", sval ? sval : "unknown");

    /* AOS.RTC.DET.DST.L0MEAN */
    median = zp_median_keyword(zpc->input_list, zpc->nframes, "l0mean") ;
    fprintf(paf, "AOS.RTC.DET.DST.L0MEAN   \"%g\"\n", median);
    
    /* AOS.RTC.DET.DST.R0MEAN */
    median = zp_median_keyword(zpc->input_list, zpc->nframes, "r0mean") ;
    fprintf(paf, "AOS.RTC.DET.DST.R0MEAN   \"%g\"\n", median);
    
    /* AOS.RTC.DET.DST.T0MEAN */
    median = zp_median_keyword(zpc->input_list, zpc->nframes, "t0mean") ;
    fprintf(paf, "AOS.RTC.DET.DST.T0MEAN   \"%g\"\n", median);
    
    /* AOS.RTC.DET.DST.ECMEAN */
    median = zp_median_keyword(zpc->input_list, zpc->nframes, "ecmean") ;
    fprintf(paf, "AOS.RTC.DET.DST.ECMEAN   \"%g\"\n", median);
    
    /* AOS.RTC.DET.DST.FLUXMEAN */
    median = zp_median_keyword(zpc->input_list, zpc->nframes, "fluxmean") ;
    fprintf(paf, "AOS.RTC.DET.DST.FLUXMEAN   \"%g\"\n", median);

    /* INS.OPTI7.NAME */
    sval = pfits_get(INSID, zpc->input_list[0], "opti7_name") ;
    fprintf(paf, "INS.OPTI7.NAME          \"%s\"\n", sval ? sval : "unknown");
    
    /* DET.NCORRS.NAME */
    sval = pfits_get(INSID, zpc->input_list[0], "rom_name") ;
    fprintf(paf, "DET.NCORRS.NAME         \"%s\"\n", sval ? sval : "unknown");
    
    /* DET.MODE.NAME */
    sval = pfits_get(INSID, zpc->input_list[0], "mode") ;
    fprintf(paf, "DET.MODE.NAME           \"%s\"\n", sval ? sval : "unknown");
    
	/* OBS.ID */
	sval = pfits_get(INSID, zpc->input_list[0], "obs_id");
	fprintf(paf, "OBS.ID                \"%s\"\n", sval ? sval : "unknown");
	
	fprintf(paf, "\n# Detector section\n");
	if (zpc->pixscale_found) {
		fprintf(paf, "INS.PIXSCALE          %f ;#"
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
	e_comment(2, "-> Filter    : %s", conica_get_filtername(zpc->filter_obs));
	if (zpc->filter_obs != zpc->filter_comp) {
		e_warning("different filter used for computation");
		e_warning("acquired in filter [%s]",
					conica_get_filtername(zpc->filter_obs)) ;
		e_warning("computed with filter [%s]",
					conica_get_filtername(zpc->filter_comp)) ;
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
			conica_get_filtername(zpc->filter_comp));
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
		
	/* Reject highest and lowest value */
	qsort(comp_mag, zpc->np, sizeof(double), double_sort);
	for (i=1 ; i<(zpc->np-1) ; i++) {
		if (comp_mag[i] > 0.0) {
            avg_mag += comp_mag[i] ;
		    sqsum += comp_mag[i] * comp_mag[i] ;
		    nfluxes_ok++ ;
        }
	}
    
    /* Write QC params */
    fprintf(paf, "QC.AIRMASS            %g\n", avg_airmass);
    fprintf(paf, "QC.FILTER.OBS         \"%s\"\n",
            conica_get_filtername(zpc->filter_obs)) ;
    fprintf(paf, "QC.FILTER.REF         \"%s\"\n",
            conica_get_filtername(zpc->filter_comp)) ;
    sval = pfits_get(INSID, zpc->input_list[0], "opti3_name") ;
    fprintf(paf, "QC.FILTER.NDENS       \"%s\"\n", sval ? sval : "unknown");
    sval = pfits_get(INSID, zpc->input_list[0], "opti4_id") ;
    fprintf(paf, "QC.FILTER.POL         \"%s\"\n", sval ? sval : "unknown");

    if (nfluxes_ok<1) {
		e_error("no valid flux measurement: cannot compute ZP");
		fprintf(paf, "# Cannot compute zero point\n") ;
		fprintf(paf, "QC.ZPOINT             -1\n") ;
		fprintf(paf, "QC.ZPOINTRMS          -1\n") ;
		fprintf(paf, "QC.STDNAME            \"unknown\"\n");
		fprintf(paf, "QC.CATNAME            \"unknown\"\n");
	} else {
		avg_mag /= (double)(nfluxes_ok) ;
		sqsum   /= (double)(nfluxes_ok) ;
        /* Rounding errors can cause the variance to be negative */
        rms_mag = sqsum - avg_mag * avg_mag ;
        rms_mag = rms_mag > 0 ? sqrt(rms_mag) : 0 ;
		fprintf(paf, "QC.ZPOINT             %g\n", avg_mag) ;
		fprintf(paf, "QC.ZPOINTRMS          %g\n", rms_mag) ;
		fprintf(paf, "QC.STDNAME            \"%s\"\n", zpc->star_name);
		fprintf(paf, "QC.CATNAME            \"%s\"\n", 
                irstd_catalog_name(zpc->star_source));

		e_comment(0, "Computation results\n") ;
		e_comment(0, "ZeroPoint    = %g\n", avg_mag) ;
		e_comment(0, "ZeroPointRMS = %g\n", rms_mag) ;
	}
	free(comp_mag) ;

    /* Compute the average strehl */
	fprintf(paf, "\n# Strehl result section\n");
	fprintf(paf, "\n");
    
    fprintf(paf, 
    "# Nb Star peak Star flux PSF peak PSF flux Background Strehl  Error\n");
    for (i=0 ; i<zpc->np ; i++) {
        fprintf(paf, "# %2d %9.2f %9.2f %8.2g %8.2g %10.4g %5.4g %6.4g\n", i+1, 
            zpc->star_peak[i], zpc->star_flux[i], zpc->psf_peak[i],
            zpc->psf_flux[i], zpc->star_bg[i], zpc->strehl[i], 
            zpc->strehl_error[i]) ;
    }
    fprintf(paf, "\n") ;
    
    str_val    = 0.0 ;
    str_err    = 0.0 ;
    str_peak   = 0.0 ;
    str_flux   = 0.0 ;
    str_rms    = 0.0 ;
    nfluxes_ok = 0 ;
    for (i=0 ; i<zpc->np ; i++) {
        if ((zpc->strehl[i] > 0.0) && (zpc->strehl[i] < 1.0)) {
            str_val  += zpc->strehl[i] ;
            str_err  += zpc->strehl_error[i] ;
            str_peak += zpc->star_peak[i] ;
            str_flux += zpc->star_flux[i] ;
            str_rms  += zpc->bg_noise[i] ;
            nfluxes_ok++ ;
        }
    }
    if (nfluxes_ok<1) {
        e_error("no valid strehl measurement: cannot compute STREHL") ;
        fprintf(paf, "# Cannot compute STREHL\n") ;
        fprintf(paf, "QC.STREHL             -1\n") ;
        fprintf(paf, "QC.STREHL.ERROR       -1\n") ;
        fprintf(paf, "QC.STREHL.RMS         -1\n") ;
        fprintf(paf, "QC.STREHL.PEAK        -1\n") ;
        fprintf(paf, "QC.STREHL.FLUX        -1\n") ;
    } else {
        str_val  /= (double)(nfluxes_ok) ;
        str_err  /= (double)(nfluxes_ok) ;
        str_rms  /= (double)(nfluxes_ok) ;
        str_peak /= (double)(nfluxes_ok) ;
        str_flux /= (double)(nfluxes_ok) ;
        fprintf(paf, "QC.STREHL             %g\n", str_val) ;
        fprintf(paf, "QC.STREHL.ERROR       %g\n", str_err) ;
        fprintf(paf, "QC.STREHL.RMS         %g\n", str_rms) ;
        fprintf(paf, "QC.STREHL.PEAK        %g\n", str_peak) ;
        fprintf(paf, "QC.STREHL.FLUX        %g\n", str_flux) ;
        fprintf(paf, "\n\n# end of file\n");

        e_comment(0, "Strehl       = %g\n", str_val) ;
        e_comment(0, "Strehl error = %g\n", str_err) ;
    }
    fclose(paf) ;
    return 0 ;
}

