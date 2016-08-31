/*----------------------------------------------------------------------------*/
/**
   @file    skybg.c
   @author  N. Devillard
   @date    January 2001
   @version	$Revision: 1.29 $
   @brief   ISAAC sky background measurement
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: skybg.c,v 1.29 2002/12/10 15:00:54 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/10 15:00:54 $
	$Revision: 1.29 $
*/

/*
    $Id: skybg.c,v 1.29 2002/12/10 15:00:54 yjung Exp $
    $Author: yjung $
    $Date: 2002/12/10 15:00:54 $
    $Revision: 1.29 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                                New types
 -----------------------------------------------------------------------------*/

typedef enum _skybg_mode_ {
    skybg_auto,
    skybg_lw_imag,
    skybg_lw_spec,
    skybg_unknown
} skybg_mode ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int compute_skybg(char *, char *, skybg_mode) ;
static skybg_mode skybg_findmode(char * filename);
static double * skybg_lw_imag_compute(char * name_i, int * nval);
static double * skybg_lw_spec_compute(char * name_i, int * nval);
static int skybg_printpaf(skybg_mode, double*, int, char*, char*);
static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                                Main 
 -----------------------------------------------------------------------------*/
int isaac_skybg_main(void * dict)
{
    dictionary  *   d ;
    char        *   name_i ;
    char        *   name_o ;
    int             status ;
    char        *   s ;
    skybg_mode      mode ;
 
    d = (dictionary*)dict ;
    
    /* Get options */
    mode = skybg_auto ;
    s = dictionary_get(d, "arg.mode", NULL);
    if (s!=NULL) {
        if (!strcmp(s, "lw-imag")) {
            mode = skybg_lw_imag;
        } else if (!strcmp(s, "lw-spec")) {
            mode = skybg_lw_spec;
        } else {
            e_error("invalid mode: %s", s);
            return -1 ;
        }
    }

    /* Get input/output file names */
    if (dictionary_getint(d, "arg.n", -1)<2) {
        e_error("missing input file name: aborting");
        return -1 ;
    }
    name_i = dictionary_get(d, "arg.1", NULL) ;
    name_o = dictionary_get(d, "arg.output", NULL) ;
    if (name_o == NULL) {
        name_o = strdup(get_rootname(get_basename(name_i))) ;
    } else {
        name_o = strdup(get_rootname(name_o)) ;
    }

    INSID = pfits_identify_insstr("isaac");

    status = compute_skybg(name_i, name_o, mode) ;
    free(name_o) ;
    return status ;
}


/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/
static int compute_skybg(char * name_i, char * name_o, skybg_mode mode)
{
    int         status ;
    double  *   bg ;
    int         nval ;

    /* If mode is automatic, determine which mode should be used */
    if (mode==skybg_auto) {
        mode = skybg_findmode(name_i) ;
    }

    /* Switch on processing mode */
    switch (mode) {
        case skybg_lw_imag:
        e_comment(0, "Mode is LW imaging");
        bg = skybg_lw_imag_compute(name_i, &nval) ;
        break ;

        case skybg_lw_spec:
        e_comment(0, "Mode is LW spectroscopy");
        bg = skybg_lw_spec_compute(name_i, &nval) ;
        break ;

        default:
        e_error("cannot determine mode: use -m/--mode option");
        bg = NULL ;
        break ;
    }

    if (bg==NULL) {
        e_error("computing background");
        status = -1 ;
    } else {
        status = skybg_printpaf(mode, bg, nval, name_i, name_o);
        free(bg);
    }
    
    return status ;
}

static skybg_mode skybg_findmode(char * filename)
{
    char    *   name ;
    char    *   dpr_tech ;
    char    *   sval ;
    skybg_mode  mode ;

    if (is_fits_file(filename)) {
        name = filename ;
    } else if (is_ascii_list(filename)) {
        name = framelist_firstname(filename);
    } else {
        e_error("unrecognized file format for file %s", filename);
        name = NULL ;
    }

    if (name==NULL) return skybg_unknown ;

    sval = pfits_get(INSID, name, "arm");
    if (sval==NULL) {
        e_error("cannot determine SW/LW mode for file %s", name);
        return skybg_unknown ;
    }

    if (toupper(sval[0]) != 'L') {
        e_error("Mode should be LW mode for file %s", name);
        mode = skybg_unknown ;
    }

    dpr_tech = pfits_get(INSID, name, "dpr_tech");
    if (dpr_tech==NULL) {
        e_error("cannot determine spectro/imaging for file %s", name);
        mode = skybg_unknown ;
    } else if (!strcmp(dpr_tech, "SPECTRUM")) {
        mode = skybg_lw_spec ;
    } else if (!strcmp(dpr_tech, "IMAGE")) {
        mode = skybg_lw_imag ;
    } else {
        e_error("unrecognized DPR TECH value for file %s: %s", name, dpr_tech);
        mode = skybg_unknown ;
    }

    return mode ;
}

static double * skybg_lw_imag_compute(char * name_i, int * nval)
{
    cube_t      *   cu ;
    image_t *   central ;
    pixelvalue      med ;
    double      *   bg ;

    /* Load input cube */
    e_comment(0, "loading [%s]", name_i);
    cu = cube_load(name_i) ;
    if (cu==NULL) {
        e_error("loading [%s]", name_i);
        return NULL ;
    }

    /* Compute median within central zone for each frame */
    e_comment(0, "computing background...");
    /* Extract central zone of the image */
    central = image_getvig(cu->plane[0], 100, 100, 900, 900);
    med = image_getmedian(central);
    image_del(central);
    e_comment(0, "Background is %g ADUs", med);

    /* Deallocate input cube */
    cube_del(cu);

    bg = malloc(sizeof(double));
    bg[0] = (double)med ;
    (*nval)=1 ;
    return bg ;
}


static double * skybg_lw_spec_compute(char * name_i, int * nval)
{
    cube_t  *   cu ;
    double  *   bg;
    int         npix ;
    int         rank ;

    /* Load input cube */
    e_comment(0, "loading [%s]", name_i);
    cu = cube_load(name_i) ;
    if (cu==NULL) {
        e_error("loading [%s]", name_i);
        return NULL ;
    }

    e_comment(0, "computing background...");
    /* Sort pixels in the first image */
    npix = cu->plane[0]->lx * cu->plane[0]->ly ;
    pixel_qsort(cu->plane[0]->data, npix);

    /* Get the percentiles at 50, 90 and 95% */

    bg = malloc(3 * sizeof(double));
    rank = (int)(0.50 * npix) ;
    bg[0] = (double)cu->plane[0]->data[rank] ;
    rank = (int)(0.90 * npix) ;
    bg[1] = (double)cu->plane[0]->data[rank] ;
    rank = (int)(0.95 * npix) ;
    bg[2] = (double)cu->plane[0]->data[rank] ;

    cube_del(cu);

    e_comment(0, "Percentile values:");
    e_comment(1, "50%% - %g", bg[0]);
    e_comment(1, "90%% - %g", bg[1]);
    e_comment(1, "95%% - %g", bg[2]);

    (*nval)=3 ;
    return bg ;
}

static int skybg_printpaf(
        skybg_mode      mode,
        double      *   bg,
        int             nval,
        char        *   name_i,
        char        *   name_o)
{
    char        name_paf[FILENAMESZ];
    FILE    *   paf_out ;
    char    *   sval ;
    char    *   first_filename;
    int         i ;
    
    /* Store results into a PAF file */
    sprintf(name_paf, "%s.paf", name_o);
    e_comment(0, "writing results to PAF file [%s]", name_paf);
    paf_out = qfits_paf_print_header(name_paf,
                               "ISAAC/skybg",
                               "Background measurement",
                               get_login_name(),
                               get_datetime_iso8601());
    fprintf(paf_out, "PRO.CATG               \"%s\"\n", 
            pfits_getprokey(INSID, procat_imag_bg)) ;
    fprintf(paf_out, "INSTRUME               \"ISAAC\"\n");

    /* Get the first file name */
    first_filename=NULL ;
    if (is_fits_file(name_i)) {
        first_filename = name_i;
    } else if (is_ascii_list(name_i)) {
        first_filename = framelist_firstname(name_i);
    }

    /* Forward  a number of header infos from the input to the PAF */
    /* ARCFILE */
    sval = pfits_get(INSID, first_filename, "arcfile") ;
    if (sval != NULL) fprintf(paf_out, "ARCFILE   \"%s\"  \n", sval) ;

    /* MJD-OBS */
    sval = pfits_get(INSID, first_filename, "mjdobs");
    if (sval!=NULL) {
        fprintf(paf_out, "MJD-OBS                %s ;# Observation date\n",
                sval);
    } else {
        fprintf(paf_out, "MJD-OBS                0.0 ;# unknown\n");
    }

    fprintf(paf_out, "\n");

    /* INS.MODE */
    sval = pfits_get(INSID, first_filename, "mode");
    if (sval!=NULL) {
        fprintf(paf_out, "INS.MODE               \"%s\"\n", sval);
    }

    if (mode==skybg_lw_imag) {
        /* INS FILT3 NAME */
        sval = qfits_query_hdr(first_filename, "ins.filt3.name");
        if (sval!=NULL) {
            fprintf(paf_out, "INS.FILT3.NAME         \"%s\"\n", sval);
        }
        /* INS FILT4 NAME */
        sval = qfits_query_hdr(first_filename, "ins.filt4.name");
        if (sval!=NULL) {
            fprintf(paf_out, "INS.FILT4.NAME         \"%s\"\n", sval);
        }
    }

    if (mode==skybg_lw_spec) {
        /* INS OPTI1 NAME */
        sval = pfits_get(INSID, first_filename, "optical_id");
        if (sval!=NULL) {
            fprintf(paf_out, "INS.OPTI1.NAME         \"%s\"\n", sval);
        }
    }

    /* INS OPTI3 NAME */
    sval = qfits_query_hdr(first_filename, "ins.opti3.name");
    if (sval!=NULL) {
        fprintf(paf_out, "INS.OPTI3.NAME         \"%s\"\n", sval);
    }

    if (mode==skybg_lw_spec) {
        /* INS GRAT WLEN */
        sval = qfits_query_hdr(first_filename, "ins.grat.wlen");
        if (sval!=NULL) {
            fprintf(paf_out, "INS.GRAT.WLEN          \"%s\"\n", sval);
        }
    }

    fprintf(paf_out, "\n");

    /* DET MODE NAME */
    sval = pfits_get(INSID, first_filename, "romode_name");
    if (sval!=NULL) {
        fprintf(paf_out, "DET.MODE.NAME          \"%s\"\n", sval);
    }
    /* DET DIT */
    sval = pfits_get(INSID, first_filename, "dit");
    if (sval!=NULL) {
        fprintf(paf_out, "DET.DIT                %s\n", sval);
    }

    /* Print out measured background value */
    fprintf(paf_out, "\n");
    if (mode==skybg_lw_imag) {
        fprintf(paf_out, "QC.SKY.BACKGROUND      %g\n", bg[0]);
    }
    if (mode==skybg_lw_spec) {
        for (i=0 ; i<nval ; i++) {
            fprintf(paf_out, "QC.SKY.BACKGROUND.P%d   %g\n", i, bg[i]);
        }
    }
    fprintf(paf_out, "\n");
    fclose(paf_out);
    e_comment(0, "done");

    return 0 ;
}
