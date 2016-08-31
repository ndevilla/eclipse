/*----------------------------------------------------------------------------*/
/**
   @file    twflat.c
   @author  N. Devillard
   @date    January 2002
   @version	$Revision: 1.43 $
   @brief   ISAAC imaging flat-field creation from twilight images
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: twflat.c,v 1.43 2005/03/10 13:12:58 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/10 13:12:58 $
	$Revision: 1.43 $
*/

/*-----------------------------------------------------------------------------
								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"
#include "pfits.h"
#include "pfitspro.h"

/*-----------------------------------------------------------------------------
								Defines
 -----------------------------------------------------------------------------*/

#define LO_THRESH_BADPIX	0.5
#define HI_THRESH_BADPIX	2.0

/*-----------------------------------------------------------------------------
                            Static variables
 -----------------------------------------------------------------------------*/

static struct {
    int         error_map_flag ;
    int         pixmap_flag ;
    int         nb_badpix ;
    int         intercepts_flag ;
    int         proportional_flag ;
    image_t *   dark_frame ;
    char    *   dark_name ;
    double      lo_thresh ;
    double      hi_thresh ;
    double      med_stdev ;
    double      med_avg ;
    double      med_min ;
    double      med_max ;
    char    *   name_o ;
    int         nbframes ;
    int         set_rank ;
} tw_config ;

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int isaac_twflat_process(framelist * set);
static int isaac_twilight_save(framelist*, image_t **);
static int isaac_twflat_engine(char *);

/*-----------------------------------------------------------------------------
							        Main
 -----------------------------------------------------------------------------*/
int isaac_twflat_main(void * dict)
{
	dictionary	*	d ;
	char		*	name_i ;
	char        *   sval ;
	int             errors ;

	d = (dictionary*)dict ;
	/* Get options */
	sval = dictionary_get(d, "arg.threshold", NULL) ; 
	if (sval == NULL) {
		tw_config.lo_thresh = LO_THRESH_BADPIX ;
		tw_config.hi_thresh = HI_THRESH_BADPIX ;
	} else {
		if (sscanf(sval, "%lg %lg",
                   &tw_config.lo_thresh,
                   &tw_config.hi_thresh)!=2) {
			tw_config.lo_thresh = LO_THRESH_BADPIX ;
			tw_config.hi_thresh = HI_THRESH_BADPIX ;
		}
	}

    /* Get various flags */
	tw_config.intercepts_flag   = dictionary_getint(d, "arg.intercepts", 0);
	tw_config.error_map_flag    = dictionary_getint(d, "arg.errmap", 0);
	tw_config.pixmap_flag       = dictionary_getint(d, "arg.pixmap", 0);
	tw_config.nb_badpix         = -1 ;
	tw_config.proportional_flag = dictionary_getint(d, "arg.prop", 0);

    /* Get dark frame if required */
	tw_config.dark_name = dictionary_get(d, "arg.dark", NULL) ;

	/* Get input/output file names */
    name_i = dictionary_get(d, "arg.1", NULL) ;
    if (name_i==NULL) {
		e_error("missing input file name(s): aborting");
        return -1 ;
    }

    sval = dictionary_get(d, "arg.output", NULL) ;
    if (sval == NULL) {
        tw_config.name_o = strdup(get_rootname(get_basename(name_i))) ;
    } else {
        tw_config.name_o = strdup(get_rootname(sval)) ;
    }

    /* Initialize set_rank */
    tw_config.set_rank = 0 ;

    INSID = pfits_identify_insstr("isaac");
    /*
     * Command-line options have been cleared out, call the main
     * computing function.
     */
    errors = isaac_twflat_engine(name_i);
    free(tw_config.name_o) ;
	return errors ;
}


/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

static int frame_compare(char * f1, char * f2)
{
    int    comparison ;
    char * v1 ;
    char * v2 ;

    comparison = 1 ;
    if ((v1 = pfits_get(INSID, f1, "filter"))==NULL) {
        e_error("cannot get filter from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "filter"))==NULL) {
        e_error("cannot get filter from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2))
        comparison = 0 ;
    return comparison ;
}

static int isaac_twflat_engine(char * name_i)
{
    framelist   *   f_all ;
    framelist   *   f_one ;
    int             i ;
    int             nsets ;
    int             err ;

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

    /* Load dark frame if needed */
    if (tw_config.dark_name!=NULL) {
        e_comment(0, "---> Loading dark frame: %s", tw_config.dark_name);
        tw_config.dark_frame = image_load(tw_config.dark_name);
        if (tw_config.dark_frame==NULL) {
            e_error("cannot load specified dark: %s",
                    tw_config.dark_name);
            return 1 ;
        }
        e_comment(1, "switching to proportional fit");
        tw_config.proportional_flag = 1 ;
    } else {
        e_comment(0, "---> No dark frame specified");
        tw_config.dark_frame = NULL ;
    }

    /* Process all batches */
    e_comment(0, "---> Processing %d data set(s)", nsets);
    err=0 ;
    for (i=0 ; i<nsets ; i++) {
        /* Build relevant frame list */
        f_one = framelist_select(f_all, i);
        if (f_one==NULL) {
            e_error("classifying batch %d", i+1);
            err++ ;
        } else {
            err += isaac_twflat_process(f_one);
            framelist_del(f_one);
        }
    }
    /* Deallocate objects */
    framelist_del(f_all);
    if (tw_config.dark_frame != NULL) {
        image_del(tw_config.dark_frame);
    }
    e_comment(0, "done");
    return err ;
}

static int isaac_twflat_process(framelist * set)
{
    char        *   filt_name ;
    cube_t      *   in ;
	image_t	    **	results ;
    image_t     *   norm_gain ;
    image_stats *   stat ;
    int             i ;
    double      *   med_list ;
    double          gradient ;
    double          med_sum, med_sqsum ;
    int             err ;

    /* Print out some comments */
    filt_name = pfits_get(INSID, set->name[0], "filter");
    e_comment(0, "---> Filter: [%s]", filt_name ? filt_name : "unknown");
    for (i=0 ; i<set->n ; i++) {
        e_comment(1, "%s", get_basename(set->name[i]));
    }

    /* Load input cube */
	e_comment(1, "---> loading input set");
    in = cube_load_strings(set->name, set->n);
	if (in == NULL)  {
		e_error("loading cube: aborting") ;
		return 1 ;
	}

    /* Set set_rank */
    tw_config.set_rank ++ ;

    /* Set nbframes */
    tw_config.nbframes = set->n ;
    
    /* Allocate median array */
    med_list = malloc(in->np * sizeof(double)) ; 

    /* Compute some stats on input planes */
	e_comment(1, "---> computing stats");
    e_comment(0, "\n"
                 "plane       min        max        med        rms\n"
                 "------------------------------------------------\n"
                 "\n");
    for (i=0 ; i<in->np ; i++) {
        stat = image_getstats(in->plane[i]);
        if (stat!=NULL) {
            e_comment(0, "%02d   %10.2f %10.2f %10.2f %10.2f",
                    i+1,
                    (float)stat->min_pix, (float)stat->max_pix,
                    (float)stat->median_pix, (float)stat->stdev) ;
            med_list[i] = (double)stat->median_pix ;
            if (stat->median_pix < 1e-6) {
                e_error("plane %d has negative flux: aborting", i+1);
                cube_del(in);
                free(med_list) ;
                free(stat);
                return 1 ;
            }
            free(stat);
        }
    }
    e_comment(0, "------------------------------------------------\n");
 
    /* Compute min max stdev and mean of the medians */
    tw_config.med_avg = 0 ;
    tw_config.med_min = tw_config.med_max = med_list[0] ;
    med_sum = med_sqsum = 0 ;
    for (i=0 ; i<in->np ; i++) {
        if (med_list[i] < tw_config.med_min) tw_config.med_min = med_list[i] ;
        if (med_list[i] > tw_config.med_max) tw_config.med_max = med_list[i] ;
        med_sqsum += med_list[i] * med_list[i] ;
        med_sum += med_list[i] ;
        tw_config.med_avg += med_list[i] ;
    } 
    tw_config.med_avg /= in->np ;
    /* Rounding errors can cause the variance to be negative */
    tw_config.med_stdev = (med_sqsum-((med_sum*med_sum)/(double)in->np))
            /((double)in->np-1.0) ;
    tw_config.med_stdev = tw_config.med_stdev > 0
                        ? sqrt(tw_config.med_stdev) : 0;
    free(med_list) ;
    
    /* See if flux gradient is large enough for a correct fit */
    if (tw_config.proportional_flag==0) {
        gradient = fabs((double)tw_config.med_max/(double)tw_config.med_min) ;
        if (gradient < 4.0) {
            e_warning("low flux gradient: %g < 4.0\n"
                      "a proportional fit may give better results\n"
                      "(requires a master dark frame)\n",
                      gradient);
        }
    }

	/* Apply dark correction to all planes if requested */
	if (tw_config.dark_name!=NULL) {
        e_comment(1, "---> subtracting dark");
        cube_sub_im(in, tw_config.dark_frame);
	}
	/* Fit slopes, get results */
	e_comment(1, "---> fitting slopes");
	if (tw_config.proportional_flag) {
        results = cube_create_gainmap_proportional(in);
    } else {
        results = cube_create_gainmap_robust(in) ;
    }
	cube_del(in) ;

	if (results == NULL) {
		e_error("creating twilight flat-field: aborting") ;
		return 1 ;
	}
    if ((results[0] == NULL) || (results[1] == NULL)) {
		e_error("creating twilight flat-field: aborting") ;
		return 1 ;
	}

    /* Normalize the gain */
    norm_gain = image_normalize(results[0], NORM_MEAN) ;
    image_del(results[0]) ;
    results[0] = norm_gain ;
        
    /* Save results */
	e_comment(1, "---> saving output");
    err = isaac_twilight_save(set, results);

	if (results[0]!=NULL) image_del(results[0]);
	if (results[1]!=NULL) image_del(results[1]);
	if (!tw_config.proportional_flag) 
        if (results[2]!=NULL) image_del(results[2]);
	free(results) ;

    return err ;
}


static int isaac_twilight_save(
        framelist   *   ilist,
        image_t     **  results)
{
	qfits_header*	fh ;
	pixelmap	*	badpixmap ;
	char			full_name[FILENAMESZ];
	image_t		*	promoted ;
    FILE        *   paf ;
    char        *   sval ;

	/* SAVE FLAT-FIELD FITS PRODUCTS */
	sprintf(full_name, "%s_%d_flat.fits",
            tw_config.name_o,
            tw_config.set_rank);
    e_comment(1, "saving twilight flat:    [%s]", full_name);
	e_comment(1, "using header from frame [%s]", get_basename(ilist->name[0]));
	 
	/* Get FITS header from reference file */
	if ((fh = qfits_header_read(ilist->name[0])) == NULL) {
		e_error("getting header from reference frame");
		return -1 ;
	}
	/* Prepare the header */
	isaac_header_for_image(fh) ;
	
	/* Add DataFlow keywords */
	isaac_pro_fits(fh,
					full_name,
					"REDUCED",
					NULL,
                    procat_imag_sw_flat_result,
					"Ok",
					"cal_twflats",
					ilist->n,
					ilist,
					NULL);
	
	/* Save list of input files as HISTORY in the header */
	qfits_header_add(fh, "COMMENT", "list of input files", NULL, NULL);
    isaac_add_files_history(fh, ilist) ;
	image_save_fits_hdrdump(results[0], full_name, fh, BPP_DEFAULT);
	qfits_header_destroy(fh);

	/* Create and save badpixel map if requested */
	if (tw_config.pixmap_flag) {
        sprintf(full_name, "%s_%d_badpix.fits",
                tw_config.name_o,
                tw_config.set_rank);
        e_comment(1, "saving bad pixel map:    [%s]", full_name);
		badpixmap = image_threshold2pixelmap(results[0],
											 tw_config.lo_thresh,
											 tw_config.hi_thresh);
		if (badpixmap == NULL) {
			e_error("creating bad pixel map");
		} else {
            tw_config.nb_badpix = badpixmap->lx * badpixmap->ly - 
				pixelmap_getselected(badpixmap) ;
			if ((promoted = pixelmap_2_image(badpixmap)) == NULL) {
				e_error("cannot promote pixelmap") ;
			} else {
				pixelmap_del(badpixmap);
				fh = qfits_header_read(ilist->name[0]);
				isaac_header_for_image(fh) ;
				isaac_pro_fits(fh,
						full_name,
						"REDUCED",
						NULL,
                        procat_imag_sw_flat_badpix,
						"OK",
						"cal_twflats",
						ilist->n,
						ilist,
						NULL);
				image_save_fits_hdrdump(promoted,full_name,fh,BPP_8_UNSIGNED);
				image_del(promoted) ;
				qfits_header_destroy(fh) ;
			}
		}
	}
	
	/* Out results: diff. cases if the fit was linear or purely proportional */
	/* Linear fit: results[1] has intercept map */
	            /* results[2] has error map */

	if (!tw_config.proportional_flag) {
		/* Save intercepts map if requested */
		if (tw_config.intercepts_flag) {
            sprintf(full_name, "%s_%d_intercept.fits",
                    tw_config.name_o,
                    tw_config.set_rank);
            e_comment(1, "saving intercept map:    [%s]", full_name);
			if (results[1]!=NULL) {
				fh = qfits_header_read(ilist->name[0]);
				isaac_header_for_image(fh) ;
				isaac_pro_fits(fh,
						full_name,
						"REDUCED",
						NULL,
                        procat_imag_sw_flat_interce,
						"OK",
						"cal_twflats",
						ilist->n,
						ilist,
						NULL);
				image_save_fits_hdrdump(results[1], full_name, fh, BPP_DEFAULT);
				qfits_header_destroy(fh) ;
			} else {
				e_error("null intercept map: cannot save");
			}
		}
		/* Save error map if requested */
		if (tw_config.error_map_flag) {
            sprintf(full_name, "%s_%d_errmap.fits",
                    tw_config.name_o,
                    tw_config.set_rank);
            e_comment(1, "saving error map    :    [%s]", full_name);
			if (results[2]!=NULL) {
				fh = qfits_header_read(ilist->name[0]);
				isaac_header_for_image(fh) ;
				isaac_pro_fits(fh,
						full_name,
						"REDUCED",
						NULL,
                        procat_imag_sw_flat_errmap,
						"OK",
						"cal_twflats",
						ilist->n,
						ilist,
						NULL);
				image_save_fits_hdrdump(results[2], full_name, fh, BPP_DEFAULT);
				qfits_header_destroy(fh) ;
			} else {
				e_error("null error map: cannot save");
			}
		}
	} else {

	/* Proportional fit: results[1] has error map - no intercept map */
		if (tw_config.intercepts_flag) {
			e_warning("no intercept map for proportional fit");
		}

		if (tw_config.error_map_flag) {
            sprintf(full_name, "%s_%d_errmap.fits",
                    tw_config.name_o,
                    tw_config.set_rank);
            e_comment(1, "saving error map    :    [%s]", full_name);
			if (results[1]!=NULL) {
				fh = qfits_header_read(ilist->name[0]);
				isaac_header_for_image(fh) ;
                isaac_pro_fits(fh,
                        full_name,
                        "REDUCED",
                        NULL,
                        procat_imag_sw_flat_errmap,
                        "OK",
                        "cal_twflats",
                        ilist->n,
						ilist,
						NULL);
                image_save_fits_hdrdump(results[1], full_name, fh, BPP_DEFAULT);
                qfits_header_destroy(fh) ;
			} else {
				e_error("null error map: cannot save");
			}
		}
	}
	
    /* SAVE FLAT-FIELD PAF PRODUCT */
	sprintf(full_name, "%s_%d_flat.paf", tw_config.name_o, tw_config.set_rank);
    e_comment(1, "saving PAF file:        [%s]", full_name);

    /* Open output PAF file */
    paf = qfits_paf_print_header(full_name,
                                    "ISAAC/twflat",
                                    "Isaac twflat QC parameters",
                                    get_login_name(),
                                    get_datetime_iso8601());
    if (paf == NULL) {
        e_error("cannot open file [%s] for output: aborting RON", full_name);
        return -1 ;
    }

    /* Add PRO.CATG */
    if ((sval = pfits_getprokey(INSID, procat_imag_sw_flat_qc)) != NULL)
        fprintf(paf, "PRO.CATG       \"%s\" ;# Product category\n", sval);

    /* Add date */
    if ((sval = pfits_get(INSID, ilist->name[0], "date_obs")) != NULL)
        fprintf(paf, "DATE-OBS        \"%s\" ;# Date\n", sval) ;

    /* Add DET.CHIP.NAME */
    if ((sval = pfits_get(INSID, ilist->name[0], "chip")) != NULL)
        fprintf(paf, "DET.CHIP.NAME   \"%s\" ;#\n", sval) ;
    
    /* Add ARCFILE */
    if ((sval = pfits_get(INSID, ilist->name[0], "arcfile")) != NULL)
        fprintf(paf, "ARCFILE         \"%s\" ;#\n", sval) ;
    
    /* Add TPL.ID  */
    if ((sval = pfits_get(INSID, ilist->name[0], "templateid")) != NULL)
        fprintf(paf, "TPL.ID          \"%s\" ;\n", sval) ;
    
    /* Add DET.MODE.NAME  */
    if ((sval = pfits_get(INSID, ilist->name[0], "romode_name")) != NULL)
        fprintf(paf, "DET.MODE.NAME   \"%s\" ;\n", sval) ;

    /* Add DET.NCORRS.NAME  */
    if ((sval = pfits_get(INSID, ilist->name[0], "romode_name2")) != NULL)
        fprintf(paf, "DET.NCORRS.NAME  \"%s\" ;\n", sval) ;

    /* Add DET.CHIP.NAME  */
    if ((sval = pfits_get(INSID, ilist->name[0], "chip")) != NULL)
        fprintf(paf, "DET.CHIP.NAME    \"%s\" ;\n", sval) ;

    /* Add DET.RSPEED  */
    if ((sval = pfits_get(INSID, ilist->name[0], "rspeed")) != NULL)
        fprintf(paf, "DET.RSPEED       \"%s\" ;\n", sval) ;

    /* Add DET.DIT  */
    if ((sval = pfits_get(INSID, ilist->name[0], "dit")) != NULL)
        fprintf(paf, "DET.DIT          \"%s\" ;\n", sval) ;
    
    /* Add PRO.DATANCOM */
    fprintf(paf, "PRO.DATANCOM         \"%d\" ;\n", tw_config.nbframes);
     
    /* Add FILTER */
    if ((sval = pfits_get(INSID, ilist->name[0], "filter")) != NULL) 
        fprintf(paf, "QC.FILTER.OBS    \"%s\" ;\n", sval) ;
   
    /* Add OBJECTIVE */
    if ((sval = pfits_get(INSID, ilist->name[0], "objective")) != NULL) 
        fprintf(paf, "QC.OBJECTIVE    \"%s\" ;\n", sval) ;

    /* QC.TWFLAT.MEDMIN */
    fprintf(paf, "QC.TWFLAT.MEDMIN    %g\n", tw_config.med_min) ;
    /* QC.TWFLAT.MEDMAX */
    fprintf(paf, "QC.TWFLAT.MEDMAX    %g\n", tw_config.med_max) ;
    /* QC.TWFLAT.MEDAVG */
    fprintf(paf, "QC.TWFLAT.MEDAVG    %g\n", tw_config.med_avg) ;
    /* QC.TWFLAT.MEDSTDEV */
    fprintf(paf, "QC.TWFLAT.MEDSTDEV  %g\n", tw_config.med_stdev) ;
    /* QC.TWFLST.NBADPIX */
    if (tw_config.nb_badpix >= 0) 
        fprintf(paf, "QC.TWFLAT.NBADPIX  %d\n", tw_config.nb_badpix) ;
  
    fclose(paf) ;
	return 0 ;
}
