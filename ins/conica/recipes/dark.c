/*----------------------------------------------------------------------------*/
/**
   @file	dark.c
   @author	Y. Jung
   @date	January 2002
   @version	$Revision: 1.24 $
   @brief	CONICA dark recipe
*/
/*----------------------------------------------------------------------------*/

/*
   $Id: dark.c,v 1.24 2003/08/18 11:13:46 yjung Exp $
   $Author: yjung $
   $Date: 2003/08/18 11:13:46 $
   $Revision: 1.24 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eclipse.h"
#include "conicap_lib.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#define COLD_THRESH     6
#define HOT_THRESH      10
#define DEV_THRESH      5
#define REJ_LEFT        200
#define REJ_RIGHT       200
#define REJ_TOP         200
#define REJ_BOTTOM      200
#define RON_NBSAMPLES   100 
#define RON_HS          2

/*-----------------------------------------------------------------------------
                            Static variables
 -----------------------------------------------------------------------------*/

static struct {
    double      hot_thresh ;
    double      cold_thresh ;
    double      dev_thresh ;
    int         rej_left ;
    int         rej_right ;
    int         rej_bottom ;
    int         rej_top ;
    double      ron ;
    double      dark_med ;
    int         hotpix_nb ;
    int         devpix_nb ;
    int         coldpix_nb ;
} dark_config ;

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

static int conica_dark_engine(char *, char *, int, int, int, int) ;
static int conica_dark_avg_engine(framelist *, char *, int, int) ; 
static int conica_dark_ron_engine(char *, char *, char *, int, int) ;
static int conica_dark_compare(char *, char *) ;
static int conica_dark_ron_save(char *, char *, char *) ; 

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/

int conica_dark_main(void * dict)	
{
	dictionary  *   d ;

	char            argname[10] ;
	char        *   name_i ;
	char        *   name_o ;

	int				only_avg ;
	int				only_ron ;
    int             ron_hsize ;
    int             ron_nsamp ;
	
    char        *   sval ;
	int				nfiles ;
	int				errors ;
	int				i ;
	
	/* Initialize */
	only_avg = 0 ;
	only_ron = 0 ;
    ron_hsize = -1 ;
    ron_nsamp = -1 ;
    dark_config.ron        = -1.0 ;
    dark_config.dark_med    = -1.0 ;
    dark_config.hotpix_nb  = -1 ;
    dark_config.devpix_nb  = -1 ;
    dark_config.coldpix_nb = -1 ;

	d = (dictionary*)dict ;
	/* Get options */
	only_avg = dictionary_getint(d, "arg.average", 0) ;
	only_ron = dictionary_getint(d, "arg.ron", 0) ;
    if (only_avg && only_ron) {
        e_error("Incompatible flags - stop") ;
        return -1 ;
    }
    ron_hsize = dictionary_getint(d, "arg.hsize", -1) ;
    ron_nsamp = dictionary_getint(d, "arg.nsamples", -1) ;

    if (ron_hsize < 0) ron_hsize = RON_HS ;
    if (ron_nsamp < 0) ron_nsamp = RON_NBSAMPLES ;
    
    /* Threshold for pixels map */
    dark_config.dev_thresh = DEV_THRESH ;
    sval = dictionary_get(d, "arg.thresholds", NULL) ;
    if (sval == NULL) {
        dark_config.cold_thresh = COLD_THRESH ;
        dark_config.hot_thresh = HOT_THRESH ;
    } else {
        if (sscanf(sval, "%lg %lg",
                   &dark_config.cold_thresh,
                   &dark_config.hot_thresh)!=2) {
            dark_config.cold_thresh = COLD_THRESH ;
            dark_config.hot_thresh = HOT_THRESH ;
        }
    }
	
    /* Rejected borders */
    sval = dictionary_get(d, "arg.rej_bord", NULL) ;
    if (sval == NULL) {
        dark_config.rej_left =   REJ_LEFT ;
        dark_config.rej_right =  REJ_RIGHT ;
        dark_config.rej_bottom = REJ_BOTTOM ;
        dark_config.rej_top =    REJ_TOP ;
    } else {
        if (sscanf(sval, "%d %d %d %d",
                    &dark_config.rej_left,
                    &dark_config.rej_right,
                    &dark_config.rej_bottom,
                    &dark_config.rej_top)!=4) {
            dark_config.rej_left =   REJ_LEFT ;
            dark_config.rej_right =  REJ_RIGHT ;
            dark_config.rej_bottom = REJ_BOTTOM ;
            dark_config.rej_top =    REJ_TOP ;
        }
    }

	/* Get input/output file names */
	nfiles = dictionary_getint(d, "arg.n", -1) ;
	if (nfiles<0) {
		e_error("missing input file name(s): aborting");
		return -1 ;
	}

    INSID = pfits_identify_insstr("naco");

	/* Loop on input file names */
    errors = 0 ;
    for (i=1 ; i<nfiles ; i++) {
        sprintf(argname, "arg.%d", i);
        name_i = dictionary_get(d, argname, NULL) ;
        name_o = dictionary_get(d, "arg.output", NULL) ;
        if (name_o == NULL) name_o = strdup(get_rootname(get_basename(name_i)));
        else name_o = strdup(get_rootname(name_o)) ;

        /*
         * Once command-line options have been cleared out, call the main
         * computing function.
         */
        errors += conica_dark_engine(name_i, name_o, only_avg, only_ron,
                ron_hsize, ron_nsamp) ;
        free(name_o) ;
    }
    return errors ;
}

/*-----------------------------------------------------------------------------
                        Function ANSI C code 
 -----------------------------------------------------------------------------*/

static int conica_dark_engine(
		char 	* 	name_i, 
		char 	* 	name_o,
		int			only_avg,
		int			only_ron,
        int         ron_hsize,
        int         ron_nsamp)
{
	framelist	*	lnames ;
	int             nsettings ;
	framelist	*	sublist ;
	char			outname[FILENAMESZ] ;	
	int				i, j ;
	
	/* Test inputs */
	if (only_avg && only_ron) return -1 ; 
	
	/* Read the input ASCII file */
	if ((lnames=framelist_load(name_i)) == NULL) {
		e_error("cannot read the input ASCII file") ;
		return -1 ;
	}
   
	/* Number of different settings */
	if ((nsettings=framelist_labelize(lnames, conica_dark_compare)) == -1) {
		e_error("in getting the number of different settings") ;
		framelist_del(lnames) ;
		return -1 ;
	}
	e_comment(0, "there are %d different setting(s)", nsettings) ;

	/* For each setting */
	for (i=0 ; i<nsettings ; i++) {
		if ((sublist=framelist_select(lnames, i)) == NULL) {
			e_error("cannot get files for current setting") ;
			framelist_del(lnames) ;
			framelist_del(sublist) ;
			return -1 ;
		}
		e_comment(1, "concerned files:") ;
		for (j=0 ; j<sublist->n ; j++) {
			e_comment(2, "%s", sublist->name[j]) ;
		}

		/* Compute AVG if required */
		if (!only_ron) {
			sprintf(outname, "%s_%02d.fits", name_o, i+1) ;
			conica_dark_avg_engine(sublist, outname, ron_hsize, ron_nsamp) ;
		}
		
		/* Compute RON if required */
		if (!only_avg) {
            for (j=0 ; j<sublist->n-1 ; j++) {
                sprintf(outname, "%s_set%02d_pair%02d_ron.paf", name_o, i+1,
                                                j+1) ;
                conica_dark_ron_engine(sublist->name[j], sublist->name[j+1],
                        outname, ron_hsize, ron_nsamp) ;
            }
        }
        framelist_del(sublist) ;
	}

    /* Free and return */
	framelist_del(lnames) ;
	return 0 ;
}

static int conica_dark_ron_save(
		char		*	outname, 
        char        *   frame1,
        char        *   frame2)
{
	FILE    *   ron_out ;
	char    *   sval ;
    
	/* Open output PAF file (formatted ASCII, see fits/pafs.c) */
    e_comment(0, "saving results to %s", outname);
    if ((ron_out = qfits_paf_print_header(outname,
                               "CONICA/dark",
                               "Readout noise computation results",
                               get_login_name(),
                               get_datetime_iso8601())) == NULL) {
        e_error("cannot open file [%s] for output", outname) ;
        return -1 ;
    }

    /* Add PRO.CATG */
    if ((sval = pfits_getprokey(INSID, procat_dark_ron)) != NULL)
        fprintf(ron_out, "PRO.CATG       \"%s\" ;# Product category\n", sval);

    /* Add date */
	if ((sval = pfits_get(INSID, frame1, "date_obs")) != NULL) {
		fprintf(ron_out, "DATE-OBS           \"%s\" ; #Date\n", sval) ;
	}
    /* Add ARCFILE */
    if ((sval = pfits_get(INSID, frame1, "arcfile")) != NULL)
        fprintf(ron_out, "ARCFILE         \"%s\" ;#\n", sval) ;
    /* Add TPL ID */
	if ((sval = pfits_get(INSID, frame1, "templateid")) != NULL) {
		fprintf(ron_out, "TPL.ID             \"%s\"; # Template id\n", sval) ;
	}
	
    fprintf(ron_out,"#\n") ;
    fprintf(ron_out,"# Read-out noise measurements\n") ;
    fprintf(ron_out,"#\n") ;

    /* Add MJD-OBS for file classification */
    if ((sval = pfits_get(INSID, frame1, "mjdobs")) == NULL) {
        fprintf(ron_out, "MJD-OBS               0.0 ; # could not find\n") ;
    } else {
        fprintf(ron_out, "MJD-OBS               %s ; # Obs start\n", sval) ;
    }

    /* Add input list of frames */
    fprintf(ron_out, "\n");
    fprintf(ron_out, "PRO.REC1.RAW1.NAME   \"%s\" ;#\n", get_basename(frame1));
    fprintf(ron_out, "PRO.REC1.RAW2.NAME   \"%s\" ;#\n", get_basename(frame2));
    fprintf(ron_out, "\n");

    fprintf(ron_out, "\n");
    /* Forward DET.DIT */
    if ((sval = pfits_get(INSID, frame1, "dit")) != NULL) {
        fprintf(ron_out, "DET.DIT          \"%s\"\n", sval) ; 
    }
    /* Forward DET.NDIT */
    if ((sval = pfits_get(INSID, frame1, "ndit")) != NULL) {
        fprintf(ron_out, "DET.NDIT         \"%s\"\n", sval) ; 
    }
    /* Forward DET.NCORRS */
    if ((sval = pfits_get(INSID, frame1, "rom")) != NULL) {
        fprintf(ron_out, "DET.NCORRS       \"%s\"\n", sval) ; 
    }
    /* Forward DPR.TECH */
    if ((sval = pfits_get(INSID, frame1, "dpr_tech")) != NULL) {
        fprintf(ron_out, "DPR.TECH         \"%s\"\n", sval) ; 
    }
    /* Forward DET.NCORRS.NAME */
    if ((sval = pfits_get(INSID, frame1, "rom_name")) != NULL) {
        fprintf(ron_out, "DET.NCORRS.NAME   \"%s\"\n", sval) ; 
    }
    /* Forward DET.MODE.NAME */
    if ((sval = pfits_get(INSID, frame1, "mode")) != NULL) {
        fprintf(ron_out, "DET.MODE.NAME   \"%s\"\n", sval) ; 
    }
    /* Forward DET.NDSAMPLES */
    if ((sval=pfits_get(INSID, frame1, "ndsamples")) != NULL) {
        fprintf(ron_out, "DET.NDSAMPLES    \"%s\"\n", sval) ; 
    }


    if (fabs(dark_config.ron+1.0) > 1e-10) {
        fprintf(ron_out,
                "\n"
                "#\n"
                "# Warning:\n"
                "# Read-out noise is measured by computing\n"
                "# pixel standard deviations over a large number\n"
                "# of randomly picked (Poisson-scattered) areas,\n"
                "# which explains why you will get different values\n"
                "# out of each recipe execution. If the method is\n"
                "# correct these values should not vary much, though.\n"
                "#\n"
            "\n");
        
        fprintf(ron_out, "QC.RON           %.4f\n", dark_config.ron);
    }

    if (fabs(dark_config.dark_med+1.0) > 1e-10) {
        fprintf(ron_out, "QC.DARKMED       %.4f\n", dark_config.dark_med);
    }
    if (dark_config.coldpix_nb != -1) {
        fprintf(ron_out, "QC.NBCOLPIX      %d\n", dark_config.coldpix_nb);
    }
    if (dark_config.hotpix_nb != -1) {
        fprintf(ron_out, "QC.NBHOTPIX      %d\n", dark_config.hotpix_nb);
    }
    if (dark_config.devpix_nb != -1) {
        fprintf(ron_out, "QC.NBDEVPIX      %d\n", dark_config.devpix_nb);
    }

    
    if (verbose_active()) {
        fprintf(stderr, "RON: %.2f\n", dark_config.ron);
    }
    fprintf(ron_out, "\n");

    fclose(ron_out) ;
    e_comment(1, "end of read-out noise computation") ;
    return 0 ;
}

static int conica_dark_avg_engine(
		framelist	* 	in, 
		char		*	outname,
        int             hsize,
        int             nbsamples) 
{
	cube_t			*	images ;
	image_t			*	avg_dark ;
	qfits_header	*	fh ;
    char                full_name[FILENAMESZ];
    image_t         *   promoted ;
    pixelmap        *   coldpixmap ;
    pixelmap        *   hotpixmap ;
    pixelmap        *   devpixmap ;
    double              rms ;
    double              mean ;
    image_t         *   diff_img_dev ;
    int                 zone[4] ;
    
	/* Load the cube */
	if ((images=cube_load_strings(in->name, in->n)) == NULL) return -1 ;
			
	/* Create MASTER_DARK */
	if (images->np > 1) {
		avg_dark = cube_avg_linear(images) ;
        diff_img_dev = image_sub(   images->plane[images->np-2], 
                                    images->plane[images->np-1]) ;
	} else {
		e_warning("only 1 frame used for this group") ;
		avg_dark = image_copy(images->plane[0]) ;
        diff_img_dev = NULL ;
	}
	cube_del(images) ;

	/* Save MASTER DARK with correct keywords */
	fh = qfits_header_read(in->name[0]) ;
    conica_header_for_image(fh) ;
	conica_pro_fits(fh, outname, "REDUCED", NULL, procat_dark_result, "OK", 
            "cal_darks", in->n, in, NULL) ; 
	conica_add_files_history(fh, in) ;
	e_comment(0, "saving file [%s]", outname);
	image_save_fits_hdrdump(avg_dark, outname, fh, BPP_DEFAULT) ;
	qfits_header_destroy(fh) ;
   
    /* Compute median-rms of the central part of the dark  */
    dark_config.dark_med = (double)image_getmedian_vig(avg_dark,
            dark_config.rej_left+1,
            dark_config.rej_bottom+1,
            avg_dark->lx - dark_config.rej_right,
            avg_dark->ly - dark_config.rej_top) ;

    zone[0] = dark_config.rej_left+1 ;
    zone[1] = avg_dark->lx - dark_config.rej_right ;
    zone[2] = dark_config.rej_bottom+1 ;
    zone[3] = avg_dark->ly - dark_config.rej_top ;
    image_rect_readout_noise(avg_dark, zone, hsize, nbsamples, &rms, NULL) ;
    
    /* Create the cold pixel map */
    sprintf(full_name, "%s_coldpix.fits", get_rootname(outname)) ;
    e_comment(1, "saving cold pixel map: [%s]", full_name) ;
    coldpixmap = image_threshold2pixelmap(avg_dark,
                    MIN_PIX_VALUE,
                    dark_config.dark_med - dark_config.cold_thresh*rms);
    if (coldpixmap == NULL) {
        e_error("creating cold pixel map");
    } else {
        dark_config.coldpix_nb = pixelmap_getselected(coldpixmap) ;
        if ((promoted = pixelmap_2_image(coldpixmap)) == NULL) {
            e_error("cannot promote pixelmap") ;
        } else {
            pixelmap_del(coldpixmap);
            fh = qfits_header_read(in->name[0]);
            conica_header_for_image(fh) ;
            conica_pro_fits(fh, full_name, "REDUCED", NULL, procat_dark_cold, 
                    "OK", "cal_darks", in->n, in, NULL);
            image_save_fits_hdrdump(promoted,full_name,fh,BPP_8_UNSIGNED);
            image_del(promoted) ;
            qfits_header_destroy(fh) ;
        }
    }

    /* Create the hot pixel map */
    sprintf(full_name, "%s_hotpix.fits", get_rootname(outname)) ;
    e_comment(1, "saving hot pixel map: [%s]", full_name) ;
    hotpixmap = image_threshold2pixelmap(avg_dark,
                    dark_config.dark_med + dark_config.hot_thresh*rms,
                    MAX_PIX_VALUE);
    if (hotpixmap == NULL) {
        e_error("creating hot pixel map");
    } else {
        dark_config.hotpix_nb = pixelmap_getselected(hotpixmap) ;
        if ((promoted = pixelmap_2_image(hotpixmap)) == NULL) {
            e_error("cannot promote pixelmap") ;
        } else {
            pixelmap_del(hotpixmap);
            fh = qfits_header_read(in->name[0]);
            conica_header_for_image(fh) ;
            conica_pro_fits(fh, full_name, "REDUCED", NULL, procat_dark_hot, 
                    "OK", "cal_darks", in->n, in, NULL);
            image_save_fits_hdrdump(promoted, full_name, fh, BPP_8_UNSIGNED);
            image_del(promoted) ;
            qfits_header_destroy(fh) ;
        }
    }
	image_del(avg_dark) ;

    if (diff_img_dev != NULL) {
        /* Create the deviant pixel map */
        sprintf(full_name, "%s_devpix.fits", get_rootname(outname)) ;
        e_comment(1, "saving deviant pixel map: [%s]", full_name) ;
        mean = image_getmean(diff_img_dev) ;
        image_rect_readout_noise(diff_img_dev,zone,hsize, nbsamples,&rms,NULL);
        devpixmap = image_threshold2pixelmap(diff_img_dev,
                mean-dark_config.dev_thresh*rms, 
                mean+dark_config.dev_thresh*rms) ;
        pixelmap_binary_NOT(devpixmap) ;
        if (devpixmap == NULL) {
            e_error("creating deviant pixel map");
        } else {
            dark_config.devpix_nb = pixelmap_getselected(devpixmap) ;
            if ((promoted = pixelmap_2_image(devpixmap)) == NULL) {
                e_error("cannot promote pixelmap") ;
            } else {
                pixelmap_del(devpixmap);
                fh = qfits_header_read(in->name[0]);
                conica_header_for_image(fh) ;
                conica_pro_fits(fh, full_name, "REDUCED", NULL, procat_dark_dev,
                        "OK", "cal_darks", in->n, in, NULL);
                image_save_fits_hdrdump(promoted, full_name, fh,BPP_8_UNSIGNED);
                image_del(promoted) ;
                qfits_header_destroy(fh) ;
            }
        }
        image_del(diff_img_dev) ;
    }

    /* Free and return */
	return 0 ;
}

static int conica_dark_ron_engine(
        char    *   frame1,
        char    *   frame2,
        char    *   outname,
        int         hsize,
        int         nsamp)
{
	image_t		*	plane1 ;
	image_t		*	plane2 ;
	double			norm ;
	double			noise ;
	char		*	s ;
    int             zone[4] ;

    /* Load current planes */
    if ((plane1 = image_load(frame1)) == NULL) {
        e_error("cannot load plane") ;
        return -1 ;
    }
    if ((plane2 = image_load(frame2)) == NULL) {
        e_error("cannot load plane") ;
        image_del(plane2) ;
		return -1 ;
    }
	
    /* Subtraction */
    if (image_sub_local(plane1, plane2) == -1) {
        e_error("cannot subtract planes") ;
        image_del(plane1) ;
        image_del(plane2) ;
        return -1 ;
    }
    image_del(plane2) ;
    
    /* Compute readout noise */
    zone[0] = dark_config.rej_left+1 ;
    zone[1] = plane1->lx - dark_config.rej_right ;
    zone[2] = dark_config.rej_bottom+1 ;
    zone[3] = plane1->ly - dark_config.rej_top ;
    image_rect_readout_noise(plane1, zone, hsize, nsamp, &noise, NULL) ;
    image_del(plane1) ;

    /* Compute norm from NDIT */
    if ((s = pfits_get(INSID, frame1, "ndit")) == NULL) {
        e_error("cannot get DET.NDIT from [%s]", frame1) ;
        return -1 ;
    }
    norm = atof(s) ;
    norm *= 0.5 ;
    norm = sqrt(norm) ; 

    /* Compute RON */
    dark_config.ron = noise * norm ;

    /* Write out the PAF file */
    if (conica_dark_ron_save(outname, frame1, frame2) == -1) {
        e_error("cannot write PAF file") ;
        return -1 ;
    }

	return 0 ;
}

static int conica_dark_compare(
        char    *   file1,
        char    *   file2)
{
    int         comparison ;
    double		exptime1, 
				exptime2 ;
	double		rom1,
				rom2 ;
    char        mode1[FILENAMESZ] ;
    char        mode2[FILENAMESZ] ;
	int			expno1,
				expno2 ;
    char    *   s ;
        
    comparison = 1 ;    
    
	/* Compare the EXPTIME  */
	if ((s = pfits_get(INSID, file1, "exptime")) == NULL) {
		e_error("cannot get EXPTIME from [%s]", file1) ;
		return -1 ;
	}
	exptime1 = (double)atof(s) ;
	if ((s = pfits_get(INSID, file2, "exptime")) == NULL) {
		e_error("cannot get EXPTIME from [%s]", file2) ;
		return -1 ;
	}
	exptime2 = (double)atof(s) ;
	if (fabs(exptime1-exptime2) > 1e-5) comparison = 0 ; 

	/* Compare the readout mode */
	if (comparison == 1) {
		if ((s = pfits_get(INSID, file1, "rom")) == NULL) {
			e_error("cannot get DET.NCORRS from [%s]", file1) ;
			return -1 ;
		}
		rom1 = (double)atof(s) ;
		if ((s = pfits_get(INSID, file2, "rom")) == NULL) {
			e_error("cannot get DET.NCORRS from [%s]", file2) ;
			return -1 ;
		}
		rom2 = (double)atof(s) ;
		if (fabs(rom1-rom2) > 1e-5) comparison = 0 ;
	}

	/* Compare the detector mode */
    if (comparison == 1) {
        if ((s = pfits_get(INSID, file1, "mode")) == NULL) {
            e_error("cannot get DET.MODE.NAME from [%s]", file1) ;
            return -1 ;
        }
        strcpy(mode1, s) ;
        if ((s = pfits_get(INSID, file2, "mode")) == NULL) {
            e_error("cannot get DET.MODE.NAME from [%s]", file2) ;
            return -1 ;
        }
        strcpy(mode2, s) ;
        if (strcmp(mode1, mode2) != 0) comparison = 0 ;
    }

	/* Files have to be consequtive */
	if (comparison == 1) {
		if ((s = pfits_get(INSID, file1, "expno")) == NULL) {
			e_error("cannot get TPL.EXPNO from [%s]", file1) ;
			return -1 ;
		}
		expno1 = (int)atoi(s) ;
		if ((s = pfits_get(INSID, file2, "expno")) == NULL) {
			e_error("cannot get TPL.EXPNO from [%s]", file2) ;
			return -1 ;
		}
		expno2 = (int)atoi(s) ;
		if (fabs(expno1 - expno2) > 1) comparison = 0 ;
	}
    return comparison ;
}



