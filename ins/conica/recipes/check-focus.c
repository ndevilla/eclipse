/*----------------------------------------------------------------------------*/
/**
   @file    check-focus.c
   @author  Y. Jung
   @date    Mai 2003
   @version $Revision: 1.11 $
   @brief   Focus check
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: check-focus.c,v 1.11 2003/09/13 21:59:03 yjung Exp $
    $Author: yjung $
    $Date: 2003/09/13 21:59:03 $
    $Revision: 1.11 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "conicap_lib.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define CO_FOCUS_DARK                   1
#define CO_FOCUS_NODARK                 2

#define PRIMARY_UT4                     (8)
#define SECONDARY_UT4                   (1.1)
#define STREHL_BOX_SIZE                 (64)
#define STREHL_STAR_RADIUS              (1.0)
#define STREHL_BACKGROUND_R1            (2.0)
#define STREHL_BACKGROUND_R2            (3.0)

#define DEF_LOCATE_SX                   100
#define DEF_LOCATE_SY                   100
#define ENERGY_RADIUS_PIX               11
#define FITTING_POLY_DEG                2

/*-----------------------------------------------------------------------------
                                Global object
 -----------------------------------------------------------------------------*/

static struct {
    /* Input framelist name */
    char            *   name_i ;
    /* Instrument ID */
    instrument_t        INSID ;
    /* ID of the filter */
    conica_filter_id    filter_obs ;
    /* Pixel scale */
    double              pixscale ;
    /* Computed values */
    double              best_strehl ;
    double              best_strehl_err ;
    double              fwhm ;
    double              energy ;
    double              focus ;
    double              focus_optimal ;
    /* Display */
    int                 display ;
    /* Output file */
    char            *   name_o ;
} config ;

/*-----------------------------------------------------------------------------
                                Functions prototypes
 -----------------------------------------------------------------------------*/

static int co_focus_engine() ;
static int co_focus_isdark(char *) ;
static double * co_focus_fit(double *, double *, int, int) ;
static int co_focus_save(framelist *, double *, double *, double *, double *,
        double *) ;

/*-----------------------------------------------------------------------------
                                    Main
 -----------------------------------------------------------------------------*/
int conica_checkfocus_main(void * dict)
{
    dictionary  *   d ;
    char            argname[10] ;
    int             nfiles ;
    int             errors ;
    int             i ;

    /* Initialize */
    d = (dictionary*)dict ;
    config.INSID = pfits_identify_insstr("naco");
    config.display = dictionary_getint(d, "arg.display", 0) ;

    /* Get input/output names */
    nfiles = dictionary_getint(d, "arg.n", -1) ;
    if (nfiles<0) {
        e_error("missing input file name(s): aborting");
        return -1 ;
    }

    /* Loop on input file names */
    errors = 0 ;
    for (i=1 ; i<nfiles ; i++) {
        sprintf(argname, "arg.%d", i);
        config.name_i = dictionary_get(d, argname, NULL) ;
        config.name_o = dictionary_get(d, "arg.output", NULL) ;
        if (config.name_o == NULL) 
            config.name_o = strdup(get_rootname(get_basename(config.name_i)));
        else config.name_o = strdup(get_rootname(config.name_o)) ;
    
        /* Main processing loop starts here */
        e_comment(0, "--> START check-focus engine");
        errors = co_focus_engine() ;
        e_comment(0, "--> STOP check-focus engine");
        free(config.name_o) ;
    }
    return errors ;
}

/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Engine for the check focus recipe
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int co_focus_engine(void) 
{
    framelist   *   flist_all ;
    framelist   *   flist ;
    image_t     *   curr_im ;
    image_t     *   dark ;

    double      *   fwhm ;
    double      *   energy ;
    double      *   focus ;
    double      *   strehl ;
    double      *   strehl_err ;
    int             nb_obj ;
    
    double      *   fwhm_filt ;
    double      *   energy_filt ;
    double      *   focus_filt ;
    double      *   strehl_filt ;
    double      *   strehl_err_filt ;
    int             nb_obj_filt ;

    strehl_parm     spar ;
    char        *   str ; 
    int             refpos[2] ;
    double      *   fwhm_point ;
    double      *   coeffs ;
    int             i, j ;
    
    /* Initialize */
    dark = NULL ;
    
    /* Load the data */
    e_comment(0, "---> Loading input frame list: %s", config.name_i) ;
    if ((flist_all = framelist_load(config.name_i)) == NULL) {
        e_error("cannot read input frame list : %s", config.name_i) ;
        return -1 ;
    }
    if (flist_all->n < 1) {
        framelist_del(flist_all) ;
        return -1 ; 
    }
    
    /* Get the filter used */
    str = pfits_get(config.INSID, flist_all->name[0], "filter") ;
    config.filter_obs = conica_get_filterid(str) ;

    /* Get the pixel scale */
    str = pfits_get(config.INSID, flist_all->name[0], "pixscale") ;
    config.pixscale = (double)atof(str) ;
    
    /* Identify the darks */
    e_comment(0, "---> Identifying dark frames");
    for (i=0 ; i<flist_all->n ; i++) {
        if (i==0) flist_all->label[i] = CO_FOCUS_DARK ;
        else flist_all->label[i] = CO_FOCUS_NODARK ;
    }
    
    /* Get the first dark */
    for (i=0 ; i<flist_all->n ; i++) {
        if (flist_all->label[i] == CO_FOCUS_DARK) {
            dark = image_load(flist_all->name[i]) ;
            break ;
        }
    }
    
    /* Get the NODARK frames */
    if ((flist = framelist_select(flist_all, CO_FOCUS_NODARK)) == NULL) {
        if (dark != NULL) image_del(dark) ;
        framelist_del(flist_all) ;
        return -1 ;
    }
    framelist_del(flist_all) ;
    nb_obj = flist->n ;

    /* Allocate arrays to store results */
    fwhm =       malloc(nb_obj * sizeof(double)) ;
    strehl =     malloc(nb_obj * sizeof(double)) ;
    strehl_err = malloc(nb_obj * sizeof(double)) ;
    energy =     malloc(nb_obj * sizeof(double)) ;
    focus =      malloc(nb_obj * sizeof(double)) ;
    
    /* Loop on the frames */
    for (i=0 ; i<nb_obj ; i++) {
        e_comment(0, "---> Reduce %s", flist->name[i]);
        /* Load the current frame */
        curr_im = image_load(flist->name[i]) ;
        
        /* Subtract the dark */
        if (dark != NULL) image_sub_local(curr_im, dark) ;
  
        /* Get the focus */
        str = pfits_get(config.INSID, flist->name[i], "focus") ;
        if (str != NULL) focus[i] = (double)atof(str) ;
        else             focus[i] = 0.00 ;

        /* Detect the source arround the center */
        image_locate_peak(curr_im, curr_im->lx/2, curr_im->ly/2,
                          DEF_LOCATE_SX, DEF_LOCATE_SY, refpos) ;
        
        /* Initialize spar */
        memset(&spar, 0, sizeof(strehl_parm)) ;

        /* Compute the strehl */
        spar.pos_x = refpos[0];
        spar.pos_y = refpos[1];
        spar.m1 = PRIMARY_UT4 ;
        spar.m2 = SECONDARY_UT4 ;
        conica_get_filterdef(config.filter_obs, &spar.l0, &spar.dl) ;
        spar.pscale = config.pixscale ;
        spar.size = STREHL_BOX_SIZE ;
        spar.psf_save = 0 ;
        spar.star_radius = STREHL_STAR_RADIUS ;
        spar.estim_bg = 1 ;
        spar.bg_radius1 = STREHL_BACKGROUND_R1 ;
        spar.bg_radius2 = STREHL_BACKGROUND_R2 ;
        image_compute_strehl(curr_im, &spar) ;
        strehl[i] = spar.strehl ;
        strehl_err[i] = spar.strehl_err ;

        /* Compute the energy */
        energy[i] = image_get_disk_flux(curr_im, refpos[0]-1, refpos[1]-1, 
                ENERGY_RADIUS_PIX, 0.0) ;
        
        /* Compute the fwhm */
        fwhm_point = image_getfwhm(curr_im, 0, 0, refpos[0]-1, refpos[1]-1,1,1);
        fwhm[i] = (fwhm_point[0] + fwhm_point[1])/2.0 ;
        free(fwhm_point) ;
        image_del(curr_im) ;
        /* Display results */
        e_comment(1, "Star at position: %d %d\n", refpos[0], refpos[1]);
        e_comment(1, "Strehl:           %g%% (%g%%)\n", 
                100.0*strehl[i], 100*strehl_err[i]);
        e_comment(1, "Energy:           %g\n", energy[i]) ;
        e_comment(1, "FWHM:             %g\n", fwhm[i]) ;
        e_comment(1, "Focus:            %g\n", focus[i]) ;
    }
    if (dark != NULL) image_del(dark) ;
        
    /* Filter the good values */
    e_comment(0, "---> Keep valid frames (strehl error < 10%%)") ;
    nb_obj_filt = 0 ;
    for (i=0 ; i<nb_obj ; i++) if (strehl_err[i] < 0.1) nb_obj_filt ++ ;
    if (nb_obj_filt == 0) {
        e_error("No valid strehl computed - abort") ;
        framelist_del(flist) ;
        free(fwhm) ;
        free(strehl) ;
        free(strehl_err) ;
        free(energy) ;
        free(focus) ;
        return -1 ;
    } else e_comment(1, "Number of valid frames: %d / %d", nb_obj_filt, nb_obj);
    /* Allocate and fill filtered arrays */
    fwhm_filt =       malloc(nb_obj_filt * sizeof(double)) ;
    strehl_filt =     malloc(nb_obj_filt * sizeof(double)) ;
    strehl_err_filt = malloc(nb_obj_filt * sizeof(double)) ;
    energy_filt =     malloc(nb_obj_filt * sizeof(double)) ;
    focus_filt =      malloc(nb_obj_filt * sizeof(double)) ;
    j = 0 ;
    for (i=0 ; i<nb_obj ; i++) {
        if (strehl_err[i] < 0.1) {
            fwhm_filt[j] = fwhm[i] ;
            strehl_filt[j] = strehl[i] ;
            strehl_err_filt[j] = strehl_err[i] ;
            energy_filt[j] = energy[i] ;
            focus_filt[j] = focus[i] ;
            j++ ;
        }
    }
  
    /* Fill config with the best strehl  */
    j = 0 ;
    config.best_strehl = strehl_filt[0] ;
    for (i=0 ; i<nb_obj_filt ; i++) {
        if (config.best_strehl < strehl_filt[i]) {
            config.best_strehl = strehl_filt[i] ;
            j = i ;
        }
    }
    config.best_strehl_err = strehl_err_filt[j] ;
    config.fwhm = fwhm_filt[j] ;
    config.energy = energy_filt[j] ;
    config.focus = focus_filt[j] ;
    
    /* Free */
    free(fwhm_filt) ;
    free(strehl_err_filt) ;
    free(energy_filt) ;
   
    /* Fit a 2nd deg polynomial to strehl(focus) */
    e_comment(0, "---> Fit a 2nd degree polynomial") ;
    coeffs = co_focus_fit(focus_filt, strehl_filt, nb_obj_filt, config.display);
    free(strehl_filt) ;
    free(focus_filt) ;
    if (coeffs == NULL) {
        e_error("Cannot fit strehl(focus) - abort") ;
        framelist_del(flist) ;
        free(fwhm) ;
        free(strehl) ;
        free(strehl_err) ;
        free(energy) ;
        free(focus) ;
        return -1 ;
    }

    /* Get the best focus */
    config.focus_optimal = -coeffs[1]/(2*coeffs[2]) ;
    free(coeffs) ;

    /* Produce the paf file */
    e_comment(0, "---> Create the PAF file") ;
    if (co_focus_save(flist, fwhm, strehl, strehl_err, energy, focus) == -1) {
        e_error("cannot write PAF file") ;
        framelist_del(flist) ;
        free(fwhm) ;
        free(strehl) ;
        free(strehl_err) ;
        free(energy) ;
        free(focus) ;
        return -1 ;
    }                   
    
    /* Free and return */
    free(fwhm) ;
    free(strehl) ;
    free(strehl_err) ;
    free(energy) ;
    free(focus) ;
    framelist_del(flist) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Checks if a frame is a dark or not
  @param    in      Input 
  @return   1 if dark, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int co_focus_isdark(char * in)
{
    char    *   str ;
    double      val ;

    /* Read the keyword */
    if ((str = pfits_get(config.INSID, in, "l0mean")) == NULL) {
        e_error("cannot get l0mean from [%s]", in);
        return -1 ;
    }
    val = (double)atof(str) ;
    
    /* Check if it is a dark */
    if (fabs(val) < 1e-3) return 1 ;
    
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Fit and plot 
  @param    foc         list of focii
  @param    strehl      list of strehl values
  @param    nb_samples  number of samples
  @param    display     flag to plot or not
  @return   coefficients array 
 */
/*----------------------------------------------------------------------------*/
static double * co_focus_fit(
        double  *   foc,
        double  *   strehl,
        int         nb_samples,
        int         display)
{
    double3         *   to_fit ;
    gnuplot_ctrl    *   handle ;
    char                cmd[FILENAMESZ] ;
    char                cmd2[FILENAMESZ] ;
    double          *   coeffs ;
    int                 i, j ;

    /* Fit a 2nd deg polynomial to strehl(focus) */
    to_fit = double3_new(nb_samples) ;
    for (i=0 ; i<nb_samples ; i++) {
        to_fit->x[i] = foc[i] ;
        to_fit->y[i] = strehl[i] ;
    }
    if ((coeffs = fit_1d_poly(FITTING_POLY_DEG, to_fit, NULL)) == NULL) {
        double3_del(to_fit) ;
        return NULL ;
    }
    e_comment(1, "Strehl(foc) = %g + %g * foc + %g * foc^2",
            coeffs[0], coeffs[1], coeffs[2]) ;

    /* Display results */
    if (display) {
        e_comment(0, "---> Plot the results") ;
        handle = gnuplot_init() ;
        gnuplot_setstyle(handle, "points") ;
        gnuplot_set_xlabel(handle, "Focus") ;
        gnuplot_set_ylabel(handle, "Strehl") ;
        gnuplot_plot_xy(handle,
                        to_fit->x,
                        to_fit->y,
                        to_fit->n,
                        "Strehl(focus)") ;
        e_comment(1, "press enter to continue\n") ;
        while (getchar() != '\n') {}
        sprintf(cmd, "replot %g", coeffs[0]) ;
        for (i=0 ; i<FITTING_POLY_DEG ; i++) {
            sprintf(cmd2, "%s+%g", cmd, coeffs[i+1]) ;
            sprintf(cmd, cmd2) ;
            for (j=0 ; j<i+1 ; j++) {
                sprintf(cmd2, "%s*x", cmd) ;
                sprintf(cmd, cmd2) ;
            }
        }
        sprintf(cmd2, "%s\n", cmd) ;
        sprintf(cmd, cmd2) ;
        gnuplot_cmd(handle, cmd) ;
        e_comment(1, "press enter to continue\n") ;
        while (getchar() != '\n') {}
        gnuplot_close(handle) ;
    }
    double3_del(to_fit) ;
    return coeffs ;
}
    
/*----------------------------------------------------------------------------*/
/**
  @brief	Output in the PAF file
  @param    in          Input list of frames
  @param    fwhm        list of fwhm
  @param    strehl      list of computed strehls
  @param    strehl_err  list of errors on strehl
  @param    energy      list of energies
  @param    focus       list of focii
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int co_focus_save(
        framelist   *   in,
        double      *   fwhm,
        double      *   strehl,
        double      *   strehl_error,
        double      *   energy,
        double      *   focus)
{
    FILE    *   outfile ;
    char    *   sval ;
    char        outname[FILENAMESZ] ;
    int         i ;
    
    /* Output file name */
    sprintf(outname, "%s.paf", config.name_o) ;
    
    /* Open output PAF file (formatted ASCII, see fits/pafs.c) */
    e_comment(1, "saving results to %s", outname) ;
    if ((outfile = qfits_paf_print_header(outname,
                               "CONICA/check-focus",
                               "Check focus recipe results",
                               get_login_name(),
                               get_datetime_iso8601())) == NULL) {
        e_error("cannot open file [%s] for output", outname) ;
        return -1 ;
    }

    /* Print informations about reduction */
    fprintf(outfile, "# File name \n") ;
    fprintf(outfile, "#       fwhm   strehl (error)       energy   focus\n\n");
    for (i=0 ; i<in->n ; i++) {
        fprintf(outfile, "# %s\n", in->name[i]) ;
        fprintf(outfile, "#     %g\t%g (%g)\t%g\t%g\n\n",
                fwhm[i], strehl[i], strehl_error[i], energy[i], focus[i]) ;
    }
    fprintf(outfile, "\n") ;
    
    /* Add PRO.CATG */
    if ((sval = pfits_getprokey(config.INSID, procat_focus)) != NULL)
        fprintf(outfile, "PRO.CATG       \"%s\" ;# Product category\n", sval);
    /* Add date */
    if ((sval = pfits_get(config.INSID, in->name[0], "date_obs")) != NULL) {
        fprintf(outfile, "DATE-OBS           \"%s\" ; #Date\n", sval) ;
    }
    /* Add ARCFILE */
    if ((sval = pfits_get(config.INSID, in->name[0], "arcfile")) != NULL)
        fprintf(outfile, "ARCFILE         \"%s\" ;#\n", sval) ;
    /* Add TPL ID */
    if ((sval = pfits_get(config.INSID, in->name[0], "templateid")) != NULL) {
        fprintf(outfile, "TPL.ID             \"%s\"; # Template id\n", sval) ;
    }
    /* Add MJD-OBS for file classification */
    if ((sval = pfits_get(config.INSID, in->name[0], "mjdobs")) == NULL) {
        fprintf(outfile, "MJD-OBS               0.0 ; # could not find\n") ;
    } else {
        fprintf(outfile, "MJD-OBS               %s ; # Obs start\n", sval) ;
    }
    
    /* QC.FOCUS.STREHL */
    fprintf(outfile, "QC.STREHL           \"%.4f\"\n", config.best_strehl) ;
    /* QC.FOCUS.STREHLE */
    fprintf(outfile, "QC.STREHL.ERROR     \"%.4f\"\n", config.best_strehl_err) ;
    /* QC.FOCUS.FWHM */
    fprintf(outfile, "QC.FWHM.PIX         \"%.4f\"\n", config.fwhm) ;
    /* QC.FOCUS.ENERGY */
    fprintf(outfile, "QC.ENERGY           \"%.4f\"\n", config.energy) ;
    /* QC.FOCUS.FOCUS */
    fprintf(outfile, "QC.FOCUS            \"%.4f\"\n", config.focus) ;
    /* QC.FOCUS.FOCUSOPT */
    fprintf(outfile, "QC.FOCUSOPT         \"%.4f\"\n", config.focus_optimal) ;

    fprintf(outfile, "\n");
    fclose(outfile) ;

    /* Print out results */
    e_comment(1, "best_strehl:     %g\n", config.best_strehl) ;
    e_comment(1, "best_strehl_err: %g\n", config.best_strehl_err) ;
    e_comment(1, "fwhm:            %g\n", config.fwhm) ;
    e_comment(1, "energy:          %g\n", config.energy) ;
    e_comment(1, "focus:           %g\n", config.focus) ;
    e_comment(1, "focus_optimal:   %g\n", config.focus_optimal) ;
    return 0 ;
}

