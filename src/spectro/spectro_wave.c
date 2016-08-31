/*----------------------------------------------------------------------------*/
/**
   @file    spectro_wave.c
   @author  N.Devillard
   @date    October 1999
   @version $Revision: 1.89 $
   @brief   spectroscopy routines
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: spectro_wave.c,v 1.89 2003/12/05 12:32:35 yjung Exp $
    $Author: yjung $
    $Date: 2003/12/05 12:32:35 $
    $Revision: 1.89 $
*/

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

#include <float.h>

#include "spectro_wave.h"
#include "spectral_lines.h"

/*-----------------------------------------------------------------------------
                                   Includes
 -----------------------------------------------------------------------------*/

/* Default zone to discard in pixels */
#define ZEROPIX_LE                  10
#define ZEROPIX_RI                  10
/* Beginning of thermal regime in angstroms */
#define THERMAL_START               20000.0

/* Number of coefficients in the wavelength calibration polynomial
   - also the order of the resulting error term */
#define CALIB_COEFFS  4

/* Number of iterations for the cross-correlation */
#define XCORR_NPASS                 8

/* Number of pixel steps on each side to compare
  - Twice largest delta observed so far */

   /* For 2nd degree polynomial (31 w. ISAAC.1999-11-17T13:03:23.096) */
#define XCORR_WIDTH_PIX_2           50

   /* For 3rd degree polynomial (-11 w. ISAAC.2003-09-16T12:44:32.959) */
#define XCORR_WIDTH_PIX_3           50

   /* For 3rd degree polynomial (29 w. ISAAC.2000-07-19T00:29:03.034) */
#define XCORR_WIDTH_PIX_3OH         50

   /* Whenever a good solution is evaluated (expected delay from -1 to 1) */
#define XCORR_FINE                  3

/* Number of (sub-pixel) steps for the zero degree coefficient */
#define C0_NSTEPS                   8

/* The search range for the 1st and 2nd degree coefficents is narrower when
   phdisprel is 3rd degree, because this polynomial is a better first guess */

/* Begin, end and number of steps for the first degree coefficient */
#define C12_START                   0.97
#define C12_NSTEPS                  21
#define C12_STOP                    1.02

/* Begin, end and number of steps for the second degree coefficient */
#define C22_START                   0.97
#define C22_NSTEPS                  21
#define C22_STOP                    1.02

/* Begin, end and number of steps for the first degree coefficient
   - with a margin of 50% */
#define C13_START                   0.98
#define C13_NSTEPS                  29
#define C13_STOP                    1.05

/* Begin, end and number of steps for the 2nd and 3rd degree coefficients */
#define C23_START                   0.9
#define C23_NSTEPS                  11
#define C23_STOP                    1.1

#define C3_START                    0.8
#define C3_NSTEPS                   11
#define C3_STOP                     1.1

struct bound {
    double min;   /* Lower bound for the correction */
    double max;   /* Upper bound for the correction */
    int    steps; /* Number of steps in the search  */
};

/*-----------------------------------------------------------------------------
                                   Function prototypes
 -----------------------------------------------------------------------------*/

static double get_line_offset(const spectral_table *, const pixelvalue *,
        const int, const int, const double, const int, const int, const int,
        const int, const double, const double *, double *, double *);
static computed_disprel * spectro_refine_solution(spectral_table *,
      const pixelvalue *, const int, const int, const int, const int,
      const double, const double *);
static double * wavecal_search(spectral_table *, pixelvalue *, int, int,
   const int, const int, const int, const double, const int,
   const double *, const double, struct bound *,double *);
static void wave_shift(double *, const double);

/*-----------------------------------------------------------------------------
                                Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief                  Compute a 3rd degree dispersion relation
  @param  in              Allocated spectroscopic image
  @param  discard_lo      Number of pixels to discard at the bottom
  @param  discard_hi      Number of pixels to discard at the top
  @param  discard_le      Number of pixels to discard on the left
  @param  discard_ri      Number of pixels to discard on the right
  @param  remove_thermal  Flag to force thermal background removal.
  @param  table_name      Spectral table name (see below)
  @param  slit_width      Width in pixels of the slit used
  @param  order           Order used in the spectral table look-up
  @param  phdisprel       4 polynomial coefficients as first guess (phys. mod.?)
  @return a computed_disprel object containing among others the solution poly.

  Compute a dispersion relation from a spectroscopic image showing some
  strong emission lines. A vital assumption is that strong emission lines
  can be seen in the image.

  The first guess polynomial may have degree 2. In that case the 3rd degree 
  coefficient (4th value) must be zero.

  The removal of thermal background will attempt to remove from the
  spectroscopic image any low-frequency components, i.e. any features that
  are a lot wider than the slit width.
  
  The spectral table name is a character string. Possible values are:
  
  \begin{tabular}{ll}
  "oh"            &   OH lines \\
  "Xe"            &   Xenon lines \\
  "Ar"            &   Argon lines \\
  "Xe+Ar"         &   Xenon and Argon lines \\
  "/path/file"    &   Full pathname of an ASCII table
  \end{tabular}
  
  The latter specifies an ASCII table containing the lines you want to use for 
  spectral calibration. Notice that this table must respect the format 
  described in spectral_lines.c.

  The order determines at which location in the spectral table the look-up
  for emission lines will be done.

  The output cross-correlation factor is a number between 0 and 1. A good
  fit typically results in a value in the range 0.7 to 0.95, while a
  spectroscopic image without a number of well separated strong emission
  lines typically will yield a value down to about 0.5. Early spectroscopic
  images from the ESO ISAAC instrument cross-correlate down to 0.38.
  
  Algorithm:
  \begin{itemize}
  \item Spectrum extraction along the spectrum direction (horizontal).
  For each column the lower and higher
  pixels are discarded, then a median value of the remaining pixels in the 
  column is returned. This forms a 1d signal of same size as the image in the 
  spectrum direction.
  \item If a thermal background should be removed, this is done at this point.
  \item If some values must be discarded (set to zero) in the input spectrum, 
  they are zeroed at that point.
  \item Spectral lines are retrieved from a catalog (internal).
  \item A search is done for the polynomial that maximizes the
  cross-correlation of the extracted signal and the spectral lines catalog.
  \end{itemize}

  Setting parts of the input spectrum to zero enables a correct wavelength 
  calibration in the thermal regime. You should provide -1 and -1 if you want 
  to let this function decide for you about a correct zeroing interval, (0,0) 
  if you do not want to zero the spectrum at all, and any other values 
  depending on which interval you want to set to zero on each side of the 
  spectrum.
  Notice that setting these values to (10,20) will set to zero the 10 left 
  pixels and 20 rightmost pixels of the input spectrum before throwing it into 
  the cross-correlation procedure.
*/
/*----------------------------------------------------------------------------*/
computed_disprel * spectro_compute_disprel(
        image_t *   in,
        int         discard_lo,
        int         discard_hi,
        int         discard_le,
        int         discard_ri,
        int         remove_thermal,
        char    *   table_name,
        double      slit_width,
        int         order,
        double  *   phdisprel)
{
    computed_disprel        *   solution;
    spectral_table          *   spt ;
    struct bound            *   bounds;
    double                  *   disprel ;
    double                  *   ddisprel ;
    image_t                 *   collapsed ;
    image_t                 *   thresholded ;
    pixelvalue              *   line_i ;
    pixelvalue              *   line_l ;
    pixelvalue              *   line_m ;
    pixelvalue              *   line_s ;
    double                      xcorr_max;
    int                         npix ;
    int                         gmax_w;
    double                      wl_min, wl_max;
    /* The limit for sub-pixel correction */
    double                      sublim = 0.99 ;
    int                         emil;
    int                         i ;

    /* Initialize */
    xcorr_max = 0;

    /* Sanity checks */
    if (in==NULL || table_name==NULL || phdisprel == NULL) return NULL;

    /* Initialize */
    npix =  in->lx ;
    wl_min = WAVELEN(phdisprel, 0.5);
    wl_max = WAVELEN(phdisprel, npix+0.5);

    /* Check acceptable wavelength range */
    if ((wl_min > wl_max) || (wl_min < MIN_WAVELENGTH) ||
        (wl_max > MAX_WAVELENGTH)) {
        e_error("in provided wavelength range: [%g %g] ([min max] is [%g %g])",
            wl_min, wl_max, MIN_WAVELENGTH, MAX_WAVELENGTH);
        return NULL ;
    }

    /* Get a list of spectral lines from official sources */
    if ((spt = spectral_table_init(table_name)) == NULL) {
        e_error("cannot initialize table: [%s]", table_name);
        return NULL ;
    }

    /* Get the number of lines in the catalog for the specified range */
    emil = spectral_table_count_lines(spt, wl_min, wl_max, order);
    /* Cannot afford to do much sub-pixel correction with many lines */
    if (emil > 300) sublim = 1;

    if (debug_active() >= 1) {
        e_comment(1, "Spectral order: %d", order) ;
        e_comment(1, "First guess poly. wave = f(pix) (pix in 1-%d):", npix) ;
        e_comment(1, "f(x) = %g + %g * x + %g * x^2  + %g * x^3\n", 
                phdisprel[0], phdisprel[1], phdisprel[2], phdisprel[3]) ;
        e_comment(1, "Spectral range [%g %g] with %02d out of %d lines",
                wl_min, wl_max, emil, spt->nlines);
    }

    /* Test if some lines are found in the catalog */
    if (emil < 1) {
        e_error("No line found in catalog in the specified range - abort") ; 
        spectral_table_destroy(spt);
        return NULL ;
    }

    /* Threshold the image to remove negative values */
    if ((thresholded = image_threshold(in, 0.0, MAX_PIX_VALUE, 0.0, 
                    0.0)) == NULL) {
        e_error("thresholding input image: aborting wavelength calibration");
        spectral_table_destroy(spt);
        return NULL ;
    }

    if (debug_active() >= 1) {
        const double mean_in = image_getmean(in);
        const double mean_th = image_getmean(thresholded);
        if (mean_in < mean_th) {
            e_comment(1, "Image has negative intensities (min = %g)", 
                    image_getmin(in)) ;
            e_comment(1, "Mean increased of %g (%4.2f%%) by thresholding", 
                    mean_th-mean_in, 100*(mean_th/mean_in-1));
        }
    }

    /* Get a list of lines in the image by median-collapsing it over */
    /* the horizontal direction. With a little help from the median, */
    /* should get rid of cosmics and other spiky defects. */
    if ((collapsed = image_collapse_median(thresholded, 0, discard_lo, 
                    discard_hi)) == NULL) {
        e_error("collapsing input image: aborting wavelength calibration");
        spectral_table_destroy(spt);
        image_del(thresholded) ;
        return NULL ;
    }
    image_del(thresholded) ;

    line_i  = collapsed->data ;
    /* Remove thermal background contributions above THERMAL_START */
    if ((remove_thermal != 0 && wl_max > THERMAL_START) ||
         !strcmp(table_name, "oh")) {

        line_m = malloc(npix * sizeof(pixelvalue));
        memcpy(line_m, line_i, npix * sizeof(pixelvalue));

        if (debug_active() >= 1)
            e_comment(1, "Removing low-frequency background");

        /* Filter away very wide (8 * slit_width) features - Could try to
            remove remainder at peak (ISAAC.1999-11-17T13:03:23.096) */
        /* Could be done by function1d_remove_thermalbg() if improved */
        if (image_sub_lowpass(collapsed, 0, (int)(0.5+8*slit_width)) < 0) {
            e_error("sub_lowpass failed: aborting wavelength calibration");
            image_del(collapsed) ;
            spectral_table_destroy(spt);
            free(line_m);
            return NULL ;
        }
        /* Threshold negative intensities */
        for (i=0 ; i<npix ; i++) if (line_i[i] < 0) line_i[i] = 0;
    } else {
        line_m = line_i;
    } 

    /* See if default zeroing widths have been requested */
    if (discard_le < 0 || discard_le >= npix) discard_le = ZEROPIX_LE;

    if (discard_ri < 0 || discard_le + discard_ri >= npix) {
        if (!strcmp(table_name, "oh") && wl_max > THERMAL_START) {
            /* Calibrate using OH lines */
            /* Thermal regime: discard the right side of the signal */
            discard_ri = npix/2 ;
        } else {
            /* Calibrate using standard lamps */
            discard_ri = ZEROPIX_RI;
        }
    }

    /* Zero the signal where requested */
    if ((discard_le>0) || (discard_ri>0)) {
        if (discard_le>0) {
            if (debug_active() >= 2) 
                e_comment(2, "Zeroing input %d pixels [1-%d]", 
                        discard_le, discard_le) ;
                /* The signal does no longer need to be zeroed, and is kept
                    for plotting purposes */
            /* for (i=0 ; i<discard_le ; i++) line_i[i] = 0 ; */
        }
        if (discard_ri>0) {
            if (debug_active() >= 2) 
                e_comment(2,"Zeroing input %d pixels [%d-%d]", 
                        discard_ri, npix-discard_ri+1, npix) ;
                /* The signal does no longer need to be zeroed, and is kept
                    for plotting purposes */
            /* for (i=0 ; i<discard_ri ; i++) line_i[npix-i-1] = 0 ; */
        }
    }

    /* Put less weight on the intensity by taking the logarithm */
    /* Add 1 to ensure continuity around zero */
    line_l = malloc(npix * sizeof(pixelvalue));

    for (i=0 ; i<npix ; i++)
      line_l[i] = line_i[i] > 0 ? log( 1 + line_i[i] ) : 0; /* Often zero */

    if (debug_active() >= 2) {
        /* Verify that Cross-correlation of same signal yields 1. */
        const int   idelay = 1 + slit_width; /* Some positive number */
        double      delay;
        double      xcerr = 1 - function1d_xcorrelate(line_l, npix, line_l, 
                                                        npix, idelay, &delay) ;
        e_comment(2, "Test of Cross-correlation - zero (0:%g): %g%%\n",
                  delay, 100*xcerr) ;

        /* Same - with a positive delay */
        line_s = calloc(npix, sizeof(pixelvalue));
        memcpy(line_s, &(line_l[idelay]), (npix-idelay)* sizeof(pixelvalue));

        xcerr = 1 - function1d_xcorrelate(line_l, npix,
                                          line_s, npix,
                                          2*idelay,
                                          &delay);

        e_comment(2, "Test of Cross-correlation - plus (%d:%g): %g%%\n",
                  idelay, delay, 100*xcerr);

        /* Same - with a negative delay */
        xcerr = 1 - function1d_xcorrelate(line_s, npix,
                                          line_l, npix,
                                          2*idelay,
                                          &delay);

        e_comment(2, "Test of Cross-correlation - minus (%d:%g): %g%%\n",
                  -idelay, delay, 100*xcerr);
        free(line_s);
    }

    /* Create a signal corresponding to the list of lines at a given */
    /* scale. Loop over all scales, first in a coarse pass, then in a */
    /* finer way on the second pass. */

    /* Create the array of search related data */
    bounds = malloc( CALIB_COEFFS *sizeof(*bounds));

    /* The lower bound for the offset are used to store the wavelength offset
       as detected by the delay in the cross-correlation */
    bounds[0].min   = 0;
    bounds[0].max   = 0;
    bounds[0].steps = C0_NSTEPS ;
    /* Can afford finer search with fewer lines */
    if (emil < 50) bounds[0].steps *= 2;

    if (phdisprel[3] != 0) {

        bounds[1].min   = C13_START;
        bounds[1].max   = C13_STOP;
        bounds[1].steps = C13_NSTEPS ;

        bounds[2].min   = C23_START;
        bounds[2].max   = C23_STOP;
        bounds[2].steps = C23_NSTEPS;

        bounds[3].min   = C3_START;
        bounds[3].max   = C3_STOP;
        bounds[3].steps = C3_NSTEPS;

    } else {

        bounds[1].min   = C12_START;
        bounds[1].max   = C12_STOP;
        bounds[1].steps = C12_NSTEPS ;

        if (phdisprel[2] != 0) {
            bounds[2].min   = C22_START;
            bounds[2].max   = C22_STOP;
            bounds[2].steps = C22_NSTEPS;
        } else {
            bounds[2].min   = bounds[2].max = bounds[2].steps = 1;
        }

        bounds[3].min   = bounds[3].max = bounds[3].steps = 1;
    }

    if (phdisprel[3] == 0) {
        gmax_w = XCORR_WIDTH_PIX_2;
    } else if (!strcmp(table_name, "oh")) {
        gmax_w = XCORR_WIDTH_PIX_3OH;
    } else {
        gmax_w = XCORR_WIDTH_PIX_3;
    }

    if (emil < 4) {
        bounds[3].min = bounds[3].max = bounds[3].steps = 1;
        if (emil == 3) {
            if (phdisprel[3] != 0)
                e_warning("Calibrating with 2nd degree polynomial using "
                          "just 3 emission lines from table (w. %d lines)",
                          spt->nlines);
        } else {
            bounds[2].min = bounds[2].max = bounds[2].steps = 1;
            if (emil == 2) {
                e_warning("Calibrating with 1st degree polynomial using "
                          "just 2 emission lines from table (w. %d lines)",
                          spt->nlines);
            } else {
                bounds[1].min = bounds[1].max = bounds[1].steps = 1;
                e_warning("Calibrating with 1st degree polynomial using 1st "
                          "degree coefficient from physical model and just 1 "
                          "emission line from table (w. %d lines)",
                          spt->nlines);
            }
            phdisprel[2] = 0;
        }
        phdisprel[3] = 0;
    } 

    if (debug_active() >= 1)
        for (i=0; i<CALIB_COEFFS; i++)
            e_comment(1,"Search for coefficient nb. %d: %d steps in [%g %g]", 
                    i+1, bounds[i].steps, bounds[i].min,  bounds[i].max);

    /* Initialize convergence check */
    disprel  = phdisprel;
    ddisprel = malloc( CALIB_COEFFS *sizeof(double));

    for (i=0 ; i<XCORR_NPASS ; i++) {
        double  dxcorr = xcorr_max;
        double  wl_d, wl_dlim ;
        double  wl_rat = 0 ; 
        int     imax = -1 ;
        int     ipix ;

        /* Given the search bounds, find the disprel */
        /* that maximizes the cross-correlation between the signal */
        /* generated from the catalog and the collapsed calibration signal. */
        /* The searching ranges are modified by the function so that it can */
        /* compute more accurate search at the next pass (if we iterate) */

        ddisprel[0] = disprel[0];
        ddisprel[1] = disprel[1];
        ddisprel[2] = disprel[2];
        ddisprel[3] = disprel[3];

        /* If not the first pass, free the previous pass solution */
        if (disprel != phdisprel) free(disprel);

        /* Search the new dispersion relation */
        if ((disprel = wavecal_search(spt, line_l, npix, order, discard_le,
                        discard_ri, gmax_w, slit_width, i, phdisprel, sublim,
                        bounds, &xcorr_max)) == NULL) {
            e_error("Cannot find the next polynomial solution - abort") ;
            free(line_l) ;
            image_del(collapsed);
            if (line_m != line_i) free(line_m);
            free(ddisprel);
            free(bounds);
            spectral_table_destroy(spt);
            return NULL ;
        }

        /* Determine convergence state */
        if (xcorr_max <= dxcorr) {
            /* wavecal_search is not guaranteed to improve the solution
               - use previous in that case and stop */
            xcorr_max = dxcorr;
            disprel[0] = ddisprel[0];
            disprel[1] = ddisprel[1];
            disprel[2] = ddisprel[2];
            disprel[3] = ddisprel[3];
        }
        dxcorr      = xcorr_max - dxcorr;
        ddisprel[0] = disprel[0] - ddisprel[0];
        ddisprel[1] = disprel[1] - ddisprel[1];
        ddisprel[2] = disprel[2] - ddisprel[2];
        ddisprel[3] = disprel[3] - ddisprel[3];

        /* Compute the ratio used to determine the stopping criteria */
        for (ipix = 0; ipix < npix; ipix++) {
            double ratio;
            wl_d    = WAVELEN(ddisprel, ipix+1);
            wl_dlim = WAVEDLT( disprel, ipix+1);
            ratio   = fabs(wl_d) * FLT_EPSILON > wl_dlim
                    ? 1/FLT_EPSILON : fabs(wl_d) / wl_dlim;
            if (imax < 0 || ratio > wl_rat) {
                wl_rat = ratio;
                imax = ipix;
            }
        }

        wl_d    = WAVELEN(ddisprel, imax+1);
        wl_dlim = WAVEDLT( disprel, imax+1);

        if (debug_active() >= 2) {
            e_comment(2, "Wave change at pixel %d:", imax);
            e_comment(2, "    %g / %g = %g (dxcorr: %g)",
                    wl_d, wl_dlim, wl_rat, dxcorr);

            e_comment(2, "Coeffs change: %g %g %g %g",
                    ddisprel[0], ddisprel[1], ddisprel[2], ddisprel[3]);
        }

        if (disprel[0] != 0) ddisprel[0] /= disprel[0];
        if (disprel[1] != 0) ddisprel[1] /= disprel[1];
        if (disprel[2] != 0) ddisprel[2] /= disprel[2];
        if (disprel[3] != 0) ddisprel[3] /= disprel[3];

        if (debug_active() >= 2)
            e_comment(2, "Coeffs rel. change: %g %g %g %g\n",
                      ddisprel[0], ddisprel[1], ddisprel[2], ddisprel[3]);

        /* Test for convergence 
           - at least 2 iterations
           - Biggest wavelength change less than 1/4 the wavelength width
               for all pixels
           - cross-correlation change small
           - Change in 2nd & 3rd degree coefficient bounded
           */
        if (i > 0 && wl_rat < 0.25 && npix * dxcorr < 1 &&
            fabs(ddisprel[2]) < 0.25  && fabs(ddisprel[3]) < 0.25 ) {
            if (debug_active() >= 1)
                e_comment(1, "Convergence after %d iterations", i+1);
            break;
        }
    }
    free(bounds);
    free(ddisprel);

    /* Test if it converged before the end */
    if (i == XCORR_NPASS) {
        e_error("Search for polynomial did not converge in %d iterations", 
                XCORR_NPASS) ;
        free(disprel) ;
        free(line_l) ;
        image_del(collapsed);
        if (line_m != line_i) free(line_m);
        spectral_table_destroy(spt);
        return NULL ;
    }

    if ((discard_le>0) || (discard_ri>0)) {
        double wl_min_z = wl_min;
        double wl_max_z = wl_max;
        int emil_z;
        if (discard_le>0) wl_min_z = WAVELEN(phdisprel, discard_le+0.5);
        if (discard_ri>0) wl_max_z = WAVELEN(phdisprel, npix-discard_ri+0.5);

        emil_z = spectral_table_count_lines(spt, wl_min_z, wl_max_z, order);

        if (debug_active() >= 1) e_comment(1,
            "Zeroed calibration signal [%g %g] has %d lines, dropped %d",
            wl_min_z, wl_max_z, emil_z, emil-emil_z);
    }

    /* FIXME Refining is still experimental ... */
    if (debug_active() >= 1)
        for (i=0; i<CALIB_COEFFS; i++)
            e_comment(1, "Coef nb. %d correction rate: %g / %g = %g", 
                    i+1,
                    disprel[i], phdisprel[i], phdisprel[i] != 0 ? 
                    disprel[i]/ phdisprel[i]  : disprel[i]) ;

    /* Narrow refinable range by HALF_CENTROID_DOMAIN */
    solution = spectro_refine_solution(spt, line_i, discard_le+5, discard_ri-5,
                                       npix, order, slit_width, disprel);
    if (solution == NULL) {
        free(disprel) ;
        spectral_table_destroy(spt);
        image_del(collapsed);
        if (line_m != line_i) free(line_m);
        free(line_l) ;
        return NULL;
    }

    /* Produce some ASCII files with the computed signals */
    if (debug_active() >= 2) {
        FILE * out_file ;

        /* First guess solution */
        out_file = fopen("collapsed_physical.txt", "w");
        if (out_file == NULL) {
            free(disprel) ;
            spectral_table_destroy(spt);
            image_del(collapsed);
            if (line_m != line_i) free(line_m);
            free(line_l) ;
            free(solution);
            return NULL;
        }
        for (i=0; i<npix; i++)
            fprintf(out_file, "%g\t%g\n", WAVELEN(phdisprel, i+1), line_m[i]);
        fclose(out_file) ;

        /* Computed solution */
        out_file = fopen("collapsed_calibrated.txt", "w");
        if (out_file == NULL) {
            image_del(collapsed);
            free(disprel) ;
            spectral_table_destroy(spt);
            if (line_m != line_i) free(line_m);
            free(line_l) ;
            free(solution);
            return NULL;
        }
        for (i=0; i<npix; i++)
            fprintf(out_file, "%g\t%g\n", WAVELEN(disprel, i+1), line_m[i]);
        fclose(out_file) ;

        /* Computed solution with the low frequency part removed */
        out_file = fopen("submin.txt", "w") ;
        if (out_file == NULL) {
            image_del(collapsed);
            free(disprel) ;
            spectral_table_destroy(spt);
            free(line_l) ;
            free(solution);
            return NULL;
        }
        for (i=0 ; i<npix ; i++)
            fprintf(out_file, "%g\t%g\n", WAVELEN(disprel, i+1), line_i[i]);
        fclose(out_file) ;
    }

    /* Free memory */
    if (line_m != line_i) free(line_m);
    free(line_l);
    image_del(collapsed);
    spectral_table_destroy(spt);

    /* Produce a list of spectral lines in the requested range */
    if (debug_active() >= 2)
        spectral_table_build_spectrum(table_name, "spectral_table", disprel, 
                order, slit_width, npix) ;

    /* Display results */
    if (debug_active() >= 1) {
        e_comment(1, "Computed poly. wave = f(pix) (pix in 1-%d):", npix) ;
        e_comment(1, "f(x) = %g + %g * x + %g * x^2  + %g * x^3", 
                disprel[0], disprel[1], disprel[2], disprel[3]) ;
        e_comment(1, "Spectral range [%g %g]",
                WAVELEN(disprel,1), WAVELEN(disprel,npix)) ;
    }

    /* Fill in the solution and remaining parameters */
    solution->poly   = disprel;
    solution->degree = 3;
    solution->cc     = xcorr_max;
    solution->offset = disprel[0] - phdisprel[0] ;
    solution->scal1  = disprel[1] / phdisprel[1] ;
    solution->scal2  = disprel[2] ;
    if (phdisprel[2] != 0) solution->scal2 /= phdisprel[2] ;
    solution->scal3  = disprel[3] ;
    if (phdisprel[3] != 0) solution->scal3 /= phdisprel[3] ;
    disprel          = NULL ;

    /* Return the solution */
    return solution ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief  Search for the polynomial that maximizes the Cross-correlation
  @param  spt          The list of spectral lines from official sources
  @param  line_i       The spectrum from the collapsed image
  @param  npix        Number of pixels in spectrum
  @param  order        Order
  @param  slit_width   Width (in pixels) of the slit used.
  @param  pm           Dispersion coeff. given by the physical model
  @param  bounds       array with start/stop/nsteps values (modified)
  @param  pxcorr_max    Cross-correlation factor - indicate the fit quality
  @return The wl calib coefficients or NULL
*/
/*----------------------------------------------------------------------------*/
static double * wavecal_search(
        spectral_table  *   spt,
        pixelvalue      *   line_i,
        int                 npix,
        int                 order,
        const int           discard_le,
        const int           discard_ri,
        const int           gmax_width,
        const double        slit_width,
        const int           niter,
        const double    *   pm,
        const double        sublim,
        struct bound    *   bounds,
        double          *   pxcorr_max)
{
    double     * d_t ;
    double       xcorr ;
    pixelvalue * line_t ;
    /* The delays found by cross-correlation */
    double fdelta;
    double pdelta;
    double cdelta;
    /* The number of pixels to actually cross-correlate */
    const int    dpix      = npix - discard_le - discard_ri;
    /* The last pixel for which the spectrum should be computed - only for
       optimization. Avoiding the first discard_le pixels takes a bit more */
    const int    rpix      = npix - discard_ri;
    int          p_delta   = 0; /* The best pre delay */
    int          c_delta   = 0; /* The best coarse delay */
    int          mcross    = 0; /* The number of correlations computed */

    static struct {
        double corr; /* The candidate correction of the physical model */
        double best; /* The best      correction of the physical model */
        double step; /* The step length used in the search */
        int maxpos;  /* The best step nr. */
    } * search;

    double  *   best ;
    double  *   cand;
    double      delta_wl;
    double      delta_wl_best = 0;
    double      delta_pix ;
    int         n_lines ;
    int         i0, i1, i2, i3 ;
    int         i2dir, i3dir;

    if (debug_active() >= 1)
    e_comment(1, "Pass %d. Compute best match using xcorrelation (offset %g)", 
            1+niter, bounds[0].min);

    /* Create the array of search related data */
    search = malloc( CALIB_COEFFS *sizeof(*search));

    search[0].best = 0; /* The best 0nd degree offset */
    search[1].best = 0; /* The best 1st degree scaling */
    search[2].best = 0; /* The best 2nd degree scaling */
    search[3].best = 0; /* The best 3rd degree scaling */

    search[0].step = 1 / (double) bounds[0].steps;
    search[1].step = bounds[1].steps < 2 ? 0
                   : (bounds[1].max - bounds[1].min)/(bounds[1].steps-1);
    search[2].step = bounds[2].steps < 2 ? 0
                   : (bounds[2].max - bounds[2].min)/(bounds[2].steps-1);

    search[3].step = bounds[3].steps < 2 ? 0
                   : (bounds[3].max - bounds[3].min)/(bounds[3].steps-1);

    search[0].maxpos = -1;
    search[1].maxpos = -1;
    search[2].maxpos = -1;
    search[3].maxpos = -1;

    /* Create the candidate array */
    cand = malloc( CALIB_COEFFS * sizeof(double)) ;

    /* Create the array of maximizing polynomial coefficients */
    best = malloc( CALIB_COEFFS * sizeof(double)) ;
    best[0] = pm[0];
    best[1] = pm[1];
    best[2] = pm[2];
    best[3] = pm[3];

    /* Iterate through the search space */
    i2dir = 1;
    i3dir = 1;
    pdelta = cdelta = 0;
    for (i1=0 ; i1<bounds[1].steps ; i1++) {
      int i2start = i2dir < 0 ?  0 : bounds[2].steps-1;
      int i2stop  = i2dir > 0 ? -1 : bounds[2].steps;

      int mmwidth = niter ? XCORR_FINE : gmax_width;

      i2dir = - i2dir;

      /* Cannot reuse a too large pdelta */
      if (fabs(pdelta) > 2*gmax_width) pdelta = cdelta = 0;

      for (i2=i2start ; i2 != i2stop ; i2 += i2dir) {
        /* Change only one candidate coefficient at a time so pdelta can be
           reused */
        int i3start = i3dir < 0 ?  0 : bounds[3].steps-1;
        int i3stop  = i3dir > 0 ? -1 : bounds[3].steps;

        i3dir = - i3dir;

        for (i3=i3start ; i3 != i3stop ; i3 += i3dir) {
            double substep;

            /* The biggest allowed correlation delay */
            int mwidth = mmwidth;

            mmwidth = XCORR_FINE;

            /* The expected correlation delay caused by the difference
               between the physical model and the candidate */
            pdelta += cdelta;

            /* Remove some rounding errors */
            search[1].corr = bounds[1].min + i1 * search[1].step;
            if (fabs(search[1].corr) < FLT_EPSILON * search[1].step)
                search[1].corr = 0;

            search[2].corr = bounds[2].min + i2 * search[2].step;
            if (fabs(search[2].corr) < FLT_EPSILON * search[2].step)
                search[2].corr = 0;

            search[3].corr = bounds[3].min + i3 * search[3].step;
            if (fabs(search[3].corr) < FLT_EPSILON * search[3].step)
                search[3].corr = 0;

            cand[3] = search[3].corr * pm[3];
            cand[2] = search[2].corr * pm[2];
            cand[1] = search[1].corr * pm[1];

            /* The constant term of the candidate polynomial is shifted
               so the central wavelength (at 0.5*(1+npix)) is unchanged
               This shift should really be implemented as something like
               WAVELEN(ddisprel, 0.5*(1+npix)) - ddisprel[0] */
             
            search[0].corr = 0.5*(1+npix) * ( pm[1] - cand[1]
                           + 0.5*(1+npix) * ( pm[2] - cand[2]
                           + 0.5*(1+npix) * ( pm[3] - cand[3])));

            search[0].corr += bounds[0].min; /* Add global offset */

            cand[0] = pm[0] + search[0].corr;

            /* Transform the polynomial according to the delta-shift
               - the subsequent delta should not not exceed +1 / -1 */
            wave_shift(cand, pdelta);

            /* Candidate polynomial now generated */

            d_t = spectral_table_build_signal(spt, cand, order, slit_width,
                                              rpix, &n_lines);

            if (d_t == NULL) continue;
            line_t = double2pixel_array(d_t, rpix);
            free(d_t);
            if (line_t == NULL) continue;

            mcross += 2*mwidth+1;
            xcorr = function1d_xcorrelate(&(line_t[discard_le]), dpix,
                                          &(line_i[discard_le]), dpix,
                                          mwidth, &cdelta);
            free(line_t);

            /* Look for best correlation point - and set offset */
            if (xcorr <= *pxcorr_max * sublim) continue;

            /* sub-pixel fine-tuning of offset */

            /* Transform the polynomial according to the delta-shift
               - the subsequent delta should not not exceed +1 / -1 */
            wave_shift(cand, cdelta);

            /* delta_wl is the offset due to the correlation delay */
            delta_wl = cand[0] - (pm[0] + search[0].corr);

            /* The candidate polynomial is shifted
               so the central wavelength (at 0.5*(1+npix)) is unchanged */
            substep = WAVEDLT(cand, 0.5*(1+npix));
            cand[0] -= 0.5 * substep;

            substep *= search[0].step;
            
            for (i0 = 0; i0 < bounds[0].steps; i0++) {

                cand[0] += substep;

                d_t = spectral_table_build_signal(spt, cand, order, slit_width,
                                                  rpix, &n_lines);

                if (d_t == NULL) continue;
                line_t = double2pixel_array(d_t, rpix);
                free(d_t);
                if (line_t == NULL) continue;
                mcross += 5;
                xcorr = function1d_xcorrelate(&(line_t[discard_le]), dpix,
                                              &(line_i[discard_le]), dpix,
                                              2, &fdelta);
                free(line_t);

                /* Look for best correlation point - and set offset.
                   Will only accept non-delay solutions (but why ?) */
                if (xcorr <= *pxcorr_max || fdelta != 0) continue;

                p_delta = pdelta;
                c_delta = cdelta ;

                delta_wl_best = delta_wl;

                best[0] = cand[0];
                best[1] = cand[1];
                best[2] = cand[2];
                best[3] = cand[3];

                 /* The offset correction changes in the sub-pixel search */
                search[0].best = cand[0] - pm[0];

                /* Due to the correlation correction */
                search[1].best = best[1]*FLT_EPSILON < pm[1]
                               ? best[1]/pm[1] : 0;
                search[2].best = fabs(best[2])*FLT_EPSILON < fabs(pm[2])
                               ? best[2]/pm[2] : 0;

                search[3].best = search[3].corr;

                *pxcorr_max  = xcorr ;
                search[0].maxpos = i0;
                search[1].maxpos = i1;
                search[2].maxpos = i2;
                search[3].maxpos = i3;

            }
        }
      }
    }

    free(cand);

    /* Test if no candidate produced signals with enough lines */
    if (search[1].maxpos < 0) {
        free(search);
        if (niter) return best; /* Search result not improved */
        free(best);
        e_error("No useful candidates found (%d)", n_lines);
        return NULL ;
    }

    if (debug_active() >= 2) {
        /*  Possible indicators of some fitting problem */
        if (search[2].best <= 0) e_comment(2,
            "Reversed sign on 2nd degree term: %g -> %g. %g",
            pm[2], best[2], search[2].best);
        /* Use only on 2nd degree  fit */
        if (search[2].best >= C22_STOP) e_comment(2,
            "2nd degree term scaled at upper limit: %g -> %g. %g",
            pm[2], best[2], search[2].best);
        /* Use only on 3rd degree fit */
        if (search[3].best <= C3_START) e_comment(2,
            "3rd degree term scaled at lower limit: %g -> %g. %g",
            pm[3], best[3], search[3].best);
        if (search[3].best >= C3_STOP) e_comment(2,
            "3rd degree term scaled at upper limit: %g -> %g. %g",
            pm[3], best[3], search[3].best);
        if (search[3].best <= 0) e_comment(2,
            "Reversed sign on 3rd degree term: %g -> %g. %g",
            pm[3], best[3], search[3].best);

        e_comment(2,"Correlation of %g (%d evaluations) with %d lines",
                  *pxcorr_max, mcross, n_lines);
        e_comment(2,"Delay %d+%d at search position (%d:%d:%d:%d)"
            ,p_delta, c_delta, search[0].maxpos
            ,search[1].maxpos, search[2].maxpos, search[3].maxpos);
    
        e_comment(2,"Offset %g", search[0].best);
        e_comment(2,"Scale Dim1 %g <= %g <= %g",
                  bounds[1].min, search[1].best, bounds[1].max) ;
        e_comment(2,"Scale Dim2 %g <= %g <= %g",
                  bounds[2].min, search[2].best, bounds[2].max) ;
        e_comment(2,"Scale Dim3 %g <= %g <= %g",
                  bounds[3].min, search[3].best, bounds[3].max);
    }    
    
    /* Test if the best scale is at the edge */
    if (niter == 0 && bounds[1].min < bounds[1].max && 
        (search[1].maxpos == 0 || (search[1].maxpos == bounds[1].steps - 1))) {
        free(best);
        e_error("Found best fit at limit (%d:%d:%d:%d). "
                "Increase the scale range", search[0].maxpos,
                search[1].maxpos, search[2].maxpos,search[3].maxpos);
        free(search);
        return NULL ;
    }

    /* Test if change in offset is extreme 
       - in which case the change in scale should also be extreme */
    delta_pix = fabs(search[0].best) / WAVEDLT(pm, 0.5*(1+npix));

    if (delta_pix > gmax_width) {
        free(best);
        e_error("Constant term has changed too much (wl %g): %g > %d pixels "
                "(scale: %g)", search[0].best, delta_pix, gmax_width,
                search[1].best);
        free(search);
        return NULL ;
    }

    /* Update search boundaries for the next pass */
    bounds[1].min = search[1].best - search[1].step;
    bounds[1].max = search[1].best + search[1].step;

    bounds[2].min = search[2].best - search[2].step;
    bounds[2].max = search[2].best + search[2].step;    

    bounds[3].min = search[3].best - search[3].step;
    bounds[3].max = search[3].best + search[3].step;    

    /* Remove rounding in case boundary should be zero
       - only relevent when coefficient can change sign */
    if (fabs(bounds[2].min) < FLT_EPSILON*search[2].step) bounds[2].min = 0;
    if (fabs(bounds[2].max) < FLT_EPSILON*search[2].step) bounds[2].max = 0;

    free(search);

    bounds[0].min += delta_wl_best;

    return best ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief  Generate polynomial coefficients of p(x + h)
  @param  p  Four coefficients of a 3rd degree polynomial
  @param  h  The shift

  The coefficients of a 3rd degree polynomial are changed according to a shift
  of h in the abscissa.

*/
/*----------------------------------------------------------------------------*/

static void wave_shift(double * p, const double h)
{

    p[0] = WAVELEN(p, h);
    p[1] = WAVEDIF(p, h);
    p[2] = p[2] + 3 * h * p[3]; /* Half the second order derivative */

}



/*----------------------------------------------------------------------------*/
/**
  @brief  EXPERIMENTAL: Distance from the pixel with the peak to the real peak
  @param  spt          The list of spectral lines from official sources
  @param  line_i       The spectrum from the collapsed image
  @param  pix_low      First pixel to consider
  @param  ipix         the pixel with the (locally) maximum intensity
  @param  pix_high     Last pixel to consider
  @param  iline        The index of the current emission line in the list
  @param  order        Order
  @param  nline        The number of the current emission line
  @param  slit_width   Width (in pixels) of the slit used.
  @param  disprel      Dispersion 3rd degree polynomial
  @param  poffset      pointer to the subpixel offset
  @return -1 on error, otherwise the absolute value of the offset

*/
/*----------------------------------------------------------------------------*/
static double get_line_offset(
        const spectral_table * spt,
        const pixelvalue     * line_i,
        const int              pix_low,
        const int              ipix,
        const double           isub,
        const int              pix_high,
        const int              iline,
        const int              order,
        const int              nline,
        const double           slit_width,
        const double         * disprel,
        double               * poffset,
        double               * pmaxval)
{

    double       offset;
    const double wl = (spt->lines[iline]).wavel * order;
    /* The expected location with sub-pixel precision */
    const double spix = 1+ipix + isub;
    double       lastval;
    double       relint;
    const double sigma  = slit_width * SLITWIDTH_TO_SIGMA;
    /* Tolerance for noise at emission-line center */
    const double centernoise = 1.25;

    const int    mpix = pix_high-pix_low+1;

    const int maxdist = slit_width;
    
    pixelvalue maxval = line_i[ipix];
    int maxpos        = ipix;
    int i;

    int        imin, imax;


    *pmaxval = -1;

    if (sigma <= 0) return -1;

    if (mpix <= 2) return -1;

    for (i=pix_low; i<=pix_high; i++)
        if (line_i[i] > maxval) maxval = line_i[maxpos = i];

    *pmaxval = maxval;

    if (maxval == 0 ) {
        if (debug_active()>2)
        e_comment(2, "0LINE %d (%g) at pixel: %d <= %g <= %d",
                  nline, wl, 1+pix_low, spix, 1+pix_high);
        return -1;
    }

    lastval = maxval;
    imax    = maxpos;

    while (imax+1 <= pix_high && (line_i[imax+1] < lastval || line_i[imax+1] ==0
          || (imax-maxpos < maxdist && line_i[imax+1] < centernoise * lastval)))
        lastval = line_i[++imax];

    lastval = maxval;
    imin    = maxpos;

    while (imin-1 >= pix_low && (line_i[imin-1] < lastval || line_i[imin-1] == 0
         || (maxpos-imin < maxdist &&  line_i[imin-1] < centernoise * lastval)))
        lastval = line_i[--imin];

    offset = function1d_find_centroid( (pixelvalue*) &(line_i[imin]),
                                       imax-imin+1);

    if (abs(maxpos - ipix) > maxdist ) {
        /* This must be another (unidenfitied) line which is
           brighter than the catalog line in this interval */
        const int plow  = maxpos > ipix ? pix_low  : (maxpos + ipix)/2;
        const int phigh = maxpos < ipix ? pix_high : (maxpos + ipix)/2;
        if (debug_active()>1) {
            if (offset < 0) {
                e_comment(2, "LINE %d (%g) at pixel: %d <= %g/%d <= %d", nline,
                          WAVELEN(disprel,1+maxpos), 1+pix_low, spix, 1+maxpos,
                          1+pix_high);
            } else {
                offset -= isub;
                e_comment(2, "LIne %d (%g) at pixel: %d <= %g/%d/%g <= %d",
                          nline, WAVELEN(disprel,1+offset+imin), 1+pix_low,
                          spix, 1+maxpos, 1+offset+imin, 1+pix_high);
            }
        }
        /* Try to find the catalog line */
        return get_line_offset(spt, line_i, plow, ipix,
                                isub, phigh, iline, order, nline,
                                slit_width, disprel, poffset, pmaxval);
    }

    relint = 100*(spt->lines[iline]).intens/(sigma*sqrt(2*4*atan(1))*maxval);

    if (offset < 0) {
        /* maxpos too close to boundary or something like that */
        if (debug_active()>2)
        e_comment(2, "LINe %d (%g) at pixel: %d/%d <= %g/%d <= %d/%d "
                  "(%4.2f%%)\n", nline, wl, 1+pix_low, 1+imin, spix, 1+maxpos,
                  1+imax, 1+pix_high, relint);
        return -1;
    }

    offset -= isub;
    *poffset = offset + imin - ipix;

    if (debug_active()>1) e_comment(2,
        "Line %d (%g) at pixel: %d/%d <= %g/%d/%g <= %d/%d (%4.2f%%) %g",
        nline, wl, 1+pix_low, 1+imin, spix, 1+maxpos, 1+offset + imin,
        1+imax, 1+pix_high, relint, *poffset);

    return fabs(*poffset);

}


/*----------------------------------------------------------------------------*/
/**
  @brief  EXPERIMENTAL: Locate the emissionlines with sub-pixel precision
  @param  spt          The list of spectral lines from official sources
  @param  line_i       The spectrum from the collapsed image
  @param  discard_le   Number of pixels to discard on the left
  @param  discard_ri   Number of pixels to discard on the left
  @param  npix         Number of pixels in spectrum
  @param  order        Order
  @param  slit_width   Width (in pixels) of the slit used.
  @param  disprel      Dispersion 3rd degree polynomial
  @return void

*/
/*----------------------------------------------------------------------------*/
static computed_disprel * spectro_refine_solution(
        spectral_table   * spt,
        const pixelvalue * line_i,
        const int          discard_le,
        const int          discard_ri,
        const int          npix,
        const int          order,
        const double       slit_width,
        const double     * disprel)
{
    computed_disprel * solution;
    const int istart = discard_le > 0 ? discard_le : 0;
    const int istop  = discard_ri > 0 ? npix-discard_ri : npix;

    const double wl_min  = WAVELEN(disprel, 0 + 0.5);
    const double wl_max  = WAVELEN(disprel, npix  + 1.5);

    double isub = 0;
    double sum_offset = 0;
    double sum_aboffs = 0;
    double sum_sqoffs = 0;
    double offset;
    double maxval;
    double maxint = 0;
    double faint = 0;

    /* double3 * fixpoints; */

    int pix_high = 0;
    int ical = 0;

    int iline = 0;

    const int emil  = spectral_table_count_lines(spt, wl_min, wl_max, order);
    const int emilz = spectral_table_count_linez(spt, wl_min, wl_max, order);
    int nline  = 0;
    int nfound = 0;
    int nzero  = 0;


    if (emil < 1) return NULL;

    solution = malloc(sizeof(*solution));

     
    /* fixpoints = double3_new(emilz); */

    /* Find first line within range */
    while (iline < spt->nlines &&
           (spt->lines[iline]).wavel*order < wl_min) iline++;

    /* Do not try to locate the very faint lines on a very crowded spectrum */
    if (emilz * (11 + slit_width) > npix) {
        int i;

        /* Find the maximum intensity */
        for (i=iline ; i<spt->nlines ; i++) {
            if ((spt->lines[i]).wavel*order > wl_max) break;
            if ((spt->lines[i]).intens > maxint) maxint =(spt->lines[i]).intens;
        }
        /* Ignore lines with intensity less than 0.01 of the maximum */
        faint = 0.01;
        if (debug_active()>1) e_comment(2,
            "No line detection of faint lines (I < %g) in crowded spectrum: %g",
             maxint * faint, npix/(emilz * slit_width));
    }

    while (pix_high < npix - 1) {
        double       wl;
        const double isub_prev  = isub;
        const int    icalprev   = ical;
        const int    pix_low    = pix_high;

        /* Do not try to locate the very faint lines
           - zero-intensity means unknown intensity - not to be ignored
           - also ignore the duplicate OH sky lines */
        while (iline<spt->nlines && 0 < (spt->lines[iline]).intens &&
               ((spt->lines[iline]).intens < maxint * faint ||
                ((spt->lines[iline]).intens == (spt->lines[iline-1]).intens &&
                ((spt->lines[iline]).wavel - (spt->lines[iline-1]).wavel)*order
                < 0.5*WAVEDLT(disprel, 1+icalprev)*slit_width))) {
            if (debug_active()>2) e_comment(2,"Skipping line(%d): %g %g", iline,
                         (spt->lines[iline]).wavel, (spt->lines[iline]).intens);
            iline++;
        }

        if (iline < spt->nlines &&
           /* The wavelength of the emission line */
           (wl = (spt->lines[iline]).wavel*order) < wl_max) {
            double wl_low, wl_high;
            double delta;
            int i=3;

            /* Find the expected pixel of the emission line
                - sample nr. x (with index i = x-1) has wavelengths
                from p(x-0.5) to p(x+0.5) */

            ical--; /* More than one wavelength can belong to a pixel */

            while (++ical < npix && WAVELEN(disprel, ical+1 + 0.5) < wl);
            wl_low  = WAVELEN(disprel, ical+1 - 0.5);
            wl_high = WAVELEN(disprel, ical+1 + 0.5);

            /* -0.5 <= isub < 0.5 */
            isub = (wl - wl_low) / (wl_high - wl_low) - 0.5;
            /* Just since its easy: Determine isub with maximum precision
               - using Newton-Raphson */
            do {
                delta = (WAVELEN(disprel, ical+1+isub)-wl)
                      /  WAVEDIF(disprel, ical+1+isub);
                isub -= delta;
            } while (--i && fabs(delta) > DBL_EPSILON);
            /* At this point delta is most likely true 0 :-) */

            pix_high = (ical + icalprev)/2;

        } else {
            pix_high = npix - 1;
        }

        if (nline > 0) {
            if (get_line_offset(spt, line_i, pix_low, icalprev, isub_prev,
                                pix_high, iline-1, order, nline, slit_width,
                                disprel, &offset, &maxval) >=0) {
                if (istart <= icalprev && icalprev <= istop) {
                    /* Only include lines that are not in the discard range */
                    sum_offset += offset;
                    sum_aboffs += fabs(offset);
                    sum_sqoffs += offset * offset;
                    /* Don't need these yet 
                    fixpoints->x[nfound] = 1 + icalprev + offset;
                    fixpoints->y[nfound] = WAVELEN(disprel, fixpoints->x[nfound]);
                    */
                    nfound++;
                }
            }
            if (maxval == 0) nzero++;
        }

        iline++;
        nline++;
    }

    /* double3_dump(fixpoints, stderr); */

    solution->dlines = nfound;
    solution->clines = emilz;
    solution->rms    = -1;
    solution->mean   = -1;

    if (nfound > 0) {
        double stdev = nfound ==1 ? 0 :
                       (sum_sqoffs-sum_offset*sum_offset/nfound)/(nfound-1);
        stdev = stdev > 0 ? sqrt(stdev) : 0;
        
        if (debug_active() >= 1) e_comment(2,
            "Mean & RMS pixel-offset on calibration (%d:%d:%d:%d): %g %g", 
            emilz, emilz-emil,nzero, nfound, sum_aboffs/nfound, stdev);

        /* The bias-corrected standard deviation */
        solution->rms    = stdev;
        /* The mean deviation */
        solution->mean   = sum_aboffs/nfound;
    }

    /* double3_del(fixpoints); */

    return solution;

}

#undef ZEROPIX_LE         
#undef ZEROPIX_RI        
#undef THERMAL_START
#undef XCORR_NPASS
#undef XCORR_WIDTH_PIX_2
#undef XCORR_WIDTH_PIX_3
#undef XCORR_WIDTH_PIX_3OH
#undef XCORR_FINE
#undef C0_NSTEPS
#undef C1_NSTEPS
#undef C1_START
#undef C1_STOP          
#undef C2_NSTEPS
#undef C2_START
#undef C2_STOP          
#undef CALIB_COEFFS
