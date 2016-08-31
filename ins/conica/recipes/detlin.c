/*----------------------------------------------------------------------------*/
/**
   @file    detlin.c
   @author  Y. Jung
   @date    Aug 2003
   @version	$Revision: 1.16 $
   @brief   CONICA detector linearity test.
    Inputs:
        - A list of frames to process, with various DITs like:
        0.5 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0 0.5 6.0 7.0 8.0 9.0 10.0 0.5
        - A list of corresponding dark frames (same DITs as the ones used
        above).

    Process:
        - Subtract darks from input frames.
        - Check the stability of the level in the DIT=0.5 frames.
          exit if changes too much (1% level).
        - Reject the 0.5 frames. 
        - Determine the linearity limit (-> 2d linearity limit image)
        - Fit to each pixel the function
          DIT = a*flux + b*flux^2 + c*flux^3 + d*flux^4
          Determine a, b, c, d (-> 4 images with linearity coefficients)
        - Flag the bad pixels (-> Image of bad pixels)

    Outputs:
        - Image of the linearity limit.
        - Image of a, b, c, d coefficients.
        - Image of bad pixels
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: detlin.c,v 1.16 2004/04/22 07:25:56 yjung Exp $
	$Author: yjung $
	$Date: 2004/04/22 07:25:56 $
	$Revision: 1.16 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"
#include "conicap_lib.h"

#include "pfits.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define FRAME_DARK		    1
#define FRAME_LAMP		    2

#define DETLIN_LIMIT        0.75 
#define DETLIN_STABILITY    1

#define NPARTS			    8

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

static int conica_detlin_engine(char *, char *, int) ;
static cube_t * conica_detlin_load(char *, double **, int) ;
static int conica_detlin_save(cube_t *, image_t *, image_t *, char *, char *, 
        int) ;
static image_t * conica_detlin_limit(cube_t *) ;
static image_t * conica_detlin_bpm(cube_t *, double *, image_t *) ;

static instrument_t INSID ;
static int part=0 ;

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int conica_detlin_main(void * dict)
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

    INSID = pfits_identify_insstr("naco");

	return conica_detlin_engine(name_i, name_o, force);
}

static int conica_detlin_engine(
        char    *   name_i,
        char    *   name_o,
        int         force)
{
	cube_t	*	detlin ;
	double	*	ditval ;
    image_t *   lin_limit ;
    image_t *   bpm ;
	cube_t	*	fitres ;
	int			datancom ;

	/* Load inputs - Get and verify DITs - Subtract darks - Check stability */
	if ((detlin = conica_detlin_load(name_i, &ditval, force))==NULL) return -1 ;
     
    /* Determine the linearity limit */
	e_comment(0, "-> part %d of %d: determine linearity limit", part++,NPARTS);
    if ((lin_limit = conica_detlin_limit(detlin)) == NULL) {
		e_warning("cannot compute the linearity limit") ;
    }
    
    /* Fit the polynomials and create the coefficient images */
	e_comment(0, "-> part %d of %d: fitting polynomials (long)", part++,NPARTS);
	fitres = detector_linearity_fit(detlin, ditval, 4);
    datancom = detlin->np ;
	free(ditval);
	if (fitres==NULL) {
		e_error("fitting function to planes: aborting");
        cube_del(detlin);
        if (lin_limit!=NULL) image_del(lin_limit) ;
        if (bpm!=NULL) image_del(bpm) ;
		return -1 ;
	}

    /* Determine the bad pixels map */
	e_comment(0, "-> part %d of %d: determine bad pixels map", part++,NPARTS);
    if ((bpm = conica_detlin_bpm(detlin, ditval, fitres->plane[4])) == NULL) {
		e_warning("cannot compute the bad pixels map") ;
    }
	cube_del(detlin);

	/* Save results */
	e_comment(0, "-> part %d of %d: saving results", part++, NPARTS);
	if (conica_detlin_save(fitres, lin_limit, bpm, name_i, name_o, 
                datancom) == -1) {
        e_error("saving products");
        cube_del(fitres);
        if (lin_limit!=NULL) image_del(lin_limit) ;
        if (bpm!=NULL) image_del(bpm) ;
        return -1 ;
    }
            
    /* Free and return */
    cube_del(fitres);
    if (lin_limit!=NULL) image_del(lin_limit) ;
    if (bpm!=NULL) image_del(bpm) ;
	return 0 ;
}

/*
 * Load all input frames, check that there are as many darks as linearity
 * images and that they have the corresponding integration times.
 * The files with identical DITS are also checked for intensity variations.
 */
static cube_t * conica_detlin_load(
        char    *   listname, 
        double  **  ditval,
        int         force)
{
	framelist	*	in_list ;
	framelist	*	dark_list ;
	framelist	*	lamp_list ;
	int				i, j ;
	char		*	sval ;
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
	e_comment(0, "-> part %d of %d: frame identification", part++, NPARTS);
	if ((in_list = framelist_load(listname)) == NULL) {
		e_error("cannot load %s", listname);
		return NULL ;
	}
	/* Assign labels to frames */
	n_dark=n_lamp=0 ;
	for (i=0 ; i<in_list->n ; i++) {
		sval = pfits_get(INSID, in_list->name[i], "dpr_type");
		if (sval==NULL) {
			e_error("no DPR TYPE for frame %s", in_list->name[i]);
            framelist_del(in_list);
            return NULL ;
		} else {
			if (!strcmp(sval, "OTHER,LINEARITY")) {
				/* Frame is a dark */
				in_list->label[i] = FRAME_DARK ;
				n_dark++ ;
			} else if (!strcmp(sval, "LAMP,LINEARITY")) {
				/* Frame is a linearity image */
				in_list->label[i] = FRAME_LAMP ;
				n_lamp++ ;
			} else {
				/* Invalid DPR TYPE for this frame */
				e_error("invalid DPR TYPE in %s: [%s]", in_list->name[i], sval);
                framelist_del(in_list);
                return NULL ;
			}
		}
	}
	/* Check that there are as many darks as input images */
	if (n_dark!=n_lamp) {
		e_error("inconsistent data: %d darks for %d images", n_dark, n_lamp);
        framelist_del(in_list);
        return NULL ;
	}
	
    /* Create new framelists for linearity and dark frames */
	lamp_list = framelist_select(in_list, FRAME_LAMP);
	dark_list = framelist_select(in_list, FRAME_DARK);

    /* Discard initial list */
	framelist_del(in_list);

	/* Check out that they have consistent integration times */
	e_comment(0, "-> part %d of %d: checking DIT consistency", part++, NPARTS);
	same_dit = malloc(lamp_list->n * sizeof(int));
	ditval_load = malloc(lamp_list->n * sizeof(double));
	n_same_dit=0 ;
	for (i=0 ; i<lamp_list->n ; i++) {
		/* Get integration time for lamp and dark */
		lamp_integ = pfits_get(INSID, lamp_list->name[i], "dit");
		dark_integ = pfits_get(INSID, dark_list->name[i], "dit");
		if ((lamp_integ==NULL) || (dark_integ==NULL)) {
			e_error("frames %s or %s has no DET.DIT", 
                    lamp_list->name[i], dark_list->name[i]);
            free(ditval_load);
            free(same_dit);
            framelist_del(lamp_list);
            framelist_del(dark_list);
            return NULL ;
		}
		e_comment(1, "LAMP %s DIT %s", get_basename(lamp_list->name[i]),
				  lamp_integ);
		e_comment(1, "DARK %s DIT %s", get_basename(dark_list->name[i]),
				  lamp_integ);
		
        /* If first loaded frame, record integration time */
		if (i==0) {
			init_dit = strdup(lamp_integ);
			same_dit[i] = 1 ;
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
	
        /* Compare DIT for lamp and dark */
		if (strcmp(lamp_integ, dark_integ)) {
			e_error("DIT inconsistency");
			e_error("file %s has DIT=%s", lamp_list->name[i], lamp_integ);
			e_error("file %s has DIT=%s", dark_list->name[i], dark_integ);
            free(ditval_load);
            free(same_dit);
            framelist_del(lamp_list);
            framelist_del(dark_list);
            return NULL ;
		}
	}
	free(init_dit);

	/* Check that there are frames with identical DITs */
	if (n_same_dit<2) {
		e_error("no two frames with identical DIT");
		free(ditval_load);
		free(same_dit);
		framelist_del(lamp_list);
		framelist_del(dark_list);
		return NULL ;
	}
	e_comment(1, "DIT consistency Ok");

	/* Compute level in frames of identical DIT */
	e_comment(0, "-> part %d of %d: checking lamp stability", part++, NPARTS);
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
	for (i=1 ; i<n_same_dit ; i++) {
		if (((level_same_dit[i]-level_same_dit[0]) / level_same_dit[0]) 
                > (DETLIN_STABILITY/100.0)) {
			if (force == 1) {
                e_warning("level difference #%d too high - proceed anyway",i+1);
            } else {
                e_error("level difference #%d too high", i+1);
                free(ditval_load);
                free(same_dit);
                free(level_same_dit);
                framelist_del(lamp_list);
                framelist_del(dark_list);
                return NULL ;
            }
		}
	}
	/* Discard levels */
	free(level_same_dit);
    e_comment(1, "lamp level check Ok");

	/* Load frames and subtract them as they load */
	e_comment(0, "-> part %d of %d: load dark-subtracted frames",part++,NPARTS);
	np = lamp_list->n - n_same_dit ;
	lampcube = cube_new(lx, ly, np);
	j=0 ;
	for (i=0 ; i<lamp_list->n ; i++) {
		if (same_dit[i]==0) {
			e_comment(1, "loading/subtracting DIT %g", ditval_load[i]);
			lamp_1 = image_load(lamp_list->name[i]);
			dark_1 = image_load(dark_list->name[i]);
			if ((lamp_1==NULL) || (dark_1==NULL)) {
				e_error("loading pair %s / %s ", 
                        lamp_list->name[i], dark_list->name[i]);
                free(ditval_load);
                free(same_dit);
                cube_del(lampcube);
                framelist_del(lamp_list);
                framelist_del(dark_list);
                return NULL ;
			}
			image_sub_local(lamp_1, dark_1);
			image_del(dark_1);
			lampcube->plane[j]=lamp_1 ;
			j++ ;
		}
	}
	framelist_del(dark_list);
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

static image_t * conica_detlin_limit(cube_t * data) 
{
    image_t     *   lin_limit ;
    pixelvalue      ref_diff, curr_diff ;
    int             i, k ;
    
    /* Test entries */
    if (data->np < 3) {
        e_error("Not enough planes to compute the limit") ;
        return NULL ;
    }
    
    /* Create the output image */
    if ((lin_limit = image_new(data->lx, data->ly)) == NULL) {
        e_error("cannot create limit image") ;
        return NULL ;
    }

    /* Compute the limit for each pixel */
    for (i=0 ; i<lin_limit->lx * lin_limit->ly ; i++) {
        ref_diff = data->plane[1]->data[i] - data->plane[0]->data[i] ;
        for (k=1 ; k<data->np-1 ; k++) {
            curr_diff = data->plane[k+1]->data[i] - data->plane[k]->data[i] ;
            if (curr_diff < DETLIN_LIMIT*ref_diff) {
                /* The limit is reached */
                lin_limit->data[i] = data->plane[k]->data[i] ;
                break ;
            }
            /* The limit is undefined */
            if (k+1 == data->np-1) lin_limit->data[i] = (pixelvalue)0.0 ;
        }
    }

    /* Return */
    return lin_limit ;
}

static image_t * conica_detlin_bpm(
        cube_t  *   data,
        double  *   dits,
        image_t *   chisq) 
{
    image_t     *   bpm ;
    pixelvalue      ref_val, curr_val ;
    int             i, k ;
    
    /* Test entries */
    if (data->np < 3) {
        e_error("Not enough planes to compute the bad pixels") ;
        return NULL ;
    }
    
    /* Create the output image */
    if ((bpm = image_new(data->lx, data->ly)) == NULL) {
        e_error("cannot create bad pixels map image") ;
        return NULL ;
    }

    /* Compute the bad pixel value for each pixel */
    for (i=0 ; i<bpm->lx * bpm->ly ; i++) {
        /* A pixel is a priori good */
        bpm->data[i] = 0.0 ;

        /* First test */
        if (fabs(dits[1] - dits[0]) < 1e-3) {
            image_del(bpm) ;
            e_error("consecutive DITS are identical - abort") ;
            return NULL ;
        }
        ref_val = (data->plane[1]->data[i] - data->plane[0]->data[i]) /
            (dits[1] - dits[0]) ;
        for (k=1 ; k<data->np-1 ; k++) {
            if (fabs(dits[k+1] - dits[k]) < 1e-3) {
                image_del(bpm) ;
                e_error("consecutive DITS are identical - abort") ;
                return NULL ;
            }
            curr_val = (data->plane[k+1]->data[i] - data->plane[k]->data[i]) /
                (dits[k+1] - dits[k]) ;
            if (curr_val > 1.5 * ref_val) {
                bpm->data[i] += 1.0 ;
                break ;
            }
        }
        
        /* Second test */
        if (data->plane[1]->data[i]-data->plane[0]->data[i] < 0)
            bpm->data[i] += 2.0 ;
        
        /* Third test */
        if (chisq->data[i] > 10.0) bpm->data[i] += 4.0 ;
    }

    /* Return */
    return bpm ;
}

static int conica_detlin_save(
	cube_t 	*   fitres,
    image_t *   lin_limit,
    image_t *   bpm,
	char 	*   name_i,
	char 	*   name_o,
	int		    datancom)
{
	double			med ;
	image_t		*	div ;
	char			outname[FILENAMESZ];
	char		*	refname ;
	qfits_header*	fh,
				*	fh_spec ;
	framelist	*	raw ;

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
	conica_header_for_image(fh);

	/* Save coeff A image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_A.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	raw = framelist_load(name_i) ;
	conica_pro_fits(fh_spec,
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
	conica_pro_fits(fh_spec,
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
	conica_pro_fits(fh_spec,
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

	/* Save coeff D image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_D.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	conica_pro_fits(fh_spec,
					outname,
					NULL,	/* Not a reduced file */
					NULL,	/* No reduction level applicable */
					procat_imag_detlin_coeff_D,
					"OK",	/* Status */
					"detlin",	/* Recipe ID */
					datancom,
					raw,
					NULL);
	image_save_fits_hdrdump(fitres->plane[3], outname, fh_spec, BPP_DEFAULT);
	qfits_header_destroy(fh_spec);

	/* Save linearity limit image */
	if (lin_limit != NULL) {
        fh_spec = qfits_header_copy(fh);
        sprintf(outname, "%s_limit.fits", name_o);
        e_comment(1, "saving image [%s]", outname);
        conica_pro_fits(fh_spec,
                        outname,
                        NULL,	/* Not a reduced file */
                        NULL,	/* No reduction level applicable */
                        procat_imag_detlin_limit,
                        "OK",	/* Status */
                        "detlin",	/* Recipe ID */
                        datancom,
                        raw,
                        NULL);
        image_save_fits_hdrdump(lin_limit, outname, fh_spec,BPP_DEFAULT);
        qfits_header_destroy(fh_spec);
    }

	/* Save bad pixels map image */
	if (bpm != NULL) {
        fh_spec = qfits_header_copy(fh);
        sprintf(outname, "%s_bpm.fits", name_o);
        e_comment(1, "saving image [%s]", outname);
        conica_pro_fits(fh_spec,
                        outname,
                        NULL,	/* Not a reduced file */
                        NULL,	/* No reduction level applicable */
                        procat_imag_detlin_bpm,
                        "OK",	/* Status */
                        "detlin",	/* Recipe ID */
                        datancom,
                        raw,
                        NULL);
        image_save_fits_hdrdump(bpm, outname, fh_spec,BPP_DEFAULT);
        qfits_header_destroy(fh_spec);
    }

	/* Save goodness of fit image */
	fh_spec = qfits_header_copy(fh);
	sprintf(outname, "%s_Q.fits", name_o);
	e_comment(1, "saving image [%s]", outname);
	conica_pro_fits(fh_spec,
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
	image_save_fits_hdrdump(fitres->plane[4], outname, fh_spec, BPP_DEFAULT);
	qfits_header_destroy(fh_spec);

	/* Delete reference header */
	qfits_header_destroy(fh);
	return 0 ;
}

