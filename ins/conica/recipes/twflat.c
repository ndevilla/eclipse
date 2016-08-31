/*----------------------------------------------------------------------------*/
/**
   @file    twflat.c
   @author  N. Devillard
   @date    January 2002
   @version	$Revision: 1.21 $
   @brief   CONICA imaging flat-field creation from twilight images
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: twflat.c,v 1.21 2003/08/07 09:46:17 yjung Exp $
	$Author: yjung $
	$Date: 2003/08/07 09:46:17 $
	$Revision: 1.21 $
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

#define LO_THRESH_BADPIX	0.5
#define HI_THRESH_BADPIX	2.0

#define REJ_LEFT            200
#define REJ_RIGHT           200
#define REJ_BOTTOM          200
#define REJ_TOP             200

/*-----------------------------------------------------------------------------
                            Static variables
 -----------------------------------------------------------------------------*/

static struct {
    int         error_map_flag ;
    int         pixmap_flag ;
    int         intercepts_flag ;
    int         proportional_flag ;
    char    *   dark_name ;
    double      lo_thresh ;
    double      hi_thresh ;
    char    *   name_o ;
    int         set_rank ;
    int         rej_left ;
    int         rej_right ;
    int         rej_bottom ;
    int         rej_top ;
} tw_config ;

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int conica_twflat_process(framelist * set, framelist * darks) ;
static int conica_twilight_save(framelist*, framelist*, image_t **) ;
static int conica_twflat_engine(char *) ;

/*-----------------------------------------------------------------------------
							Main code
 -----------------------------------------------------------------------------*/
int conica_twflat_main(void * dict)
{
	dictionary	*	d ;
	char		*	name_i ;
	char        *   sval ;
	int             errors ;

	d = (dictionary*)dict ;
	/* Get options */
    /* Threshold for bad pixels map */
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

    /* Rejected borders */
    sval = dictionary_get(d, "arg.rej_bord", NULL) ;
    if (sval == NULL) {
        tw_config.rej_left =   REJ_LEFT ;
        tw_config.rej_right =  REJ_RIGHT ;
        tw_config.rej_bottom = REJ_BOTTOM ;
        tw_config.rej_top =    REJ_TOP ;
    } else {
        if (sscanf(sval, "%d %d %d %d",
                    &tw_config.rej_left,
                    &tw_config.rej_right,
                    &tw_config.rej_bottom,
                    &tw_config.rej_top)!=4) {
            tw_config.rej_left =   REJ_LEFT ;
            tw_config.rej_right =  REJ_RIGHT ;
            tw_config.rej_bottom = REJ_BOTTOM ;
            tw_config.rej_top =    REJ_TOP ;
        }
    }
    
    /* Get various flags */
	tw_config.intercepts_flag   = dictionary_getint(d, "arg.intercepts", 0);
	tw_config.error_map_flag    = dictionary_getint(d, "arg.errmap", 0);
	tw_config.pixmap_flag       = dictionary_getint(d, "arg.pixmap", 0);
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

    INSID = pfits_identify_insstr("naco");
    /*
     * Command-line options have been cleared out, call the main
     * computing function.
     */
    errors = conica_twflat_engine(name_i);
    free(tw_config.name_o) ;
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

    /* Initialize */
    comparison = 1 ;

    /* Compare the filter */
    if ((v1 = pfits_get(INSID, f1, "filter"))==NULL) {
        e_error("cannot get filter from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "filter"))==NULL) {
        e_error("cannot get filter from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;

    /* Compare the read-out mode */
    if ((v1 = pfits_get(INSID, f1, "rom_name"))==NULL) {
        e_error("cannot get rom name from [%s]", f1);
        return -1 ;
    }
    if ((v2 = pfits_get(INSID, f2, "rom_name"))==NULL) {
        e_error("cannot get rom name from [%s]", f2);
        return -1 ;
    }
    if (strcmp(v1, v2)) comparison = 0 ;
    
    /* Return */
    return comparison ;
}

static int conica_twflat_engine(char * name_i)
{
    framelist   *   f_all ;
    framelist   *   f_one ;
    framelist   *   f_dark ;
    framelist   *   dark_list ;
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
        /* Get the list of dark names */
        dark_list = framelist_load(tw_config.dark_name) ;
        if (dark_list == NULL) {
			e_error("invalid file list") ;
			return 1 ;
		}
		/* Nb of frame has to be 1 or f_all->n */
		if ((dark_list->n != f_all->n) && (dark_list->n != 1)) {
            e_error("bad nb of provided dark frames") ;
            return 1 ;
        }
        /* Copy the labels from f_all */
        if (dark_list->n != 1) {
            for (i=0 ; i<f_all->n ; i++) dark_list->label[i] = f_all->label[i];
        }
        e_comment(1, "switching to proportional fit");
        tw_config.proportional_flag = 1 ;
    } else {
        e_comment(0, "---> No dark frame specified");
    }

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
            if (tw_config.dark_name!=NULL) {
                /* Build the framelist with the dark frames */
                if (dark_list->n != 1) {
                   f_dark = framelist_select(dark_list, i) ; 
                } else {
                   f_dark = framelist_copy(dark_list) ;
                }
            } else f_dark = NULL ;
            err += conica_twflat_process(f_one, f_dark) ;
            framelist_del(f_one);
            if (f_dark != NULL) framelist_del(f_dark);
        }
    }
    /* Deallocate objects */
    framelist_del(f_all);
    if (tw_config.dark_name != NULL) framelist_del(dark_list);
    e_comment(0, "done");
    return err ;
}

static int conica_twflat_process(
        framelist   *   set,
        framelist   *   darks)
{
    char        *   filt_name ;
    char        *   rom_name ;
    char        *   tpl_id ;
    char        *   rom ;
    cube_t      *   in ;
    cube_t      *   dark_cube ;
	image_t	    **	results ;
    image_t     *   norm_gain ;
    double          norm ;
    image_stats *   stat ;
    int             i ;
    pixelvalue      min_count, max_count ;
    double          gradient ;
	int				corr_dark ;
    int             err ;

	/* Initialize */
	corr_dark = 1 ;
	
	/* Check darks validity */
	if (darks != NULL) {
		for (i=0 ; i<darks->n ; i++) {
			if (file_exists(darks->name[i])!=1) corr_dark = 0 ;
		}
	} else corr_dark = 0 ;
	
    /* Print out some comments */
    filt_name = pfits_get(INSID, set->name[0], "filter");
    e_comment(0, "  ---> * Filter:   [%s]", filt_name ? filt_name : "unknown");
    rom_name = pfits_get(INSID, set->name[0], "rom_name");
    e_comment(0, "       * Read-out: [%s]", rom_name ? rom_name : "unknown");
    for (i=0 ; i<set->n ; i++) {
        e_comment(1, "%s", get_basename(set->name[i]));
    }

    /* Get templateid and rom_name to remove the median test in case  */
    /* the values are NACO_img_cal_SkyFlats and Uncorr */
    tpl_id = pfits_get(INSID, set->name[0], "templateid");
    rom = pfits_get(INSID, set->name[0], "rom_name");
    if ((tpl_id == NULL) || (rom == NULL)) {
        e_error("cannot read TPL.ID or DET.NCORRS.NAME") ;
        return -1 ;
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
                    (float)stat->min_pix,
                    (float)stat->max_pix,
                    (float)stat->median_pix,
                    (float)stat->stdev);
            if (i==0) {
                min_count = stat->median_pix ;
                max_count = stat->median_pix ;
            } else {
                if (stat->median_pix < min_count)
                    min_count = stat->median_pix ;

                if (stat->median_pix > max_count)
                    max_count = stat->median_pix ;
            }
            /* Test only if not Uncorr or not skyflats */
            if (strcmp(tpl_id, "NACO_img_cal_SkyFlats") || 
                    strcmp(rom, "Uncorr")) {
                if (stat->median_pix < 1e-6) {
                    e_error("plane %d has negative flux: aborting", i+1);
                    cube_del(in);
                    free(stat);
                    return 1 ;
                }
            }
            free(stat);
        }
    }
    e_comment(0, "------------------------------------------------\n");

    /* See if flux gradient is large enough for a correct fit */
    if (tw_config.proportional_flag==0) {
        gradient = fabs((double)max_count/(double)min_count) ;
        if (gradient < 4.0) {
            e_warning("low flux gradient: %g\n"
                      "a proportional fit may give better results\n"
                      "(requires a master dark frame)\n",
                      gradient);
        }
    }

	/* Apply dark correction to all planes if requested */
	if (corr_dark) {
        e_comment(1, "---> subtracting dark");
        /* Load dark cube */
        e_comment(2, "---> loading dark set");
        dark_cube = cube_load_strings(darks->name, darks->n);
        if (dark_cube == NULL)  {
            e_error("loading cube: aborting") ;
            return 1 ;
        }
        for (i=0 ; i<darks->n ; i++) {
            e_comment(2, "dark %2d ---> %s", i+1, darks->name[i]) ;
        }
        /* Dark correction */
        if (dark_cube->np == 1) {
            cube_sub_im(in, dark_cube->plane[0]);
        } else {
            cube_sub(in, dark_cube) ;
        }
        cube_del(dark_cube) ;
	}

	/* Fit slopes, get results */
	e_comment(1, "---> fitting slopes");
	if (tw_config.proportional_flag) {
        results = cube_create_gainmap_proportional(in);
    } else {
        results = cube_create_gainmap_robust(in) ;
    }
	cube_del(in) ;

	if ((results == NULL) || (results[0] == NULL) || (results[1] == NULL)) {
		e_error("creating twilight flat-field: aborting") ;
		return 1 ;
	}

    /* Normalize gain */
    norm = image_getmean_vig(results[0], tw_config.rej_left, 
            results[0]->lx-tw_config.rej_right,
            tw_config.rej_bottom,
            results[0]->ly-tw_config.rej_top) ;
    norm_gain = image_cst_op(results[0], norm, '/');
    image_del(results[0]) ;
    results[0] = norm_gain ;
    
    /* Save results */
	e_comment(1, "---> saving output");
    if (corr_dark==0) err = conica_twilight_save(set, NULL, results);
    else err = conica_twilight_save(set, darks, results);
    

	if (results[0]!=NULL) image_del(results[0]);
	if (results[1]!=NULL) image_del(results[1]);
	if (!tw_config.proportional_flag)
		if (results[2]!=NULL)
			image_del(results[2]);
	free(results) ;

    return err ;
}


static int conica_twilight_save(
    framelist * ilist,
    framelist * dlist,
    image_t  ** results)
{
	qfits_header*	fh ;
	pixelmap	*	badpixmap ;
	char			full_name[FILENAMESZ];
	image_t		*	promoted ;

	e_comment(1, "using header from frame [%s]", get_basename(ilist->name[0]));
	/* Save flat-field */
	sprintf(full_name, "%s_%d_flat.fits",
            tw_config.name_o,
            tw_config.set_rank);
    e_comment(1, "saving twilight flat:    [%s]", full_name);
	 
	/* Get FITS header from reference file */
	if ((fh = qfits_header_read(ilist->name[0])) == NULL) {
		e_error("getting header from reference frame");
		return -1 ;
	}
	/* Prepare the header */
	conica_header_for_image(fh) ;
	
	/* Add DataFlow keywords */
	conica_pro_fits(fh,
					full_name,
					"REDUCED",
					NULL,
                    procat_imag_sw_flat_result,
					"Ok",
					"cal_twflats",
					ilist->n,
					ilist,
					dlist);
	
	/* Save list of input files as HISTORY in the header */
	qfits_header_add(fh, "COMMENT", "list of input files", NULL, NULL);
    conica_add_files_history(fh, ilist) ;
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
			if ((promoted = pixelmap_2_image(badpixmap)) == NULL) {
				e_error("cannot promote pixelmap") ;
			} else {
				pixelmap_del(badpixmap);
				fh = qfits_header_read(ilist->name[0]);
				conica_header_for_image(fh) ;
				conica_pro_fits(fh,
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
	
	/*
	 * Output results: different cases if the fit was linear or purely
	 * proportional
	 */
		
	/*
	 * Linear fit: results[1] has intercept map
	 *             results[2] has error map
	 */

	if (!tw_config.proportional_flag) {
		/* Save intercepts map if requested */
		if (tw_config.intercepts_flag) {
            sprintf(full_name, "%s_%d_intercept.fits",
                    tw_config.name_o,
                    tw_config.set_rank);
            e_comment(1, "saving intercept map:    [%s]", full_name);
			if (results[1]!=NULL) {
				fh = qfits_header_read(ilist->name[0]);
				conica_header_for_image(fh) ;
				conica_pro_fits(fh,
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
				conica_header_for_image(fh) ;
				conica_pro_fits(fh,
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

	/*
	 * Proportional fit: results[1] has error map
	 *					 no intercept map
	 */
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
				conica_header_for_image(fh) ;
                conica_pro_fits(fh,
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
	return 0 ;
}
