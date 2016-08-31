/*----------------------------------------------------------------------------*/
/**
   @file    spjextract.c
   @author  Y. Jung
   @date    Jan. 2003
   @version	$Revision: 1.5 $
   @brief   Spectroscopic jitter spectrum extraction
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjextract.c,v 1.5 2003/11/19 12:02:52 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/19 12:02:52 $
	$Revision: 1.5 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a spectrum from a combined image
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_extract(spjitter_config_t * spjc)
{
    double              sky_signal ;
    double3         *   position ;
    int                 low_side,
                        up_side ;
    int                 lo_dist,
                        hi_dist,
                        lo_width,
                        hi_width ;
    int                 sky_pos[4] ;
    image_t         *   filtered ;
    pixelvalue          res_sky_estim ;
    image_t         *   extr_line ;
    double              median_1,
                        median_2 ;
    int                 i ;
    
    /* First test if the spectrum extraction is requested */
    if (spjc->spectrum_extr_active == 0) {
        spjc->status_extraction = SKIPPED ;
        return 0 ;
    }

    /* Check if the combine image has been computed */
    if (spjc->combined == NULL) {
        e_warning("No combined image - cannot extract the spectrum") ;
        spjc->status_extraction = SKIPPED ;
        return -1 ;
    }
    
    /* Initialize */
    sky_signal = 0.0 ;
    lo_dist = spjc->res_sky_lo_dist ;
    hi_dist = spjc->res_sky_hi_dist ;
    lo_width = spjc->res_sky_lo_width ;
    hi_width = spjc->res_sky_hi_width ;

    /* Try to detect the brightest spectrum if its position is unknown */
    if (spjc->spectrum_position < 0) {
        i = 0 ;
        /* Try to find 2 black shadows */
        position = NULL ;
        while ((position == NULL) && (i < spjc->nobjframes)) {
            if ((position=find_brightest_spectrum_1d(spjc->combined,
                            (int)fabs(spjc->main_offset_diff[i]),
                            EQUALLY_SPACED_SHADOW_SPECTRA,
                            0)) == NULL) {
                if (i == spjc->nobjframes - 1) {
                    e_warning("Detection failed - try with lower criteria") ;
                } else {
                    e_warning("Detection failed - try with next offset") ;
                }
                i++ ;
            }
        }

        /* Try with 1 black shadow   */
        if (position == NULL) {
            i = 0 ;
            while ((position == NULL) && (i < spjc->nobjframes)) {
                if ((position=find_brightest_spectrum_1d(spjc->combined,
                                (int)fabs(spjc->main_offset_diff[i]),
                                ONE_SHADOW_SPECTRUM,
                                0)) == NULL) {
                    if (i < spjc->nobjframes - 1) {
                        e_warning("Detection failed - try with next offset") ;
                    }
                    i++ ;
                }
            }
        }

        /* Exit if detection fails */
        if (position == NULL) {
            spjc->spectrum_detected = 0 ;
            spjc->spectrum_extracted = 0 ;
            spjc->status_extraction = SKIPPED ;
            return -1 ;
        } else spjc->spectrum_detected = 1 ;

        /* Set the spectrum position */
        spjc->spectrum_position = (int)(position->y[0]) ;
        double3_del(position) ;

    /* Otherwise use the position specified in the INI file */
    } else spjc->spectrum_detected = 0 ;

    /* Set the parameters for the extraction */

    /* Spectrum position */
    low_side = (int)(spjc->spectrum_position - (spjc->spectrum_width/2)) ;
    up_side = low_side + spjc->spectrum_width ;
    if ((low_side < 1) || (up_side > spjc->ly)) {
        e_error("spectrum position out of the image - aborting") ;
        spjc->spectrum_extracted = 0 ;
        spjc->status_extraction = FAILED ;
        return -1 ;
    }

    /* Residual Sky position */
    if (lo_dist < 0) lo_dist = 2*spjc->spectrum_width ;
    if (hi_dist < 0) hi_dist = 2*spjc->spectrum_width ;
    if (lo_width < 0) lo_width = 10 ;
    if (hi_width < 0) hi_width = 10 ;
    sky_pos[1] = (int)(spjc->spectrum_position - lo_dist) ;
    sky_pos[0] = (int)(sky_pos[1] - lo_width) ;
    sky_pos[2] = (int)(spjc->spectrum_position + hi_dist) ;
    sky_pos[3] = (int)(sky_pos[2] + hi_width) ;

    /* Allocate extracted array */
    spjc->extracted_values = malloc(spjc->lx * sizeof(double)) ;
    spjc->extr_x_coordinate = malloc(spjc->lx * sizeof(double)) ;
    spjc->sky_signal = calloc(spjc->lx, sizeof(double)) ;

    /* Filter the combined image if requested */
    if (spjc->apply_filter == 1) filtered=image_filter_median(spjc->combined);
    else                         filtered=image_copy(spjc->combined) ;

    /* Extract the spectrum and get rid of the residual sky */
    for (i=0 ; i<spjc->lx ; i++) {
        /* Estimate the SKY */
        if (((sky_pos[0] < 1) || (lo_width == 0)) &&
                ((sky_pos[3] <= spjc->ly) && (hi_width > 0))) {
            res_sky_estim = image_getmedian_vig(filtered,
                    i+1,
                    sky_pos[2],
                    i+1,
                    sky_pos[3]) ;
            if (spjc->sky_lines != NULL) {
                spjc->sky_signal[i] = image_getmedian_vig(spjc->sky_lines,
                        i+1,
                        sky_pos[2],
                        i+1,
                        sky_pos[3]) ;
            }
        } else if (((sky_pos[3] > spjc->ly) || (hi_width == 0))
                && ((sky_pos[0] > 0) && (lo_width > 0))) {
            res_sky_estim = image_getmedian_vig(filtered,
                    i+1,
                    sky_pos[0],
                    i+1,
                    sky_pos[1]) ;
            if (spjc->sky_lines != NULL) {
                spjc->sky_signal[i] = image_getmedian_vig(spjc->sky_lines,
                        i+1,
                        sky_pos[0],
                        i+1,
                        sky_pos[1]) ;
            }
        } else if ((lo_width != 0) && (hi_width != 0)
                && (sky_pos[0] > 0) && (sky_pos[3] <= spjc->ly)) {
            median_1 = image_getmedian_vig(filtered,
                    i+1,
                    sky_pos[0],
                    i+1,
                    sky_pos[1]) ;
            median_2 = image_getmedian_vig(filtered,
                    i+1,
                    sky_pos[2],
                    i+1,
                    sky_pos[3]) ;
            res_sky_estim=(median_1 + median_2)/2 ;
            if (spjc->sky_lines != NULL) {
                median_1 = image_getmedian_vig(spjc->sky_lines,
                        i+1,
                        sky_pos[0],
                        i+1,
                        sky_pos[1]) ;
                median_2 = image_getmedian_vig(spjc->sky_lines,
                        i+1,
                        sky_pos[2],
                        i+1,
                        sky_pos[3]) ;
                spjc->sky_signal[i] = (median_1 + median_2)/2 ;
            }
        } else {
            res_sky_estim = 0 ;
            spjc->sky_signal[i] = 0 ;
        }

        /* Estimate the spectrum */
        if ((extr_line = image_getvig(filtered,
                        i+1,
                        low_side,
                        i+1,
                        up_side)) == NULL) {
            e_error("error in line extraction - aborting") ;
            spjc->spectrum_extracted = 0 ;
            spjc->status_extraction = FAILED ;
            image_del(filtered) ;
            return -1 ;
        }

        /* Write in output double3 */
        spjc->extracted_values[i] = (double)image_getsumpix(extr_line) ;
        image_del(extr_line);
        spjc->extracted_values[i] -= spjc->spectrum_width * res_sky_estim ;
        if (spjc->status_wavecal_done == OK) {
            spjc->extr_x_coordinate[i] =
                (double)((spjc->wavecal_disprel)->poly[0] +
                    (spjc->wavecal_disprel)->poly[1]*(i+1)+
                    (spjc->wavecal_disprel)->poly[2]*(i+1)*(i+1)+
                    (spjc->wavecal_disprel)->poly[3]*(i+1)*(i+1)*(i+1)) ;
        } else spjc->extr_x_coordinate[i]=(double)(i+1) ;
    }
    spjc->spectrum_extracted = 1 ;
    spjc->status_extraction = OK ;

    /* Free and return */
    image_del(filtered) ;
    return 0 ;
}
