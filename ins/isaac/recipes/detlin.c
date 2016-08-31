/*----------------------------------------------------------------------------*/
/**
   @file    detlin.c
   @author  N. Devillard
   @date    April 2001
   @version	$Revision: 1.10 $
   @brief   ISAAC detector linearity test.
    Inputs:
        - A list of frames to process, with various DITs like:
        0.1384
        0.2
        0.3
        0.4
        0.1384
        0.5
        0.6
        0.7
        0.8
        0.1384
        0.9
        1.0
        1.1
        1.2
        0.1384
        - A list of corresponding dark frames (same DITs as the ones used
        above).

    Process:
        - Subtract darks from input frames.
        - Check the stability of the level in the DIT=0.1384 frames.
          exit if changes too much (1% level).
        - Use all frames but 0.1384 frames. Fit to each pixel the function
          DIT = a*flux + b*flux^2 + c*flux^3
        - Determine a, b, c, fit error and chi-square estimate of the goodness
          of fit.
        - Construct 4 images: a, b, c, goodness of fit.

    Outputs:
        - Image of a, b, c coefficients
        - Image of the goodness of fit.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: detlin.c,v 1.10 2003/11/21 15:33:47 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/21 15:33:47 $
	$Revision: 1.10 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"
#include "isaacp_lib.h"

#include "pfits.h"
#include "pfitspro.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define FRAME_DARK		1
#define FRAME_LAMP		2

#define NPARTS			6	

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

static int isaac_detlin_engine(char *, char *, int) ;
static cube_t * isaac_detlin_load(char *, double **, int) ;
static int isaac_detlin_save(cube_t *, char *, char *, int) ;

static instrument_t INSID ;
static int part=0 ;

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int isaac_detlin_main(void * dict)
{
	dictionary	*	d ;
	char		*	name_i ;
	char		*	name_o ;
    int             force ;

	d = (dictionary*)dict ;

	/* Get options */
    force = dictionary_getint(d, "arg.force", 0) ;
    
	/* Get input/output file names */
	name_i = dictionary_get(d, "arg.1", NULL);
	if (name_i==NULL) {
		e_error("missing input file name: aborting");
		return -1 ;
	}
	name_o = dictionary_get(d, "arg.output", NULL);
	if (name_o==NULL) {
		name_o = "detlin" ;
	}

    INSID = pfits_identify_insstr("isaac");

	return isaac_detlin_engine(name_i, name_o, force);
}

static int isaac_detlin_engine(
        char    *   name_i,
        char    *   name_o,
        int         force)
{
	cube_t	*	detlin ;
	double	*	ditval ;
	cube_t	*	fitres ;
	int			datancom ;
	int			sta ;

	/* Load inputs */
	detlin = isaac_detlin_load(name_i, &ditval, force) ;
	if (detlin==NULL) {
		return -1 ;
	}
	datancom = detlin->np ;
	part ++ ;
	e_comment(0, "-> part %d of %d: fitting polynomials (long)", part, NPARTS);
	fitres = detector_linearity_fit(detlin, ditval, 3);
	/* Discard data */
	cube_del(detlin);
	free(ditval);

	if (fitres==NULL) {
		e_error("fitting function to planes: aborting");
		return -1 ;
	}

	/* Save results */
	sta = isaac_detlin_save(fitres, name_i, name_o, datancom);
	cube_del(fitres);

	e_comment(0, "done");
	return sta ;
}


/*
 * Load all input frames, check that there are as many darks as linearity
 * images and that they have the corresponding integration times.
 * The files with identical DITS are also checked for intensity variations.
 */
static cube_t * isaac_detlin_load(
        char    *   listname, 
        double  **  ditval,
        int         force)
{
	framelist	*	in_list ;
	framelist	*	dark_list ;
	framelist	*	lamp_list ;
	int				i, j ;
	char		*	sval ;
	int				err ;
	int				n_dark, n_lamp ;
	cube_t		*	lampcube ;
	char		*	dark_integ,
				*	lamp_integ ;
	char		*	init_dit ;
	int			*	same_dit ;
	int				n_same_dit ;
	double		*	level_same_dit ;
	double		*	ditval_load ;
	double		*	ditval_load_purged ;
	image_t		*	lamp_1,
				*	dark_1 ;
	int				lx, ly, np ;

	/* Load framelist */
	part++;
	e_comment(0, "-> part %d of %d: frame identification", part, NPARTS);
	in_list = framelist_load(listname);
	if (in_list==NULL) {
		e_error("cannot load %s", listname);
		return NULL ;
	}
	e_comment(1, "framelist [%s] parsed Ok", listname);
	/* Assign labels to frames */
	err=0 ;
	n_dark=n_lamp=0 ;
	for (i=0 ; i<in_list->n ; i++) {
		sval = pfits_get(INSID, in_list->name[i], "dpr_type");
		if (sval==NULL) {
			e_error("no DPR TYPE for frame %s", in_list->name[i]);
			err++ ;
		} else {
			if ((!strcmp(sval, "DARK")) 
                    || (!strcmp(sval, "OTHER"))
                    || (!strcmp(sval, "OTHER,LINEARITY"))) {
				/* Frame is a dark */
				in_list->label[i] = FRAME_DARK ;
				n_dark++ ;
			} else if ((!strcmp(sval, "LAMP")) 
                    || (!strcmp(sval, "LAMP,LINEARITY"))) {
				/* Frame is a linearity image */
				in_list->label[i] = FRAME_LAMP ;
				n_lamp++ ;
			} else {
				/* Invalid DPR TYPE for this frame */
				e_error("invalid DPR TYPE for frame %s: [%s]", in_list->name[i],
						sval);
				err++;
			}
		}
	}
	/* Check that there are as many darks as input images */
	if (n_dark!=n_lamp) {
		e_error("inconsistent data: %d darks for %d images", n_dark, n_lamp);
		err++;
	}
	if (err) {
		e_error("%d error(s) parsing list %s", err, listname);
		framelist_del(in_list);
		return NULL ;
	}
	e_comment(1, "all frames correctly labelled");
	/* Create new framelists for linearity and dark frames */
	lamp_list = framelist_select(in_list, FRAME_LAMP);
	dark_list = framelist_select(in_list, FRAME_DARK);
	/* Discard initial list */
	framelist_del(in_list);

	/*
	 * Check out that they have consistent integration times
	 * Remember which frames have the same integration time.
	 */
	part++;
	e_comment(0, "-> part %d of %d: checking DIT consistency", part, NPARTS);
	err=0 ;
	same_dit = malloc(lamp_list->n * sizeof(int));
	ditval_load = malloc(lamp_list->n * sizeof(double));
	n_same_dit=0 ;
	for (i=0 ; i<lamp_list->n ; i++) {
		/* Get integration time for lamps */
		lamp_integ = pfits_get(INSID, lamp_list->name[i], "dit");
		if (lamp_integ==NULL) {
			e_error("frame %s has no DET.DIT", lamp_list->name[i]);
			err++ ;
			break ;
		}
		e_comment(1, "LAMP %s DIT %s",
				  get_basename(lamp_list->name[i]),
				  lamp_integ);
		/* If first loaded frame, record integration time */
		if (i==0) {
			init_dit = strdup(lamp_integ);
			same_dit[0] = 1 ;
			n_same_dit++ ;
		} else {
			/* Store integration time */
			if (!strcmp(init_dit, lamp_integ)) {
				same_dit[i] = 1 ;
				n_same_dit ++ ;
			} else {
				same_dit[i] = 0 ;
			}
		}
		ditval_load[i] = (double)atof(lamp_integ);
		/* Get integration time for dark */
		dark_integ = pfits_get(INSID, dark_list->name[i], "dit");
		if (dark_integ==NULL) {
			e_error("frame %s has no DET.DIT: aborting", dark_list->name[i]);
			err++ ;
			break ;
		}
		e_comment(1, "DARK %s DIT %s",
				  get_basename(dark_list->name[i]),
				  lamp_integ);
		/* Compare DIT for lamp and dark */
		if (strcmp(lamp_integ, dark_integ)) {
			e_error("DIT inconsistency");
			e_error("file %s has DIT=%s", lamp_list->name[i], lamp_integ);
			e_error("file %s has DIT=%s", dark_list->name[i], dark_integ);
			err++ ;
		}
	}
	if (init_dit!=NULL) free(init_dit);

	/* Check that there are frames with identical DITs */
	if (n_same_dit<1) {
		e_error("no two frames with identical DIT");
		err++ ;
	}
	/* Report errors */
	if (err) {
		e_error("%d error(s) in data set", err);
		free(ditval_load);
		free(same_dit);
		framelist_del(lamp_list);
		framelist_del(dark_list);
		return NULL ;
	}
	e_comment(1, "DIT consistency Ok");

	/* Compute level in frames of identical DIT */
	part++;
	e_comment(0, "-> part %d of %d: checking lamp stability", part, NPARTS);
	level_same_dit = malloc(n_same_dit * sizeof(double));
	j=0 ;
	lx=ly=0 ;
	for (i=0 ; i<n_lamp ; i++) {
		if (same_dit[i]) {
			/* Load lamp frame */
			if ((lamp_1 = image_load(lamp_list->name[i])) == NULL) {
				e_error("loading frame %s: aborting", lamp_list->name[i]);
				free(ditval_load);
				free(level_same_dit) ;
				free(same_dit);
				framelist_del(lamp_list);
				framelist_del(dark_list);
				return NULL ;
			}
			/* Load dark frame */
			if ((dark_1 = image_load(dark_list->name[i])) == NULL) {
				e_error("loading frame %s: aborting", dark_list->name[i]);
				free(ditval_load);
				free(level_same_dit);
				free(same_dit);
				framelist_del(lamp_list);
				framelist_del(dark_list);
				return NULL ;
			}
			if (lx==0 || ly==0) {
				lx = lamp_1->lx ;
				ly = lamp_1->ly ;
			}
			/* Subtract dark from lamp */
			image_sub_local(lamp_1, dark_1);
			/* Discard dark frame */
			image_del(dark_1);
			/* Record level in subtracted frame */
			level_same_dit[j] = image_getmean(lamp_1);
			/* Discard lamp frame */
			image_del(lamp_1);
			e_comment(1, "level for LAMP %02d: %g", i+1, level_same_dit[j]);
			j++ ;
		}
	}
	/* Check level in frames of identical DIT */
	e_comment(1, "checking level in frames");
	err=0 ;
	for (i=1 ; i<n_same_dit ; i++) {
		if (((level_same_dit[i]-level_same_dit[0]) / level_same_dit[0])>0.01) {
			if (force == 1) {
                e_warning("level difference #%d too high - proceed anyway",i+1);
            } else {
                e_error("level difference #%d too high", i+1);
                err++ ;
            }
		}
	}
	/* Discard levels */
	free(level_same_dit);
	if (err) {
		e_error("too much difference in frames of identical DIT: aborting");
		free(ditval_load);
		free(same_dit);
		framelist_del(lamp_list);
		framelist_del(dark_list);
		return NULL ;
	} else {
		e_comment(1, "lamp level check Ok");
	}

	/* Load frames and subtract them as they load */
	part++;
	e_comment(0, "-> part %d of %d: load dark-subtracted frames", part, NPARTS);
	np = lamp_list->n - n_same_dit ;
	lampcube = cube_new(lx, ly, np);
	j=0 ;
	err=0 ;
	for (i=0 ; i<lamp_list->n ; i++) {
		if (same_dit[i]==0) {
			e_comment(1, "loading/subtracting DIT %g", ditval_load[i]);
			lamp_1 = image_load(lamp_list->name[i]);
			if (lamp_1==NULL) {
				e_error("loading frame %s", lamp_list->name[i]);
				err++ ;
				break ;
			}
			dark_1 = image_load(dark_list->name[i]);
			if (dark_1==NULL) {
				e_error("loading frame %s", dark_list->name[i]);
				err++ ;
				break ;
			}
			image_sub_local(lamp_1, dark_1);
			image_del(dark_1);
			lampcube->plane[j]=lamp_1 ;
			j++ ;
		}
	}
	framelist_del(dark_list);
	if (err) {
		e_error("loading data: aborting");
		free(ditval_load);
		cube_del(lampcube);
	    free(same_dit);
		return NULL ;
	}
	e_comment(1, "frame loading Ok");

    /* Purge ditval_load */
	ditval_load_purged = malloc(lampcube->np * sizeof(double));
	j=0 ;
    for (i=0 ; i<lamp_list->n ; i++) {
        if (same_dit[i]==0) {
            ditval_load_purged[j] = ditval_load[i] ;
            j++;
        }
    }    
	free(same_dit);
	framelist_del(lamp_list);
    free(ditval_load) ;
    
	/* Assign list of loaded DITs */
	(*ditval) = ditval_load_purged ;
	/* Returned loaded cube */
	return lampcube ;
}


static int isaac_detlin_save(
	    cube_t  *   fitres,
	    char 	*   name_i,
	    char 	*   name_o,
	    int		    datancom)
{
	double			med ;
    double          med_a, med_b, med_c ;
	image_t		*	div ;
	char			outname[FILENAMESZ];
	char		*	refname ;
	qfits_header*	fh,
				*	fh_spec ;
	framelist	*	raw ;
    FILE        *   paf ;
    char        *   sval ;
				
	part++;
	e_comment(0, "-> part %d of %d: saving results", part, NPARTS);

    /* Compute med_a med_b and med_c */
    med_a = image_getmedian(fitres->plane[0]) ;
    med_b = image_getmedian(fitres->plane[1]) ;
    med_c = image_getmedian(fitres->plane[2]) ;
    
	/* Compute B/A and find its median */
	div = image_div(fitres->plane[1], fitres->plane[0]);
	med = image_getmedian(div);
	image_del(div);
	printf("median B/A: %g\n", med);

	/* Compute C/A and find its median */
	div = image_div(fitres->plane[2], fitres->plane[0]);
	med = image_getmedian(div);
	image_del(div);
	printf("median C/A: %g\n", med);

	/* Load header from first input file */
	if (is_ascii_list(name_i)==1) {
		refname = framelist_firstname(name_i) ;
	} else {
		refname = name_i ;
	}
	if ((fh=qfits_header_read(refname))==NULL) {
		e_error("getting header from reference frame [%s]", refname);
		return -1 ;
	}
	/* Prepare header for image output */
	isaac_header_for_image(fh);

	/* Save coeff A image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_A.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	raw = framelist_load(name_i) ;
	isaac_pro_fits(	fh_spec,
					outname,
					NULL,	/* Not a reduced file */
					NULL,	/* No reduction level applicable */
					procat_imag_detlin_coeff_A,
					"OK",	/* Status */
					"detlin",	/* Recipe ID */
					datancom,
					raw,
					NULL);
	image_save_fits_hdrdump(fitres->plane[0], outname, fh_spec, BPP_DEFAULT);
	qfits_header_destroy(fh_spec);

	/* Save coeff B image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_B.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	isaac_pro_fits(	fh_spec,
					outname,
					NULL,	/* Not a reduced file */
					NULL,	/* No reduction level applicable */
					procat_imag_detlin_coeff_B,
					"OK",	/* Status */
					"detlin",	/* Recipe ID */
					datancom,
					raw,
					NULL);
	image_save_fits_hdrdump(fitres->plane[1], outname, fh_spec, BPP_DEFAULT);
	qfits_header_destroy(fh_spec);

	/* Save coeff C image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_C.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	isaac_pro_fits(	fh_spec,
					outname,
					NULL,	/* Not a reduced file */
					NULL,	/* No reduction level applicable */
					procat_imag_detlin_coeff_C,
					"OK",	/* Status */
					"detlin",	/* Recipe ID */
					datancom,
					raw,
					NULL);
	image_save_fits_hdrdump(fitres->plane[2], outname, fh_spec, BPP_DEFAULT);
	qfits_header_destroy(fh_spec);


	/* Save goodness of fit image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_Q.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	isaac_pro_fits(	fh_spec,
					outname,
					NULL,	/* Not a reduced file */
					NULL,	/* No reduction level applicable */
					procat_imag_detlin_coeff_Q,
					"OK",	/* Status */
					"detlin",	/* Recipe ID */
					datancom,
					raw,
					NULL);
	framelist_del(raw) ;
	image_save_fits_hdrdump(fitres->plane[3], outname, fh_spec, BPP_DEFAULT);
	qfits_header_destroy(fh_spec);

	/* Delete reference header */
	qfits_header_destroy(fh);

    /* Produce a PAF file */
    sprintf(outname, "%s_QC.paf", name_o);
    e_comment(1, "saving QC paf file [%s]", outname);

    paf = qfits_paf_print_header(outname,
                               "ISAAC/detlin",
                               "Detector linearity estimation",
                               get_login_name(),
                               get_datetime_iso8601());
    if (paf == NULL) {
        e_error("cannot open file [%s] for output", outname);
        return -1 ;
    }

    /* Add PRO.CATG */
    if ((sval = pfits_getprokey(INSID, procat_imag_detlin_QC)) != NULL)
        fprintf(paf, "PRO.CATG       \"%s\" ;# Product category\n", sval);

    /* Add date */
    if ((sval = pfits_get(INSID, refname, "date_obs")) != NULL)
        fprintf(paf, "DATE-OBS        \"%s\" ;# Date\n", sval) ;

    /* Add ARCFILE */
    if ((sval = pfits_get(INSID, refname, "arcfile")) != NULL)
        fprintf(paf, "ARCFILE         \"%s\" ;#\n", sval) ;

    /* Add TPL ID */
    if ((sval = pfits_get(INSID, refname, "templateid")) != NULL)
        fprintf(paf, "TPL.ID          \"%s\" ;# Template ID\n", sval) ;

    /* Add MJD-OBS for file classification */
    if ((sval = pfits_get(INSID, refname, "mjdobs")) == NULL) {
        fprintf(paf, "MJD-OBS             0.0 ; # could not find value\n");
    } else {
        fprintf(paf, "MJD-OBS             %s ; # Obs start\n", sval);
    }

    /* Forward DET.DIT */
    if ((sval = pfits_get(INSID, refname, "dit")) != NULL) {
        fprintf(paf, "DET.DIT          %s\n", sval);
    }
    /* Forward DET.NDIT */
    if ((sval = pfits_get(INSID, refname, "ndit")) != NULL) {
        fprintf(paf, "DET.NDIT         %s\n", sval);
    }
    /* Forward DET.NCORRS */
    if ((sval = pfits_get(INSID, refname, "romode_id")) != NULL) {
        fprintf(paf, "DET.NCORRS       %s\n", sval);
    }
    /* Forward DET.NCORRS.NAME */
    if ((sval = pfits_get(INSID, refname, "romode_name")) != NULL) {
        fprintf(paf, "DET.MODE.NAME  \"%s\"\n", sval);
    }

    /* QC parameters */
    fprintf(paf, "QC.DETLIN.MEDA       %g\n", med_a) ;
    fprintf(paf, "QC.DETLIN.MEDB       %g\n", med_b) ;
    fprintf(paf, "QC.DETLIN.MEDC       %g\n", med_c) ;
    
    fprintf(paf, "\n");
    fclose(paf) ;
	return 0 ;
}

