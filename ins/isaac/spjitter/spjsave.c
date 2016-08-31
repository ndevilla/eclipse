/*----------------------------------------------------------------------------*/
/**
   @file    spjsave.c
   @author  Y. Jung
   @date    Jan. 2003
   @version	$Revision: 1.7 $
   @brief   Spectroscopic jitter saving utilities
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjsave.c,v 1.7 2003/12/03 16:59:29 yjung Exp $
	$Author: yjung $
	$Date: 2003/12/03 16:59:29 $
	$Revision: 1.7 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "spjtypes.h"
#include "spjconfig.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    saving function
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_save(spjitter_config_t * spjc)
{
    procat              pro_comb,
                        pro_extr ;
    char            *   sval ;
    char                cval[80] ;
    char                outname[FILENAMESZ] ;
    framelist       *   lnames ;
    qfits_header    *   fh ;
    qfits_table     *   table ;
    qfits_col       *   col ;
    double          *   out_table[3] ;
    FILE            *   paf ;
    char                wl_method[FILENAMESZ] ;
    int                 i ;
       
    /* Initialize */
    spjc->status_save = OK ;
    
    /* The PRO CATG keywords depend on the arm used */
    /* Default */
    pro_comb = pro_extr = procat_invalid ;
    if ((sval=pfits_get(spjc->data_type, spjc->frame[0].name, "arm")) != NULL) {
        if (toupper(sval[0])=='S') {
            pro_comb = procat_spec_sw_jitter_comb ;
            pro_extr = procat_spec_sw_jitter_extr ;
        } else if (toupper(sval[0])=='L') {
            pro_comb = procat_spec_lw_jitter_comb ;
            pro_extr = procat_spec_lw_jitter_extr ;
        }
    }

    /* WRITE THE FITS COMBINED IMAGE */
    sprintf(outname, "%s.fits", spjc->output_basename);

    /* Create the framelist object */
    lnames = framelist_new(spjc->nframes) ;
    for (i=0 ; i<spjc->nframes ; i++) {
        lnames->name[i] = strdup(spjc->frame[i].name) ;
        lnames->type[i] = strdup(spjc->frame[i].docatg) ;
    }

    /* Read FITS header from the first frame */
    fh = qfits_header_read(lnames->name[0]) ;

    /* Prepare it for image output */
    if (isaac_header_for_image(fh) == -1) {
        e_error("in writing the output fits file") ;
        qfits_header_destroy(fh) ;
        framelist_del(lnames) ;
        spjc->status_save = FAILED ;
        return -1 ;
    }

    /* Update FITS header with PRO keywords for the combined image */
    isaac_pro_fits(fh, outname, "REDUCED", NULL, pro_comb, "OK",
                   "spec_obs_nodonslit", lnames->n, lnames, NULL);

    /* Write HISTORY keywords in the header */
    if (isaac_add_files_history(fh, lnames) == -1) {
        e_warning("cannot write HISTORY keywords in out file") ;
    }
    if (spjc->wavecal_disprel != NULL) {
        sprintf(cval, "DISPCOE1= %g", (spjc->wavecal_disprel)->poly[0]) ;
        qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
        sprintf(cval, "DISPCOE2= %g", (spjc->wavecal_disprel)->poly[1]) ;
        qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
        sprintf(cval, "DISPCOE3= %g", (spjc->wavecal_disprel)->poly[2]) ;
        qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
        sprintf(cval, "DISPCOE4= %g", (spjc->wavecal_disprel)->poly[3]) ;
        qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
    }

    /* Change WCS keywords to the computed wavelength solution */
    if (spjc->wavecal_disprel != NULL) {
        /* Change CRVAL1 */
        sprintf(cval, "%g", (spjc->wavecal_disprel)->poly[0]) ;
        qfits_header_mod(fh, "CRVAL1", cval, NULL) ;
        /* Change CRVAL2 */
        qfits_header_mod(fh, "CRVAL2", "1", NULL) ;
        /* Change CRPIX1 */
        qfits_header_mod(fh, "CRPIX1", "1", NULL) ;
        /* Change CRPIX2 */
        qfits_header_mod(fh, "CRPIX2", "1", NULL) ;
        /* Change CDELT1 */
        sprintf(cval, "%g", (spjc->wavecal_disprel)->poly[1]) ;
        qfits_header_mod(fh, "CDELT1", cval, NULL) ;
        /* Change CDELT2 */
        qfits_header_mod(fh, "CDELT2", "1", NULL) ;
        /* Change CTYPE1 */
        qfits_header_mod(fh, "CTYPE1", "LINEAR", NULL) ;
        /* Change CTYPE2 */
        qfits_header_mod(fh, "CTYPE2", "LINEAR", NULL) ;
        /* Add CD1_1 */
        qfits_header_add_after(fh, "CTYPE2", "CD1_1", cval, NULL, NULL) ;
        /* Add CD1_2 */
        qfits_header_add_after(fh, "CD1_1", "CD2_2", "1", NULL, NULL) ;
    }

    /* Write the file    */
    image_save_fits_hdrdump(spjc->combined, outname, fh, BPP_DEFAULT) ;
    qfits_header_destroy(fh) ;
    e_comment(0, "combined image produced: [%s]", outname) ;

    /* WRITE THE TFITS EXTRACTED SPECTRUM TABLE */
    if (spjc->status_extraction == OK) {
        sprintf(outname, "%s.tfits", spjc->output_basename);
        /* Create the table */
        table = qfits_table_new(outname, QFITS_BINTABLE, -1, 3, spjc->lx) ;
        col = table->col ;
        for (i=0 ; i<table->nc ; i++) {
            qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, " ",
                    " ", " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
            col++ ;
        }
        col = table->col ;
        sprintf(col->tlabel, "X_coordinate") ;
        col++ ;
        sprintf(col->tlabel, "Extracted_spectrum_value") ;
        col++ ;
        sprintf(col->tlabel, "Sky_spectrum") ;

        out_table[0] = spjc->extr_x_coordinate ;
        out_table[1] = spjc->extracted_values ;
        out_table[2] = spjc->sky_signal ;

        /* Read the input header */
        if ((fh = qfits_header_read(lnames->name[0])) == NULL) {
            e_error("in writing the output fits file") ;
            qfits_table_close(table) ;
            framelist_del(lnames) ;
            spjc->status_save = FAILED ;
            return -1 ;
        }

        /* Prepare it for table output */
        if (isaac_header_for_table(fh) == -1) {
            e_error("in writing the output fits file") ;
            qfits_header_destroy(fh) ;
            qfits_table_close(table) ;
            framelist_del(lnames) ;
            spjc->status_save = FAILED ;
            return -1 ;
        }

        /* Write the PRO keywords in the header */
        if (isaac_pro_fits(fh, outname, "REDUCED", NULL, pro_extr, "OK",
                    "isaac_spec_obs_nodonslit", lnames->n, lnames, NULL) == -1){
            e_error("in writing PRO keywords in output file") ;
            qfits_header_destroy(fh) ;
            framelist_del(lnames) ;
            qfits_table_close(table) ;
            spjc->status_save = FAILED ;
            return -1 ;
        }

        /* Write the HISTORY keywords with the input file names */
        if (isaac_add_files_history(fh, lnames) == -1) {
            e_warning("cannot write HISTORY keywords in out file") ;
        }

        /* Write the file on disk */
        if (qfits_save_table_hdrdump((void**)out_table, table, fh) == -1) {
            e_error("cannot write file: %s", outname) ;
            qfits_header_destroy(fh) ;
            qfits_table_close(table) ;
            framelist_del(lnames) ;
            spjc->status_save = FAILED ;
            return -1 ;
        }
        qfits_table_close(table) ;
        qfits_header_destroy(fh) ;

        e_comment(0, "extracted spectrum table produced: [%s]", outname) ;
    }
    framelist_del(lnames) ;

    /* WRITE THE QC PAF FILE */
    sprintf(outname, "%s.paf", spjc->output_basename) ;
    if ((paf = qfits_paf_print_header(outname,
                            "ISAAC/spjitter",
                            "spjitter recipe results",
                            get_login_name(),
                            get_datetime_iso8601())) == NULL) {
        e_warning("cannot output PAF file") ;
        spjc->status_save = FAILED ;
    } else {
        fprintf(paf, "\n");
        /* ARCFILE */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "arcfile") ;
        if (sval != NULL) fprintf(paf, "ARCFILE    \"%s\"  \n", sval) ;
        
        /* MJD-OBS */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "mjdobs") ;
        if (sval != NULL) fprintf(paf, "MJD-OBS  %s; # Obs start\n\n", sval);
        else fprintf(paf, "MJD-OBS  0.0; # Obs start unknown\n\n");
        
        /* INSTRUME keyword  */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "instrument") ;
        if (sval != NULL) fprintf(paf, "INSTRUME \"%s\" \n", sval) ;
        
        /* TPL.ID  */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "templateid") ;
        if (sval != NULL) fprintf(paf, "TPL.ID  \"%s\" \n", sval) ;
        
        /* TPL.NEXP */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "numbexp") ;
        if (sval != NULL) fprintf(paf, "TPL.NEXP  %s \n", sval) ;

        /* DPR.CATG */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "dpr_catg") ;
        if (sval != NULL) fprintf(paf, "DPR.CATG  \"%s\" \n", sval) ;

        /* DPR.TYPE */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "dpr_type") ;
        if (sval != NULL) fprintf(paf, "DPR.TYPE  \"%s\" \n", sval) ;

        /* DPR.TECH */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "dpr_tech") ;
        if (sval != NULL) fprintf(paf, "DPR.TECH  \"%s\" \n", sval) ;

        /* Add PRO.CATG */
        fprintf(paf, "PRO.CATG \"%s\" ;# Product category\n",
                pfits_getprokey(spjc->data_type, procat_spec_sw_jitter_qc)) ;

        /* Add the date */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "date_obs") ;
        if (sval != NULL) fprintf(paf, "DATE-OBS \"%s\" ;# Date\n", sval) ;

        /* INS.GRAT.NAME */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "resolution") ;
        if (sval != NULL) fprintf(paf, "INS.GRAT.NAME  \"%s\" \n", sval) ;

        /* INS.GRAT.WLEN */
        fprintf(paf, "INS.GRAT.WLEN  %g \n", 
                isaac_get_central_wavelength(spjc->frame[0].name)) ;

        /* QC.STDNAME */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "obs_targ_name");
        if (sval != NULL) fprintf(paf, "QC.STDNAME  \"%s\" \n", sval) ;

        /* INS.FILTER.ID */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "filter") ;
        if (sval != NULL) fprintf(paf, "INS.FILTER.ID  \"%s\" \n", sval) ;

        /* INS.OPTI1.ID */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "optical_id") ;
        if (sval != NULL) fprintf(paf, "INS.OPTI1.ID  \"%s\" \n", sval) ;
        
        /* OBS.ID */
        sval = pfits_get(spjc->data_type, spjc->frame[0].name, "obs_id") ;
        if (sval != NULL) fprintf(paf, "OBS.ID    \"%s\" \n", sval) ;
        
        /* Wavelength calibration method used */
        if (spjc->status_wavecal_done == OK) {
            if (spjc->status_wavecal_sky == OK) {
                sprintf(wl_method, "sky lines") ;
            } else if (spjc->status_wavecal_arc == OK) {
                sprintf(wl_method, "arc file") ;
            } else {
                sprintf(wl_method, "physical model") ;
            }
            /* QC.CENWL */
            fprintf(paf, "QC.WLEN     %g \n", 
                    (double)((spjc->wavecal_disprel)->poly[0]+
                        (spjc->wavecal_disprel)->poly[1]*512+
                        (spjc->wavecal_disprel)->poly[2]*512*512+
                        (spjc->wavecal_disprel)->poly[3]*512*512*512));
            /* QC.DISPCO1 */
            fprintf(paf, "QC.DISPCO1  %g \n", 
                    (double)(spjc->wavecal_disprel)->poly[0]);
            /* QC.DISPCO2 */
            fprintf(paf, "QC.DISPCO2  %g \n", 
                    (double)(spjc->wavecal_disprel)->poly[1]);
            /* QC.DISPCO3 */
            fprintf(paf, "QC.DISPCO3  %g \n", 
                    (double)(spjc->wavecal_disprel)->poly[2]);
            /* QC.DISPCO4 */
            fprintf(paf, "QC.DISPCO4  %g \n", 
                    (double)(spjc->wavecal_disprel)->poly[3]);
            /* QC.DISP.XCORR */
            fprintf(paf, "QC.DISP.XCORR   %g\n", 
                    (double)(spjc->wavecal_disprel)->cc);
        } else {
            sprintf(wl_method, "none") ;
        }
        /* QC.WLMETTHOD */
        fprintf(paf, "QC.WLMETHOD  \"%s\" \n", wl_method) ;
        fclose(paf) ;
        e_comment(0, "paf file produced: [%s]", outname) ;
    }

    /* If requested: launch an image viewer on the result */
    if (spjc->output_startviewer == 1) {
        sprintf(outname, "%s.fits", spjc->output_basename);
        show_image(outname, spjc->output_viewer) ;
    }

    /* If requested and if the spectrum has been extracted: plot it */
    if ((spjc->output_gnuplot == 1) && (spjc->spectrum_extracted == 1)) {
        plot_signal(spjc->extr_x_coordinate,
                    spjc->extracted_values,
                    spjc->lx,
                    "Wavelength (in angstroms)",
                    "Extracted spectrum (in ADU)") ;
        plot_signal(spjc->extr_x_coordinate,
                    spjc->sky_signal,
                    spjc->lx,
                    "Wavelength (in angstroms)",
                    "Sky signal (in ADU)") ;
    }

    /* If requested, output a status file as basename_status.ascii */
    if (spjc->output_statusreport == 1) {
        sprintf(outname, "%s_status.ascii", spjc->output_basename);
        if ((paf = fopen(outname, "w")) == NULL) {
            e_error("cannot create file [%s]: ", outname);
            spjc->status_save = FAILED ;
            return -1 ;
        } else {
            spjitter_config_dump(spjc, paf) ;
            fclose(paf) ;
            e_comment(0, "status file produced: [%s]", outname) ;
        }
    }
    return 0 ;
}


