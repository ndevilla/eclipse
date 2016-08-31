/*----------------------------------------------------------------------------*/
/**
   @file    sp_flat.c
   @author  Y. Jung
   @date    May 2002
   @version	$Revision: 1.11 $
   @brief   ISAAC spectroscopic flat field
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: sp_flat.c,v 1.11 2005/03/10 13:12:58 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/10 13:12:58 $
	$Revision: 1.11 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define MEDIAN_XSIZE        200
#define MEDIAN_YSIZE        200

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int sp_flat_engine(char *, char *, int, int, int, int, double, double, 
        int, int, int, int, int) ;
static int sp_flat_compute(cube_t *, qfits_header *, char *, int, int, 
		int, int, double, double, int, int, int, int, int, int, framelist *,
        double *, double *) ;
static int sp_flat_write_paffile(char *, char *, double, double, int) ;
static image_t * divide_by_fit(image_t *, int, int, int, int, int, int) ;

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int isaac_sp_flat_main(void * dict)
{
	dictionary	*	d ;
		
	char		*	tmp_string ;
	
	int				llx,
					lly,
					urx,
					ury ;
	double			low_thresh,
					hi_thresh ;
	int				y_fit_order ;
	int				fit_size ;
	int				offset ;
	int				output_images_flag ;
	int				output_poly_images ;
	
	char			argname[10] ;
	char    	*	name_i ;
    char    	*	name_o ;
    int     		nfiles ;

	int				items ;
	int				errors ;
	int				i ;
	 
	d = (dictionary*)dict ;
   	/* Get options */
	tmp_string = dictionary_get(d, "arg.rectangle", NULL) ;
	if (tmp_string == NULL) {
		llx = lly = 256 ;
		urx = ury = 768 ;
	} else {
		items = sscanf(tmp_string, "%d %d %d %d", &llx, &lly, &urx, &ury) ;
		if (items != 4) {
			llx = lly = 256 ;
			urx = ury = 768 ;
		}
	}
	low_thresh         = dictionary_getdouble(d, "arg.low", 0.01) ;
	hi_thresh          = dictionary_getdouble(d, "arg.high", 3.0) ; 
	y_fit_order        = dictionary_getint(d, "arg.fit_order", 3) ; 
	fit_size           = dictionary_getint(d, "arg.fit_size", 200) ;
	offset             = dictionary_getint(d, "arg.offset", 40) ;
	output_images_flag = dictionary_getint(d, "arg.save", 0) ;
	output_poly_images = dictionary_getint(d, "arg.save_poly", 0) ;

	/* Get input/output file names */
    nfiles = dictionary_getint(d, "arg.n", -1) ;
    if (nfiles<0) {
        e_error("missing input file name(s): aborting");
        return -1 ;
    }
    /* Loop on input file names */
	errors = 0 ;
	for (i=1 ; i<nfiles ; i++) {
		sprintf(argname, "arg.%d", i);
		name_i = dictionary_get(d, argname, NULL) ;
		name_o = dictionary_get(d, "arg.output", NULL) ;
		if (name_o == NULL) name_o = strdup(get_rootname(get_basename(name_i)));
		else name_o = strdup(get_rootname(name_o)) ;
		
        /* Once options have been cleared out, call the computing function. */
    	errors += sp_flat_engine(name_i,
                            		name_o,
									llx,
                            		lly,
                            		urx,
                            		ury,
                            		low_thresh,
                            		hi_thresh,
                            		y_fit_order,
                            		fit_size,
                            		offset,
                            		output_images_flag,
                            		output_poly_images) ;
		free(name_o) ;
	}
	return errors ;
}


/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Create a MASTER flatfield with pairs of frames (one MASTER/setting)
  @param	in_ascii			ascii file with pairs of on-off images
  @param	outrootname			output image root name
  @param	llx					vig. lower left X coordinate
  @param	lly					lower left Y coordinate
  @param	urx					upper right X coordinate
  @param	ury					upper right Y coordinate
  @param	low_thresh			low threshold for bad pixel rejection
  @param	hi_thresh			high threshold for bad pixel rejection
  @param	y_fit_order			order in y (x_order=0) of the 2d polynomial fit
  @param	fit_size			fit size
  @param	offset				offset
  @param	output_images_flag	flag to output or not all the created flats
  @param	output_poly_image	flag to output polynomial images
  @return	-1 in case of error, 0 otherwise
  Rectangle coordinates in FITS convention (ll is (1,1))
 */
/*----------------------------------------------------------------------------*/
static int sp_flat_engine(
        char        *   in_ascii,
        char        *   outrootname,
        int             llx,
        int             lly,
        int             urx,
        int             ury,
		double			low_thresh,
		double			hi_thresh,
		int				y_fit_order,
		int				fit_size,
		int				offset,
		int				output_images_flag,
		int				output_poly_image)
{
	framelist 	*	lnames ;
	image_t		*	image ;
	cube_t		*	cube ;
	qfits_header*   hdr ;
	int				curr_ind ;
	int				nsettings ;
	char			outname[FILENAMESZ] ;
	double			median ;
	double			stdev ;
	int				nimages ;
	int				size_x,
					size_y ;
	int				i, 
					j ;
	
	/* Read the in ascii file  */
	if ((lnames=framelist_load(in_ascii)) == NULL) {
		e_error("cannot read the ascii input file") ;
		return -1 ;
	}
	if ((lnames->n % 2)) {
		e_error("An even nb of frames expected in input: %d", lnames->n) ;
		framelist_del(lnames) ;
		return -1 ;
	}
	
	/* Load the first image */
	if ((image=image_load(lnames->name[0])) == NULL) {
		e_error("cannot load the first image") ;
		framelist_del(lnames) ;
		return -1 ;
	}
	size_x = image->lx ;
	size_y = image->ly ;
	image_del(image) ;
	
	/* Number of different settings */
	if ((nsettings=framelist_labelize(lnames, compare_settings)) == -1) {
		e_error("in getting the number of different settings") ;
		framelist_del(lnames) ;
		return -1 ;
	}
	e_comment(1, "there are %d different setting(s)", nsettings) ;
	
	/* Compute the flat field for each setting */
	for (i=0 ; i<nsettings ; i++) {
		e_comment(0, "reduction for setting no. %d", i+1) ;	
	
		/* Create the cube containing the frames of this setting */
		nimages = 0 ;
		for (j=0 ; j<lnames->n ; j++) if (lnames->label[j] == i) nimages ++ ;
		if (nimages%2) {
			e_error("the number of images for a setting should be even") ;
		    continue ;
        }
        cube = cube_new(size_x, size_y, nimages) ;
        curr_ind = 0 ;
        for (j=0 ; j<lnames->n ; j++) {
            if (lnames->label[j] == i) {
                cube->plane[curr_ind] = image_load(lnames->name[j]) ;
                curr_ind ++ ;
            }
        }

        /* Read the FITS header of the first image with the current setting */
        j = 0 ;
        while (lnames->label[j] != i) j++ ;
        if ((hdr = qfits_header_read(lnames->name[j])) == NULL) {
            e_error("cannot read header file") ;
            cube_del(cube) ;
            framelist_del(lnames) ;
            return -1 ;
        }
    
        /* Output name  */
        sprintf(outname, "%s_%d.fits", outrootname, i+1) ;

        /* Flat field */
        if (sp_flat_compute(cube, 
                            hdr, 
                            outname,
                            llx,
                            lly,
                            urx,
                            ury,
                            low_thresh,
                            hi_thresh,
                            y_fit_order,
                            fit_size,
                            offset,
                            output_images_flag,
                            i+1,
                            output_poly_image,
                            lnames,
                            &median,
                            &stdev) == -1) {
            e_warning("cannot create master flatfield: [%s]", outname) ;
        } else {
            /* Write out the PAF file */
            if ((sp_flat_write_paffile(outname, lnames->name[j], median,
                            stdev, nimages))==-1) {
                e_warning("cannot write the output PAF file: [%s.paf]", 
                        get_rootname(outname)) ;
            } else {
                e_comment(1, "file [%s.paf] produced", get_rootname(outname)) ;
            }
            e_comment(1, "file [%s] produced", outname) ;
        }
    
        /* Free	 */
        qfits_header_destroy(hdr);
        cube_del(cube) ;
	}

	/* Free and return   */
	framelist_del(lnames) ;
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	create one MASTER flat with the input pairs	
  @param	in		input cube (with an even number of frames!)
  @param	hdr		fits header
  @param	outname	output name
  @param    llx     vig. lower left X coordinate
  @param    lly     lower left Y coordinate
  @param    urx     upper right X coordinate
  @param    ury     upper right Y coordinate
  @param    low     low threshold for bad pixel rejection
  @param    high    high threshold for bad pixel rejection
  @param    order   order (in y) (x_order=0) of the 2d polynomial fit
  @param    fit_size	fit size
  @param    offset  offset
  @param    flag	flag to output or not all the created master flats
  @param	setting_nb	setting identifier
  @param    output_poly_image   flag to output polynomial images
  @param	list of input file names
  @param    the median (returned value)
  @param    the stdev (returned value)
  @return  	0 if ok, -1 otherwise
  rectangle coordinates in FITS convention (ll is (1,1)).
 */
/*----------------------------------------------------------------------------*/
static int sp_flat_compute(
		cube_t		*	in,
		qfits_header*   hdr,
		char		*	outname,
		int				llx,
		int				lly,
		int				urx,
		int				ury,
		double			low,
		double			high,
		int				order,
		int				fit_size,
		int				offset,
		int				flag,
		int				setting_nb,
		int				output_poly_image,
		framelist	*	lnames,
        double      *   ret_median,
        double      *   ret_stdev)
{
	image_t	    *	diff ;
	image_t	    *	normalized ;	
	image_t	    *	outimage ;
	image_t	    *	fitted ;
	image_t	    *	collapsed ;
	image_t	    *	blank ;
	image_t	    *	extracted ;
	image_stats	*	statistics ;
	int				right_lim,
					left_lim ;
	cube_t		*	results ;
	double		*	median ;
	double			sum, sqsum ;
	char			name[FILENAMESZ] ;
	int				zone[4] ;
    char        *   val ;
    procat          pro_flat ;
    instrument_t    ins ;

	int				i ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    
	/* Allocate the results cube (contains the master flats) */
	results = cube_new(in->lx, in->ly, in->np/2) ;

	/* Allocate the median array */
	median = malloc((in->np/2)*sizeof(double)) ;
		
	/* For each pair in the input cube */
	for (i=0 ; i<in->np/2 ; i++) {
		e_comment(1, "compute flatfield for pair no. %d", i+1) ;	
		/* Compute the difference */
    	if ((diff=image_sub(in->plane[2*i], in->plane[2*i+1])) == NULL) {
    	    e_error("unable to subtract images") ;
    	    cube_del(results) ;
			free(median) ;
			return -1 ;
    	}

		/* Compute the median of the central part */
		if ((median[i]=(double)image_getmedian_vig(diff,
							((diff->lx)-MEDIAN_XSIZE)/2.0,
							((diff->ly)-MEDIAN_YSIZE)/2.0,
							((diff->lx)+MEDIAN_XSIZE)/2.0,
							((diff->ly)+MEDIAN_YSIZE)/2.0)) == 0.00) {
			image_del(diff) ;
			free(median) ;
			cube_del(results) ;
			e_error("cannot compute the median") ;
			return -1 ;
		}
		
    	/* Divide by the mean in the vig in the difference image */
		zone[0] = llx ;
		zone[1] = urx ;
		zone[2] = lly ;
		zone[3] = ury ;
    	if ((statistics = image_getstats_opts(diff, NULL, NULL, zone))==NULL) {
        	e_error("failed while getting statistics of the image") ;
			image_del(diff) ;
			free(median) ;
			cube_del(results) ;
			return -1 ;
    	}
    	if ((normalized = image_cst_op(diff, statistics->avg_pix, '/'))==NULL) {
    	    e_error("cannot divide by the mean") ;
    	    image_del(diff) ;
			free(median) ;
			cube_del(results) ;
			free(statistics) ;
    	    return -1 ;
    	}
    	free(statistics) ;
    	image_del(diff) ;

    	/* Replace by 0 the pixels whose value is <low and >high */
    	if ((outimage = image_threshold(normalized, low, high, 0, 0))==NULL) {
    	    e_error("cannot threshold the image") ;
    	    image_del(normalized) ;
			free(median) ;
			cube_del(results) ;
			return -1 ;
    	}
    	image_del(normalized) ;
			
		results->plane[i] = outimage ;
	}

	/* Compute the mean/stdev of the medians */
    sum = sqsum = 0.0 ;
	for (i=0 ; i<in->np/2 ; i++) {
        sum += median[i] ;
        sqsum += median[i] * median[i] ;
    }
	free(median) ;
    *ret_median = sum / (in->np/2) ;
    if (in->np/2 > 2) {
        *ret_stdev = (sqsum-((sum*sum)/(double)(in->np/2))) / 
            ((double)(in->np/2) - 1) ;
        *ret_stdev = *ret_stdev> 0 ? sqrt(*ret_stdev) : 0 ;
    } else *ret_stdev = -1.0 ;
        
	if (results->np > 1) {
		if (flag) {
			/* Output all the created master flats */
			for (i=0 ; i<in->np/2 ; i++) {
				if ((fitted=divide_by_fit(results->plane[i], 
										order,
										fit_size,
										offset,
										setting_nb,
										i+1,
										output_poly_image)) == NULL) {
					e_warning("cannot divide by fit") ;
				} else {
					sprintf(name, "tmp_%d_%s", i+1, get_basename(outname)) ;
					image_save_fits(fitted, name, BPP_DEFAULT) ;
					image_del(fitted) ;
				}
			}
		}
	}
				
	if (results->np > 1) {
		/* Average the results cube  */
		if ((outimage=cube_avg_linear(results)) == NULL) {
			e_error("cannot average the results cube") ;
			cube_del(results) ;
			return -1 ;
		}
	} else {
		if ((outimage=image_copy(results->plane[0])) == NULL) {
			e_error("cannot copy image") ;
			cube_del(results) ;
			return -1 ;
		}
	}
	cube_del(results) ;

	/* Divide the output image by the fit	 */
	if ((fitted=divide_by_fit(outimage, 
								order,	
								fit_size,
								offset,
								setting_nb,
								0,
								output_poly_image)) == NULL) {
		e_warning("cannot divide by fit") ;
		fitted = image_copy(outimage) ;
	} 
	image_del(outimage) ;

	/* Erase neighbour orders */
	if ((collapsed=image_collapse(fitted, 0)) == NULL) {
		e_warning("cannot collapse the fitted image") ;
	} else {
		right_lim = left_lim = collapsed->lx / 2 ;
		while ((collapsed->data[left_lim] > (pixelvalue)1) && (left_lim > 0)) 
            left_lim-- ;
		while ((collapsed->data[right_lim] > (pixelvalue)1) &&
			(right_lim < collapsed->lx - 1)) right_lim++ ;

		image_del(collapsed) ;
		
		/* Create BLANK image	 */
		blank = image_new(fitted->lx, fitted->ly) ;
		
		/* Extract the interessant part */
		if ((extracted = image_getvig(fitted,
                        left_lim+1,
                        1,
                        right_lim+1,
                        fitted->ly)) == NULL) {
			e_warning("cannot extract slit from image") ;
			image_del(blank) ;
		} else {
			image_del(fitted) ;
	
			/* Paste the extracted part in the blank image */
			fitted = image_paste(blank, extracted, left_lim+1, 1) ;
			image_del(blank) ;
			image_del(extracted) ;
		}
	}

	/* Prepare it for image output */
    if (isaac_header_for_image(hdr) == -1) {
        e_error("in writing the output fits file") ;
        image_del(fitted) ;
        return -1 ;
    }
	
    /* The written PRO CATG keyword depends on the arm */
    /* Default */
    pro_flat = procat_invalid ;
    if ((val = pfits_get(ins, lnames->name[0], "arm")) != NULL) {
        if (toupper(val[0])=='S') pro_flat = procat_spec_sw_flat ;
        else if (toupper(val[0])=='L') pro_flat = procat_spec_lw_flat ;
    }

    /* Write the PRO keywords in the FITS header */
    if (isaac_pro_fits(hdr,
                        outname,
                        "REDUCED",
                        NULL,
                        pro_flat,
                        "OK",
                        "spec_tec_specflats",
                        in->np,
						lnames,
						NULL) == -1) {
        e_error("unable to write the PRO keyword in the fits header") ;
        image_del(fitted) ;
        return -1 ;
    }
	
	/* Write HISTORY keywords in the header */
	isaac_add_files_history(hdr, lnames) ;
	
    /* Output the master flatfield */
    image_save_fits_hdrdump(fitted, outname, hdr, BPP_DEFAULT) ;

    /* Free and return   */
    image_del(fitted) ;
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Fit the input image and divide it by the fit
  @param    in          input image
  @param    order       order of the fit
  @param    xsize       central X size to be used for the fit
  @param    offset      offset
  @param    setting_nb  setting id
  @param    pair_nb     pair id
  @param    output_poly_image   flag to output the polynomial images
  @return   corrected image
 */
/*----------------------------------------------------------------------------*/
static image_t * divide_by_fit(
        image_t    *   in,
        int             order,
        int             xsize,
        int             offset,
        int             setting_nb,
        int             pair_nb,
        int             output_poly_image)
{
    image_t    *   out ;
    int             xstart,
                    xend,
                    ystart,
                    yend ;
    image_t    *   extracted ;
    image_t    *   collapsed ;
    image_t    *   extracted2 ;
    image_t    *   fit_image ;

    int             nb_samples ;
    double3     *   to_fit ;
    double      *   coeffs ;
    char            poly_string[FILENAMESZ] ;
    char            poly_string2[FILENAMESZ] ;
    char            poly_name[FILENAMESZ] ;

    int             i ;

    /* Determine the zone to extract */
    xstart = (int)((in->lx - xsize)/2) + 1 ;
    xend = xstart + xsize - 1 ;
    if ((xstart<1) || (xend>in->lx)) {
        e_error("bad X size specified") ;
        return NULL ;
    }

    /* Extract the central zone */
    if ((extracted=image_getvig(in,
                                    xstart,
                                    1,
                                    xend,
                                    in->ly)) == NULL) {
        e_error("cannot extract image") ;
        return NULL ;
    }

    /* Collapse the extracted zone */
    if ((collapsed=image_collapse(extracted, 1)) == NULL) {
        e_error("cannot collapse the image") ;
        image_del(extracted) ;
        return NULL ;
    }
    image_del(extracted) ;

    /* Find the 'valid' zone in the 1D image */
    ystart = 1 ;
    while ((collapsed->data[ystart-1] == (pixelvalue)0) &&
            (ystart < in->lx)) ystart++ ;
    ystart += offset ;

    yend = collapsed->ly ;
    while ((collapsed->data[yend-1] == (pixelvalue)0) &&
            (yend > 1)) yend-- ;
    yend -= offset ;

    if (ystart > yend) {
        e_error("invalid coordinates of the zone to extract") ;
        image_del(collapsed) ;
        return NULL ;
    }

    /* Extract the 1D signal to fit */
    if ((extracted2=image_getvig(collapsed,
                                            1,
                                            ystart,
                                            1,
                                            yend)) == NULL) {
        e_error("cannot extract 1D image") ;
        image_del(collapsed) ;
        return NULL ;
    }
    image_del(collapsed) ;

    nb_samples = extracted2->ly ;

    /* Fill to_fit */
    to_fit = double3_new(nb_samples) ;
    for (i=0 ; i<nb_samples ; i++) {
        to_fit->x[i] = (double)(ystart + i) ;
        to_fit->y[i] = (double)(extracted2->data[i] / xsize) ;
    }
    image_del(extracted2) ;

    /* Find polynomial coefficients */
    coeffs=fit_1d_poly(order-1, to_fit, NULL);
    double3_del(to_fit);
    if (coeffs==NULL) {
        e_error("cannot fit the 1D signal") ;
        return NULL ;
    }

    /* Build the fit image   */
    sprintf(poly_string, "(0,0)") ;
    for (i=1 ; i<order ; i++) {
        sprintf(poly_string2, "%s (0,%d)", poly_string, i) ;
        sprintf(poly_string, poly_string2) ;
    }
    if ((fit_image=image_gen_polynomial(in->lx,
                                in->ly,
                                coeffs,
                                order,
                                order-1,
                                poly_string)) == NULL) {
        e_error("cannot generate polynomial image") ;
        free(coeffs) ;
        return NULL ;
    }
    free(coeffs) ;

    if (output_poly_image) {
        if (pair_nb == 0) {
            sprintf(poly_name, "poly_set%d.fits", setting_nb) ;
        } else {
            sprintf(poly_name, "poly_set%d_pair%d.fits", setting_nb, pair_nb) ;
        }
        image_save_fits(fit_image, poly_name, BPP_DEFAULT) ;
    }


    /* Divide the input image by the polynomial image */
    if ((out=image_div(in, fit_image)) == NULL) {
        e_error("cannot divide the images") ;
        image_del(fit_image) ;
        return NULL ;
    }

    /* Free and return */
    image_del(fit_image) ;
    return out ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Write the PAF file for flat
  @param	outname	output file name
  @param	inimage_name	one input image name (for header reference)
  @param	median	median on the MASTER center part	
  @param	stdev	stdev of the median values
  @param    nb_im   number of input images for this product
  @return	-1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int sp_flat_write_paffile(
		char	*	outname,
		char	*	inimage_name,
		double		median,
        double      stdev,
        int         nb_im)
{
	FILE	    *	paf ;
	char            pafname[FILENAMESZ] ;
	char        *   mjd_obs ;
	char	    *	strvar ;
    instrument_t    ins ;
    procat          pro_flat_qc ;
	
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    
	sprintf(pafname, "%s.paf", get_rootname(outname)) ;
	paf = qfits_paf_print_header( pafname,
							"ISAAC/flatfield",
							"Flat field recipe results",
                            get_login_name(),
                            get_datetime_iso8601()) ;
	if (paf == NULL) {
		e_warning("cannot output PAF file") ;
	} else {
		fprintf(paf, "\n");
		/* ARCFILE */
		strvar = pfits_get(ins, inimage_name, "arcfile") ;
		if (strvar != NULL) {
			fprintf(paf, "ARCFILE   \"%s\"  \n", strvar) ;
		}
		/* MJD-OBS */
		mjd_obs = pfits_get(ins, inimage_name, "mjdobs") ;
		if (mjd_obs!=NULL) {
            fprintf(paf, "MJD-OBS  %s; # Obs start\n\n", mjd_obs);
        } else {
            fprintf(paf, "MJD-OBS  0.0; # Obs start unknown\n\n");
        }
		/* INSTRUME keyword  */
        strvar = pfits_get(ins, inimage_name, "instrument") ;
        if (strvar != NULL) {
            fprintf(paf, "INSTRUME \"%s\" ;\n", strvar) ;
        }
        /* TPL.ID  */
        strvar = pfits_get(ins, inimage_name, "templateid") ;
        if (strvar != NULL) {
            fprintf(paf, "TPL.ID  \"%s\" ;\n", strvar) ;
        }
        /* TPL.NEXP */
        strvar = pfits_get(ins, inimage_name, "numbexp") ;
        if (strvar != NULL) {
            fprintf(paf, "TPL.NEXP  %s ;\n", strvar) ;
        }
        /* DPR.CATG */
        strvar = pfits_get(ins, inimage_name, "dpr_catg") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.CATG  \"%s\" ;\n", strvar) ;
        }
        /* DPR.TYPE */
        strvar = pfits_get(ins, inimage_name, "dpr_type") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.TYPE  \"%s\" ;\n", strvar) ;
        }
        /* DPR.TECH */
        strvar = pfits_get(ins, inimage_name, "dpr_tech") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.TECH  \"%s\" ;\n", strvar) ;
        }
		/* Add PRO.CATG */
        /* The PRO CATG key depends on the arm used */
        pro_flat_qc = procat_invalid ;
        if ((strvar = pfits_get(ins, inimage_name, "arm")) != NULL) {
            if (toupper(strvar[0])=='S') pro_flat_qc=procat_spec_sw_flat_qc ;
            else if (toupper(strvar[0])=='L') 
                pro_flat_qc=procat_spec_lw_flat_qc ;
        }
        fprintf(paf, "PRO.CATG \"%s\" ;# Product category\n",
                pfits_getprokey(ins, pro_flat_qc)) ;
        /* Add the date */
        fprintf(paf, "DATE-OBS \"%s\" ;# Date\n",
                pfits_get(ins, inimage_name, "date_obs")) ;
        /* INS.GRAT.NAME */
        strvar = pfits_get(ins, inimage_name, "resolution") ;
        if (strvar != NULL) {
            fprintf(paf, "INS.GRAT.NAME  \"%s\" ;\n", strvar) ;
        }
		/* INS.GRAT.WLEN */
        fprintf(paf, "INS.GRAT.WLEN  %g ;\n", 
                isaac_get_central_wavelength(inimage_name)) ;
		/* INS.OPTI1.ID */
		strvar = pfits_get(ins, inimage_name, "optical_id") ;
		if (strvar != NULL) {
			fprintf(paf, "INS.OPTI1.ID  \"%s\" ;\n", strvar) ;
		}
		/* ESO.DET.DIT */
		strvar = pfits_get(ins, inimage_name, "dit") ;
		if (strvar != NULL) {
			fprintf(paf, "ESO.DET.DIT  \"%s\" ;\n", strvar) ;
		}
		/* ESO.INS.LAMP3.SET */
		strvar = pfits_get(ins, inimage_name, "lamp3_intensity") ;
		if (strvar != NULL) {
			fprintf(paf, "ESO.INS.LAMP3.SET  %s ;\n", strvar) ;
		}
        /* PRO DATANCOM */
        fprintf(paf, "PRO.DATANCOM           \"%d\" ;\n", nb_im) ;
        
		/* QC.SPECFLAT.NCOUNTS	 */
		fprintf(paf, "QC.SPECFLAT.NCOUNTS  %g ;\n", median) ;
		/* QC.SPECFLAT.STDEV	 */
		fprintf(paf, "QC.SPECFLAT.STDEV    %g ;\n", stdev) ;
		/* QC.FILTER.OBS */
		strvar = pfits_get(ins, inimage_name, "filter") ;	
		if (strvar != NULL) {
			fprintf(paf, "QC.FILTER.OBS        \"%s\" ;\n", strvar) ;
		}
		fclose(paf) ;
	}
	return 0 ;
}


