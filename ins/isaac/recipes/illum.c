/*----------------------------------------------------------------------------*/
/**
   @file    illum.c
   @author  N. Devillard
   @date    February 2001
   @version	$Revision: 1.30 $
   @brief   ISAAC illumination frame handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: illum.c,v 1.30 2005/03/10 15:27:42 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/10 15:27:42 $
	$Revision: 1.30 $
*/

/*-----------------------------------------------------------------------------
		                    		Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
				                    Defines
 -----------------------------------------------------------------------------*/

/* Default radii for photometry in pixels */
#define PHOT_RADIUS_STAR    10.0
#define PHOT_RADIUS_BGIN    12.0
#define PHOT_RADIUS_BGOUT   30.0

/* Default size for search domain (in pixels) */
#define SEARCH_DOMAIN_HX    50
#define SEARCH_DOMAIN_HY    50

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int isaac_illum_calibration(cube_t *, char *, char *, char *) ;
static int isaac_illumination_frame_process(char *, char *, char *, char *, 
		char *, double *, int *, char *) ;

/*-----------------------------------------------------------------------------
                                Main
 -----------------------------------------------------------------------------*/
int  isaac_illum_main(void * dict)
{
	dictionary	*	d ;

	char		*	dark ;
	char		*	flat ;
	char		*	badpix ;
	double			radii[3] ;
	int				search[2] ;
	char		*	fluxfile ;

	char            argname[10] ;
	int             nfiles ; 
	char		*	name_i ;
	char		*	name_o ;
	
	char        *   tmp_string ;
	int				items ;
    int             errors ;
    int             i ;
	
    INSID = pfits_identify_insstr("isaac");
    
	d = (dictionary*)dict ;
	/* Get options */
	dark       = dictionary_get(d, "arg.dark", NULL);
    flat       = dictionary_get(d, "arg.flat", NULL);
    badpix     = dictionary_get(d, "arg.badpix", NULL);
	fluxfile   = dictionary_get(d, "arg.flux", NULL);
	tmp_string = dictionary_get(d, "arg.search", NULL);
	if (tmp_string == NULL) {
		search[0] = SEARCH_DOMAIN_HX ;
		search[1] = SEARCH_DOMAIN_HY ;
	} else {
		items = sscanf(tmp_string, "%d %d", &search[0], &search[1]) ;
		if (items != 2) {
			search[0] = SEARCH_DOMAIN_HX ;
			search[1] = SEARCH_DOMAIN_HY ;
		}
	}
	tmp_string = dictionary_get(d, "arg.radius", NULL);
	if (tmp_string == NULL) {
		radii[0] = PHOT_RADIUS_STAR ;
		radii[1] = PHOT_RADIUS_BGIN ;
		radii[2] = PHOT_RADIUS_BGOUT ;
	} else {
		items = sscanf(tmp_string, "%lg %lg %lg", 
                &radii[0], &radii[1], &radii[2]) ;
		if (items != 3) {
			radii[0] = PHOT_RADIUS_STAR ;
			radii[1] = PHOT_RADIUS_BGIN ;
			radii[2] = PHOT_RADIUS_BGOUT ;
		}
	}

	/* Get input/output file names */
	nfiles = dictionary_getint(d, "arg.n", -1) ;
	if (nfiles<0) {
		e_error("missing input file name(s): aborting");
		return -1 ;
	}
	/* Loop on input file names */
	errors = 0 ;
	for (i=1 ; i<nfiles ; i++) {
        sprintf(argname, "arg.%d", i);
        name_i = dictionary_get(d, argname, NULL) ;
        name_o = dictionary_get(d, "arg.output", NULL) ;
        if (name_o == NULL) name_o = strdup(get_rootname(get_basename(name_i)));
        else name_o = strdup(get_rootname(name_o)) ;

        /* Once command-line options have been cleared out, call the engine */
		e_comment(0,
				"Running with the following parameters:\n"
				"\n"
				"input        = %s\n"
				"output       = %s\n"
				"dark         = %s\n"
				"flat         = %s\n"
				"badpix       = %s\n"
				"searchsize   = %d x %d\n"
				"radius star  = %g\n"
				"radius bgin  = %g\n"
				"radius bgout = %g\n"
				"flux file    = %s\n"
				"\n"
				"\n",
				name_i,
				name_o,
				dark ? dark : "none",
				flat ? flat : "none",
				badpix ? badpix : "none",
				2 * search[0] + 1, 2 * search[1] + 1,
				radii[0], radii[1], radii[2],
				fluxfile ? fluxfile : "none") ;
				
		errors += isaac_illumination_frame_process(name_i,
													name_o,
													dark,
													flat,
													badpix,
													radii,
													search,
													fluxfile) ;
		free(name_o) ;
	}
	return errors ;
}


/*-----------------------------------------------------------------------------
				                Function code
 -----------------------------------------------------------------------------*/
static int isaac_illumination_frame_process(
		char 	*	name_in,
		char 	* 	name_out,
		char 	* 	dark_in,
		char 	* 	ff_in,
		char 	* 	badpix_in,
		double	* 	radii,
		int	 	* 	search_d,
		char 	* 	fluxes_out)
{
	cube_t		*	in ;
	double3 	*	hdr_offs ;
	int				i ;	
	char		*	first_name ;
	double3		*	plist ;
	int				nvalid ;
	double			flux ;
	double			background ;
	double		*	fit_parms ;
	int				ncoeffs ;
	int				poly_deg ;
	double			mse ;
	char			fitstring[ASCIILINESZ] ;
	image_t		*	illum ;
	double		*	illum_d;
	char			full_name_out[FILENAMESZ] ;
	FILE		*	fluxes_txt ;
	int				refpos[2] ;
	qfits_header*   hdr ;
	framelist   *   raw ;
    char        *   sval ;
    FILE        *   paf ;

	/* Load input cube */
	in = cube_load(name_in);
	if (in==NULL) {
		e_error("loading %s: aborting", name_in);
		return -1 ;
	}
	/* Get offsets from headers */
	hdr_offs = isaac_get_offsets(name_in);
	if (hdr_offs == NULL) {
		e_error("reading offsets from FITS headers: aborting") ;
		cube_del(in) ;
		return -1 ;
	}

	/* Dark subtraction and Flat-fielding without illumination correction */
	isaac_illum_calibration(in, ff_in, dark_in, badpix_in) ;

	/* Refine the star position in all frames */
	e_comment(1, "---> locating standard star in all frames");
	for (i=0 ; i<in->np ; i++) {
		image_locate_peak(in->plane[i],
						  in->lx/2 + hdr_offs->x[i],
						  in->ly/2 + hdr_offs->y[i],
						  search_d[0],
						  search_d[1],
						  &refpos[0]);
		if (i==0) e_comment(1, "picked reference at [%d %d] in first plane",
					  refpos[0], refpos[1]);
		hdr_offs->x[i] = (double)refpos[0] - in->lx/2 ;
		hdr_offs->y[i] = (double)refpos[1] - in->ly/2 ; 
	}

	/* Aperture photometry on all positions */
	e_comment(1, "---> computing photometry on all images...") ;
	plist = double3_new(in->np);
	nvalid = 0 ;
	for (i=0 ; i<in->np ; i++) {
		background = image_get_disk_background(in->plane[i],
									 in->lx/2 + hdr_offs->x[i],
									 in->ly/2 + hdr_offs->y[i],
									 radii[1],
									 radii[2],
									 BG_METHOD_MEDIAN) ;
		if (background==-1.0) {
			e_warning("cannot get background in plane %d: using null val", i+1);
			background = 0.0 ;
		}
		flux = image_get_disk_flux(in->plane[i],
								in->lx/2 + hdr_offs->x[i],
								in->ly/2 + hdr_offs->y[i],
								radii[0],
								background) ;
		if (flux == -1.0) {
			e_warning("cannot compute flux: discarding point") ;
		} else {
			plist->x[nvalid] = in->lx/2 + hdr_offs->x[i] ;
			plist->y[nvalid] = in->ly/2 + hdr_offs->y[i] ;
			plist->z[nvalid] = flux ;
			nvalid++ ;
		}
	}
	double3_del(hdr_offs) ;

	if (nvalid<1) {
		e_error("not a single valid photometric measurement: aborting") ;
		double3_del(plist) ;
        cube_del(in) ;
		return -1 ;
	}

	if (fluxes_out != NULL) {
		if (fluxes_out[0]!=(char)0) {
			e_comment(0, "outputting flux info to [%s]", fluxes_out) ;
			fluxes_txt = fopen(fluxes_out, "w") ;
			if (fluxes_txt == NULL) {
				e_error("cannot create file %s: output to stdout", fluxes_out);
				for (i=0 ; i<in->np ; i++) {
					fprintf(stderr, "\tflux in plane % 3d is [%g]", i, 
                            plist->z[i]);
				}
			} else {
				fprintf(fluxes_txt, "# xoffset\tyoffset\tflux\n") ;
				for (i=0 ; i<in->np ; i++) {
					fprintf(fluxes_txt, "%g\t%g\t%g\n",
							plist->x[i],
							plist->y[i],
							plist->z[i]) ;
				}
				fclose(fluxes_txt) ;
			}
		}
	}

	/* Polynomial fit to the surface */
	e_comment(1, "---> polynomial fit to the surface") ;

	/*
	 * Depending on how many points were found, the polynomial fit may
	 * be restricted to a constant (2 pts), a plane (3 to 5 pts) or a
	 * second-degree polynomial in x and y with cross-term (6 pts or
	 * more).
	 */
	switch (nvalid) {
		case 2:
            e_warning("2 valid points found: fitting a constant") ;
            strcpy(fitstring, "(0,0)") ;
            poly_deg=0 ;
            break ;

		case 3:
		case 4:
		case 5:
            e_warning("%d valid points found: fitting a plane", nvalid) ;
            strcpy(fitstring, "(0,0) (1,0) (0,1)") ;
            poly_deg=1 ;
            break ;

		default:
            strcpy(fitstring, "(0,0) (1,0) (0,1) (1,1) (2,0) (0,2)") ;
            poly_deg=2 ;
            break ;
	}
	fit_parms = fit_surface_polynomial(plist, fitstring,poly_deg,&ncoeffs,&mse);
	double3_del(plist) ;
    switch (poly_deg) {
        case 0:
            e_comment(1, "P(x,y)= %g\n", fit_parms[0]) ;
            break ;
        case 1:
            e_comment(1, "P(x,y)= %g + %g*x + %g*y\n", 
                    fit_parms[0], fit_parms[1], fit_parms[2]) ;
            break ;
        case 2:
            e_comment(1, "P(x,y)= %g + %g*x + %g*y + %g*x*y\n",
                    fit_parms[0], fit_parms[1], fit_parms[2], fit_parms[3]) ;
            e_comment(2, "+ %g*x^2 + %g*y^2\n", 
                    fit_parms[4], fit_parms[5]) ;
            break ;
        default:
            break;
    }
    e_comment(1, "mean squared error: %g\n", mse) ;

	/* Generate an image of the polynomial */
	e_comment(1, "---> generating image from polynomial") ;
	illum_d = image_gen_polynomial_double(in->lx,
									   in->ly,
									   fit_parms,
									   ncoeffs,
									   poly_deg,
									   fitstring) ;
	if (illum_d == NULL) {
		e_error("cannot generate illumination frame: aborting") ;
        cube_del(in) ;
        free(fit_parms) ;
		return -1 ;
	}

	if ((illum = image_new(in->lx, in->ly)) == NULL) {
		e_error("allocating output image: aborting");
		free(illum_d);
        free(fit_parms) ;
        cube_del(in) ;
		return -1 ;
	}

	/* Convert the double array to an image */
	for (i=0 ; i<in->lx*in->ly ; i++) {
		illum->data[i] = (pixelvalue)illum_d[i] ;
	}
	free(illum_d);

	/* Save the frame */
	sprintf(full_name_out, "%s.fits", name_out) ;
	e_comment(1, "---> saving illumination frame [%s]", full_name_out);
	
	/* Read the input header */
	if ((first_name = framelist_firstname(name_in)) == NULL) {
		e_error("cannot find input ASCII list %s: aborting", name_in);
		image_del(illum);
        free(fit_parms) ;
        cube_del(in) ;
		return -1 ;
	}
	if ((hdr = qfits_header_read(first_name)) == NULL) {
		e_error("cannot read header file") ;
		image_del(illum);
        cube_del(in) ;
        free(fit_parms) ;
		return -1 ;
	}
	isaac_header_for_image(hdr) ;

	raw = framelist_load(name_in) ;
	if (isaac_pro_fits(hdr,
				full_name_out,
				"REDUCED",
				NULL,
				procat_imag_illum,
				"OK",
				"cal_illumframe",
				in->np,
				raw,
				NULL) == -1) {
		e_error("unable to write the PRO keyword in the fits header") ;
		image_del(illum);
		qfits_header_destroy(hdr) ;
        free(fit_parms) ;
        cube_del(in) ;
		framelist_del(raw) ;
		return -1 ;
	}
	framelist_del(raw) ;

	image_save_fits_hdrdump(illum, full_name_out, hdr, BPP_DEFAULT);
	qfits_header_destroy(hdr) ;
	cube_del(in) ;
	image_del(illum);

	/* Save the paf file */
	sprintf(full_name_out, "%s.paf", name_out) ;
	e_comment(1, "---> saving paf file [%s]", full_name_out);

    /* Open output PAF file */
    paf = qfits_paf_print_header(full_name_out,
                                    "ISAAC/illum",
                                    "Isaac illum QC parameters",
                                    get_login_name(),
                                    get_datetime_iso8601());
    if (paf == NULL) {
        e_error("cannot open file [%s] for output", full_name_out);
        free(fit_parms) ;
        return -1 ;
    }

    /* Add ARCFILE */
    if ((sval = pfits_get(INSID, first_name, "arcfile")) != NULL)
        fprintf(paf, "ARCFILE         \"%s\" ;#\n", sval) ;

    /* QC.ILLUM parameters */
    fprintf(paf, "QC.ILLUM1    %g\n", fit_parms[0]) ;
    if (poly_deg > 0) {
        fprintf(paf, "QC.ILLUMX    %g\n", fit_parms[1]) ;
        fprintf(paf, "QC.ILLUMY    %g\n", fit_parms[2]) ;
        if (poly_deg > 1) {
            fprintf(paf, "QC.ILLUMXY   %g\n", fit_parms[3]) ;
            fprintf(paf, "QC.ILLUMXX   %g\n", fit_parms[4]) ;
            fprintf(paf, "QC.ILLUMYY   %g\n", fit_parms[5]) ;
        }
    }
    fclose(paf) ;
	e_comment(0, "Ok") ;
    free(fit_parms) ;
	return 0 ;
}

static int isaac_illum_calibration(
        cube_t          *   in,
        char            *   ff_name,
        char            *   dark_name,
        char            *   badpix_name)
{
    image_t     *   dark ;
    image_t     *   ff ;
    pixelmap    *   badpix ;

    /* Load the calibration data */
    dark = image_load(dark_name) ;
    ff = image_load(ff_name) ;
    badpix = pixelmap_load(badpix_name) ;

    /* Apply the calibration corrections */
    cube_correct_ff_dark_badpix(in, ff, dark, badpix) ;

    /* Free the calibration data */
    if (dark != NULL) image_del(dark) ;
    if (ff != NULL) image_del(ff) ;
    if (badpix != NULL) pixelmap_del(badpix) ;

    /* Return */
    return 0 ;
}
