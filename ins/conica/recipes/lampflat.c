/*-----------------------------------------------------------------------------

   File name    :	lampflat.c
   Author       :	Y. Jung
   Created on   :	Sept. 2002
   Description  :	CONICA imaging flat-field creation from lamp images

 -----------------------------------------------------------------------------*/

/*
	$Id: lampflat.c,v 1.13 2003/11/07 11:03:26 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/07 11:03:26 $
	$Revision: 1.13 $
*/

/*-----------------------------------------------------------------------------
								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "conicap_lib.h"
#include "pfits.h"
#include "pfitspro.h"

/*-----------------------------------------------------------------------------
								Defines
 -----------------------------------------------------------------------------*/

#define REJ_LEFT            200
#define REJ_RIGHT           200
#define REJ_BOTTOM          200
#define REJ_TOP             200

/*-----------------------------------------------------------------------------
                            Static variables
 -----------------------------------------------------------------------------*/

static struct {
    int         rej_left ;
    int         rej_right ;
    int         rej_bottom ;
    int         rej_top ;
    double      gain ;
    double      fp_noise ;
    double      lamp_flux ;
} lampflat_config ;

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int frame_compare(char *, char *) ;
static int conica_lampflat_engine(char *, char *);
static int conica_lampflat_process(framelist *, char *) ;
static int conica_lampflat_save(image_t *, char *, framelist *) ;

/*-----------------------------------------------------------------------------
							Main code
 -----------------------------------------------------------------------------*/

int conica_lampflat_main(void * dict)
{
    dictionary  *   d ;

    char            argname[10] ;
    char        *   name_i ;
    char        *   name_o ;
    int             nfiles ;

    char        *   sval ;
    int             errors ;
    int             i ;

    d = (dictionary*)dict ;
    /* Get options */
    /* Rejected borders */
    sval = dictionary_get(d, "arg.rej_bord", NULL) ;
    if (sval == NULL) {
        lampflat_config.rej_left =   REJ_LEFT ;
        lampflat_config.rej_right =  REJ_RIGHT ;
        lampflat_config.rej_bottom = REJ_BOTTOM ;
        lampflat_config.rej_top =    REJ_TOP ;
    } else {
        if (sscanf(sval, "%d %d %d %d",
                    &lampflat_config.rej_left,
                    &lampflat_config.rej_right,
                    &lampflat_config.rej_bottom,
                    &lampflat_config.rej_top)!=4) {
            lampflat_config.rej_left =   REJ_LEFT ;
            lampflat_config.rej_right =  REJ_RIGHT ;
            lampflat_config.rej_bottom = REJ_BOTTOM ;
            lampflat_config.rej_top =    REJ_TOP ;
        }
    }
    
    /* Get input/output file names */
    nfiles = dictionary_getint(d, "arg.n", -1) ;
    if (nfiles<0) {
        e_error("missing input file name(s): aborting");
        return -1 ;
    }

    /* Used instrument */
    INSID = pfits_identify_insstr("naco");
    
    /* Loop on input file names */
    errors = 0 ;
    for (i=1 ; i<nfiles ; i++) {
        sprintf(argname, "arg.%d", i);
        name_i = dictionary_get(d, argname, NULL) ;
        name_o = dictionary_get(d, "arg.output", NULL) ;
        if (name_o == NULL) name_o = strdup(get_rootname(get_basename(name_i)));
        else name_o = strdup(get_rootname(name_o)) ;

        /* Once options have been cleared out, call the computing function. */
        errors += conica_lampflat_engine(name_i, name_o) ;
        free(name_o) ;
    }
    return errors ;
}

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*
 * Compare two frames based on their filter settings.
 */
static int frame_compare(char * f1, char * f2)
{
    int    comparison ;
    char * v1 ;
    char * v2 ;

    /* Compare filters */
    comparison = 1 ;
    if ((v1 = pfits_get(INSID, f1, "filter"))==NULL) {
        e_error("cannot get filter from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "filter"))==NULL) {
        e_error("cannot get filter from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
 
    /* Compare the objective */
    if ((v1 = pfits_get(INSID, f1, "opti7_name"))==NULL) {
        e_error("cannot get objective from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "opti7_name"))==NULL) {
        e_error("cannot get objective from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
    
    /* Compare the OPTI3_NAME */
    if ((v1 = pfits_get(INSID, f1, "opti3_name"))==NULL) {
        e_error("cannot get OPTI3.NAME from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "opti3_name"))==NULL) {
        e_error("cannot get OPTI3.NAME from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
    
    /* Compare the ROM */
    if ((v1 = pfits_get(INSID, f1, "rom_name"))==NULL) {
        e_error("cannot get rom from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "rom_name"))==NULL) {
        e_error("cannot get rom from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
    
    /* Compare the mode */
    if ((v1 = pfits_get(INSID, f1, "mode"))==NULL) {
        e_error("cannot get mode from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "mode"))==NULL) {
        e_error("cannot get mode from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
    
    /* Compare the DIT */
    if ((v1 = pfits_get(INSID, f1, "dit"))==NULL) {
        e_error("cannot get dit from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "dit"))==NULL) {
        e_error("cannot get dit from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
     
    return comparison ;
}

static int conica_lampflat_engine(
        char    *   name_i,
        char    *   name_o)
{
    framelist   *   f_all ;
    framelist   *   f_one ;
    int             nsets ;
    int             err ;
    char            outname[FILENAMESZ];
    int             i ;

	/* Sort input list of frames */
	e_comment(0, "---> Loading input frame list: %s", name_i);
    f_all = framelist_load(name_i) ;
    if (f_all == NULL) {
        e_error("cannot load %s", name_i);
        return 1 ;
    }

    /* Labelize all input frames */
	e_comment(1, "classifying frames");
    nsets = framelist_labelize(f_all, frame_compare);
    if (nsets<1) {
        e_error("cannot classify: aborting");
        framelist_del(f_all);
        return 1 ;
    }
    e_comment(1, "identified filter settings: %d", nsets);

    /* Process all batches */
    e_comment(0, "---> Processing %d data sets", nsets);
    err=0 ;
    for (i=0 ; i<nsets ; i++) {
        /* Build relevant frame list */
        f_one = framelist_select(f_all, i);
        if (f_one==NULL) {
            e_error("classifying batch %d", i+1);
            err++ ;
        } else {
            sprintf(outname, "%s_%d", name_o, i+1) ;
            err += conica_lampflat_process(f_one, outname);
            framelist_del(f_one);
        }
    }
    /* Deallocate objects */
    framelist_del(f_all);
    
    e_comment(0, "done");
    return err ;
}

static int conica_lampflat_process(
        framelist   *   set,
        char        *   outname)
{
    char        *   sval ; 
    cube_t      *   in ;
    cube_t      *   dark_corr ;
    int             lamp1,
                    lamp2 ;
    image_t     *   aver ;
    double          norm ;
    image_t     *   norm_flat ;
    image_t     *   diff ;
    double          std_onon, std_offoff, std_onoff, mean ;
    int             i ;

    /* Print out some comments */
    sval = pfits_get(INSID, set->name[0], "filter");
    e_comment(0, "---> Filter: [%s]", sval ? sval : "unknown");
    for (i=0 ; i<set->n ; i++) {
        e_comment(1, "%s", get_basename(set->name[i]));
    }

    /* Verify that the number of frames is even */
    if (set->n % 2) {
        e_error("The number of frames is not even: %d", set->n);
        return -1 ;
    }
    
    /* Each setting need at least 4 frames to compute gain */
    if (set->n < 4) {
        e_error("At least 4 frames needed to compute gain: %d", set->n) ;
        return -1 ;
    }
    
    /* Load input cube */
	e_comment(1, "---> loading input set");
    if ((in = cube_load_strings(set->name, set->n)) == NULL) {
		e_error("loading cube: aborting") ;
		return 1 ;
	}

    /* Subtract the darks */
    dark_corr = cube_new(in->lx, in->ly, in->np/2) ;
    e_comment(1, "Subtracting dark frames") ;
    for (i=0 ; i<dark_corr->np ; i++) {
        /* Verify that the sequence is lamp_on - lamp_off */
        if ((sval = pfits_get(INSID, set->name[2*i], "lamp2")) == NULL) {
            e_error("cannot identify lamp-on frame") ;
            cube_del(in) ;
            cube_del(dark_corr) ;
            return -1 ;
        }
        lamp1 = (int)atoi(sval) ;
        if ((sval = pfits_get(INSID, set->name[2*i+1], "lamp2")) == NULL) {
            e_error("cannot identify lamp-off frame") ;
            cube_del(in) ;
            cube_del(dark_corr) ;
            return -1 ;
        }
        lamp2 = (int)atoi(sval) ;
        if ((lamp1==0) || (lamp2!=0)) {
            e_error("The current pair does not fit lamp_on - lamp_off");
            cube_del(in) ;
            cube_del(dark_corr) ;
            return -1 ;
        }
        
        /* Compute dark subtraction */
        dark_corr->plane[i] = image_sub(in->plane[2*i], in->plane[2*i+1]) ;
        if (dark_corr->plane[i] == NULL) {
            e_error("cannot subtract the dark for the current pair - abort") ;
            cube_del(in) ;
            cube_del(dark_corr) ;
            return -1 ;
        }
    }

    /* Compute the QC parameters */
       
    /* GAIN */
    /* Compute std_onon on : on1 - on2 */
    diff = image_sub(in->plane[0], in->plane[2]) ;
    image_rect_readout_noise(diff, NULL, -1, -1, &std_onon, NULL) ;
    image_del(diff) ;
    /* Compute std_offoff on : off1 - off2 */
    diff = image_sub(in->plane[1], in->plane[3]) ;
    image_rect_readout_noise(diff, NULL, -1, -1, &std_offoff, NULL) ;
    image_del(diff) ;
    /* Compute mean */
    mean = image_getmean(in->plane[0]) ;  
    /* Deduce GAIN */
    lampflat_config.gain=sqrt(2*mean/(std_onon*std_onon-std_offoff*std_offoff));
    
    /* FPNOISE */
    /* Compute std_onoff on : on1 - off1 */
    diff = image_sub(in->plane[0], in->plane[1]) ;
    image_rect_readout_noise(diff, NULL, -1, -1, &std_onoff, NULL) ;
    /* Deduce FPNOISE */
    lampflat_config.fp_noise=sqrt(std_onoff*std_onoff-std_offoff*std_offoff-
            std_onon*std_onon) ; 
    
    cube_del(in) ;

    /* LAMPFLUX */
    lampflat_config.lamp_flux = (double)image_getmedian(diff) ; 
    image_del(diff) ;
    if ((sval = pfits_get(INSID, set->name[0], "dit"))==NULL) {
        e_error("cannot get dit from [%s]", set->name[0]);
        lampflat_config.lamp_flux = -1 ;
    } else {
        lampflat_config.lamp_flux /= (double)atof(sval) ;
    }
    
    /* Average the dark corrected frames */
    if ((aver = cube_avg_linear(dark_corr)) == NULL) {
        e_error("cannot average the dark corrected frames") ;
        cube_del(dark_corr) ;
        return -1 ;
    }
    cube_del(dark_corr) ;
    
    /* Normalize the flat */
    norm = image_getmean_vig(aver, lampflat_config.rej_left, 
            aver->lx-lampflat_config.rej_right,
            lampflat_config.rej_bottom,
            aver->ly-lampflat_config.rej_top) ;
    norm_flat = image_cst_op(aver, norm, '/');
    image_del(aver) ;

    /* Save the products */
    if (conica_lampflat_save(norm_flat, outname, set) == -1) { 
        e_error("cannot save products") ;
        image_del(norm_flat) ;
        return -1 ;
    }

    /* Free and return */
    image_del(norm_flat) ;
    return 0 ;
}

static int conica_lampflat_save(
        image_t     *   flat,
        char        *   outname,
        framelist   *   flist)
{
    qfits_header    *   fh ;
    char                name[FILENAMESZ] ;
    FILE            *   paf ;
    char            *   sval ;
    char                cval[80] ;

    /* First write the FITS file */
    sprintf(name, "%s.fits", outname) ;

    /* Get FITS header from reference file */
    if ((fh = qfits_header_read(flist->name[0])) == NULL) {
        e_error("getting header from reference frame");
        return -1 ;
    }

    /* Prepare the header */
    conica_header_for_image(fh) ;

    /* Add DataFlow keywords */
    conica_pro_fits(fh,
                    flist->name[0],
                    "REDUCED",
                    NULL,
                    procat_imag_lampflat_result,
                    "Ok",
                    "cal_lampflat",
                    flist->n,
                    flist,
                    NULL);

    /* Save list of input files as HISTORY in the header */
    qfits_header_add(fh, "COMMENT", "list of input files", NULL, NULL);
    conica_add_files_history(fh, flist) ;

    /* Add QC params as history keyword */
    sprintf(cval, "QC.GAIN= %g", lampflat_config.gain) ;
    qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
    sprintf(cval, "QC.FPNOISE= %g", lampflat_config.fp_noise) ;
    qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
    sprintf(cval, "QC.LAMPFLUX= %g", lampflat_config.lamp_flux) ;
    qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;

    image_save_fits_hdrdump(flat, name, fh, BPP_DEFAULT);
    qfits_header_destroy(fh);

    /* Then, write the PAF file */
    sprintf(name, "%s.paf", outname) ;
    
    /* Open output PAF file (formatted ASCII, see fits/pafs.c) */
    e_comment(0, "saving results to %s", name);
    if ((paf = qfits_paf_print_header(name,
                               "CONICA/lampflat",
                               "QC file",
                               get_login_name(),
                               get_datetime_iso8601())) == NULL) {
        e_error("cannot open file [%s] for output", name) ;
        return -1 ;
    }

    /* Add PRO.CATG */
    if ((sval = pfits_getprokey(INSID, procat_imag_lampflat_qc)) != NULL)
        fprintf(paf, "PRO.CATG       \"%s\" ;# Product category\n", sval);
    /* Add date */
    if ((sval = pfits_get(INSID, flist->name[0], "date_obs")) != NULL) {
        fprintf(paf, "DATE-OBS           \"%s\" ; #Date\n", sval) ;
    }
    /* Add ARCFILE */
    if ((sval = pfits_get(INSID, flist->name[0], "arcfile")) != NULL)
        fprintf(paf, "ARCFILE         \"%s\" ;#\n", sval) ;
    /* Add TPL ID */
    if ((sval = pfits_get(INSID, flist->name[0], "templateid")) != NULL) {
        fprintf(paf, "TPL.ID             \"%s\"; # Template id\n", sval) ;
    }
    /* Add DIT */
    if ((sval = pfits_get(INSID, flist->name[0], "dit")) != NULL) {
        fprintf(paf, "DET.DIT          \"%s\"\n", sval) ;
    }
    /* Add DET.NCORRS */
    if ((sval = pfits_get(INSID, flist->name[0], "rom")) != NULL) {
        fprintf(paf, "DET.NCORRS       \"%s\"\n", sval) ;
    }
    /* Add DET.NCORRS.NAME */
    if ((sval = pfits_get(INSID, flist->name[0], "rom_name")) != NULL) {
        fprintf(paf, "DET.NCORRS.NAME   \"%s\"\n", sval) ;
    }
    /* Add DET.MODE.NAME */
    if ((sval = pfits_get(INSID, flist->name[0], "mode")) != NULL) {
        fprintf(paf, "DET.MODE.NAME   \"%s\"\n", sval) ;
    }
    /* Add DET.NDIT */
    if ((sval = pfits_get(INSID, flist->name[0], "ndit")) != NULL) {
        fprintf(paf, "DET.NDIT         \"%s\"\n", sval) ;
    }
    /* Add INS.LAMP2.NAME */
    if ((sval = pfits_get(INSID, flist->name[0], "lamp2_name")) != NULL) {
        fprintf(paf, "INS.LAMP2.NAME    \"%s\"\n", sval) ;
    }
    /* Add INS.LAMP2.TYPE */
    if ((sval = pfits_get(INSID, flist->name[0], "lamp2_type")) != NULL) {
        fprintf(paf, "INS.LAMP2.TYPE    \"%s\"\n", sval) ;
    }
    /* Add INS.LAMP2.SET */
    if ((sval = pfits_get(INSID, flist->name[0], "lamp2")) != NULL) {
        fprintf(paf, "INS.LAMP2.SET     \"%s\"\n", sval) ;
    }
    /* Add INS.LAMP2.CURRENT */
    if ((sval = pfits_get(INSID, flist->name[0], "lamp2_cur")) != NULL) {
        fprintf(paf, "INS.LAMP2.CURRENT \"%s\"\n", sval) ;
    }
    /* Add INS.OPTI7.NAME */
    if ((sval = pfits_get(INSID, flist->name[0], "opti7_name")) != NULL) {
        fprintf(paf, "INS.OPTI7.NAME    \"%s\"\n", sval) ;
    }
    /* FILTER */
    if ((sval = pfits_get(INSID, flist->name[0], "filter")) != NULL) {
        fprintf(paf, "QC.FILTER.OBS      \"%s\"\n", sval) ;
    }
    sval = pfits_get(INSID, flist->name[0], "opti3_name") ;
    fprintf(paf, "QC.FILTER.NDENS       \"%s\"\n", sval ? sval : "unknown");
    sval = pfits_get(INSID, flist->name[0], "opti4_id") ;
    fprintf(paf, "QC.FILTER.POL         \"%s\"\n", sval ? sval : "unknown");

    /* GAIN */
    fprintf(paf, "QC.GAIN             \"%.4f\"\n", lampflat_config.gain) ;
    /* FPNOISE */
    fprintf(paf, "QC.FPNOISE          \"%.4f\"\n", lampflat_config.fp_noise) ;
    /* LAMPFLUX */
    fprintf(paf, "QC.LAMPFLUX         \"%.4f\"\n", lampflat_config.lamp_flux) ;
     
    fprintf(paf, "\n");
    fclose(paf) ;
    return 0 ;
}

