/*----------------------------------------------------------------------------*/
/**
   @file    qc-strehl.c
   @author  N. Devillard
   @date    August 2002
   @version $Revision: 1.22 $
   @brief   Strehl computation for QC1 log.
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: qc-strehl.c,v 1.22 2004/12/22 09:13:33 yjung Exp $
    $Author: yjung $
    $Date: 2004/12/22 09:13:33 $
    $Revision: 1.22 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "conicap_lib.h"
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
#define MINIMUM_FLUX                    (100000)
#define MAXIMUM_PEAK                    (4000)
#define DEF_LOCATE_SX                   200
#define DEF_LOCATE_SY                   200
#define DEF_OUTPUTNAME                  "qc-strehl"

/*-----------------------------------------------------------------------------
                        Private to this module
 -----------------------------------------------------------------------------*/

typedef struct framepair {
    
    /* Input file names */
    char * filename_a ;
    char * filename_b ;

    /* ID of the filter used for observation */
    conica_filter_id filter_obs ;

    /* Pixel scale for this pair */
    double pixscale ;

    /* Strehl */
    double strehl ;
    double strehl_error ;
    double star_bg ;
    double star_peak ;
    double star_flux ;
    double psf_peak ;
    double psf_flux ;
    double bg_noise ;
    double pos_x ;
    double pos_y ;
 
} framepair ;


static struct {

    /* Input framelist name */
    char * name_i ;

    /* Instrument ID */
    instrument_t    INSID ;

    /* Array of valid framepairs */
    int         np ;
    framepair * pair ;

    /* Output file */
    char * name_o ;

} config ;


/* Functions private to this module */
static int conica_qcs_getinput(void);
static int conica_qcs_processpair(int p, double, double, double) ;
static int conica_qcs_save(int p);
static char * conica_qcs_get_dprtype(char * filename) ;

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int conica_qcstrehl_main(void * dict)
{
    dictionary  *   d ;
    int             p ;
    int             err, sta ;
    char        *   sval ;
    double          star_r, back_r1, back_r2 ;

    d = (dictionary*)dict ;

    /* Get options */
    star_r = dictionary_getdouble(d, "arg.star_radius", -1.0) ;
    sval = dictionary_get(d, "arg.background", NULL) ;
    if (sval == NULL) {
        back_r1 = -1.0 ;
        back_r2 = -1.0 ;
    } else {
        if (sscanf(sval, "%lg %lg", &back_r1, &back_r2) != 2) {
            back_r1 = -1.0 ;
            back_r2 = -1.0 ;
        }
    }

    /* Get input/output names */
    config.name_i = dictionary_get(d, "arg.1", NULL);
    if (config.name_i==NULL) {
        e_error("missing input file name");
        return -1 ;
    }
    config.name_o = dictionary_get(d, "arg.output", NULL);
    if (config.name_o==NULL) {
        config.name_o = DEF_OUTPUTNAME ;
    }

    /* Set instrument ID */
    config.INSID = pfits_identify_insstr("naco");

    /* Main processing loop starts here */
    e_comment(0, "--> START qc-strehl engine");
    /* Load program config */
    e_comment(1, "reading input list: %s", config.name_i);
    if (conica_qcs_getinput()!=0) {
        e_error("getting input information: aborting");
        return -1 ;
    }
    /* Loop on all frame pairs */
    err=0 ;
    for (p=0 ; p<config.np ; p++) {
        sta = conica_qcs_processpair(p, star_r, back_r1, back_r2) ;
        if (sta!=0) {
            e_error("processing pair: %d\n"
                    "files:\n"
                    "%s\n"
                    "%s\n",
                    p+1,
                    config.pair[p].filename_a,
                    config.pair[p].filename_b);
        }
        err+=sta ;
    }
    /* Free all data in config */
    for (p=0 ; p<config.np ; p++) {
        free(config.pair[p].filename_a);
        free(config.pair[p].filename_b);
    }
    if (config.pair != NULL) free(config.pair);
    e_comment(0, "--> STOP qc-strehl engine");
    if (err) {
        e_comment(0, "total: %d error(s)", err);
    }
    return err ;
}

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

static char * conica_qcs_get_dprtype(char * filename)
{
    return pfits_get(config.INSID, filename, "dpr_type") ;
}

static int conica_qcs_getinput(void)
{
    int             err ;
    framelist   *   flist ;
    framelist   *   purged ;
    int             i ;
    char        *   s1 ;
    char        *   s2 ;

    /* Load list of frame names */
    flist = framelist_load(config.name_i);
    if (flist==NULL) {
        e_error("loading list [%s]", config.name_i);
        return -1 ;
    }
   
    /* Purge the framlist: only keep the PSF-CALIBRATOR frames */
    purged = framelist_select_tokenget(flist, "PSF-CALIBRATOR", 
            conica_qcs_get_dprtype) ;
    if (purged==NULL) {
        e_warning("cannot purge non PSF-CALIBRATOR - proceed without purge") ;
        purged = flist ;
    } else {
        framelist_del(flist) ;
        flist = purged ;
        purged = NULL ;
    }
    
    /* Check if frames are in even number */
    if (flist->n % 2) {
        e_warning("odd number of PSF-CALIBRATOR frames in input (%d)",flist->n);
    }
    config.np = flist->n / 2 ;
    /* Allocate framepair objects for all found frame pairs */
    config.pair = malloc(config.np * sizeof(framepair));

    /* Get frame names into config */
    for (i=0 ; i<config.np ; i++) {
        config.pair[i].filename_a = strdup(flist->name[2*i]);
        config.pair[i].filename_b = strdup(flist->name[2*i+1]);
    }
    free(flist);

    /* Load filter information into config */
    err=0 ;
    for (i=0 ; i<config.np ; i++) {
        /* Get filter setting */
        s1 = pfits_get(config.INSID, config.pair[i].filename_a, "filter");
        s2 = pfits_get(config.INSID, config.pair[i].filename_b, "filter");
        if (strcmp(s1, s2)) {
            e_error("inconsistent input planes\n"
                    "frame [%s] taken with filter [%s]\n"
                    "frame [%s] taken with filter [%s]",
                    config.pair[i].filename_a, s1,
                    config.pair[i].filename_b, s2);
            err++ ;
        }
        /* Identify filter from NACO database */
        config.pair[i].filter_obs = conica_get_filterid(s1);
        if (config.pair[i].filter_obs==conica_filter_invalid) {
            e_error("invalid filter: %s", s1);
            err++ ;
        } else {
            e_comment(1, "observation filter: [%s]",
                      conica_get_filtername(config.pair[i].filter_obs));
        }
        
        /* Get pixel scale */
        s1 = pfits_get(config.INSID, config.pair[i].filename_a, "pixscale");
        s2 = pfits_get(config.INSID, config.pair[i].filename_b, "pixscale");
        if (s1==NULL || s2==NULL) {
            e_error("cannot identify pixel scale in [%s]",
                    config.pair[i].filename_a);
            err++ ;
        } else if (strcmp(s1, s2)) {
            e_error("inconsistent pixel scales\n"
                    "frame[%s] taken with scale [%s]\n"
                    "frame[%s] taken with scale [%s]",
                    config.pair[i].filename_a, s1,
                    config.pair[i].filename_b, s2);
            err++ ;
        } else {
            config.pair[i].pixscale = (double)atof(s1);
            if (config.pair[i].pixscale < 1e-9) {
                e_error("invalid pixel scale: %s", s1);
                err++ ;
            }
        }
    }
    /* In case of errors, clean everything before leaving */
    if (err) {
        e_error("total: %d error(s) encountered", err);
        for (i=0 ; i<config.np ; i++) {
            free(config.pair[i].filename_a);
            free(config.pair[i].filename_b);
        }
        free(config.pair);
        return -1 ;
    }
   
    return 0 ;
}

static int conica_qcs_processpair(
        int     p,
        double  star_r,
        double  back_r1,
        double  back_r2)
{
    strehl_parm     spar ;
    image_t     *   im_a ;
    image_t     *   im_b ;
    int             refpos[2] ;

    memset(&spar, 0, sizeof(strehl_parm)) ;
    spar.m1 = PRIMARY_UT4 ;
    spar.m2 = SECONDARY_UT4 ;
    conica_get_filterdef(config.pair[p].filter_obs, &spar.l0, &spar.dl);
    spar.pscale = config.pair[p].pixscale ;
    spar.size = STREHL_BOX_SIZE ;
    spar.psf_save = 0 ;
    spar.estim_bg = 1 ;
    if (star_r < 0.0) spar.star_radius = STREHL_STAR_RADIUS ;
    else spar.star_radius = star_r ;
    if (back_r1 < 0.0) spar.bg_radius1 = STREHL_BACKGROUND_R1 ;
    else spar.bg_radius1 = back_r1 ;
    if (back_r2 < 0.0) spar.bg_radius2 = STREHL_BACKGROUND_R2 ;
    else spar.bg_radius2 = back_r2 ;

    /* Display parameters */
    e_comment(2, "m1:     %g\n", spar.m1) ;
    e_comment(2, "m2:     %g\n", spar.m2) ;
    e_comment(2, "l0:     %g\n", spar.l0) ;
    e_comment(2, "dl:     %g\n", spar.dl) ;
    e_comment(2, "pscale: %g\n", spar.pscale) ;
    e_comment(2, "bg:     %g (%g-%g)\n",
            spar.star_radius, spar.bg_radius1, spar.bg_radius2);

    /* Load input images */
    im_a = image_load(config.pair[p].filename_a);
    if (im_a==NULL) {
        e_error("loading file [%s]", config.pair[p].filename_a);
        return -1 ;
    }
    im_b = image_load(config.pair[p].filename_b);
    if (im_b==NULL) {
        e_error("loading file [%s]", config.pair[p].filename_b);
        return -1 ;
    }
    image_sub_local(im_a, im_b);
    image_del(im_b);

    /* Compute Strehl */

    /* Find star around the image center */
    image_locate_peak(im_a,
                      im_a->lx/2,
                      im_a->ly/2,
                      DEF_LOCATE_SX,
                      DEF_LOCATE_SY,
                      refpos);
    config.pair[p].pos_x = spar.pos_x = refpos[0];
    config.pair[p].pos_y = spar.pos_y = refpos[1];

    /* Current image background */
    spar.star_bg  = image_estimate_background(im_a, 0.1, 50);
    /* Compute the strehl */
    if (image_compute_strehl(im_a, &spar) == -1) {
        e_warning("cannot compute strehl for pair %d", p+1) ;
        config.pair[p].strehl       = -1.0 ;
        config.pair[p].strehl_error = -1.0 ;
        config.pair[p].star_bg      = -1.0 ;
        config.pair[p].star_peak    = -1.0 ;
        config.pair[p].star_flux    = -1.0 ;
        config.pair[p].psf_peak     = -1.0 ;
        config.pair[p].psf_flux     = -1.0 ;
        config.pair[p].bg_noise     = -1.0 ;
    } else {
        config.pair[p].strehl = spar.strehl ;
        config.pair[p].strehl_error = spar.strehl_err ;
        config.pair[p].star_bg = spar.star_bg ;
        config.pair[p].star_peak = spar.star_peak ;
        config.pair[p].star_flux = spar.star_flux ;
        config.pair[p].psf_peak = spar.psf_peak ;
        config.pair[p].psf_flux = spar.psf_flux ;
        config.pair[p].bg_noise = spar.bg_noise ;
    }
    image_del(im_a);
    e_comment(2, "strehl=%g%% (err: %g%%)", 100.0*spar.strehl, 
            100.0*spar.strehl_err);
    /* Save results to PAF file */
    conica_qcs_save(p);
    return 0 ;
}

static int conica_qcs_save(int p)
{
    char            out_name[FILENAMESZ];
    FILE        *   paf ;
    char        *   sval1 ;
    char        *   sval2 ;
    int             ndit_val ;
    double          val1, val2 ;
    double          mean ;
    double          avg_airmass, airmass_start, airmass_end ;
    
    /* Create output PAF file */
    sprintf(out_name, "%s_%d.paf", config.name_o, p+1);

    e_comment(1, "creating output PAF [%s]", out_name);
    paf = qfits_paf_print_header(out_name, "CONICA/qc-strehl",
                                 "QC Strehl results",
                                 get_login_name(),
                                 get_datetime_iso8601());
    if (paf==NULL)
        return -1 ;

    fprintf(paf, "\n");

    /* Add ARCFILE */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "arcfile");
    if (sval1!=NULL) fprintf(paf, "ARCFILE   \"%s\"  \n", sval1) ;
    /* Add PRO.CATG */
    fprintf(paf, "PRO.CATG              \"%s\" ;# Product category\n",
            pfits_getprokey(config.INSID, procat_qc_strehl)) ;
    /* Add date */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "date_obs");
    fprintf(paf, "DATE-OBS        \"%s\" ;# Date\n", sval1 ? sval1 : "unknown");
    /* TPL ID */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "templateid");
    fprintf(paf, "TPL.ID          %s ;# Template \n", 
            sval1 ? sval1 : "unknown");
    /* MJD-OBS */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "mjd-obs");
    fprintf(paf, "MJD-OBS         %s ;# Obs start\n", sval1 ? sval1 : "0.0");
    /* AOS.INS.DICH.POSNAM  */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "dich_posname");
    fprintf(paf, "AOS.INS.DICH.POSNAM     \"%s\"\n", sval1 ? sval1 : "unknown");
    /* AOS.OCS.WFS.MODE */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "wfs_mode");
    fprintf(paf, "AOS.OCS.WFS.MODE        \"%s\"\n", sval1 ? sval1 : "unknown");
    /* AOS.OCS.WFS.TYPE */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "wfs_type");
    fprintf(paf, "AOS.OCS.WFS.TYPE        \"%s\"\n", sval1 ? sval1 : "unknown");
    /* AOS.RTC.DET.DST.L0MEAN */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "l0mean");
    sval2 = pfits_get(config.INSID, config.pair[p].filename_b, "l0mean");
    if ((sval1==NULL) || (sval2==NULL)) mean = 0.0 ;
    else {
        val1 = (double)atof(sval1) ;
        val2 = (double)atof(sval2) ;
        if (fabs(val1) < 1e-3) mean = val2 ;
        else if (fabs(val2) < 1e-3) mean = val1 ;
        else mean = (val1+val2)/2.0 ;
    }
    fprintf(paf, "AOS.RTC.DET.DST.L0MEAN   \"%g\"\n", mean);
    /* AOS.RTC.DET.DST.T0MEAN */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "t0mean");
    sval2 = pfits_get(config.INSID, config.pair[p].filename_b, "t0mean");
    if ((sval1==NULL) || (sval2==NULL)) mean = 0.0 ;
    else {
        val1 = (double)atof(sval1) ;
        val2 = (double)atof(sval2) ;
        if (fabs(val1) < 1e-3) mean = val2 ;
        else if (fabs(val2) < 1e-3) mean = val1 ;
        else mean = (val1+val2)/2.0 ;
    }
    fprintf(paf, "AOS.RTC.DET.DST.T0MEAN   \"%g\"\n", mean);
    /* AOS.RTC.DET.DST.R0MEAN */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "r0mean");
    sval2 = pfits_get(config.INSID, config.pair[p].filename_b, "r0mean");
    if ((sval1==NULL) || (sval2==NULL)) mean = 0.0 ;
    else {
        val1 = (double)atof(sval1) ;
        val2 = (double)atof(sval2) ;
        if (fabs(val1) < 1e-3) mean = val2 ;
        else if (fabs(val2) < 1e-3) mean = val1 ;
        else mean = (val1+val2)/2.0 ;
    }
    fprintf(paf, "AOS.RTC.DET.DST.R0MEAN   \"%g\"\n", mean);
    /* AOS.RTC.DET.DST.ECMEAN */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "ecmean");
    sval2 = pfits_get(config.INSID, config.pair[p].filename_b, "ecmean");
    if ((sval1==NULL) || (sval2==NULL)) mean = 0.0 ;
    else {
        val1 = (double)atof(sval1) ;
        val2 = (double)atof(sval2) ;
        if (fabs(val1) < 1e-3) mean = val2 ;
        else if (fabs(val2) < 1e-3) mean = val1 ;
        else mean = (val1+val2)/2.0 ;
    }
    fprintf(paf, "AOS.RTC.DET.DST.ECMEAN   \"%g\"\n", mean);
    /* AOS.RTC.DET.DST.FLUXMEAN */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "fluxmean");
    sval2 = pfits_get(config.INSID, config.pair[p].filename_b, "fluxmean");
    if ((sval1==NULL) || (sval2==NULL)) mean = 0.0 ;
    else {
        val1 = (double)atof(sval1) ;
        val2 = (double)atof(sval2) ;
        if (fabs(val1) < 1e-3) mean = val2 ;
        else if (fabs(val2) < 1e-3) mean = val1 ;
        else mean = (val1+val2)/2.0 ;
    }
    fprintf(paf, "AOS.RTC.DET.DST.FLUXMEAN \"%g\"\n", mean);
    /* INS.OPTI7.NAME */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "opti7_name");
    fprintf(paf, "INS.OPTI7.NAME          \"%s\"\n", sval1 ? sval1 : "unknown");
    /* DET.NCORRS.NAME */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "rom_name");
    fprintf(paf, "DET.NCORRS.NAME         \"%s\"\n", sval1 ? sval1 : "unknown");
    /* DET.MODE.NAME */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "mode");
    fprintf(paf, "DET.MODE.NAME           \"%s\"\n", sval1 ? sval1 : "unknown");
    /* OBS.ID */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "obs_id");
    fprintf(paf, "OBS.ID                \"%s\"\n", sval1 ? sval1 : "unknown");
    fprintf(paf, "\n# Detector section\n");
    /* PIXSCALE */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "pixscale");
    fprintf(paf, "INS.PIXSCALE          %s\n", sval1 ? sval1 : "-1");
    /* DET.DIT */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "dit");
    fprintf(paf, "DET.DIT               %s\n", sval1 ? sval1 : "-1");
    fprintf(paf, "\n");
    /* DET.NDIT */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "ndit");
    ndit_val = (int)atoi(sval1) ;
    fprintf(paf, "DET.NDIT               %s\n", sval1 ? sval1 : "-1");
    fprintf(paf, "\n");

    /* List of input frames */
    fprintf(paf, "# Input frames\n");
    fprintf(paf, "# FRAMELIST.START\n");
    fprintf(paf, "# %s\n", config.pair[p].filename_a);
    fprintf(paf, "# %s\n", config.pair[p].filename_b);
    fprintf(paf, "# FRAMELIST.END\n");
    fprintf(paf, "\n");
    
    /* Observation filter */
    fprintf(paf, "QC.FILTER.OBS         \"%s\"\n",
            conica_get_filtername(config.pair[p].filter_obs)) ;

    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "opti3_name") ;
    fprintf(paf, "QC.FILTER.NDENS       \"%s\"\n", sval1 ? sval1 : "unknown");
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "opti4_id") ;
    fprintf(paf, "QC.FILTER.POL         \"%s\"\n", sval1 ? sval1 : "unknown");

    /* QC.AIRMASS */
    sval1 = pfits_get(config.INSID, config.pair[p].filename_a, "airmass_start");
    sval2 = pfits_get(config.INSID, config.pair[p].filename_b, "airmass_end") ;
    if ((sval1 != NULL) && (sval2 != NULL)) {
        airmass_start = (double)atof(sval1);
        airmass_end = (double)atof(sval2);
        avg_airmass = (airmass_start + airmass_end)/2.0 ;
        fprintf(paf, "QC.AIRMASS            %g\n", avg_airmass);
    }
    
    /* Strehl */
    /* QC STREHL */
    if (ndit_val*config.pair[p].star_flux < MINIMUM_FLUX) {
        fprintf(paf, "# Flux too low (%g), the Strehl may be unreliable !!!\n",
                ndit_val*config.pair[p].star_flux);
    }
    fprintf(paf, "QC.STREHL             %g\n", config.pair[p].strehl);

    /* QC STREHL FLUX */
    fprintf(paf, "QC.STREHL.FLUX         %g\n", config.pair[p].star_flux);

    /* QC STREHL PEAK */
    if (config.pair[p].star_peak > MAXIMUM_PEAK) {
        fprintf(paf, "# Peak too high (%g), the Strehl may be unreliable !!!\n",
                config.pair[p].star_peak);
    }
    fprintf(paf, "QC.STREHL.PEAK         %g\n", config.pair[p].star_peak);
    
    /* QC STREHL ERROR */
    fprintf(paf, "QC.STREHL.ERROR        %g\n", config.pair[p].strehl_error);
    /* QC STREHL RMS */
    fprintf(paf, "QC.STREHL.RMS          %g\n", config.pair[p].bg_noise);

    /* QC STREHL POSX */
    fprintf(paf, "QC.STREHL.POSX         %g\n", config.pair[p].pos_x);
    /* QC STREHL POSY */
    fprintf(paf, "QC.STREHL.POSY         %g\n", config.pair[p].pos_y);
    fprintf(paf, "\n") ;
    fclose(paf) ;

    return 0 ;
}

/* vim: set ts=4 et sw=4 tw=75 */
