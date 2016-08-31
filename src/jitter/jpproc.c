/*----------------------------------------------------------------------------*/
/**
   @file    jpproc.c
   @author  Y. Jung
   @date    March 2002
   @version	$Revision: 1.29 $
   @brief   Jitter post-processing
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jpproc.c,v 1.29 2005/03/14 15:01:47 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/14 15:01:47 $
	$Revision: 1.29 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "pfits.h"
#include "pfitspro.h"

#include "jtypes.h"
#include "jconfig.h"
#include "jload.h"

/*-----------------------------------------------------------------------------
   						Instrument defaults	
 -----------------------------------------------------------------------------*/

static double rseeing_isaac[2]   = {0.1, 5.0};
static double rseeing_naco[2]    = {0.025, 3.0};
static double rseeing_default[2] = {0.1, 5.0};

/*-----------------------------------------------------------------------------
   								Functions prototypes
 -----------------------------------------------------------------------------*/

static int jitter_qc(jitter_config_t * jc) ;

/*-----------------------------------------------------------------------------
   								Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Jitter post-processing
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 if error occurred.

  Apply various post-processing filters to output data.
 */
/*----------------------------------------------------------------------------*/
int jitter_postproc(jitter_config_t * jc)
{
    int     i ;

    /* Test if the post-processing is requested or not */
    if (jc->pproc_active == 0) {
        jc->status_postproc = ALGO_SKIPPED ;
        return 0 ;
    }
 
    /* Test if the median subtraction has to be applied */
    if (jc->pproc_rowmediansub) { 
        e_comment(1, "subtracting median from each row") ;
        if (jc->final == NULL) {
            /* Do the subtraction on all object frames */
            for (i=0 ; i<jc->nframes ; i++) {
                if (jc->frame[i].type == type_obj) {
                    if (image_sub_rowmedian(jc->final) != 0) {
                        e_error("during median subtraction from each row") ;
                        jc->status_postproc = ALGO_FAILED ;
                        return -1 ;
                    }
                }
            }
        } else {
            /* Do the subtraction the combined frame */
            if (image_sub_rowmedian(jc->final) != 0) {
                    e_error("during median subtraction from each row") ;
                    jc->status_postproc = ALGO_FAILED ;
                    return -1 ;
            }
        }
    }
  
    /* Output the QC paf file */
    if (jitter_qc(jc)!=0) {
        e_error("cannot create QC PAF file") ;
        jc->status_postproc = ALGO_FAILED ;
        return -1 ;
    }
  
    /* Update the status to OK */
    jc->status_postproc = ALGO_OK ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	View jitter results
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 otherwise

  Launch viewer command requested in ini file to see the results
  of the jitter run.
 */
/*----------------------------------------------------------------------------*/
int jitter_viewer(jitter_config_t * jc)
{
    char                filename[FILENAMESZ] ;

    if (jc->pproc_startviewer == 0) return -1 ;

    sprintf(filename, "%s.fits", jc->output_basename);
    show_image(filename, jc->pproc_viewer) ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Quality control parameters derived from the output image    
  @param    jc      jitter_config object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int jitter_qc(jitter_config_t * jc)
{
    FILE        *   paf ;
    char            pafname[FILENAMESZ] ;
    char        *   strvar ;
    detected    *   det ;
    double          fwhm_avg_med ;
    double          pix_scale ;
    double          iq ;
    double          sum, sqsum, bg_mean, bg_stdev, bg_instmag, dit ;
    int             i ;
    char            prodname[ASCIILINESZ];
    double      *   rseeing ;

    e_comment(1, "creating output PAF file for QC1...");
    sprintf(pafname, "%s_qc.paf", get_rootname(jc->output_basename)) ;
    sprintf(prodname, "%s/jitter", jconv_ins(jc->data_type));
    paf = qfits_paf_print_header(pafname,
                                 prodname,
                                 "jitter recipe results",
                                 get_login_name(),
                                 get_datetime_iso8601()) ;
    if (paf==NULL) {
        e_warning("cannot create PAF file: no QC output") ;
        return 0 ;
    }

    /* Instrument-specific stuff: get default seeing range. */
    switch ((jc->data_type).ins) {
        case instrument_isaac: rseeing = rseeing_isaac ;   break ;
        case instrument_naco:  rseeing = rseeing_naco ;    break ;
        default:               rseeing = rseeing_default ; break ;
    }

    fprintf(paf, "\n");
    /* MJD-OBS */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "mjdobs") ;
    if (strvar!=NULL) {
        fprintf(paf, "MJD-OBS  %s; # Obs start\n\n", strvar);
    } else {
        fprintf(paf, "MJD-OBS  0.0; # Obs start unknown\n\n");
    }
    /* ARCFILE keyword  */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "arcfile") ;
    if (strvar != NULL) {
        fprintf(paf, "ARCFILE       \"%s\"\n", strvar) ;
    }
    /* INSTRUME keyword  */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "instrument") ;
    if (strvar != NULL) {
        fprintf(paf, "INSTRUME       \"%s\"\n", strvar) ;
    }
    /* TPL.ID  */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "templateid") ;
    if (strvar != NULL) {
        fprintf(paf, "TPL.ID         \"%s\"\n", strvar) ;
    }
    /* TPL.NEXP */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "numbexp") ;
    if (strvar != NULL) {
        fprintf(paf, "TPL.NEXP       %s\n", strvar) ;
    }
    /* DPR.CATG */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "dpr_catg") ;
    if (strvar != NULL) {
        fprintf(paf, "DPR.CATG       \"%s\"\n", strvar) ;
    }
    /* DPR.TYPE */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "dpr_type") ;
    if (strvar != NULL) {
        fprintf(paf, "DPR.TYPE       \"%s\"\n", strvar) ;
    }
    /* DPR.TECH */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "dpr_tech") ;
    if (strvar != NULL) {
        fprintf(paf, "DPR.TECH       \"%s\"\n", strvar) ;
    }
    /* Add PRO.CATG */
    /* Default */
    strvar = pfits_getprokey(jc->data_type, procat_imag_jitter_qc) ;
    if (strvar!=NULL) {
        fprintf(paf, "PRO.CATG       \"%s\" ;# Product category\n", strvar);
    }
    /* Add the date */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "date_obs") ;
    if (strvar!=NULL) {
        fprintf(paf, "DATE-OBS       \"%s\" ;# Date\n", strvar);
    }
    /* Add OBS.ID */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "obs_id") ;
    if (strvar!=NULL) {
        fprintf(paf, "OBS.ID         %s ;# Obs id\n", strvar);
    }
    /* Add INS.PIXSCALE */
    pix_scale = -1.0 ;
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "pixscale") ;
    if (strvar != NULL) {
        pix_scale = (double)atof(strvar);
        fprintf(paf, "INS.PIXSCALE   %s\n", strvar) ;
    }

    /* Write the stats in the PAF file */

    /* Write out sky background measurements if any */
    sum = sqsum = 0.0 ;
    e_comment(2, "printing out sky background measurements...");
    fprintf(paf, "\n\nJITTER.SKYBG.START\n");
    for (i=0 ; i<jc->nframes ; i++) {
        fprintf(paf, "%g\n", jc->frame[i].skyval);
        sum += jc->frame[i].skyval ;
        sqsum += jc->frame[i].skyval * jc->frame[i].skyval ;
    }
    bg_mean = sum / jc->nframes ;
    if (jc->nframes>1) 
        bg_stdev = (sqsum - (sum*sum)/jc->nframes) / (jc->nframes - 1) ;
    else
        bg_stdev = -1.0 ;
    fprintf(paf, "JITTER.SKYBG.END\n\n\n");

    /* Compute statistics on output jittered frame */
    e_comment(2, "detecting objects on final frame...");
    if ((det = detected_ks_engine(jc->final,
                                    DETECTED_KAPPA,
                                    0)) == NULL) {
        e_warning("cannot find objects on result frame...") ;
        fwhm_avg_med = -1.0 ;
    } else {
        e_comment(2, "computing median FWHM on final frame...");
        /* Compute FWHMs */
        detected_compute_fwhm(det, jc->final);
        /* Display results in PAF file */
        fprintf(paf, "JITTER.OBJECTS.START\n") ;
        detected_dump(det, paf);
        fprintf(paf, "JITTER.OBJECTS.END\n") ;
        /* Save median FWHM in frame */
        fwhm_avg_med = det->fwhm_meda ;

        /* Compute image quality */
        if (pix_scale>0.0) {
            iq = detected_compute_iq(det, pix_scale, rseeing);
        } else {
            iq = -1.0 ;
        }
        detected_del(det) ;
    }
    fprintf(paf, "\n\n");

    fprintf(paf, "QC.BACKGD.MEAN     %g \n", bg_mean) ;
    fprintf(paf, "QC.BACKGD.STDEV    %g \n", bg_stdev) ;
    /* Get the DIT */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "dit") ;
    if (strvar != NULL) 
        dit = (double)atof(strvar);
    else 
        dit = -1.0 ;
    if (dit>0 && pix_scale>0 && bg_mean>0) {
        bg_instmag = -2.5 * log(bg_mean/(pix_scale*pix_scale*dit)) ;
        fprintf(paf, "QC.BACKGD.INSTMAG  %g \n", bg_instmag) ;
    }
    
    if (fwhm_avg_med>0.0) {
        e_comment(2, "median FWHM: %g pixels", fwhm_avg_med);
        fprintf(paf, "QC.FWHM.PIX    %g \n", fwhm_avg_med) ;
    }

    /* Median FWHM in arcsec */
    if ((pix_scale>0.0) && (fwhm_avg_med>0.0)) {
        e_comment(2, "median FWHM: %g arcsec", fwhm_avg_med * pix_scale);
        fprintf(paf, "QC.FWHM.ARCSEC %g\n", fwhm_avg_med * pix_scale) ;
    }

    /* Image quality in arcsec */
    if (iq>0.0) {
        e_comment(2, "image quality: %g arcsec", iq);
        fprintf(paf, "QC.IQ          %g\n", iq);
    } else { 
        fprintf(paf, "QC.IQ          --\n");

    }
      
    /* Add FILTER */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "filter") ;
    if (strvar!=NULL) {
        fprintf(paf, "QC.FILTER.OBS  \"%s\"\n", strvar);
    }

    /* Add FILTER.NDENS */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "opti3_name") ;
    if (strvar!=NULL) {
        fprintf(paf, "QC.FILTER.NDENS \"%s\"\n", strvar);
    }

    /* Add FILTER.POL */
    strvar = pfits_get(jc->data_type, jc->frame[0].name, "opti4_id") ;
    if (strvar!=NULL) {
        fprintf(paf, "QC.FILTER.POL  \"%s\"\n", strvar);
    }

    e_comment(1, "output PAF file complete");
    fclose(paf);
    return 0 ;
}



