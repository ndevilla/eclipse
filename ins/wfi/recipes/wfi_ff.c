
/*---------------------------------------------------------------------------
   
   File name 	:	wfi_ff.c
   Author 		:	N. Devillard
   Created on	:	Sept 2000
   Description	:	WFI flat-field creation recipe

 *--------------------------------------------------------------------------*/

/*
	$Id: wfi_ff.c,v 1.19 2001/10/22 11:57:29 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/22 11:57:29 $
	$Revision: 1.19 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "eclipse.h"
#include "wfip_lib.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define OPT_XTNUM			1000

#define OPT_BIASNAME		1005
#define OPT_DOMENAME		1010
#define OPT_DOMETHRESH		1011
#define OPT_DOMEMINFRAME	1012
#define OPT_DOMESAVE		1013
#define OPT_SKYNAME			1020
#define OPT_SKYTHRESH		1021
#define OPT_SKYMINFRAME		1022
#define OPT_SKYSAVE			1023
#define OPT_PRESCAN			1030
#define OPT_OVERSCAN		1040
#define OPT_SCANREJ			1041
#define OPT_TRIMMING		1045
#define OPT_READOUTNOISE	1050
#define OPT_PIXELGAIN		1060
#define OPT_PIXELREJ		1070
#define OPT_MEDIANREG		1080
#define OPT_KERNELSIZE		1090


#define WFI_MIN_NUM_FRAMES	5
#define WFIFF_DEFAULTNAME	"wfi_mff.fits"

static char recipe_version[] = "$Revision: 1.19 $";

static void usage(char *pname) ;
static char prog_desc[] = "WFI master flat-field creation" ;


typedef struct _wfiff_bb_ {
	/* Input/Output file names */
	char	*	name_dome ;
	char	*	name_sky ;
	char	*	name_o ;
	char	*	name_bias ;

	int			xtnum ;

	/* Dome parameters */
	double		dome_framethr_lo ;
	double		dome_framethr_hi ;
	int			dome_minframe ;
	int			dome_save ;

	/* Sky parameters */
	double		sky_framethr_lo ;
	double		sky_framethr_hi ;
	int			sky_minframe ;
	int			sky_save ;

	/* Pre/Over scan parameters */
	int			prescan_x[2] ; 	/* xmin xmax */
	int			overscan_x[2] ;	/* xmin xmax */
	int			scanrej[2] ;	/* min max */
	int			trimreg[4] ;	/* xmin xmax ymin ymax */

	/* Readout noise */
	double		readout_noise ;
	double		pixel_gain ;

	/* Pixel rejection */
	double		pixrej_lo ;
	double		pixrej_hi ;

	/* Median region definition */
	int			median_reg[4] ; /* xmin xmax ymin ymax */
	int			median_surf ;

	/* Smoothing kernel dimension */
	int			kern_hsize ;

} wfiff_bb ;


wfiff_bb * wfiff_bb_new(void)
{
	wfiff_bb	*	bb ;

	bb = calloc(1, sizeof(wfiff_bb));

	/* Set default parameters for all */

	bb->dome_framethr_lo = 10000 ;
	bb->dome_framethr_hi = 45000 ;
	bb->dome_minframe    = 5 ;

	bb->sky_framethr_lo  = 10000 ;
	bb->sky_framethr_hi  = 45000 ;
	bb->sky_minframe     = 5 ;

	bb->prescan_x[0]	= WFI_PRESCAN_X_MIN ;
	bb->prescan_x[1]	= WFI_PRESCAN_X_MAX ;
	bb->overscan_x[0]	= WFI_OVERSCAN_X_MIN ;
	bb->overscan_x[1]	= WFI_OVERSCAN_X_MAX ;
	bb->scanrej[0]		= 10 ;
	bb->scanrej[1]		= 10 ;

	bb->trimreg[0]	= WFI_CROP_X_MIN ;
	bb->trimreg[1]	= WFI_CROP_X_MAX ;
	bb->trimreg[2]	= WFI_CROP_Y_MIN ;
	bb->trimreg[3]	= WFI_CROP_Y_MAX ;

	bb->readout_noise	= 4.8 ;
	bb->pixel_gain		= 2.2 ;

	bb->pixrej_lo	= 3.0 ;
	bb->pixrej_hi	= 3.0 ;

	bb->median_reg[0] = 500 ;
	bb->median_reg[1] = 1500 ;
	bb->median_reg[2] = 1500 ;
	bb->median_reg[3] = 2500 ;

	bb->median_surf   = (bb->median_reg[1]-bb->median_reg[0]+1) *
						(bb->median_reg[3]-bb->median_reg[2]+1);

	bb->kern_hsize  = 4 ;

	return bb ;
}

void wfiff_bb_del(wfiff_bb * bb)
{
	if (bb==NULL) return ;

	if (bb->name_dome) free(bb->name_dome);
	if (bb->name_sky) free(bb->name_sky);
	if (bb->name_bias) free(bb->name_bias);
	if (bb->name_o) free(bb->name_o);

	free(bb);
}


void wfiff_bb_dump(wfiff_bb * bb, FILE * out)
{
	fprintf(out,
"\n"
"Parameters for this command:\n"
"\n"
"[Extension]\n"
"xtnum       = %d\n"
"\n"
"[Output]\n"
"Name        = %s\n"
"\n"
"[Bias]\n"
"Name        = %s\n"
"\n"
"[Dome]\n"
"Name        = %s\n"
"LoThresh    = %g\n"
"HiThresh    = %g\n"
"MinFrames   = %d\n"
"SaveFlat    = %s\n"
"\n"
"[Sky]\n"
"Name        = %s\n"
"LoThresh    = %g\n"
"HiThresh    = %g\n"
"MinFrames   = %d\n"
"SaveFlat    = %s\n"
"\n"
"[Prescan]\n"
"xmin        = %d\n"
"xmax        = %d\n"
"\n"
"[Overscan]\n"
"xmin        = %d\n"
"xmax        = %d\n"
"\n"
"[Scan rejection]\n"
"min         = %d\n"
"max         = %d\n"
"\n"
"[Trimming]\n"
"xmin        = %d\n"
"xmax        = %d\n"
"ymin        = %d\n"
"ymax        = %d\n"
"\n"
"[Pixel]\n"
"ReadoutNoise  = %g\n"
"Gain          = %g\n"
"LoSigmaRej    = %g\n"
"HiSigmaRej    = %g\n"
"\n"
"[Median region]\n"
"xmin        = %d\n"
"xmax        = %d\n"
"ymin        = %d\n"
"ymax        = %d\n"
"\n"
"[Smoothing kernel]\n"
"HalfSize    = %d\n"
"\n\n",

	bb->xtnum,
	bb->name_o ? bb->name_o : WFIFF_DEFAULTNAME,
	bb->name_bias ? bb->name_bias : "none",
	bb->name_dome ? bb->name_dome : "unknown",
	bb->dome_framethr_lo,
	bb->dome_framethr_hi,
	bb->dome_minframe,
	bb->dome_save ? "yes" : "no",
	bb->name_sky ? bb->name_sky : "unknown",
	bb->sky_framethr_lo,
	bb->sky_framethr_hi,
	bb->sky_minframe,
	bb->sky_save ? "yes" : "no",
	bb->prescan_x[0],
	bb->prescan_x[1],
	bb->overscan_x[0],
	bb->overscan_x[1],
	bb->scanrej[0],
	bb->scanrej[1],
	bb->trimreg[0],
	bb->trimreg[1],
	bb->trimreg[2],
	bb->trimreg[3],
	bb->readout_noise,
	bb->pixel_gain,
	bb->pixrej_lo,
	bb->pixrej_hi,
	bb->median_reg[0],
	bb->median_reg[1],
	bb->median_reg[2],
	bb->median_reg[3],
	bb->kern_hsize
	);
}

image_t * wfiff_buildflat(wfiff_bb * bb, char * what)
{
	cube_t	 *	i_cube ;
	image_t *	scancorr ;
	image_t *  bias ;
	image_t *  cur_p ;
	image_t *  flat ;
	char	 *	name_i ;
	double		lo_thr, hi_thr ;
	int			minframe ;
	double		avg ;
	int			i, j, p ;
	int		 *	valid ;
	int			nval ;
	double		sigma ;
	pixelvalue * validpix ;
	pixelvalue	curpix ;
	pixelvalue	medval ;

	/* Check entries */
	if (bb==NULL) return NULL ;
	if (!what || !*what) return NULL;

	/* Set parameters to requested frame type */
	e_comment(1, "setting input parameters...");
	if (!strcmp(what, "dome")) {
		name_i = bb->name_dome ;
		lo_thr = bb->dome_framethr_lo ;
		hi_thr = bb->dome_framethr_hi ;
		minframe = bb->dome_minframe ;
	} else if (!strcmp(what, "sky")) {
		name_i = bb->name_sky ;
		lo_thr = bb->sky_framethr_lo ;
		hi_thr = bb->sky_framethr_hi ;
		minframe = bb->sky_minframe ;
	} else {
		e_error("unknown frame type [%s] should be 'dome' or 'sky'",what);
		return NULL ;
	}

	/* Load input cube */
	e_comment(1, "loading input cube [%s]", what);
	i_cube = wfi_cube_load(name_i, bb->xtnum) ;
	if (i_cube==NULL) {
		e_error("loading input data: [%s]", name_i);
		return NULL ;
	}
	if (i_cube->np < WFI_MIN_NUM_FRAMES) {
		e_error("found only [%d] frames - need at least %d",
				i_cube->np,
				WFI_MIN_NUM_FRAMES);
		cube_del(i_cube);
		return NULL ;
	}
    /* Compute average value for each frame, keep those in interval */
	valid = malloc(i_cube->np * sizeof(int));
	nval = 0 ;
    for (i=0 ; i<i_cube->np ; i++) {
        compute_status("computing averages", i, i_cube->np, 1);
        avg = image_getmean(i_cube->plane[i]);
		/*
		if (debug_active()>1) {
			printf("\n");
			printf("avg/thr: %g [%g - %g]\n", avg, lo_thr, hi_thr);
		}
		*/
		if ((avg>lo_thr) && (avg<hi_thr)) {
			valid[i] = 1 ;
			nval++ ;
		} else {
			valid[i] = 0 ;
		}
    }
	e_comment(1, "frame rejection status:");
	for (i=0 ; i<i_cube->np ; i++) {
		if (valid[i]) {
			e_comment(1, "frame %02d Ok", i+1);
		} else {
			e_comment(1, "frame %02d rejected", i+1);
		}
	}
	if (nval<minframe) {
		e_error("not enough frames to continue: %d", nval);
		free(valid);
		cube_del(i_cube);
		return NULL ;
	}

    /* Reduce cube to valid frames only */
    if (nval < i_cube->np) {
		e_comment(1, "reducing cube to valid planes only");
		cube_reject_planes(&i_cube, valid);
    }
    free(valid);
	
	/* Apply prescan/overscan/trimming correction */
	e_comment(1, "applying overscan/prescan/trimming correction");
	for (i=0 ; i<i_cube->np ; i++) {
		compute_status("correcting", i, i_cube->np, 1);
		scancorr = wfi_overscan_correction(i_cube->plane[i],
										   bb->prescan_x,
										   bb->overscan_x,
										   bb->scanrej,
										   bb->trimreg);
		if (scancorr == NULL) {
			e_error("during overscan correction in plane %d", i+1);
			cube_del(i_cube);
			return NULL ;
		}
		image_del(i_cube->plane[i]);
		i_cube->plane[i] = scancorr ;
	}
	/* Cube size has changed, recompute sizes */
	i_cube->lx = i_cube->plane[0]->lx;
	i_cube->ly = i_cube->plane[0]->ly;

	/* Subtract bias if requested */
	if (bb->name_bias != NULL) {
		e_comment(1, "subtracting bias frame from all frames...");
		bias = image_load(bb->name_bias);
		if (bias==NULL) {
			e_error("cannot load bias [%s]: aborting", bb->name_bias);
			cube_del(i_cube);
			return NULL ;
		}
		if ((bias->lx != i_cube->lx) || (bias->ly != i_cube->ly)) {
			e_error("bias and cube have incompatible sizes: aborting");
			e_error("bias is [%d %d]", bias->lx, bias->ly);
			e_error("cube is [%d %d]", i_cube->lx, i_cube->ly);
			cube_del(i_cube);
			image_del(bias);
			return NULL ;
		}
		cube_sub_im(i_cube, bias);
		image_del(bias);
	}

	/* CCD clip rejection */
	e_comment(1, "CCD clip rejection");

	for (p=0 ; p<i_cube->np ; p++) {
		compute_status("clip rejection on frame", p, i_cube->np, 1);
		cur_p = i_cube->plane[p] ;

        avg = image_getmean(cur_p);
		sigma = sqrt(pow(bb->readout_noise/bb->pixel_gain, 2.0) +
					 (avg / bb->pixel_gain));
		lo_thr = avg - bb->pixrej_lo * sigma ;
		hi_thr = avg + bb->pixrej_hi * sigma ;
		e_comment(1, "rejection interval");
		e_comment(1, "[%g - %g]]", lo_thr, hi_thr);
		/*
		if (debug_active()>1) {
			printf("avg= %g sigma= %g thr= [%g - %g]\n",
					avg, sigma, lo_thr, hi_thr);
		}
		*/

		/* Accumulate valid pixels in an array */
		validpix = malloc(bb->median_surf * sizeof(pixelvalue));
		nval = 0 ;
		for (j=0 ; j<cur_p->ly ; j++) {
			if (j>=bb->median_reg[2] && j<=bb->median_reg[3]) {
				for (i=0 ; i<cur_p->lx ; i++) {
					if (i>=bb->median_reg[0] && i<=bb->median_reg[1]) {
						curpix = cur_p->data[i+j*cur_p->lx];
						if (((double)curpix > lo_thr) &&
							((double)curpix < hi_thr)) {
							validpix[nval] = curpix ;
							nval ++ ;
						}
					}
				}
			}
		}
		if (nval<1) {
			e_error("no valid pixel found in frame %02d: aborting", p+1);
			cube_del(i_cube);
			free(validpix);
			return NULL ;
		}
		/* Get the median from the list of valid pixels */
		medval = median_pixelvalue(validpix, nval);
		free(validpix);

		if (fabs(medval)<1e-4) {
			e_error("zero value for median: aborting");
			/* cube_del(i_cube); */
			/* return NULL ; */
		} else {
			/* Divide the frame by this value */
			for (i=0 ; i<(cur_p->lx * cur_p->ly) ; i++) {
				cur_p->data[i] /= medval ;
			}
		}
	}

	/* Stack frames to single frames using a median */
	e_comment(1, "stacking cube to single frame");
	flat = cube_avg_median(i_cube);
	cube_del(i_cube);

	if (flat==NULL) {
		e_error("in final stacking: aborting");
		return NULL ;
	}
	return flat ;
}

image_t * wfiff_combine(wfiff_bb * bb, image_t * dome, image_t * sky)
{
	image_t	*	smoothed ;

	if (bb==NULL || dome==NULL || sky==NULL) return NULL ;

	/* Divide sky by dome */
	e_comment(1, "dividing sky by dome...");
	image_div_local(sky, dome);

	/* Smooth result */
	e_comment(1, "applying smoothing filter...");
	smoothed = image_filter_flat(sky, bb->kern_hsize);

	/* Multiply by dome */
	e_comment(1, "multiplying by dome flat...");
	image_mul_local(smoothed, dome);

	return smoothed ;
}

void wfiff_save(wfiff_bb * bb, image_t * flat)
{
	qfits_header*	fh ;
	qfits_header*	fh_ref ;
	char			cval[80] ;
	char		*	getval ;
	int				i ;
	char		*	first_name ;
	framelist	*	fnames ;
	int				nchips ;

	fh = qfits_header_default();
	
	/*
	 * Read header from the first reference frame, which is the first frame
	 * in the sky batch
	 */
	if (is_fits_file(bb->name_sky)) {
		fh_ref = qfits_header_read(bb->name_sky);
	} else {
		first_name = framelist_firstname(bb->name_sky);
		if (first_name==NULL) {
			e_error("cannot get reference FITS header from %s", bb->name_sky);
			e_error("aborting save");
			return ;
		}
		fh_ref = qfits_header_read(first_name);
	}

	/* BITPIX */
	sprintf(cval, "%d", BPP_DEFAULT);
	qfits_header_add(fh, "BITPIX", cval, "Bits per pixel", NULL);
	/* NAXIS* */
	qfits_header_add(fh, "NAXIS", "2", "Dimensions", NULL);
	sprintf(cval, "%d", flat->lx);
	qfits_header_add(fh, "NAXIS1", cval, "size in X", NULL);
	sprintf(cval, "%d", flat->ly);
	qfits_header_add(fh, "NAXIS2", cval, "size in Y", NULL);
	/* ORIGIN */
	qfits_header_add(fh, "ORIGIN", "ESO", "File originator", NULL);
	/* INSTRUME */
	qfits_header_add(fh, "INSTRUME", "WFI", "Instrument", NULL);
	/* TELESCOP */
	qfits_header_add(fh, "TELESCOP", "MPI-2.2", "Telescope", NULL);
    /* DATE-OBS */
    getval=NULL ;
    if (fh_ref!=NULL) {
        getval = qfits_header_getstr(fh_ref, "DATE-OBS");
    }
    if (getval==NULL) {
        qfits_header_add(fh, "DATE-OBS", "Unknown", "Date of observation", NULL);    } else {
        qfits_header_add(fh, "DATE-OBS", getval, "Date of observation", NULL);
    }
    /* MJD-OBS */
    getval=NULL ;
    if (fh_ref!=NULL) {
        getval = qfits_header_getstr(fh_ref, "MJD-OBS");
    }
    if (getval==NULL) {
        qfits_header_add(fh, "MJD-OBS", "Unknown", "MJD start of 1st frame", NULL);
	} else {
        qfits_header_add(fh, "MJD-OBS", getval, "MJD start of 1st frame", NULL);
    }
    /* ECLIPSE */
    qfits_header_add(fh, "ECLIPSE",
                    get_eclipse_version(), "Eclipse version", NULL);
    /* PRO.CATG */
    qfits_header_add(fh, "HIERARCH ESO PRO CATG", "MASTER_FLAT",
                    "product frame type", NULL);

	/* Add REC.FLAT.DOMEi */
	fnames = framelist_load(bb->name_dome);
	if (fnames!=NULL) {
		for (i=0 ; i<fnames->n ; i++) {
			sprintf(cval, "HIERARCH ESO REC FLAT DOME%03d", i+1);
			qfits_header_add(fh,
							cval,
							get_basename(fnames->name[i]),
							"Input frame name",
							NULL);
		}
		framelist_del(fnames);
	} else {
		e_warning("cannot read dome list [%s]: no output in header",
				  bb->name_dome);
	}

	/* Add REC.FLAT.SKYi */
	fnames = framelist_load(bb->name_sky);
	if (fnames!=NULL) {
		for (i=0 ; i<fnames->n ; i++) {
			sprintf(cval, "HIERARCH ESO REC FLAT SKY%03d", i+1);
			qfits_header_add(fh,
							cval,
							get_basename(fnames->name[i]),
							NULL,
							NULL);
		}
		framelist_del(fnames);
	} else {
		e_warning("cannot read sky list [%s]: no output in header",
				  bb->name_sky);
	}

	/* REC.FLAT.MBIAS */
	if (bb->name_bias!=NULL) {
		qfits_header_add(fh,
						"HIERARCH ESO REC FLAT MBIAS",
						get_basename(bb->name_bias),
						NULL,
						NULL);
	} else {
		qfits_header_add(fh,
						"HIERARCH ESO REC FLAT MBIAS",
						"none",
						NULL,
						NULL);
	}

	/*
	 * REC.FLAT.LFRATHD
	 * REC.FLAT.HFRATHD
	 * REC.FLAT.MINFRAD
	 */
	sprintf(cval, "%g", bb->dome_framethr_lo);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT LFRATHD",
					cval,
					"low frame threshold for dome",
					NULL);

	sprintf(cval, "%g", bb->dome_framethr_hi);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT HFRATHD",
					cval,
					"high frame threshold for dome",
					NULL);

	sprintf(cval, "%d", bb->dome_minframe);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT MINFRAD",
					cval,
					"high frame threshold for dome",
					NULL);

	/*
	 * REC.FLAT.LFRATHS
	 * REC.FLAT.HFRATHS
	 * REC.FLAT.MINFRAS
	 */
	sprintf(cval, "%g", bb->sky_framethr_lo);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT LFRATHS",
					cval,
					"low frame threshold for sky",
					NULL);

	sprintf(cval, "%g", bb->sky_framethr_hi);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT HFRATHS",
					cval,
					"high frame threshold for sky",
					NULL);

	sprintf(cval, "%d", bb->sky_minframe);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT MINFRAS",
					cval,
					"high frame threshold for sky",
					NULL);

    /* REC.FLAT.PRSCX */
    sprintf(cval, "'%d %d'", bb->prescan_x[0], bb->prescan_x[1]);
    qfits_header_add(fh, "HIERARCH ESO REC FLAT PRSCX", cval,
                    "Prescan xmin xmax", NULL);
    /* REC.FLAT.OVSCX */
    sprintf(cval, "'%d %d'", bb->overscan_x[0], bb->overscan_x[1]);
    qfits_header_add(fh, "HIERARCH ESO REC FLAT OVSCX", cval,
                    "Overscan xmin xmax", NULL);
    /* REC.FLAT.RJOVSC */
    sprintf(cval, "'%d %d'", bb->scanrej[0], bb->scanrej[1]);
    qfits_header_add(fh, "HIERARCH ESO REC FLAT RJOVSC", cval,
                    "Rejection min max", NULL);
    /* REC.BIAS.TRIM */
    sprintf(cval, "'%d %d %d %d'",
            bb->trimreg[0],
            bb->trimreg[1],
            bb->trimreg[2],
            bb->trimreg[3]) ;
    qfits_header_add(fh, "HIERARCH ESO REC FLAT TRIM", cval,
                    "xmin xmax ymin ymax", NULL);

	/* REC.FLAT.RDNOISE */
	sprintf(cval, "%f", bb->readout_noise);
	qfits_header_add(fh,
					"HIERARCH ESO REC FLAG RDNOISE",
					cval,
					"Read out noise",
					NULL);

	/* REC.FLAT.LPIXSIG */
	sprintf(cval, "%f", bb->pixrej_lo);
	qfits_header_add(fh,
					"HIERARCH ESO REC FLAT LPIXSIG",
					cval,
					"low sigma for pixel rejection",
					NULL);

	/* REC.FLAT.HPIXSIG */
	sprintf(cval, "%f", bb->pixrej_hi);
	qfits_header_add(fh,
					"HIERARCH ESO REC FLAT HPIXSIG",
					cval,
					"high sigma for pixel rejection",
					NULL);
	/* REC.FLAT.MREGDEF */
	sprintf(cval, "'%d %d %d %d'",
			bb->median_reg[0],
			bb->median_reg[1],
			bb->median_reg[2],
			bb->median_reg[3]);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT MREGDEF",
					cval,
					"Region for median computation",
					NULL);
	/* REC.FLAT.KERNS */
	sprintf(cval, "%d", bb->kern_hsize);
	qfits_header_add(fh,
					"HIERARCH ESO FLAT KERNS",
					cval,
					"Smoothing kernel size",
					NULL);

	/* REC.FLAT.RECVERS */
	qfits_header_add(fh,
					"HIERARCH ESO REC FLAT RECVERS",
					recipe_version,
					"Recipe version",
					NULL);

	/* INS.FILT.NAME */
	getval = NULL ;
	if (fh_ref!=NULL) {
		getval = qfits_header_getstr(fh_ref, "HIERARCH ESO INS FILT NAME");
	}
	if (getval==NULL) {
		qfits_header_add(fh,
						"HIERARCH ESO INS FILT NAME",
						"unknown",
						"Filter name",
						NULL);
	} else {
		qfits_header_add(fh,
						"HIERARCH ESO INS FILT NAME",
						getval,
						"Filter name",
						NULL);
	}

	/* DET.CHIP.ID */
    /* Find out how many CCD chips */
	nchips=-1 ;
	if (fh_ref != NULL) {
		nchips = qfits_header_getint(fh_ref, "HIERARCH ESO DET CHIPS", -1);
	}
    if (nchips<0) {
        nchips = WFI_NCHIPS ;
    }
    getval = NULL ;
    for (i=0 ; i<nchips ; i++) {
        sprintf(cval, "HIERARCH ESO DET CHIP%d ID", i+1);
        getval = qfits_header_getstr(fh_ref, cval);
        if (getval!=NULL)
            break ;
    }
    if (getval!=NULL) {
        qfits_header_add(fh, "HIERARCH ESO DET CHIP ID", getval,
                        "Chip ID", NULL);
    } else {
        qfits_header_add(fh, "HIERARCH ESO DET CHIP ID", "Unknown",
                        "Chip ID", NULL);
    }
	/* DET.OUT.GAIN */
    getval = NULL ;
    for (i=0 ; i<nchips ; i++) {
        sprintf(cval, "HIERARCH ESO DET OUT%d GAIN", i+1);
        getval = qfits_header_getstr(fh_ref, cval);
        if (getval!=NULL)
            break ;
    }
    if (getval!=NULL) {
        qfits_header_add(fh, "HIERARCH ESO DET OUT GAIN", getval,
                        "pixel gain", NULL);
    } else {
        qfits_header_add(fh, "HIERARCH ESO DET OUT GAIN", "Unknown",
                        "pixel gain", NULL);
    }

    qfits_header_destroy(fh_ref);

	e_comment(1, "saving result image [%s]", bb->name_o);
	image_save_fits_hdrdump(flat, bb->name_o, fh, BPP_DEFAULT);
	qfits_header_destroy(fh);
}



#define ALGPARTS		3
int wfiff_engine(wfiff_bb * bb)
{
	image_t	*	flat_dome ;
	image_t	*	flat_sky ;
	image_t	*	flat_combined ;
	int				part ;

	part = 0;
	e_comment(0, "--> START WFI Flat-field engine");

	part++ ;
	e_comment(0,"---> Part %d in %d: Dome flat construction",part,ALGPARTS);
	flat_dome = wfiff_buildflat(bb, "dome");
	if (flat_dome==NULL) {
		e_error("in dome flat production: aborting");
		return -1 ;
	}
	if (bb->dome_save) {
		e_comment(1, "saving dome flat [flat_dome.fits]");
		image_save_fits(flat_dome, "flat_dome.fits", BPP_DEFAULT);
	}

	part++ ;
	e_comment(0,"---> Part %d in %d: Sky flat construction",part,ALGPARTS);
	flat_sky = wfiff_buildflat(bb, "sky");
	if (flat_sky==NULL) {
		e_error("in sky flat production: aborting");
		image_del(flat_dome);
		return -1 ;
	}
	if (bb->sky_save) {
		e_comment(1, "saving sky flat  [flat_sky.fits]");
		image_save_fits(flat_sky, "flat_sky.fits", BPP_DEFAULT);
	}

	part++ ;
	e_comment(0,"---> Part %d in %d: Dome/Sky combination",part,ALGPARTS);
	flat_combined = wfiff_combine(bb, flat_dome, flat_sky);
	image_del(flat_dome);
	image_del(flat_sky);

	if (flat_combined==NULL) {
		e_error("combining dome and sky to make flat");
		return -1 ;
	}
	wfiff_save(bb, flat_combined);
	image_del(flat_combined);
	e_comment(0, "--> STOP  WFI Flat-field engine");
	return 0 ;
}
#undef ALGPARTS


/*---------------------------------------------------------------------------
									Main	
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int             c ;
	int				display_params ;
	wfiff_bb	*	bb ;

	display_params = 0 ;
	bb = wfiff_bb_new();
 
    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 	0, 0, OPT_LICENSE},
            {"help",    	0, 0, OPT_HELP},
            {"version", 	0, 0, OPT_VERSION},

			{"xtnum",	  	1, 0, OPT_XTNUM},

			{"bias",	  	1, 0, OPT_BIASNAME},

			{"dome",	  	1, 0, OPT_DOMENAME},
			{"domethr",	  	1, 0, OPT_DOMETHRESH},
			{"domemin",	  	1, 0, OPT_DOMEMINFRAME},
			{"domesave",  	0, 0, OPT_DOMESAVE},

			{"sky",	  		1, 0, OPT_SKYNAME},
			{"skythr",	  	1, 0, OPT_SKYTHRESH},
			{"skymin",	  	1, 0, OPT_SKYMINFRAME},
			{"skysave",  	0, 0, OPT_SKYSAVE},

			{"prescan",	  	1, 0, OPT_PRESCAN},
			{"overscan",  	1, 0, OPT_OVERSCAN},
			{"scanrej",  	1, 0, OPT_SCANREJ},
			{"trim",  		1, 0, OPT_TRIMMING},

			{"readout",	  	1, 0, OPT_READOUTNOISE},
			{"gain",	  	1, 0, OPT_PIXELGAIN},

			{"pixrej",	  	1, 0, OPT_PIXELREJ},
			{"medianreg", 	1, 0, OPT_MEDIANREG},
			{"ksize",	  	1, 0, OPT_KERNELSIZE},

            {0, 0, 0, 0}
 
        } ;
        c = getopt_long(argc,
                        argv,
                        "Ldhx:",
                        long_options,
                        &option_index) ;
        if (c==-1)
            break ;
 
        switch(c)
        {
 
        /* Standard option: display license undocumented option */
            case OPT_LICENSE:
            case 'L':
            eclipse_display_license() ;
            return 0 ;
 
        /* Standard option : help */
            case OPT_HELP:
            case 'h':
            usage(argv[0]) ;
            break ;
 
        /* Standard option: version */
            case OPT_VERSION:
            print_eclipse_version() ;
			printf("recipe version: %s\n", recipe_version);
            return 0 ;

		/* Local option: extension number to consider */
			case OPT_XTNUM:
			case 'x':
			bb->xtnum = atoi(optarg);
			break ;

		/* Local option: dump default values and exit */
			case 'd':
			display_params = 1 ;
			break ;

			case OPT_BIASNAME:
			bb->name_bias = strdup(optarg);
			break ;

			case OPT_DOMENAME:
			bb->name_dome = strdup(optarg);
			break ;

			case OPT_DOMETHRESH:
			sscanf(optarg, "%lg %lg",
					&(bb->dome_framethr_lo),
					&(bb->dome_framethr_hi));
			break;

			case OPT_DOMEMINFRAME:
			sscanf(optarg, "%d", &(bb->dome_minframe));
			break ;

			case OPT_DOMESAVE:
			bb->dome_save = 1 ;
			break;

			case OPT_SKYNAME:
			bb->name_sky = strdup(optarg);
			break ;

			case OPT_SKYTHRESH:
			sscanf(optarg, "%lg %lg",
					&(bb->sky_framethr_lo),
					&(bb->sky_framethr_hi));
			break ;

			case OPT_SKYMINFRAME:
			sscanf(optarg, "%d", &(bb->sky_minframe));
			break ;

			case OPT_SKYSAVE:
			bb->sky_save = 1 ;
			break;

			case OPT_PRESCAN:
			sscanf(optarg, "%d %d",
					&(bb->prescan_x[0]),
					&(bb->prescan_x[1]));
			break ;

			case OPT_OVERSCAN:
			sscanf(optarg, "%d %d",
					&(bb->overscan_x[0]),
					&(bb->overscan_x[1]));
			break ;

			case OPT_SCANREJ:
			sscanf(optarg, "%d %d",
					&(bb->scanrej[0]),
					&(bb->scanrej[1]));
			break ;

			case OPT_TRIMMING:
			sscanf(optarg, "%d %d %d %d",
					&(bb->trimreg[0]),
					&(bb->trimreg[1]),
					&(bb->trimreg[2]),
					&(bb->trimreg[3]));
			break ;

			case OPT_READOUTNOISE:
			sscanf(optarg, "%lg", &(bb->readout_noise));
			break ;

			case OPT_PIXELGAIN:
			sscanf(optarg, "%lg", &(bb->pixel_gain));
			break ;

			case OPT_PIXELREJ:
			sscanf(optarg, "%lg %lg",
					&(bb->pixrej_lo),
					&(bb->pixrej_hi));
			break ;

			case OPT_MEDIANREG:
			sscanf(optarg, "%d %d %d %d",
					&(bb->median_reg[0]),
					&(bb->median_reg[1]),
					&(bb->median_reg[2]),
					&(bb->median_reg[3]));
			bb->median_surf   = (bb->median_reg[1]-bb->median_reg[0]+1) *
								(bb->median_reg[3]-bb->median_reg[2]+1);

			break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }
 
    /* Initialize eclipse environment */
    eclipse_init() ;

	/* Get output file name */
	if ((argc-optind)<1) {
		bb->name_o = strdup(WFIFF_DEFAULTNAME);
	} else {
		bb->name_o = strdup(argv[optind]);
	}

	if (display_params) {
		wfiff_bb_dump(bb, stdout);
	}

	/* Check that input names are there */
	if (bb->name_dome==NULL || bb->name_sky==NULL) {
		e_error("no name provided for dome or sky");
		wfiff_bb_del(bb);
		return -1 ;
	}

	/* Startup WFI flat-field engine */
	wfiff_engine(bb);
	wfiff_bb_del(bb);

    if (debug_active()) xmemory_status() ;
    return 0 ;
} 



/*
 * This function only gives the usage for the program
 */
 
static void
usage(char *pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] <WFI extension> [outname]\n", pname) ;
	printf(
"options are:\n"
"\t-x or --xtnum <n>  Xtension to consider (0=all)\n"
"\t--bias <name>      Name of a bias to subtract (optional)\n"
"\n"
"\t--dome <name>      Name of the dome input list\n"
"\t--domethr 'lo hi'  Low and high threshold for dome frames\n"
"\t--domemin <n>      Min number of frames for dome flat\n"
"\t--domesave         Save dome flat to flat_dome.fits in cwd\n"
"\n"
"\t--sky <name>       Name of sky input list\n"
"\t--skythr 'lo hi;   Low and high threshold for sky frames\n"
"\t--skymin <n>       Min number of frames for sky flat\n"
"\t--skysave          Save sky flat to flat_sky.fits in cwd\n"
"\n"
"\t--prescan 'xmin xmax'              Prescan region definition\n"
"\t--overscan 'xmin xmax'             Overscan region definition\n"
"\t--scanrej 'min max'                Scan rejection definition\n"
"\t--trim 'xmin xmax ymin ymax'       Trimming region definition\n"
"\n"
"\t--readout <f>      Readout noise value\n"
"\t--gain <f>         Pixel gain\n"
"\t--pixrej 'lo hi'   Pixel rejection sigma threshold\n"
"\t--medianreg 'xmin xmax ymin ymax'  Median region definition\n"
"\n"
"\t--ksize <n>        Smoothing kernel size\n"
"\n"
"\t-d                 Print out configuration parameters and run\n"
"\n\n");
    exit(1) ;
} 

