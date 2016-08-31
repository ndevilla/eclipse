/*----------------------------------------------------------------------------*/
/**
   @file    jsave.c
   @author
   @date    March 2002
   @version	$Revision: 1.19 $
   @brief   Jitter result save
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jsave.c,v 1.19 2002/12/12 13:02:56 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/12 13:02:56 $
	$Revision: 1.19 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"

#include "jtypes.h"
#include "jconfig.h"
#include "pfits.h"
#include "jload.h"

/*-----------------------------------------------------------------------------
   						    Functions prototypes 
 -----------------------------------------------------------------------------*/

static void jitter_add_comments(jitter_config_t *, qfits_header *) ;

/*-----------------------------------------------------------------------------
                            Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Add PRO keywords in a FITS header
  @param    jc          Jitter config object.
  @param    fh          FITS header to update.      
  @param    cat         pro category id 
  @return   void
 */
/*----------------------------------------------------------------------------*/
void jitter_add_pro_keys(
        jitter_config_t *   jc, 
        qfits_header    *   fh,
        procat              cat) 
{
    char        sval[1024] ;
    char        cval[80];
    char        cval2[80];
    int         nb ;

    char    *   arcfile ;
    int         i ;

    /* Find the new nb in RECnb */
    nb = 1 ;

    if (fh==NULL) return ;

    /* Parameter Name:    PIPEFILE */
    sprintf(sval, "%s.fits", get_basename(jc->output_basename)) ;
    qfits_header_add(fh, "PIPEFILE", sval, "pipeline filename", NULL);

    /* Parameter Name:    PRO TYPE */
    qfits_header_add(fh, "HIERARCH ESO PRO TYPE", "REDUCED", "product type", 
            NULL) ;

    /* Parameter Name:    PRO STATUS */
    qfits_header_add(fh, "HIERARCH ESO PRO STATUS", "OK", "pipeline status", 
            NULL) ;

   /* Parameter Name:    PRO DATE */
    qfits_header_add(fh, "HIERARCH ESO PRO DATE", get_date_iso8601(), 
            "pipeline execution date", NULL) ;

    /* Parameter Name:    PRO DATANCOM */
    sprintf(sval, "%d", jc->nframes) ;
    qfits_header_add(fh, "HIERARCH ESO PRO DATANCOM", sval, 
            "# of combined frames", NULL);

    /* Parameter Name:    PRO CATG */
    qfits_header_add(fh, "HIERARCH ESO PRO CATG", 
           pfits_getprokey(jc->data_type, cat), "product category", NULL);

    /* Parameter Name:    PRO RECi ID */
    sprintf(cval, "HIERARCH ESO PRO REC%d ID", nb) ;
    qfits_header_add(fh, cval, "img_jitter", "recipe ID", NULL);

    /* Parameter Name:    PRO RECi DRS ID */
    sprintf(cval2, "eclipse-%s", get_eclipse_version());
    sprintf(cval, "HIERARCH ESO PRO REC%d DRS ID", nb) ;
    qfits_header_add(fh, cval, cval2, "data reduction system ID", NULL);

    /* Raw files */
    for (i=0 ; i<jc->nframes ; i++) {
        sprintf(cval, "HIERARCH ESO PRO REC%d RAW%d NAME", nb, i+1) ;
        if ((arcfile = pfits_get(jc->data_type, jc->frame[i].name, 
                        "arcfile")) != NULL) {
            qfits_header_add(fh, cval, arcfile, NULL, NULL) ;
        }
        if (jc->frame[i].docatg != NULL) {
            sprintf(cval, "HIERARCH ESO PRO REC%d RAW%d CATG", nb, i+1) ;
            sprintf(cval2, jc->frame[i].docatg) ;
            qfits_header_add(fh, cval, cval2, NULL, NULL) ;
        }
    }

    /* Calibration files */
    /* FIXME
    for (i=0 ; i<calibfiles->n ; i++) {
        sprintf(cval, "HIERARCH ESO PRO REC%d CALIB%d NAME", nb, i+1) ;
        if ((arcfile = pfits_get(pfits_identify_ins(calibfiles->name[i]), 
                        calibfiles->name[i], "arcfile")) != NULL) {
            qfits_header_add(fh, cval, arcfile, NULL, NULL) ;
        }
        if (calibfiles->type != NULL) {
            if (calibfiles->type[i] != NULL) {
                sprintf(cval, "HIERARCH ESO PRO REC%d CALIB%d CATG", nb, i+1) ;
                qfits_header_add(fh, cval, calibfiles->type[i], NULL, NULL) ;
            }
        }
    }
    */

    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Save the FITS combined image
  @param    jc          Jitter config object.
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int jitter_save(jitter_config_t * jc)
{
    qfits_header    *   fh ;
    char                outname[1024] ;
    char                status_file[1024] ;
    cube_t          *   cube ;
    FILE            *   sta ;
    procat              pro_catg ;
    char            *   val ;

    /* Read FITS header from the first frame */
    if ((fh = qfits_header_read(jc->frame[0].name)) == NULL) {
        e_error("cannot read header from %s", jc->frame[0].name) ;
        jc->status_save = ALGO_FAILED ;
        return -1 ;
    }

    /* Output file name */
    sprintf(outname, "%s.fits", jc->output_basename);
    
    /* Update FITS header with PRO keywords */
    /* Default */
    pro_catg = procat_imag_sw_jitter_result ;
    /* Find out the pro catg keyword to write : depends on the used arm */
    if ((val = pfits_get(jc->data_type, jc->frame[0].name, "arm")) != NULL) {
        if (toupper(val[0])=='S') pro_catg = procat_imag_sw_jitter_result ;
        else if (toupper(val[0])=='L') pro_catg = procat_imag_lw_jitter_result;
    }
    jitter_add_pro_keys(jc, fh, pro_catg) ;

    /* Add various comments */
    jitter_add_comments(jc, fh) ;

    /* Create the cube to save */
    if (jc->final == NULL) {
       cube = jitter_cubeget(jc, NULL);
    } else {
       cube = cube_from_image(jc->final) ;
    }
    
    /* Save to disk */
    e_comment(1, "saving final output [%s]", outname);
    cube_save_fits_hdrdump(cube, outname, fh);
    qfits_header_destroy(fh);
   
    /* Destroy the cube */
    if (jc->final == NULL) {
        cube_del_shallow(cube) ; 
    } else {
        cube_del(cube) ;
    }
     
    /* Update the status */
    jc->status_save = ALGO_OK ;

    /* output a status file as basename_status.ascii */
    
    /* Build the output status file name */
    sprintf(status_file, "%s_status.ascii", jc->output_basename);
    
    /* Open the file */
    if ((sta = fopen(status_file, "w")) == NULL) {
        e_error("cannot create file [%s]: ", status_file);
        jc->status_save = ALGO_FAILED ;
        return -1 ;
    }
   
    /* Update the status */
    jc->status_save = ALGO_OK ;

    /* Create the file */
    jitter_config_dump(jc, sta);

    /* Close the file */
    fclose(sta) ;
   
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Add comments to fits header.
  @param    jc          Jitter config object.
  @param    fh          FITS header to update.      
  @return   void
  This function fetches various processing informations from the blackboard
  and adds them as commens to the output jitter cube. These will end up as
  HISTORY keywords in the output FITS file.
 */
/*----------------------------------------------------------------------------*/
static void jitter_add_comments(
        jitter_config_t *   cfg, 
        qfits_header    *   fh)
{
    history *   hs ;
    int         i ;

    /*
     * The following data will be written in the output header as
     * HISTORY fields:
     *
     * Version of the jitter software
     * List of input files
     * List of flat-field and dark frames if used
     * Sky estimation statistics (background measurements)
     * Which object was used for Xcorrelation
     * Offset measurements + reliability
     * Total exposure time
     */

    hs = history_new();

    history_add(hs,  "--- eclipse jitter imaging data reduction");
    history_add(hs, "jitter software version: %s", get_eclipse_version()) ;
    history_add(hs,  "[AlgorithmStatus]");
    history_add(hs, "Cosmetics     = %s", jconv_algo(cfg->status_calib));
    history_add(hs, "SkyEngine     = %s", jconv_algo(cfg->status_sky));
    history_add(hs, "ShiftAndAdd   = %s", jconv_algo(cfg->status_saa));
    history_add(hs, "PostProc      = %s", jconv_algo(cfg->status_postproc));

    history_add(hs,  "[Frames]");
    history_add(hs, "FileList    = %s", cfg->in_name);
    history_add(hs, "Path        = %s", get_dirname(cfg->frame[0].name)) ;
    history_add(hs, "NFrames     = %d", cfg->nframes);

    for (i=0 ; i<cfg->nframes ; i++) {
        history_add(hs, "Frame:%03d (%s) %s",
                        i+1,
                        jconv_ftype(cfg->frame[i].type),
                        get_basename(cfg->frame[i].name));
    }

    history_add(hs,  "[Dark]");
    history_add(hs, "Subtraction = %s", cfg->dark_sub ? "yes" : "no");
    if (cfg->dark_sub) {
        history_add(hs, "Filename    = %s", get_basename(cfg->dark_name));
    }
    history_add(hs, "[FlatField]");
    history_add(hs, "Division    = %s", cfg->ff_div ? "yes" : "no");
    if (cfg->ff_div) {
        history_add(hs, "Filename    = %s", get_basename(cfg->ff_name));
    }
    history_add(hs,  "[BadPixels]");
    history_add(hs, "Replacement = %s", cfg->badpix_rep ? "yes" : "no");
    if (cfg->badpix_rep) {
        history_add(hs, "Filename    = %s", get_basename(cfg->badpixmap));
    }
    history_add(hs,  "[SkyEngine]");
    history_add(hs, "EstimateSky       = %s", cfg->sky_active ? "yes" : "no");
    if (cfg->sky_active) {
        history_add(hs, "MinNumberOfFrames = %d", cfg->skyfilter_minframes);
        history_add(hs,  "[SkyCombine]");
        history_add(hs, "RejectHalfWidth   = %d", cfg->skyfilter_rejhw);
        history_add(hs, "RejectMin         = %d", cfg->skyfilter_rejmin);
        history_add(hs, "RejectMax         = %d", cfg->skyfilter_rejmax);
    }
    history_add(hs,  "[ShiftAndAdd]");
    history_add(hs, "ApplyShiftAndAdd = %s", cfg->saa_active ? "yes" : "no");
    history_add(hs,  "[PostProcessing]");
    if (cfg->pproc_active && cfg->pproc_rowmediansub) {
        history_add(hs, "RowSubtractMedian = yes");
    } else {
        history_add(hs, "RowSubtractMedian = no");
    }

    /* Dump history into FITS header */
    history_addfits(hs, fh);
    history_del(hs);
    return ;
}

