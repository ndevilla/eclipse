/*----------------------------------------------------------------------------*/
/**
   @file    wavecal.c
   @author  N. Devillard
   @date    February 2001
   @version	$Revision: 1.25 $
   @brief   ISAAC wavelength calibration
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: wavecal.c,v 1.25 2003/11/18 18:01:36 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/18 18:01:36 $
	$Revision: 1.25 $
*/

/*-----------------------------------------------------------------------------
                				Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "spectral_lines.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define DISCARD_LO_BORDER		80
#define DISCARD_HI_BORDER		80

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int isaac_wavecal_engine(char *, int, int, int, int, int, char *, 
		int, int, double, double) ;
static char * identify_spectral_table(char *) ;
static void insert_disprel_in_header(char *, double, double) ;
static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                                Main 
 -----------------------------------------------------------------------------*/
int isaac_wavecal_main(void * dict)
{
	dictionary	*	d ;
	
	int				discard_lo,
					discard_hi,
					discard_le,
					discard_ri ;
	int				remove_thermal ;
	int				modify_header ;
	double			wave_min,
					wave_max ;
	char		*	table_name ;
    int             order ;

	char            argname[10] ;
	char		*	name_i ;
	int             nfiles ;

	char        *   tmp_string ;
	int             errors ;
	int             i ;
	
	d = (dictionary*)dict ;
	/* Get options */
	remove_thermal = dictionary_getint(d, "arg.thermal", 0);
	table_name     = dictionary_get(d, "arg.table", NULL);
	if (table_name == NULL) table_name = "auto";
	modify_header  = dictionary_getint(d, "arg.header", 0);
	/* Get image border definition */
	tmp_string = dictionary_get(d, "arg.border", NULL);
	if (tmp_string != NULL) {
		if (sscanf(tmp_string, "%d %d", &discard_lo, &discard_hi)!=2) {
			e_error("in -b/--border: expected two values");
		}
	} else {
		discard_lo = DISCARD_LO_BORDER ;
		discard_hi = DISCARD_HI_BORDER ;
	}
	/* Get zero spectrum definition */
	tmp_string = dictionary_get(d, "arg.zero", NULL);
	if (tmp_string != NULL) {
		if (sscanf(tmp_string, "%d %d", &discard_le, &discard_ri)!=2) {
			e_error("in -z/--zero: expected two values");
		}
	} else {
		discard_le = -1 ;
		discard_ri = -1 ;
	}
	/* Get wavelength input range */
	tmp_string = dictionary_get(d, "arg.wave", NULL);
	if (tmp_string != NULL) {
		if (sscanf(tmp_string, "%lg %lg", &wave_min, &wave_max)!=2) {
			e_error("in -w/--wave: expected two values");
		}
	} else {
		wave_min = -1.0 ;
		wave_max = -1.0 ;
	}
    /* Get order */
    order = dictionary_getint(d, "arg.order", -1);
    
	/* Get input/output file names */
	nfiles = dictionary_getint(d, "arg.n", -1) ;
    if (nfiles<0) {
        e_error("missing input file name(s): aborting");
        return -1 ;
    }

    INSID = pfits_identify_insstr("isaac");
    /* Loop on input file names */
    errors = 0 ;
    for (i=1 ; i<nfiles ; i++) {
        sprintf(argname, "arg.%d", i);
        name_i = dictionary_get(d, argname, NULL) ;

        /* Once command-line options have been cleared out, call the engine */
		errors += isaac_wavecal_engine(name_i,
                                	discard_lo,
                                	discard_hi,
                                	discard_le,
                                	discard_ri,
                                	remove_thermal,
                                	table_name,
                                	modify_header,
                                    order,
                                	wave_min,
                                	wave_max) ;
	}
	return errors ;
}

/*-----------------------------------------------------------------------------
							Function code
 -----------------------------------------------------------------------------*/
static int isaac_wavecal_engine(
        char     *    name_i,
        int         discard_lo,
        int         discard_hi,
        int         discard_le,
        int         discard_ri,
        int         remove_thermal,
        char    *     table_name,
        int         modify_header,
        int         order,
        double         wave_min,
        double         wave_max)
{
    computed_disprel    *   disprel ;
    double              *   phdisprel ;
    int                     npix ;
    image_t             *   image_in ;
    char                *   sval ;
    double                  slit_width ;

    /* Compute the slit width */
    slit_width = isaac_get_slitwidth(name_i) ;
    if (slit_width == -1) {
        e_error("cannot get the slit width") ;
        return -1 ;
    }
    
    /* Gather number of pixels in the input spectrum */
    if ((sval = qfits_query_hdr(name_i, "NAXIS1")) == NULL) {
        e_error("cannot read NAXIS1 in input file") ;
        return -1 ;
    }
    npix = atoi(sval);

    /* If the order is not specified, read it from the header */
    if (order < 0) {
        if ((sval = pfits_get(INSID, name_i, "order")) == NULL) {
            e_error("order not specified and not readable in the header") ;
            return -1 ;
        } else {
            order = atoi(sval) ;
        }
    }

    /*
     * If nothing was provided for wavelength range, determine one
     * through a physical model of the instrument.
     */
    if ((0 < wave_min) && (wave_min < wave_max)) {
        /* Create 1st order model based on wave range */

        /* wave_min and wave_max are the extremities of the wavelength range */

        phdisprel = malloc(4 * sizeof(double)) ;
        phdisprel[3] = 0;
        phdisprel[2] = 0;
        phdisprel[1] = (wave_max - wave_min) / npix;
        phdisprel[0] =  wave_min - phdisprel[1];

    } else {
        /* No wave range provided - use physical model */
        double * ph_tmp ;
        if ((ph_tmp = isaac_get_disprel_estimate(name_i, 2)) == NULL) {
            e_error("getting estimate for dispersion relation: aborting");
            return -1 ;
        }
        phdisprel = malloc(4 * sizeof(double));
        phdisprel[3] = 0;
        memcpy(phdisprel, ph_tmp, 3 * sizeof(double));
        free(ph_tmp);
    }

    if (!strcmp(table_name, "auto"))
        table_name = identify_spectral_table(name_i);
    e_comment(0, "using spectral table: [%s]\n", table_name);

    if (remove_thermal == 0) remove_thermal = isaac_has_thermal(name_i) > 0;

    /* Load input image */
    image_in = image_load(name_i) ;
    if (image_in == NULL) {
        e_error("in loading image [%s]: aborting", name_i) ;
        return -1 ;
    }

    /* Compute dispersion relation */
    disprel = spectro_compute_disprel(image_in,
                                    discard_lo,
                                    discard_hi,
                                    discard_le,
                                    discard_ri,
                                    remove_thermal,
                                    table_name,
                                    slit_width,
                                    order,
                                    phdisprel);

    free(phdisprel);
    image_del(image_in);
    if (disprel==NULL) {
        e_error("computing dispersion relation: aborting");
        return -1 ;
    }

    /* Modify input file header if requested */
    if (modify_header) {
        insert_disprel_in_header(name_i, disprel->poly[0], disprel->poly[1]);
    }

    /* Print out results on stdout */
    e_comment(0, "Cross-correlation quality: %g\n", disprel->cc) ;
    e_comment(0, "Wavelength calib.: wave = f(pix), pix in [1 1024] with:");
    e_comment(0, "    f(x) = %g + %g*x + %g*x^2 + %g*x^3",
            disprel->poly[0], disprel->poly[1], disprel->poly[2], 
            disprel->poly[3]) ;
    if (disprel->poly != NULL) free(disprel->poly) ;
    free(disprel);
    return 0 ;
}

static char * identify_spectral_table(char * filename)
{
	char  *	name ;
	int		xenon, argon ;

	xenon = isaac_is_xenon_lamp_active(filename) ;
	argon = isaac_is_argon_lamp_active(filename) ;

	if ((argon==-1) || (xenon==-1)) {
		e_error("cannot determine lamp status: using OH line table");
		return "oh" ;
	}

	if (argon && xenon) {
		name = "Xe+Ar";
	} else if (argon && !xenon) {
		name = "Ar";
	} else if (!argon && xenon) {
		name = "Xe";
	} else {
		name = "oh";
	}
	return name ;
}

static void insert_disprel_in_header(char * filename, double a, double b)
{
	char			line[81];
	char			val[81] ;

	if (test_write_permission(filename)) {
		e_comment(1, "setting CRPIX1 to: 1.0");
		/* Modify CRPIX1 */
		keytuple2str(line, "CRPIX1", "1.0", "Ref pixel in X");
		line[80]=0 ;
		qfits_replace_card(filename, "CRPIX1", line);
		/* Modify CRVAL1 */
		e_comment(1, "setting CRVAL1 to: %g", a+b);
		sprintf(val, "%g", a+b);
		keytuple2str(line, "CRVAL1", val, "wavelength at ref pixel");
		line[80]=0 ;
		qfits_replace_card(filename, "CRVAL1", line);
		/* Modify CDELT1 */
		e_comment(1, "setting CDELT1 to: %g", b);
		sprintf(val, "%g", b);
		keytuple2str(line, "CDELT1", val, "Angstroems per pixel");
		line[80]=0 ;
		qfits_replace_card(filename, "CDELT1", line);
		/* Modify CTYPE1 */
		e_comment(1, "setting CTYPE1 to: LINEAR");
		keytuple2str(line, "CTYPE1", "LINEAR", "pixel coordinate system");
		line[80]=0 ;
		qfits_replace_card(filename, "CTYPE1", line);
	} else {
		e_warning("cannot modify input file: access is read-only");
	}
	return ;
}

