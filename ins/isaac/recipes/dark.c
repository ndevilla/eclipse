/*----------------------------------------------------------------------------*/
/**
   @file    dark.c
   @author  N. Devillard
   @date    January 2001
   @version	$Revision: 1.24 $
   @brief   ISAAC dark recipe
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: dark.c,v 1.24 2004/08/04 08:34:51 yjung Exp $
	$Author: yjung $
	$Date: 2004/08/04 08:34:51 $
	$Revision: 1.24 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <ctype.h>
#include <math.h>

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

#define	ISAAC_DARK_HSIZE_SW_DEF		6
#define	ISAAC_DARK_HSIZE_LW_DEF		2

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

static int isaac_dark_engine(char *, char *, int, int, int, int) ;
static int isaac_dark_avg_engine(framelist *, char *) ;
static int isaac_dark_ron_engine(char *, char *, char *, int, int) ;
static int isaac_dark_ron_save(char *, char *, char *, double, double,
		double, double, double, int) ;
static int isaac_dark_compare(char *, char *) ;

/*-----------------------------------------------------------------------------
                           Global variable 
 -----------------------------------------------------------------------------*/

static struct {
    double      dark_med ;
    double      dark_stdev ;
} dark_config ;

static instrument_t INSID ;

/*-----------------------------------------------------------------------------
							Main code
 -----------------------------------------------------------------------------*/
int isaac_dark_main(void * dict)
{
    dictionary  *   d ;

    char            argname[10] ;
    char        *   name_i ;
    char        *   name_o ;

    int             only_avg ;
    int             only_ron ;
    int             ron_hsize ;
    int             ron_nsamp ;

    int             nfiles ;
    int             errors ;
    int             i ;

    /* Initialize */
    only_avg = 0 ;
    only_ron = 0 ;
    ron_hsize = -1 ;
    ron_nsamp = -1 ;
    dark_config.dark_med    = -1.0 ;
    dark_config.dark_stdev    = -1.0 ;

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
    
    /* Get input/output file names */
    nfiles = dictionary_getint(d, "arg.n", -1) ;
    if (nfiles<0) {
        e_error("missing input file name(s): aborting");
        return -1 ;
    }

    INSID = pfits_identify_insstr("isaac");

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
        errors += isaac_dark_engine(name_i, name_o, only_avg, only_ron, 
                ron_hsize, ron_nsamp) ;
        free(name_o) ;
    }
    return errors ;
}

/*-----------------------------------------------------------------------------
						Function ANSI C code 
 -----------------------------------------------------------------------------*/
static int isaac_dark_engine(
        char    *   name_i,
        char    *   name_o,
        int         only_avg,
        int         only_ron,
        int         ron_hsize,
        int         ron_nsamp)
{
    framelist   *   lnames ;
    int             nsettings ;
    framelist   *   sublist ;
    char            outname[FILENAMESZ] ;
    int             i, j ;

    /* Test inputs */
    if (only_avg && only_ron) return -1 ;

    /* Read the input ASCII file */
    if ((lnames=framelist_load(name_i)) == NULL) {
        e_error("cannot read the input ASCII file") ;
        return -1 ;
    }

    /* Number of different settings */
    if ((nsettings=framelist_labelize(lnames, isaac_dark_compare)) == -1) {
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
            isaac_dark_avg_engine(sublist, outname) ;
        }

        /* Compute RON if required */
        if (!only_avg) {
            for (j=0 ; j<sublist->n-1 ; j++) {
				sprintf(outname, "%s_set%02d_pair%02d_ron.paf", name_o, i+1,
						j+1) ;
            	isaac_dark_ron_engine(sublist->name[j], sublist->name[j+1], 
						outname, ron_hsize, ron_nsamp) ;
			}
        }
		framelist_del(sublist) ;
    }

    /* Free and return */
    framelist_del(lnames) ;
    return 0 ;
}

static int isaac_dark_avg_engine(
		framelist	*	in,
		char		*	outname)
{
    cube_t          *   images ;
    image_t         *   avg_dark ;
    double          *   medians ;
    double              sum, sqsum ;
    qfits_header    *   fh ;
    int                 i ;
 
    /* Load the cube */ 
    if ((images=cube_load_strings(in->name, in->n)) == NULL) {
        return -1 ;
    }

    /* Average cube */
    if (images->np > 1) {
        avg_dark = cube_avg_linear(images) ;
    } else {
        e_warning("only 1 frame used for this group") ;
        avg_dark = image_copy(images->plane[0]) ;
    }

    /* Compute the average/stdev of the median values */
    if (images->np > 2) {
        medians = malloc(images->np * sizeof(double)) ;
        for (i=0 ; i<images->np ; i++) 
            medians[i] = (double)image_getmedian(images->plane[i]) ;

        sum = 0.0 ;
        sqsum = 0.0 ;
        for (i=0 ; i<images->np ; i++) {
            sum += medians[i] ; 
            sqsum += medians[i] * medians[i] ;
        }
        dark_config.dark_med = sum / images->np ;
        dark_config.dark_stdev = (sqsum-((sum*sum)/(double)images->np))
            / ((double)images->np - 1) ;
        dark_config.dark_stdev = dark_config.dark_stdev > 0 ? 
            sqrt(dark_config.dark_stdev) : 0 ;
        free(medians) ;
    } else {
        e_warning("not enough frames to compute median/stdev") ;
    }
    cube_del(images) ;

    /* Save with correct keywords */
    if ((fh = qfits_header_read(in->name[0])) == NULL) {
        e_error("cannot read header %s: creating empty header", in->name[0]) ;
    } else {
        if (isaac_header_for_image(fh) != 0) {
            e_error("filtering input header: creating empty header") ;
            qfits_header_destroy(fh) ;
            fh = NULL ;
        }
    }

    isaac_pro_fits(fh, outname, "REDUCED", NULL, procat_dark_result, "OK", 
			"cal_darks", in->n, in, NULL) ;

    if (isaac_add_files_history(fh, in) == -1) {
        e_warning("cannot write HISTORY keywords in out file") ;
    }

    e_comment(0, "saving file [%s]", outname);
    image_save_fits_hdrdump(avg_dark, outname, fh, BPP_DEFAULT) ;
    qfits_header_destroy(fh) ;
    image_del(avg_dark) ;
    return 0 ;
}

static int isaac_dark_ron_engine(
		char		*	frame1,
		char		*	frame2,
        char        *   outname,
        int             hsize,
        int             nsamp)
{
	image_t     *   plane1 ;
    image_t     *   plane2 ;
    double          norm ;
    double          noise ;
	double			ron_whole ;
	double			ron_q_ll,
					ron_q_lr,
					ron_q_ur,
					ron_q_ul ;
	int				arm ;
	int				hsize_loc ;
	int				zone_def[4] ;
    char        *   s ;

	/* Determine instrument mode: SW or LW */
    s = pfits_get(INSID, frame1, "arm");
    if (s==NULL) {
        e_error("cannot determine detector: SW or LW");
        return -1 ;
    }
    arm = (int)toupper(s[0]);
    if (arm!='S' && arm!='L') {
        e_error("cannot determine detector: SW or LW");
        return -1 ;
    }

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

	/* Compute norm from NDIT */
	if ((s = pfits_get(INSID, frame1, "ndit")) == NULL) {
		e_error("cannot get DET.NDIT from [%s]", frame1) ;
		return -1 ;
	}
	norm = atof(s) ;
	norm *= 0.5 ;
	norm = sqrt(norm) ;
	
	/* Compute readout noise according the mode */
	switch (arm) {

        /* SW mode */
		case 'S':
			/* Set default value for hsize if necessary */
			if (hsize<0) hsize_loc = ISAAC_DARK_HSIZE_SW_DEF ;
			else hsize_loc = hsize ;
			/* Get measurement for upper-left quadrant */
			zone_def[0] = 0 ;
			zone_def[1] = plane1->lx/2 ;
			zone_def[2] = plane1->ly/2 ;
			zone_def[3] = plane1->ly-1 ;
			image_rect_readout_noise(plane1, zone_def, hsize_loc, nsamp, 
					&noise, NULL);
			ron_q_ul = noise ;

			/* Get measurement for upper-right quadrant */
			zone_def[0] = plane1->lx/2 ;
			zone_def[1] = plane1->lx-1 ;
			zone_def[2] = plane1->ly/2 ;
			zone_def[3] = plane1->ly-1 ;
			image_rect_readout_noise(plane1, zone_def, hsize_loc, nsamp, 
					&noise, NULL);
			ron_q_ur = noise ;

			/* Get measurement for lower-right quadrant */
			zone_def[0] = plane1->lx/2 ;
			zone_def[1] = plane1->lx-1 ;
			zone_def[2] = 0 ;
			zone_def[3] = plane1->ly/2-1 ;
			image_rect_readout_noise(plane1, zone_def, hsize_loc, nsamp, 
					&noise, NULL);
			ron_q_lr = noise ;

			/* Get measurement for lower-left quadrant */
			zone_def[0] = 0 ;
			zone_def[1] = plane1->lx/2-1 ;
			zone_def[2] = 0 ;
			zone_def[3] = plane1->ly/2-1 ;
			image_rect_readout_noise(plane1, zone_def, hsize_loc, nsamp, 
					&noise, NULL);
			ron_q_ll = noise ;

			/* Normalize the results */
			ron_q_ul *= norm ;
			ron_q_ur *= norm ;
			ron_q_ll *= norm ;
			ron_q_lr *= norm ;
			break ;

        /* LW mode */
		case 'L':
			/* Set default value for hsize if necessary */
			if (hsize<0) hsize_loc = ISAAC_DARK_HSIZE_LW_DEF ;
			else hsize_loc = hsize ;
			
			/* Get measurement for the whole image */
			zone_def[0] = 0 ;
			zone_def[1] = plane1->lx-1 ;
			zone_def[2] = 0 ;
			zone_def[3] = plane1->ly-1 ;
			image_rect_readout_noise(plane1, zone_def, hsize_loc, nsamp, 
					&noise, NULL);
			ron_whole = noise ;
			ron_whole *= norm ;
			break ;
		
		default:
			e_error("Unrecognized mode - abort") ;
			image_del(plane1) ;
			return -1 ;
			break ;
	}
	image_del(plane1) ;

    /* Write the PAF file */
    if (isaac_dark_ron_save(outname, frame1, frame2, ron_whole, ron_q_ul, 
				ron_q_ur, ron_q_lr, ron_q_ll, arm) == -1) {
        e_error("cannot write PAF file") ;
		return -1 ;
    }

	/* Return  */
	return 0 ;
}

static int isaac_dark_ron_save(
		char		*	name_o,
		char		*	frame1,
		char		*	frame2,
		double			ron_whole,
		double			ron_q_ul,
		double			ron_q_ur,
		double			ron_q_lr,
		double			ron_q_ll,
        int             arm)
{
	FILE	        *	ron_out ;
	char	        *	sval ;
	
	/* Open output PAF file (formatted ASCII, see fits/pafs.c) */
	e_comment(0, "saving results to %s", name_o);
	ron_out = qfits_paf_print_header(name_o,
							   "ISAAC/darks",
							   "Readout noise computation results",
                               get_login_name(),
                               get_datetime_iso8601());
	if (ron_out == NULL) {
		e_error("cannot open file [%s] for output: aborting RON", name_o);
		return -1 ;
	}

	/* Add PRO.CATG */
	if ((sval = pfits_getprokey(INSID, procat_dark_ron)) != NULL)
    	fprintf(ron_out, "PRO.CATG       \"%s\" ;# Product category\n", sval);

	/* Add date */
	if ((sval = pfits_get(INSID, frame1, "date_obs")) != NULL) 
    	fprintf(ron_out, "DATE-OBS        \"%s\" ;# Date\n", sval) ;

	/* Add ARCFILE */
	if ((sval = pfits_get(INSID, frame1, "arcfile")) != NULL)
		fprintf(ron_out, "ARCFILE         \"%s\" ;#\n", sval) ;

	/* Add TPL ID */
    if ((sval = pfits_get(INSID, frame1, "templateid")) != NULL)
		fprintf(ron_out, "TPL.ID          \"%s\" ;# Template ID\n", sval) ;
    
	/* Add MJD-OBS for file classification */
	if ((sval = pfits_get(INSID, frame1, "mjdobs")) == NULL) {
		fprintf(ron_out, "MJD-OBS             0.0 ; # could not find value\n");
	} else {
		fprintf(ron_out, "MJD-OBS             %s ; # Obs start\n", sval);
	}

	/* Add input list of frames */
	fprintf(ron_out, "\n");
	fprintf(ron_out, "PRO.REC1.RAW1.NAME   \"%s\" ;#\n", get_basename(frame1));
	fprintf(ron_out, "PRO.REC1.RAW2.NAME   \"%s\" ;#\n", get_basename(frame2));
	fprintf(ron_out, "\n");

	fprintf(ron_out, "\n");
	/* Forward DET.DIT */
	if ((sval = pfits_get(INSID, frame1, "dit")) != NULL) {
		fprintf(ron_out, "DET.DIT          %s\n", sval);
	}
	/* Forward DET.NDIT */
	if ((sval = pfits_get(INSID, frame1, "ndit")) != NULL) {
		fprintf(ron_out, "DET.NDIT         %s\n", sval);
	}
	/* Forward DET.NCORRS */
	if ((sval = pfits_get(INSID, frame1, "romode_id")) != NULL) {
		fprintf(ron_out, "DET.NCORRS       %s\n", sval);
	}
	/* Forward DPR.TECH */
	sval = pfits_get(INSID, frame1, "dpr_tech");
	if (sval!=NULL) {
		fprintf(ron_out, "DPR.TECH         \"%s\"\n", sval);
	}
	/* Forward DET.NCORRS.NAME */
	sval = pfits_get(INSID, frame1, "romode_name");
	if (sval!=NULL) {
		fprintf(ron_out, "DET.MODE.NAME  \"%s\"\n", sval);
	}
	/* Try to forward DET NDSAMPLES if found in input */
	sval = pfits_get(INSID, frame1, "ndsamples");
	if (sval!=NULL) {
		sval = qfits_pretty_string(sval);
		fprintf(ron_out, "DET.NDSAMPLES    %s\n", sval);
	}

    if (fabs(dark_config.dark_med+1.0) > 1e-10) {
        fprintf(ron_out, "QC.DARKMED       %.4f\n", dark_config.dark_med);
    }
    if (fabs(dark_config.dark_stdev+1.0) > 1e-10) {
        fprintf(ron_out, "QC.DARKSTDEV     %.4f\n", dark_config.dark_stdev);
    }

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
	
	switch (arm) {
        
        /* SW mode */
		case 'S':
		fprintf(ron_out, "QC.UL.RON        %.4f\n", ron_q_ul);
		fprintf(ron_out, "QC.UR.RON        %.4f\n", ron_q_ur);
		fprintf(ron_out, "QC.LR.RON        %.4f\n", ron_q_lr);
		fprintf(ron_out, "QC.LL.RON        %.4f\n", ron_q_ll);
		if (verbose_active()) {
			fprintf(stderr, "RON: %.2f %.2f %.2f %.2f\n",
					ron_q_ul,
					ron_q_ur,
					ron_q_lr,
					ron_q_ll);
		}
		break ;

        /* LW mode */
		case 'L':
		fprintf(ron_out, "QC.RON           %.4f\n", ron_whole);
		if (verbose_active()) {
			fprintf(stderr, "RON: %.2f\n", ron_whole);
		}
		break ;

		default:
		break ;
	}
	fprintf(ron_out, "\n");
	fclose(ron_out) ;
	e_comment(1, "end of read-out noise computation") ;
	return 0 ;
}

static int isaac_dark_compare(
        char    *   file1,
        char    *   file2)
{
    int         comparison ;

    double      dit1,
                dit2 ;
	double		ndit1,
				ndit2 ;
    double      rom1,
                rom2 ;
    int         expno1,
                expno2 ;

    char    *   s ;

    comparison = 1 ;

    /* Compare the DIT  */
    if ((s = pfits_get(INSID, file1, "dit")) == NULL) {
        e_error("cannot get DET.DIT from [%s]", file1) ;
        return -1 ;
    }
    dit1 = (double)atof(s) ;
    if ((s = pfits_get(INSID, file2, "dit")) == NULL) {
        e_error("cannot get DET.DIT from [%s]", file2) ;
        return -1 ;
    }
    dit2 = (double)atof(s) ;
    if (fabs(dit1-dit2) > 1e-5) comparison = 0 ;

    /* Compare the NDIT  */
    if ((s = pfits_get(INSID, file1, "ndit")) == NULL) {
        e_error("cannot get DET.DIT from [%s]", file1) ;
        return -1 ;
    }
    ndit1 = (double)atof(s) ;
    if ((s = pfits_get(INSID, file2, "ndit")) == NULL) {
        e_error("cannot get DET.DIT from [%s]", file2) ;
        return -1 ;
    }
    ndit2 = (double)atof(s) ;
    if (fabs(ndit1-ndit2) > 1e-5) comparison = 0 ;

    /* Compare the readout mode */
    if (comparison == 1) {
        if ((s = pfits_get(INSID, file1, "romode_id")) == NULL) {
            e_error("cannot get DET.NCORRS from [%s]", file1) ;
            return -1 ;
        }
        rom1 = (double)atof(s) ;
        if ((s = pfits_get(INSID, file2, "romode_id")) == NULL) {
            e_error("cannot get DET.NCORRS from [%s]", file2) ;
            return -1 ;
        }
        rom2 = (double)atof(s) ;
        if (fabs(rom1-rom2) > 1e-5) comparison = 0 ;
    }

    /* Files have to be consequtive */
    if (comparison == 1) {
        if ((s = pfits_get(INSID, file1, "current_exp_nb")) == NULL) {
            e_error("cannot get TPL.EXPNO from [%s]", file1) ;
            return -1 ;
        }
        expno1 = (int)atoi(s) ;
        if ((s = pfits_get(INSID, file2, "current_exp_nb")) == NULL) {
            e_error("cannot get TPL.EXPNO from [%s]", file2) ;
            return -1 ;
        }
        expno2 = (int)atoi(s) ;
        if (fabs(expno1 - expno2) > 1) comparison = 0 ;
    }
    return comparison ;
}

