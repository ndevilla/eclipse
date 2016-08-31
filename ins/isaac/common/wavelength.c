/*----------------------------------------------------------------------------*/
/**
   @file    wavelength.c
   @author  Y.Jung
   @date    July 2000
   @version	$Revision: 1.31 $
   @brief   ISAAC common function to handle with wl calibration
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: wavelength.c,v 1.31 2003/12/03 16:27:18 llundin Exp $
	$Author: llundin $
	$Date: 2003/12/03 16:27:18 $
	$Revision: 1.31 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "isaacp_lib.h"
#include "wavelength.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define ISAAC_LR_DIR                 7.0    /* angle in degrees */
#define ISAAC_MR_DIR                31.5    /* angle in degrees */

#define ISAAC_LR_GRATING            40.0    /* grooves/mm */
#define ISAAC_MR_GRATING            210.0   /* grooves/mm */

#define ISAAC_FOCAL_LENGTH_MM       175.0   /* mm */

#define ISAAC_PIXEL_SIZE_S          18.5    /* microns */
#define ISAAC_PIXEL_SIZE_M          27.0    /* microns */

#define ISAAC_BEAM_DIFF             2.72    /* angle in degrees */
#define ISAAC_PUPIL_SIZE_MM         100.0   /* mm */

#define ISAAC_FLGTH_S1              1.75
#define ISAAC_FLGTH_S2              3.25
#define ISAAC_FLGTH_L1              1.56
#define ISAAC_FLGTH_L2              4.77
#define ISAAC_FLGTH_L3              9.88

#define ANGLE_IN_DEFAULT            0.00
#define ANGLE_OUT_DEFAULT           0.00

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Estimate the instrument wavelength range.
  @param    filename    input file name
  @param    poly_deg    polynomial degree
  @return   1 newly allocated array of poly_deg+1 polynomical coefficients
  
  From a physical model of the instrument, find out the wavelength range
  associated to a given instrument configuration. The returned coefficients
  are such as wave = c[0] + c[1] * pix + ... + c[poly_deg] * pix^poly_deg 

  Instrument configuration informations are fetched from the FITS header.
 */
/*----------------------------------------------------------------------------*/
double * isaac_get_disprel_estimate(
        char    *   filename, 
        int         poly_deg)
{
    double      *   c ;
    double          wl_c ;
    char        *   s ;
    char            objective[32];
    char            resolution[32];
    int             npix ;
    double3     *   plist ;
    double      *   coefs ;
    instrument_t    ins ;
    int             order;
    int             i ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

    /* Get various information from the FITS header */
    /* Central wavelength is in Angstrom */
    wl_c = isaac_get_central_wavelength(filename);
    if (wl_c <= 0) {
        e_error("cannot get central wavelength from [%s]", filename);
        return NULL ;
    }

    s = pfits_get(ins, filename, "objective") ;
    if (s==NULL) {
        e_error("cannot get objective used from [%s]", filename);
        return NULL ;
    }
    strcpy(objective, strlwc(s));

    s = pfits_get(ins, filename, "resolution");
    if (s==NULL) {
        e_error("cannot get resolution used from [%s]", filename);
        return NULL ;
    }

    /* Resolution to lower case letters */
    strcpy(resolution, strlwc(s));
    
    s = pfits_get(ins, filename, "naxis1") ;
    if (s==NULL) {
        e_warning("cannot get x size from [%s]", filename);
        return NULL ;
    } 
    npix = (int)atoi(s) ;
    if (npix < 2) {
        e_error("Cannot get x size from [%s]", filename);
        return NULL;
    }


    /* Set the order - as in isaac_physical_model() */
    if ( wl_c >= 8900 && wl_c < 9900 ) {
        order = 6;
    } else if ( wl_c >= 9900 && wl_c < 11000 ) {
        order = 5;
    } else if ( wl_c >= 11000 && wl_c < 14000 ) {
        order = 4;
    } else if ( wl_c >= 14000 && wl_c < 18500 ) {
        order = 3;
    } else if ( wl_c >= 18500 && wl_c < 25000 ) {
        order = 2;
    } else {
        order = 1;
    }

    if (poly_deg == 3 && resolution[0] == 'm' && 8900 <= wl_c && wl_c < 25000) {

        /* This method and the constants are provided by C. Lidman */

        const double  a = -1.218717e-7;
        const double  b =  0.003395204;
        const double  c =  1337.455;
        const double  d = -1.617833e-4;
        const double  e =  3.132269;
        const double  f = -2.496095;

        const double c1 = wl_c;
        const double c2 = (( a * (order*c1) + b) * (order*c1) + c) / order;
        const double c3 = ( d * (order*c1) + e ) / order;
        const double c4 = f / order;

        double       k0 = -(npix+1) / (double)(npix-1);
        double       k1 =          2 / (double)(npix-1);

        double      * p;


        /* Display configuration */
        e_comment(1, "configuration for ISAAC physical model: ") ;
        e_comment(2, "medium resolution") ;
        e_comment(2, "lambda_c     : %g", wl_c) ;
        e_comment(2, "objective    : %s", objective) ;

        p = malloc(4 * sizeof(double));

        /* The polynomial in reduced coordinates, -1 <= z <= 1 */
        p[0] = c1 - c3;

        p[1] = c2 - 3*c4;

        p[2] = 2*c3;

        p[3] = 4*c4;

        if (debug_active()>0)
            e_comment(0, "Reduced polynomial(%d:%d): %g + %g * z + %g * z^2 "
                  "+ %g * z^3\n", order, npix, p[0], p[1], p[2], p[3]);

        /* The polynomial in pixel coordinates, 1 <= pix <= npix */
        p[0] += k0 * ( p[1] + k0 * ( p[2] + k0 * p[3]));

        p[1] += k0 * ( 2*p[2] + k0 * 3*p[3]);
        p[1] *= k1;

        p[2] += k0 * 3*p[3];
        p[2] *= k1*k1;

        p[3] *= k1*k1*k1;

        return p;



    } else {


        /* c is an array of npix doubles with the wavelength for each pix. */
        if ((c=isaac_physical_model(wl_c,
                                    objective,
                                    resolution,
                                    npix)) == NULL) {
            e_error("cannot compute the physical model calibration") ;
            return NULL ;
        }

        /* A polynomial fit is computed  */
        if (poly_deg<0) {
            e_error("invalid polynomial degree specified") ;
            free(c) ;
            return NULL ;
        }


        plist = double3_new(npix);
        for (i=0 ; i<npix ; i++) {
            plist->x[i] = (double)(i+1) ;
            plist->y[i] = c[i] ;
        }
        free(c) ;
        coefs=fit_1d_poly(poly_deg, plist, NULL);
        double3_del(plist) ;
    }

    return coefs ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Determine the order according to filter, grating and wl
  @param    image_name  input image name
  @return   order, -1 in error case 
 */
/*----------------------------------------------------------------------------*/
int isaac_find_order(char * image_name)
{
    int                 order ;
    double              wl_c ;
    char                grat_name[FILENAMESZ] ;
    isaac_filter_id     f_id ;
    instrument_t        ins ;
    char            *   s ;
        
    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;
    order = 1 ;
    
    /* Get the grating name */
    if ((s = pfits_get(ins, image_name, "resolution")) == NULL) {
        e_error("cannot get resolution from [%s]", image_name) ;
        return -1 ;
    }
    strcpy(grat_name, strlwc(s));

    /* Get the central wavelength - in Angstrom */
    if ((wl_c = isaac_get_central_wavelength(image_name)) <= 0) { 
        e_error("cannot get central wavelength from [%s]", image_name) ;
        return -1 ;
    }

    /* Get the filter used */
    if ((s=pfits_get(ins, image_name, "filter")) == NULL ||
        (f_id = isaac_get_filterid(s)) == isaac_filter_invalid) {
        e_error("cannot get filter from [%s]", image_name) ;
        return -1 ;
    }
   
    /* Association rules - Medium resolution */
    if (grat_name[0] == 'm' && f_id == isaac_filter_sh &&
        27000 < wl_c && wl_c < 42000) order = 2 ;

    if (grat_name[0] == 'm' && f_id == isaac_filter_jblock &&
        35500 < wl_c && wl_c < 42000) order = 3 ;

    /* This association is currently only relevant for historical data */
    if (grat_name[0] == 'm' && f_id == isaac_filter_sk &&
        44000 < wl_c && wl_c < 51000) order = 2 ;

    if (grat_name[0] == 'm' && f_id == isaac_filter_sh &&
        44000 < wl_c && wl_c < 51000) order = 3 ;
       
    /* This association is currently only relevant for historical data */
    if (grat_name[0] == 'm' && f_id == isaac_filter_jblock &&
        44000 < wl_c && wl_c < 51000) order = 4 ;


    /* Association rules - Low resolution */
#if 1
    /* verify with DFO */
    /* This association is currently only relevant for historical data */
    if (grat_name[0] == 'l' && f_id == isaac_filter_sk &&
        35500 < wl_c && wl_c < 42000) order = 2;

    /* This association is currently only relevant for historical data */
    if (grat_name[0] == 'l' && f_id == isaac_filter_sh &&
        35500 < wl_c && wl_c < 42000) order = 2 ;

    /* This association is currently only relevant for historical data */
     if (grat_name[0] == 'l' && f_id == isaac_filter_jblock &&
        35500 < wl_c && wl_c < 42000) order = 3 ;
#endif

    if (grat_name[0] == 'l' && f_id == isaac_filter_sh &&
        44000 < wl_c && wl_c < 51000) order = 3 ;

    if (debug_active() > 0)
        e_comment(1,"Find order: %d. Resol: %c. Lambda_c: %g. Filter: %s",
                  order, grat_name[0], wl_c, isaac_get_filtername(f_id));
    
    return order ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Determine presence of thermal background according to instr. setting
  @param    image_name  input image name
  @return   1 if yes, 0, when not, -1 in error case 

 */
/*----------------------------------------------------------------------------*/
int isaac_has_thermal(char * im_name)
{
    instrument_t  ins = pfits_identify_insstr("isaac");
    double              wl_c ;
    char                grat_name[FILENAMESZ] ;
    isaac_filter_id     f_id ;
    char            *   s ;
    int                 has_thermal = 0;

    /* Get the grating name */
    if ((s = pfits_get(ins, im_name, "resolution")) == NULL) {
        e_error("cannot get resolution from [%s]", im_name) ;
        return -1 ;
    }
    strcpy(grat_name, strlwc(s));

    /* Get the central wavelength - in Angstrom */
    if ((wl_c = isaac_get_central_wavelength(im_name)) <= 0) { 
        e_error("cannot get central wavelength from [%s]", im_name) ;
        return -1 ;
    }

    /* Get the filter used */
    if ((s=pfits_get(ins, im_name, "filter")) == NULL ||
        (f_id = isaac_get_filterid(s)) == isaac_filter_invalid) {
        e_error("cannot get filter from [%s]", im_name) ;
        return -1 ;
    }

    /* LW LR SK 2.20 Xe - Added Ar after testing
       - wl_c will currently only deviate from 2.22 in historical data  */
    if (grat_name[0] == 'l' && f_id == isaac_filter_sk &&
        21900 <= wl_c) has_thermal = 1;

    /* LW LR SL 3.55 Xe+Ar */
    /* wl_c can in some (rare and actually unsupported) cases be lower */
    if (grat_name[0] == 'l' && f_id == isaac_filter_sl &&
        34000 <= wl_c) has_thermal = 1;

    /* LW LR SH 3.55 Xe+Ar */
    /* This association is currently only relevant for historical data */
    /* The above limit for wl_c is chosen */
    if (grat_name[0] == 'l' && f_id == isaac_filter_sh &&
        34000 <= wl_c && wl_c < 37000) has_thermal = 1;

    /* LW MR SK 2.35 - Added this after testing
       - 2.2 has no thermal background, while 2.26463 has */
    if (grat_name[0] == 'm' && f_id == isaac_filter_sk &&
        22500 <= wl_c) has_thermal = 1;

    /* LW MR SL 3.30 */
    /* - and above (incl. 4.08) added after testing */
    if (grat_name[0] == 'm' && f_id == isaac_filter_sl &&
        30000 <= wl_c) has_thermal = 1;

    /* LW MR SH Xe+Ar - Added after testing  */
    if (grat_name[0] == 'm' && f_id == isaac_filter_sh &&
        32000 <= wl_c) has_thermal = 1;

    /* LW MR J+Block Xe+Ar - Added after testing  */
    if (grat_name[0] == 'm' && f_id == isaac_filter_jblock &&
        34000 <= wl_c) has_thermal = 1;

    if (debug_active() > 0)
        e_comment(1,"Has thermal: %d. Resol: %c. Lambda_c: %g. Filter: %s",
                  has_thermal, grat_name[0], wl_c, isaac_get_filtername(f_id));
    
    return has_thermal;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    ISAAC physical model
  @param    lambda_c    central wavelength
  @param    objective   slit used
  @param    resolution  resolution (medium or low)
  @param    nbpix       number of pixels to calibrate
  @return   wavelengths array (index i <-> pixel nb i+1)
    
  This module determines the dispersion relation of ISAAC for the different 
  configurations of objectives, gratings, detectors, and central wavelengths.

  Assumed Optical configuration:
   
    Focal lens of objectives for short- and long-wavelength objective (at 77 K):
        S1 = f/1.75, S2 = f/3.25, L1 = f/1.56, L2 = f/4.77,  L3 = f/9.98
    Pupil size: 100 mm
    Pixel size: 18.5 microns (SW), 27 microns (LW)
    Gratings:   low resolution, 40 gr/mm, entering at about 5 degrees
                medium resolution, 210 gr/mm, entering at about 23 degrees
    Beam difference: 2.72 degrees
 */
/*----------------------------------------------------------------------------*/
double * isaac_physical_model(
        double      lambda_c,
        char    *   objective,
        char    *   resolution,
        int         nbpix)
{
    double      beam_diff, focal_length, pixel_size, pupil ; 
    double      gr, gr_dir ;
    double      isaacLRdir, isaacMRdir ;
    double      isaacLRGrating, isaacMRGrating ;
    int         order ;
    double  *   disp ;
    double      x ;
    double      angle_in, angle_out ;
    double      a, b, det ;
    int         i ;

    /* Initialize */
    isaacLRdir      = ISAAC_LR_DIR ;
    isaacMRdir      = ISAAC_MR_DIR ;
    isaacLRGrating  = ISAAC_LR_GRATING ;
    isaacMRGrating  = ISAAC_MR_GRATING ;
    focal_length    = ISAAC_FOCAL_LENGTH_MM ;
    pixel_size      = ISAAC_PIXEL_SIZE_S ;
    beam_diff       = ISAAC_BEAM_DIFF ;
    pupil           = ISAAC_PUPIL_SIZE_MM ;

    /* Check objective */
    if (objective[0] == 's') {
        if (objective[1] == '1') focal_length = pupil*ISAAC_FLGTH_S1 ;
        if (objective[1] == '2') focal_length = pupil*ISAAC_FLGTH_S2 ;
        pixel_size = ISAAC_PIXEL_SIZE_S ;
    }

    if (objective[0] == 'l') {
        if (objective[1] == '1') focal_length = pupil*ISAAC_FLGTH_L1;
        if (objective[1] == '2') focal_length = pupil*ISAAC_FLGTH_L2;
        if (objective[1] == '3') focal_length = pupil*ISAAC_FLGTH_L3;
        pixel_size = ISAAC_PIXEL_SIZE_M ;
    }

    /* Convert mm to m */
    focal_length *= 1e-3; 

    /* Resolution to lower case letters */
	strcpy(resolution, strlwc(resolution)) ;
	
    /* Display configuration */
    e_comment(1, "configuration for ISAAC physical model: ") ;
    if (resolution[0]=='l') e_comment(2, "low resolution") ;
    if (resolution[0]=='m') e_comment(2, "medium resolution") ;
    e_comment(2, "lambda_c     : %g", lambda_c) ;
    e_comment(2, "objective    : %s", objective) ;
    e_comment(2, "focal length : %g", focal_length) ;

    /* Set the grating */
    if (resolution[0] == 'l') { gr = isaacLRGrating; gr_dir = isaacLRdir;}
	else if (resolution[0] == 'm') { gr = isaacMRGrating; gr_dir = isaacMRdir;}
	else {    
		e_error("wrong grating! %c", resolution[0]) ;
        return NULL ;
    }

    /* Convert gr to grove/nm */
    gr *= 1e-6 ;  
    /* Convert beam_diff in radians. */
    beam_diff *= M_PI / 180.0 ; 
    /* Convert lambda_c : A->nm */
    lambda_c /= 10 ;

    /* Set the order */
    if ( lambda_c >= 890 && lambda_c < 8000) {
        if ( lambda_c >= 890 && lambda_c < 990 ) {
            order = 6;
        } else if ( lambda_c >= 990 && lambda_c < 1100 ) {
            order = 5;
        } else if ( lambda_c >= 1100 && lambda_c < 1400 ) {
            order = 4;
        } else if ( lambda_c >= 1400 && lambda_c < 1850 ) {
            order = 3;
        } else if ( lambda_c >= 1850 && lambda_c < 2500 ) {
            order = 2;
        } else if ( lambda_c >= 2500 && lambda_c < 5500 ) {
            order = 1;
        } else {
            order = 1;
        }
    } else {
        order = (int)(0.5 + (sin(ANGLE_IN_DEFAULT) + sin(ANGLE_OUT_DEFAULT)) /
                (lambda_c * gr)) ;
        if (order < 1) {
            e_error("wrong order! %d", order) ;
            return NULL ;
        }
    }
    e_comment(2, "order    : %d", order) ;

    /* The following is the solution of the set of equations : */
    /* (1)  sin(angle_in) + sin(angle_out) = order*gr*lambda_c  */
    /* (2)  angle_out - angle_in = beam_diff */
    det = 2 * sin(beam_diff) ;
    a   = order * gr * lambda_c * sin(beam_diff) ;
    b   = sqrt(order * gr * lambda_c * order * gr * lambda_c *
        (sin(beam_diff) * sin(beam_diff) + 2 * cos(beam_diff) - 2)
        + 2 * sin(beam_diff) * sin(beam_diff) * (1-cos(beam_diff))) ;
    angle_in = asin((a-b)/det) ;
    angle_out = angle_in + beam_diff ;

    disp = malloc(nbpix * sizeof(double)) ;

    for (i=0 ; i<nbpix ; i++) {
        /* x in meters */
        x = (double)(i-(nbpix/2.)) * pixel_size * 1e-6; 
        disp[i]=(sin(angle_in)+sin(angle_out+atan(x/focal_length)))/(order*gr);

        /* Convert disprel from nm to angstroms */
        disp[i] *= 10 ;
    }

    return disp ;
}


