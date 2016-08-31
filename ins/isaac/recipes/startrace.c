/*----------------------------------------------------------------------------*/
/**
   @file    startrace.c
   @author  Y. Jung
   @date    May 2002
   @version	$Revision: 1.14 $
   @brief   ISAAC startrace recipe
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: startrace.c,v 1.14 2004/02/25 10:58:19 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/25 10:58:19 $
	$Revision: 1.14 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define MODE_INVALID        -1
#define MODE_HAWAI          0
#define MODE_ALLADIN        1

#define Z_LR_LEFT_REJ       300
#define Z_LR_RIGHT_REJ      325
#define SZ_LR_LEFT_REJ      300
#define SZ_LR_RIGHT_REJ     325
#define J_LR_LEFT_REJ       200
#define J_LR_RIGHT_REJ      200
#define SH_LR_LEFT_REJ      150
#define SH_LR_RIGHT_REJ     175
#define SK_LR_LEFT_REJ      150
#define SK_LR_RIGHT_REJ     175
#define MR_LEFT_REJ         30
#define MR_RIGHT_REJ        30

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int startrace_compute(char *, char *, int, int, int, int, int, 
		int, int, char *, char *, int) ;
static double ** sttr_find_2d_poly(double **, int, int, int, int, double *) ;
static double * sttr_shape_analysis(image_t *, double, int, int, int, int, 
        double *, int) ;
static double * sttr_extract_spec(image_t *, double, int, int, int, double *, 
		int) ;
static double ** sttr_compute_corres(double **, int, int) ;
static int sttr_write_tables(char *, int, int, char **, procat, char *,
		double **, char *) ;
static int sttr_write_poly2d(char *, char *, int, double **, char *, procat) ;
static cube_t ** sttr_read_input(char *, int) ;
static int sttr_write_paffile(char *, char *, double, double, double, double, 
		double, double, double **, double **, double, double, procat) ;
static int sttr_correct_distortion(cube_t **, char *, int *) ;

/*-----------------------------------------------------------------------------
                                Main 
 -----------------------------------------------------------------------------*/
int isaac_startrace_main(void * dict)
{
	dictionary 	* 	d ;
		
	int				poly_degree ;
	int				spec_width ;
	int				sky_dist ;
	int				sky_width ;
	int				reject_left ;
	int				reject_right ;
	int				display ;
	char		*	disto_lr ;
	char		*	disto_mr ;
	int				out_corrected ;
    
	char			argname[10] ;
	char    	*	name_i ;
    char    	*	name_o ;
    int     		nfiles ;
	
	int				errors ;
	int				i ;

	d = (dictionary*)dict ;
   	/* Get options */
	poly_degree      = dictionary_getint(d, "arg.degree", 3) ;
	spec_width       = dictionary_getint(d, "arg.width", 40) ;
	sky_dist         = dictionary_getint(d, "arg.sky_dist", 20) ;
	sky_width        = dictionary_getint(d, "arg.sky_width", 10) ;
	reject_left      = dictionary_getint(d, "arg.reject_l", -1) ;
	reject_right     = dictionary_getint(d, "arg.reject_r", -1) ;
	display          = dictionary_getint(d, "arg.display", 0) ;
	disto_lr         = dictionary_get(d, "arg.disto_lr", NULL) ;
	disto_mr         = dictionary_get(d, "arg.disto_mr", NULL) ;
	out_corrected    = dictionary_getint(d, "arg.out_corr", 0) ;
    
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
		
		/* Once command-line options have been cleared out, call the engine */
    	errors += startrace_compute(name_i, 
									name_o,
									poly_degree,
                                    spec_width,
                                    sky_dist,
                                    sky_width,
                                    reject_left,
                                    reject_right,
                                    display,
                                    disto_lr,
                                    disto_mr,
                                    out_corrected) ;
		free(name_o) ;
	}
	return errors ;
}


/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Various operations are executed on a batch of frames composed 
            by 3 series of aquisitions (in imaging, in LR and in MR).
            See man page and data reduction manual for more information.
  @param    inname  input ascii file name
  @param    outname output files base name
  @param    poly_degree polynomial degree
  @param    spec_width  spectrum width
  @param    sky_dist    sky distance to spectrum
  @param    sky_width   sky width
  @param    reject_l    rejected border on the left
  @param    reject_r    rejected border on the right
  @param    display     flag to display results
  @param    disto_lr    file for distortion correction in LR
  @param    disto_mr    file for distortion correction in MR
  @param    out_corrected   flag to output distortion corrected images
  @return   0 if ok -1 otherwise
  The spectra are supposed to be horizontal
 */
/*----------------------------------------------------------------------------*/
static int startrace_compute(
        char    *       inname,
        char    *       outname,
        int             poly_degree,
        int             spec_width,
        int             sky_dist,
        int             sky_width,
        int             reject_l,
        int             reject_r,
        int				display,
        char    *       disto_lr,
        char    *       disto_mr,
        int				out_corrected)
{
	cube_t			**	cubes ;
	int					corrected ;
	cube_t			**	diff_cubes ;
	int					sub_id ;
	image_t			*	tmp_image ;
	double			**	positions ;
	double3			*	position ;
	double3			*	pix_pos ;	
	char				message[FILENAMESZ] ;
	double				min_brightness ;
	double			**	corr_table ;
	double			**	extracted_table ;
	int					nbcol_extr ;
	framelist		*	flist ;
	double			*	wavecal_LR ;
	double			*	wavecal_MR ;
	double			**	shapes_table ;
	int					nbcol_shape ;
	char			*	sval ;
	int					reject_l_lr,
						reject_l_mr,
						reject_r_lr,
						reject_r_mr ;
	double			**	poly_2d_LR ;
	double			**	poly_2d_MR ;					
	char				first_LR_file[FILENAMESZ] ;
	char				first_MR_file[FILENAMESZ] ;
	char				name[FILENAMESZ] ;
	char			**	col_names ;
	double				corr_il1,
                        corr_il2,
                        corr_il3,
                        corr_im1,
                        corr_im2,
                        corr_im3 ;
    double          *   fit_qualities ;
    double              fit_quality_lr, fit_quality_mr ;
	char			*	firstname ;
	qfits_header    *   fh ;
    procat              pro_catg_val[7] ;
    int                 mode ;
    instrument_t        ins ;
	int					i, j, k ;
    
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
                       
	/* Get the first fits file name */
	if ((firstname = framelist_firstname(inname)) == NULL) {
		e_error(" cannot get reference FITS header from %s", inname) ;
		return -1 ;
	}
   
    /* Check the arm to know the mode to operate */
    if ((sval=pfits_get(ins, firstname, "arm")) != NULL) {
        if (toupper(sval[0])=='S') mode = MODE_HAWAI ;
        else if (toupper(sval[0])=='L') mode = MODE_ALLADIN ;
        else mode = MODE_INVALID ;
    } else {
        e_error("cannot recognize the used arm") ;
        return -1 ;
    }

    /* Verify the mode */
    if (mode == MODE_INVALID) {
        e_error("cannot recognize the used arm") ;
        return -1 ;
    }
    
    /* The PRO CATG keywords depend on the arm used */
    /* Default */
    for (i=0 ; i<7 ; i++) pro_catg_val[i] = procat_invalid ;
    if (mode == MODE_HAWAI) {
        pro_catg_val[0] = procat_spec_sw_sttr_correct ;
        pro_catg_val[1] = procat_spec_sw_sttr_pos ;
        pro_catg_val[2] = procat_spec_sw_sttr_corresp ;
        pro_catg_val[3] = procat_spec_sw_sttr_extract ;
        pro_catg_val[4] = procat_spec_sw_sttr_shape ;
        pro_catg_val[5] = procat_spec_sw_sttr_disto ;
        pro_catg_val[6] = procat_spec_sw_sttr_qc ;
    } else if (mode == MODE_ALLADIN) {
        pro_catg_val[0] = procat_spec_lw_sttr_correct ;
        pro_catg_val[1] = procat_spec_lw_sttr_pos ;
        pro_catg_val[2] = procat_spec_lw_sttr_corresp ;
        pro_catg_val[3] = procat_spec_lw_sttr_extract ;
        pro_catg_val[4] = procat_spec_lw_sttr_shape ;
        pro_catg_val[5] = procat_spec_lw_sttr_disto ;
        pro_catg_val[6] = procat_spec_lw_sttr_qc ;
    } else {
        e_error("Unknown mode") ;
        return -1 ;
    }
    
	/* Cubes classification */
	if ((cubes=sttr_read_input(inname, mode)) == NULL) {
		e_error("cannot read the input data") ;
		return -1 ;
	}

	/* Correct the distortion in spectro LR images */
	if (sttr_correct_distortion(&(cubes[1]), disto_lr, &corrected) == -1) {
		e_warning("cannot correct distortion for LR spectro images") ;
	} else if ((corrected == 1) && (out_corrected == 1)) {
		for (i=0 ; cubes[1]->np ; i++) {
			sprintf(name, "corrected_LR_%d.fits", i+1) ;
			/* Read the FITS header of the input file */
			fh = qfits_header_read(firstname) ;	
			isaac_header_for_image(fh) ;
			isaac_pro_fits(fh,
							name,
							"REDUCED",
							NULL,
							pro_catg_val[0],
							"OK",
							"spec_tec_startrace",
							cubes[0]->np,
							NULL,
							NULL) ;
			image_save_fits_hdrdump(cubes[1]->plane[i], name, fh, BPP_DEFAULT) ;
			qfits_header_destroy(fh) ;
		}
	}
	
	/* Correct the distortion in spectro MR images */
	if (sttr_correct_distortion(&(cubes[2]), disto_mr, &corrected) == -1) {
		e_warning("cannot correct distortion for MR spectro images") ;
	} else if ((corrected == 1) && (out_corrected == 1)) {
		for (i=0 ; cubes[2]->np ; i++) {
			sprintf(name, "corrected_MR_%d.fits", i+1) ;
			/* Read the FITS header of the input file */
			fh = qfits_header_read(firstname) ;	
			isaac_header_for_image(fh) ;
			isaac_pro_fits(fh,
							name,
							"REDUCED",
							NULL,
							pro_catg_val[0],
							"OK",
							"spec_tec_startrace",
							cubes[0]->np,
							NULL,
							NULL) ;
			image_save_fits_hdrdump(cubes[2]->plane[i], name, fh, BPP_DEFAULT) ;
			qfits_header_destroy(fh) ;
		}
	}

    if (mode == MODE_HAWAI) {
        /* In SW do the differences */
        /* Compute the differences between the successive planes */
        /* For each cube with n planes, compute 1-(n/2)->1, 2-(n/2)+1->2 ... */

        /* Allocate diff_cubes */
        diff_cubes = malloc(3*sizeof(cube_t*)) ;	
        
        /* Compute the differences */
        for (i=0 ; i<3 ; i++) {
            diff_cubes[i] = cube_new(cubes[i]->lx, cubes[i]->ly, cubes[i]->np) ;
            for (j=0 ; j<cubes[i]->np ; j++) {
                sprintf(message, "Subtract images batch %d", i+1) ;
                compute_status(message, j, cubes[i]->np, 1) ;
                sub_id=(int)(j+(int)((cubes[i]->np)/2))%cubes[i]->np ;
                if ((diff_cubes[i]->plane[j]=image_sub(cubes[i]->plane[j], 
                                            cubes[i]->plane[sub_id])) == NULL) {
                    e_error("cannot subtract images") ;
                    for (k=0 ; k<3 ; k++) cube_del(cubes[k]) ;
                    free(cubes) ;
                    for (k=0 ; k<=i ; k++) cube_del(diff_cubes[k]) ;
                    free(diff_cubes) ;
                    return -1 ;
                }
            }
        }			
        /* Free cubes */
        for (i=0 ; i<3 ; i++) cube_del(cubes[i]) ;
        free(cubes) ;
    } else if (mode == MODE_ALLADIN) {
        /* In LW diff_cubes = cubes */
        diff_cubes = cubes ;
    } else {
        e_error("Unrecognized mode - abort") ;
        return -1 ;
    }
       			
	/* Find positions of points & spectra	 */
	
	/* Allocate positions array */
	positions = malloc(3*sizeof(double*)) ;
	for (i=0 ; i<3 ; i++) 
		positions[i] = malloc(diff_cubes[i]->np*sizeof(double)) ;

	/* Find positions of points */
	
	for (j=0 ; j<diff_cubes[0]->np ; j++) {
		if ((pix_pos=detected_ks_brightest_stars(diff_cubes[0]->plane[j], 
                                           1, DETECTED_KAPPA)) == NULL) {
			e_warning("object not found in image %d", j+1) ;
			positions[0][j] = -1 ;
		} else {
			e_comment(1, "object found in image %d at position %g",
					j+1, pix_pos->y[0]) ;
			positions[0][j] = pix_pos->y[0] ;
			double3_del(pix_pos) ;
		}
	}	
	
	/* Find spectra positions	 */
	for (i=1 ; i<3 ; i++) {
		for (j=0 ; j<diff_cubes[i]->np ; j++) {
			/* Set to 0 the negative pixels */
			if ((tmp_image=image_threshold(diff_cubes[i]->plane[j],
										0,
										MAX_PIX_VALUE,
										0,
										0)) == NULL) {
				e_warning("cannot threshold the image") ;
				tmp_image = image_copy(diff_cubes[i]->plane[j]) ;
			} 
		
			/* Minimum brightness required for a spectrum to be detected */
			min_brightness = 20*image_getmean(tmp_image) ;
			
			/* Detection */
			if ((position=find_brightest_spectrum_1d(tmp_image,
											0,
											NO_SHADOW_SPECTRUM,
											min_brightness)) == NULL) {
				e_warning("object not found in image %d", 
                        i*diff_cubes[i]->np+j+1) ;
				positions[i][j] = -1 ;
			} else {
				e_comment(1, "object found in image %d at position %g",
						i*diff_cubes[i]->np+j+1, 
						position->y[0]) ;
				positions[i][j] = position->y[0] ;
				double3_del(position) ;
			}
			image_del(tmp_image) ;
		}
	}

	/* Write the positions table on disk */
	sprintf(name, "%s_positions.tfits", outname) ;
	col_names = malloc(3*sizeof(char*)) ;
	for (i=0 ; i<3 ; i++) col_names[i] = malloc(FILENAMESZ) ;
	sprintf(col_names[0], "Star_positions") ;
	sprintf(col_names[1], "Spec_LR_positions") ;
	sprintf(col_names[2], "Spec_MR_positions") ;
	
	if (sttr_write_tables(name,
				diff_cubes[0]->np,
				3,
				col_names,
				pro_catg_val[1],
                "spec_tec_startrace",
				positions,
				inname) == -1) {
		e_warning("cannot write the correspondance table") ;
	}
	for (i=0 ; i<3 ; i++) free(col_names[i]) ;
	free(col_names) ;
	
	/* Free the imaging data */
	cube_del(diff_cubes[0]) ;

	/* Create the correspondance table between the positions	 */
	if ((corr_table=sttr_compute_corres(positions, 
										diff_cubes[1]->np,
										display)) == NULL) {
		e_warning("cannot create the correspondance table") ;
		corr_il1 = corr_il2 = corr_il3 = corr_im1 = corr_im2 = corr_im3 = -1 ;
	} else {
		e_comment(1, "Polynomial imaging-LR: Y = %g + %g * y + %g * y^2", 
				corr_table[0][0], corr_table[0][1], corr_table[0][2]) ;
		e_comment(1, "Polynomial imaging-MR: Y = %g + %g * y + %g * y^2",
                corr_table[1][0], corr_table[1][1], corr_table[1][2]) ;
	
		corr_il1 = corr_table[0][0] ;
		corr_il2 = corr_table[0][1] ;
		corr_il3 = corr_table[0][2] ;
		corr_im1 = corr_table[1][0] ;
		corr_im2 = corr_table[1][1] ;
		corr_im3 = corr_table[1][2] ;
		
		/* Write the output correspondance table */
		sprintf(name, "%s_corresp.tfits", outname) ;
		col_names = malloc(2*sizeof(char*)) ;
		for (i=0 ; i<2 ; i++) col_names[i] = malloc(FILENAMESZ) ;
		sprintf(col_names[0], "Imaging_LR") ;
		sprintf(col_names[1], "Imaging_MR") ;
		if (sttr_write_tables(name,
								3,
								2,
								col_names,
								pro_catg_val[2],
                                "spec_tec_startrace",
								corr_table,
								inname) == -1) {
			e_warning("cannot write the correspondance table") ;
		}
		for (i=0 ; i<2 ; i++) free(col_names[i]) ;
		free(col_names) ;
		free(corr_table[0]) ;
		free(corr_table[1]) ;
		free(corr_table) ;
	}
		

	/* Load the input file names to get header informations */
	if ((flist=framelist_load(inname)) == NULL) {
		e_error("cannot compute wavelength calibration - aborting") ;
		for (i=0 ; i<3 ; i++) free(positions[i]) ;
	    free(positions) ;
		for (i=1 ; i<3 ; i++) cube_del(diff_cubes[i]) ;
    	free(diff_cubes) ;
		return -1 ;
	}

	/* Get the first LR and MR files names  */
	strcpy(first_LR_file, flist->name[diff_cubes[1]->np]) ;
	strcpy(first_MR_file, flist->name[2*diff_cubes[1]->np]) ;
	
	/* Wavelength calibration in LR  */
	wavecal_LR=isaac_get_disprel_estimate(flist->name[diff_cubes[1]->np], 3) ;
	/* Wavelength calibration in MR */
	wavecal_MR=isaac_get_disprel_estimate(flist->name[2*diff_cubes[1]->np], 3) ;
	framelist_del(flist);

	if ((wavecal_LR == NULL) || (wavecal_MR == NULL)) {
		for (i=0 ; i<3 ; i++) free(positions[i]) ;
        free(positions) ;
        for (i=1 ; i<3 ; i++) cube_del(diff_cubes[i]) ;
        free(diff_cubes) ;
        if (wavecal_LR == NULL) free(wavecal_LR) ;
        if (wavecal_MR == NULL) free(wavecal_MR) ;
		return -1 ;
	}
	e_comment(1, "LR : Wavelength(x) = %g + %g * x + %g * x^2 + %g * x^3",
				wavecal_LR[0], wavecal_LR[1], wavecal_LR[2], wavecal_LR[3]) ;
	e_comment(1, "MR : Wavelength(x) = %g + %g * x + %g * x^2 + %g * x^3",
				wavecal_MR[0], wavecal_MR[1], wavecal_MR[2], wavecal_MR[3]) ;

	
	/* Extraction of the spectra	 */
	
	/* Allocate extracted_table */
	nbcol_extr = 2+2*diff_cubes[1]->np ;
	extracted_table = malloc(nbcol_extr*sizeof(double*)) ;
	extracted_table[0] = malloc((1+diff_cubes[1]->ly)*sizeof(double)) ;
	extracted_table[diff_cubes[1]->np+1] = 
        malloc((1+diff_cubes[1]->ly)*sizeof(double));
		
	/* Fill the LR wavelength column */
	extracted_table[0][0] = 0 ;
	for (i=1 ; i<diff_cubes[1]->lx + 1 ; i++) {
		extracted_table[0][i]=wavecal_LR[0]+wavecal_LR[1]*i+
            wavecal_LR[2]*i*i+wavecal_LR[3]*i*i*i;
	}
			
	/* Fill the MR wavelength column */
	extracted_table[diff_cubes[1]->np+1][0] = 0 ;
	for (i=1 ; i<diff_cubes[2]->lx + 1 ; i++) {
		extracted_table[diff_cubes[1]->np+1][i]=wavecal_MR[0]+
            wavecal_MR[1]*i+wavecal_MR[2]*i*i+wavecal_MR[3]*i*i*i ;
	}
	
	/* Extract LR spectra */
	for (i=0 ; i<diff_cubes[1]->np ; i++) {
		compute_status("Extract the LR spectra", i, diff_cubes[1]->np, 1) ;
		extracted_table[i+1]=sttr_extract_spec(diff_cubes[1]->plane[i],
												positions[1][i],
												spec_width,
												sky_dist,
												sky_width,
												wavecal_LR,
												display) ;
	}
	free(wavecal_LR) ;
	
	/* Extract MR spectra */
	for (i=0 ; i<diff_cubes[2]->np ; i++) {
		compute_status("Extract the MR spectra", i, diff_cubes[2]->np, 1) ;
		extracted_table[diff_cubes[1]->np+2+i]=sttr_extract_spec(
												diff_cubes[2]->plane[i],
												positions[2][i],
												spec_width,
												sky_dist,
												sky_width,
												wavecal_MR,
												display) ;
	}
	free(wavecal_MR) ;
	
	/* Write the extracted table on disk */
	sprintf(name, "%s_extracted.tfits", outname) ;
	col_names = malloc(nbcol_extr*sizeof(char*)) ;
	for (i=0 ; i<nbcol_extr ; i++) col_names[i] = malloc(FILENAMESZ) ;
	sprintf(col_names[0], "Wavelength_LR") ;
	sprintf(col_names[nbcol_extr/2], "Wavelength_MR") ;
	for (i=0 ; i<(nbcol_extr-2)/2 ; i++) {
		sprintf(col_names[i+1], "LR_%d", i+1) ;
		sprintf(col_names[(nbcol_extr)/2+i+1], "MR_%d", i+1) ;
	}
	if (sttr_write_tables(name,
				diff_cubes[2]->lx+1,
				nbcol_extr,
				col_names,
				pro_catg_val[3],
                "spec_tec_startrace",
				extracted_table,
				inname) == -1) {
		e_warning("cannot write the correspondance table") ;
	}
	for (i=0 ; i<nbcol_extr ; i++) free(col_names[i]) ;
	free(col_names) ;
	
	/* Free extracted table */
	for (i=0 ; i<nbcol_extr ; i++) free(extracted_table[i]) ;
    free(extracted_table) ;
	
	/* Spectra shape analysis */
	/* Set the rejection coefficients */
    if (mode == MODE_HAWAI) {
        /* If SW : */
        if (reject_l < 0) {
            reject_l_mr = MR_LEFT_REJ ;
            sval = pfits_get(ins, first_LR_file, "filter") ;
            switch (isaac_associate_filter(isaac_get_filterid(sval))) {
                case isaac_filter_z:  reject_l_lr = Z_LR_LEFT_REJ ;  break ;
                case isaac_filter_sz: reject_l_lr = SZ_LR_LEFT_REJ ; break ;
                case isaac_filter_jblock:
                case isaac_filter_j:  reject_l_lr = J_LR_LEFT_REJ ;  break ;
                case isaac_filter_sh: reject_l_lr = SH_LR_LEFT_REJ ; break ;
                case isaac_filter_sk: reject_l_lr = SK_LR_LEFT_REJ ; break ;
                default:
                e_warning("unsupported filter: %s", sval) ;
                reject_l_lr = 0 ;
                break ;
            }
        } else {
            reject_l_lr = reject_l ;
            reject_l_mr = reject_l ;
        }
        if (reject_r < 0) {
            reject_r_mr = MR_RIGHT_REJ ;
            sval = pfits_get(ins, first_LR_file, "filter") ;
            switch (isaac_associate_filter(isaac_get_filterid(sval))) {
                case isaac_filter_z:  reject_r_lr = Z_LR_RIGHT_REJ ;  break ;
                case isaac_filter_sz: reject_r_lr = SZ_LR_RIGHT_REJ ; break ;
                case isaac_filter_jblock:
                case isaac_filter_j:  reject_r_lr = J_LR_RIGHT_REJ ;  break ;
                case isaac_filter_sh: reject_r_lr = SH_LR_RIGHT_REJ ; break ;
                case isaac_filter_sk: reject_r_lr = SK_LR_RIGHT_REJ ; break ;
                default:
                e_warning("unsupported filter: %s", sval) ;
                reject_r_lr = 0 ;
                break ;
            }
        } else {
            reject_r_lr = reject_r ;
            reject_r_mr = reject_r ;
        }
    } else if (mode == MODE_ALLADIN) {
        /* If LW: */
        if (reject_l < 0) {
            reject_l_lr = 150 ;
            reject_l_mr = 150 ;
        } else {
            reject_l_lr = reject_l_mr = reject_l ;
        }
        if (reject_r < 0) {
            reject_r_lr = 150 ;
            reject_r_mr = 150 ;
        } else {
            reject_r_lr = reject_r_mr = reject_r ;
        }
    } else {
        e_error("Unrecognized mode - abort") ;
        return -1 ;
    }
    
	/* Allocate the shapes_table array */
	nbcol_shape = 2*diff_cubes[1]->np ;
	shapes_table = malloc(nbcol_shape*sizeof(double*)) ;
    fit_qualities = malloc((diff_cubes[1]->np) * sizeof(double)) ;

	/* Shape analysis of LR spectra */
	for(i=0 ; i<diff_cubes[1]->np ; i++) {
		compute_status("Shape analysis of LR spectra", i, diff_cubes[1]->np, 1);
		shapes_table[i]=sttr_shape_analysis(diff_cubes[1]->plane[i],
											positions[1][i],
											spec_width,
											poly_degree,
											reject_l_lr,
											reject_r_lr,
                                            &(fit_qualities[i]),
											display) ;
	}
    fit_quality_lr = double_median(fit_qualities, diff_cubes[1]->np) ;
    free(fit_qualities) ;
    fit_qualities = malloc((diff_cubes[2]->np) * sizeof(double)) ;

	/* Shape analysis of MR spectra */
	for (i=0 ; i<diff_cubes[2]->np ; i++) {
		compute_status("Shape analysis of MR spectra", i, diff_cubes[2]->np, 1);
		shapes_table[diff_cubes[1]->np+i]=sttr_shape_analysis(
												diff_cubes[2]->plane[i],
												positions[2][i],
												spec_width,
												poly_degree,
												reject_l_mr,
												reject_r_mr,
                                                &(fit_qualities[i]),
												display) ;
	}
    fit_quality_mr = double_median(fit_qualities, diff_cubes[2]->np) ;
    free(fit_qualities) ;

	/* Write the shape table on disk */
    sprintf(name, "%s_shapes.tfits", outname) ;
    col_names = malloc(nbcol_shape*sizeof(char*)) ;
    for (i=0 ; i<nbcol_shape ; i++) col_names[i] = malloc(FILENAMESZ) ;
    for (i=0 ; i<nbcol_shape/2 ; i++) {
        sprintf(col_names[i], "LR_%d", i+1) ;
        sprintf(col_names[nbcol_shape/2+i], "MR_%d", i+1) ;
    }
    if (sttr_write_tables(name,
                poly_degree+1,
                nbcol_shape,
                col_names,
                pro_catg_val[4],
                "spec_tec_startrace",
                shapes_table,
				inname) == -1) {
        e_warning("cannot write the correspondance table") ;
    }
    for (i=0 ; i<nbcol_shape ; i++) free(col_names[i]) ;
    free(col_names) ;

	/* Compute the 2d polynomial for LR */
	if ((poly_2d_LR=sttr_find_2d_poly(shapes_table,
								diff_cubes[1]->np,
								diff_cubes[1]->lx,
								diff_cubes[1]->ly,
								poly_degree,
								positions[1])) == NULL) {
		e_error("cannot compute 2d polynomial") ;
    	for (i=0 ; i<3 ; i++) free(positions[i]) ;
    	free(positions) ;
		for (i=1 ; i<3 ; i++) cube_del(diff_cubes[i]) ;
    	free(diff_cubes) ;
		for (i=0 ; i<nbcol_shape ; i++) free(shapes_table[i]) ;
    	free(shapes_table) ;
		return -1 ;
	} else {
		e_comment(1, "Startrace deformation in LR : Y=f(x,y)") ;
		for (i=0 ; i<6 ; i++) {
			e_comment(2, "%g\t%g\t%g\n", poly_2d_LR[0][i], poly_2d_LR[1][i],
				poly_2d_LR[2][i]) ;
		}						
	
		/* Write the LR 2d polynomial table on disk	(*used by spjitter*) */
		sprintf(name, "%s_poly2d_LR.tfits", outname) ;
		if (sttr_write_poly2d(first_LR_file,
								name,
								6,
								poly_2d_LR,
								inname,
                                pro_catg_val[5]) == -1) {
			e_warning("cannot write the 2d polyn. outfile for LR") ;
		}
	}
	
	/* Compute the 2d polynomial for MR */
	if ((poly_2d_MR=sttr_find_2d_poly(&shapes_table[diff_cubes[1]->np],
								diff_cubes[2]->np,
								diff_cubes[2]->lx,
								diff_cubes[2]->ly,
								poly_degree,
								positions[2])) == NULL) {
		e_error("cannot compute 2d polynomial") ;
    	for (i=0 ; i<3 ; i++) free(positions[i]) ;
    	free(positions) ;
		for (i=1 ; i<3 ; i++) cube_del(diff_cubes[i]) ;
        free(diff_cubes) ;
        for (i=0 ; i<nbcol_shape ; i++) free(shapes_table[i]) ;
        free(shapes_table) ;
        return -1 ;
	} else {
		e_comment(1, "Startrace deformation in MR : Y=f(x,y)") ;
		for (i=0 ; i<6 ; i++) {
			e_comment(2, "%g\t%g\t%g\n", poly_2d_MR[0][i], poly_2d_MR[1][i],
				poly_2d_MR[2][i]) ;
		}
		
		/* Write the MR 2d polynomial table on disk (*used by spjitter*) */
		sprintf(name, "%s_poly2d_MR.tfits", outname) ;
		if (sttr_write_poly2d(first_MR_file,
								name,
								6,
								poly_2d_MR,
								inname,
                                pro_catg_val[5]) == -1) {
			e_warning("cannot write the 2d polyn. outfile for MR") ;
		}
	}

	/* Write the PAF file on disk */
	if ((sttr_write_paffile(outname,
							first_LR_file,
							corr_il1,
							corr_il2,
							corr_il3,
							corr_im1,
							corr_im2,
							corr_im3,
							poly_2d_LR,
							poly_2d_MR,
                            fit_quality_lr,
                            fit_quality_mr,
                            pro_catg_val[6])) == -1) {
		e_warning("cannot write the output PAF file: [%s.paf]", 
                get_rootname(outname)) ; 
	}

	/* Free poly_2d_LR and poly_2d_MR */
	if (poly_2d_LR != NULL) {
		for (i=0 ; i<3 ; i++) free(poly_2d_LR[i]) ;
		free(poly_2d_LR) ;
	}
	if (poly_2d_MR != NULL) {
		for (i=0 ; i<3 ; i++) free(poly_2d_MR[i]) ;
		free(poly_2d_MR) ;
	}
	
	/* Free positions and shapes array and difference cubes	 */
    for (i=0 ; i<3 ; i++) free(positions[i]) ;
    free(positions) ;
	for (i=1 ; i<3 ; i++) cube_del(diff_cubes[i]) ;
    free(diff_cubes) ;
	for (i=0 ; i<nbcol_shape ; i++) free(shapes_table[i]) ;
	free(shapes_table) ;
	
	/* Return	 */
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	correct the distortion in a cube	
  @param	in_cube	Input cube
  @param	disto_table	Distortion table or arc calibration image
  @param	corrected	Flag to say if the correction as been done or not
  @return	-1 in error case, 0 otherwise	
 */
/*----------------------------------------------------------------------------*/
static int sttr_correct_distortion(
		cube_t	**	in_cube,
		char	*	disto_table,
		int		*	correct)
{
	image_t			*	tmp_image ;
	poly2d			*	correct_arc ;	
	poly2d			*	correct_sttr ;	
	cube_t			*	corrected ;

	int					i ;
	
	/* Initialize */
	*correct = 0 ;	

	/* Test input file */
	if (disto_table == NULL) return 0 ;
	
	/* Estimate the ARC distortion */
	if (qfits_is_table(disto_table, 0)) {
		correct_arc = read_poly2d_from_table(disto_table) ;
	} else if (is_fits_file(disto_table) == 1) {
		tmp_image = image_load(disto_table) ;
		correct_arc=isaac_compute_distortion(tmp_image,
                                                   10,
                                                   10,
                                                   1000,
                                                   1000,
												   1,
												   NULL,
												   NULL) ;
		image_del(tmp_image) ;
	} else {
		e_warning("input distortion file is not a fits file") ;
		return -1 ;
	}

	/* Test if ARC distortion was successfully estimated  */
	if (correct_arc == NULL) {
		e_warning("cannot estimate ARC distortion") ;
		return -1 ;
	}
	e_comment(1, "Correct the spectro images with:") ;
	for (i=0 ; i<6 ; i++) {
		e_comment(2, "%d\t%d\t%g", correct_arc->px[i], 
                correct_arc->py[i], correct_arc->c[i]) ;
	}	

	/* Polynomial f(x,y) = y */
	correct_sttr = poly2d_build_from_string("0 1 1.0") ;

	/* Correct the images */
	corrected = cube_new((*in_cube)->lx, (*in_cube)->ly, (*in_cube)->np) ;
	for (i=0 ; i<corrected->np ; i++) {
		compute_status("Warping images", i, corrected->np, 1) ;
		if ((corrected->plane[i] = image_warp_generic((*in_cube)->plane[i],
												"default",
                                        	    correct_arc,
                                        	    correct_sttr)) == NULL) {
			e_warning("cannot warp image") ;
			corrected->plane[i] = image_copy((*in_cube)->plane[i]) ;
		}
	}
	poly2d_free(correct_sttr) ;
	poly2d_free(correct_arc) ;
	cube_del(*in_cube) ;
	*in_cube = cube_copy(corrected) ;
	cube_del(corrected) ;
	*correct = 1 ;
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Determines a 2d polynomial with 1d polynomials interpolation
  @param	shapes	1D polynomials coefficients
  @param	nb		number of polynomials
  @param	x_size	image size in x
  @param	y_size	image size in y
  @param	deg		polynomial degree
  @param	positions	list of spectra positions
  @return	double ** identifiing a 2d polynomial, NULL in error case
 */
/*----------------------------------------------------------------------------*/
static double ** sttr_find_2d_poly(
		double	**	shapes,
		int			nb,
		int			x_size,
		int			y_size,
		int			deg,
		double	*	positions)
{
	double			**	poly_2d ;
	double			*	ret_poly ;
	double3			*	surface ;
	int					npoints ;
	int					nb_xpoints ;
	int				*	valid_poly ;
	int					nb_valid_poly ;
	double				x_coor ;
	double				val ;
	int					current_poly ;
	int					nb_coeffs ;
	
	int					i, j, k, l ;
	
	/* Allocate valid_poly */
	valid_poly = malloc(nb*sizeof(int)) ;

	nb_valid_poly = 0 ;
	for (i=0 ; i<nb ; i++) {
		if (shapes[i][0] == 0) {
			valid_poly[i] = 0 ;
		} else {
			valid_poly[i] = 1 ;
			nb_valid_poly++ ;
		}
	}

	if (nb_valid_poly < 3) {
		e_warning("not enough 1d polynomials to create the 2d polynomial") ;
		free(valid_poly) ;
		return NULL ;
	}
	
	nb_xpoints = 20 ;
	npoints = nb_xpoints * nb_valid_poly ;
	
	/* Allocate the surface */
	surface = double3_new(npoints);

	/* Fill the points of the surface */
	for (i=0 ; i<nb_xpoints ; i++) {
		x_coor = (x_size/(nb_xpoints+1))*(i+1) ;
		current_poly = 0 ;
		for (j=0 ; j<nb_valid_poly ; j++) {
			while (shapes[current_poly][0] == 0) current_poly++ ;
			surface->x[i+j*nb_xpoints] = x_coor ;
			surface->y[i+j*nb_xpoints] = (double)positions[current_poly] ;
			surface->z[i+j*nb_xpoints] = shapes[current_poly][0] ;
			val = 1 ;
			for (k=0 ; k<deg ; k++) {
				val = 1 ;
				for (l=0 ; l<k+1 ; l++) val *= x_coor ;
				surface->z[i+j*nb_xpoints] += shapes[current_poly][k+1]*val ;
			}
			current_poly++ ;
		}
	}

	/* Free valid_poly array */
	free(valid_poly) ;
	
	/* Compute the 2d polynomial	 */
	if ((ret_poly=fit_surface_polynomial(surface,
										"(0,0) (1,0) (0,1) (1,1) (2,0) (0,2)",
										2,
										&nb_coeffs,
										NULL)) == NULL) {
		e_error("cannot compute the 2D polynomial") ;
		double3_del(surface) ;
		return NULL ;
	}
	double3_del(surface) ;

	/* Allocate poly_2d */
	poly_2d=malloc(3*sizeof(double*)) ;
	for (i=0 ; i<3 ; i++) poly_2d[i] = malloc(nb_coeffs*sizeof(double)) ;

	poly_2d[0][0] = 0 ;
	poly_2d[0][1] = 1 ;
	poly_2d[0][2] = 0 ;
	poly_2d[0][3] = 1 ;
	poly_2d[0][4] = 2 ;
	poly_2d[0][5] = 0 ;
	poly_2d[1][0] = 0 ;
	poly_2d[1][1] = 0 ;
	poly_2d[1][2] = 1 ;
	poly_2d[1][3] = 1 ;
	poly_2d[1][4] = 0 ;
	poly_2d[1][5] = 2 ;
	for (i=0 ; i<nb_coeffs ; i++) poly_2d[2][i] = ret_poly[i] ;
	
	/* Free and return */
	free(ret_poly) ;
	return poly_2d ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	fit the spectrum by a polynomial
  @param	in	image
  @param	pos	spectrum position
  @param	spec_w	spectrum width
  @param	deg		polynomial degree
  @param	reject_l	rejected border on the left
  @param	reject_r	rejected border on the right
  @param    mse         Mean square error on the fit
  @param	display		flag to display results
  @return	array of poly_degree+2 coefficients (spectrum position +
  (poly_degree+1) polynomial coefficients
  if position=-1 or in error case, return an array of 0
 */
/*----------------------------------------------------------------------------*/
static double * sttr_shape_analysis(
		image_t	    *	in,
		double			pos,
		int				spec_w,
		int				deg,
		int				reject_l,
		int				reject_r,
        double      *   mse,
		int				display)
{
	double			*	coeffs ;
	double3			*	to_fit ;
	int					low_side,
						up_side ;	
	image_t			*	filtered ;
	gnuplot_ctrl    *   handle ;
	char				cmd[FILENAMESZ] ;
	char				cmd2[FILENAMESZ] ;
	image_t			*	extr_line ;
	double				centroid ;
	int					i, j ;

	/* If the spectrum were not found, return an array of 0 */
	if (pos == -1) return calloc(in->lx+1, sizeof(double)) ;

	/* Spectrum position */
    low_side = (int)(pos - spec_w/2) ;
    up_side = low_side + spec_w ;
    if ((low_side < 1) || (up_side > in->lx)) {
        e_warning("spectrum too close to the image border - cannot analyse") ;
        return calloc(in->lx+1, sizeof(double)) ;
    }

	/* Filter the input image */
    if ((filtered=image_filter_median(in)) == NULL) {
        e_warning("cannot filter the combined image") ;
        filtered = image_copy(in) ;
    }

	/* Allocate the to_fit array */
	to_fit = double3_new(in->lx);

	/* Fill to_fit	 */
	for (i=0 ; i<in->lx-reject_l-reject_r ; i++) {
		to_fit->x[i] = (double)(i+1+reject_l) ;
		if ((extr_line=image_getvig(filtered,
											i+1+reject_l, 
											low_side,
											i+1+reject_l,
											up_side)) == NULL) {
			e_warning("cannot extract line from image") ;
			image_del(filtered) ;
			double3_del(to_fit) ; 
			return calloc(in->lx+1, sizeof(double)) ;
		}
		centroid = function1d_find_centroid(extr_line->data, spec_w) ;
		if (centroid > 0.0) to_fit->y[i] = (double)low_side + centroid ;
		else to_fit->y[i] = (double)low_side + spec_w/2. ;
		image_del(extr_line) ;
	}
	image_del(filtered) ;		

	/* Compute the fit */
	to_fit->n = in->lx - reject_l - reject_r ;
    if ((coeffs=fit_1d_poly(deg, to_fit, mse)) == NULL) {
		e_warning("cannot compute the fit") ;
		double3_del(to_fit) ;
		return calloc(in->lx+1, sizeof(double)) ;
	}

	/* Display the polynomial */
	if (display) {
		handle = gnuplot_init() ;
		gnuplot_setstyle(handle, "points") ;
		gnuplot_set_xlabel(handle, "Pixels") ;
		gnuplot_set_ylabel(handle, "Spectrum position") ;
		gnuplot_plot_xy(handle,
    		            to_fit->x,
    		            to_fit->y,
    		            to_fit->n,
    	        	    "Spectrum shape") ;
		printf("press enter to continue\n") ;
		while (getchar() != '\n') {}
		sprintf(cmd, "replot %g", coeffs[0]) ;
		for (i=0 ; i<deg ; i++) {
			sprintf(cmd2, "%s+%g", cmd, coeffs[i+1]) ;
			sprintf(cmd, cmd2) ;
			for (j=0 ; j<i+1 ; j++) {
				sprintf(cmd2, "%s*x", cmd) ;
				sprintf(cmd, cmd2) ;
			}
		}
		sprintf(cmd2, "%s\n", cmd) ;
		sprintf(cmd, cmd2) ;
		
		printf("%s\n", cmd) ;
		
		gnuplot_cmd(handle, cmd) ;
        printf("press enter to continue\n") ;
        while (getchar() != '\n') {}
		gnuplot_close(handle) ;
	}
	
	/* Free and Return */
	double3_del(to_fit) ;
	return coeffs ; 
}

/*----------------------------------------------------------------------------*/
/**
  @brief	extract a spectrum at a known position
  @param	in	input image
  @param	pos	spectrum position
  @param	spec_w	spectrum width
  @param	sky_d	residual sky distance to spectrum
  @param	sky_w	residual sky width
  @param	wave	wavelength calibration coefficients (4 coeffs)
  @param	display	flag to display results
  @return	allocated array of spectrum position and extracted values 
  (size:inimage->lx+1)
  if position=-1 or in error case, return an array of 0
 */
/*----------------------------------------------------------------------------*/
static double * sttr_extract_spec(
		image_t		*	in,
		double			pos,
		int				spec_w,
		int				sky_d,
		int				sky_w,
		double		*	wave,
		int				display)
{
	double			*	extracted ;
	int					low_side,
						up_side ;
	int					sky_pos[4] ;
	image_t			*	filtered ;	
	double				median_1,
						median_2 ;
	pixelvalue   		sky_estim ;
	image_t        	*   extr_line ;
	double3			*	to_plot ;
	int					i ;
	
    /* If the spectrum were not found, return an array of 0 */
	if (pos == -1) return calloc(in->lx+1, sizeof(double)) ;
				
	/* Set the parameters for the extraction */

    /* Spectrum position */
    low_side = (int)(pos - spec_w/2) ;
    up_side = low_side + spec_w ;
    if ((low_side < 1) || (up_side > in->lx)) {
        e_warning("spectrum too close to the image border - cannot extract") ;
		return calloc(in->lx+1, sizeof(double)) ;
    }

    /* Positions of the residual sky */
    sky_pos[1] = (int)(pos - sky_d) ;
    sky_pos[0] = (int)(sky_pos[1] - sky_w) ;
    sky_pos[2] = (int)(pos + sky_d) ;
    sky_pos[3] = (int)(sky_pos[2] + sky_w) ;

    /* Allocate extracted array */
    extracted = calloc(in->lx+1, sizeof(double)) ;

    /* Filter the input image */
    if ((filtered=image_filter_median(in)) == NULL) {
        e_warning("cannot filter the combined image") ;
        filtered = image_copy(in) ;
    }

    /* Extract the spectrum and get rid of the residual sky */
    for (i=0 ; i<in->lx ; i++) {

		/* Estimate the SKY */
        if ((sky_pos[0] < 1) && (sky_w != 0)) {
			median_1 = image_getmedian_vig(filtered,
													i+1,
                                                	sky_pos[2],
                                                	i+1,
                                                	sky_pos[3]) ;
			sky_estim = median_1 ;
		} else if ((sky_pos[3] > in->ly) && (sky_w != 0)) {
			median_1 = image_getmedian_vig(filtered,
													i+1,
													sky_pos[0],
													i+1,
													sky_pos[1]) ;
			sky_estim = median_1 ;
		} else if (sky_w != 0) {
			median_1 = image_getmedian_vig(filtered,
													i+1,
													sky_pos[0],
													i+1,
													sky_pos[1]) ;
			median_2 = image_getmedian_vig(filtered,
													i+1,
													sky_pos[2],
													i+1,
													sky_pos[3]) ;
			sky_estim=(median_1 + median_2)/2 ;
		} else {
			e_comment(1, "No sky background subtraction") ;
			sky_estim = 0 ;
		}
		
		/* Estimate the SPECTRUM  */
		if ((extr_line = image_getvig(filtered,
									i+1,
									low_side,
									i+1,
									up_side)) == NULL) {
			e_warning("cannot extract image") ;
			image_del(filtered) ;
			return extracted ;
		}
        
		extracted[i+1]=(double)image_getsumpix(extr_line) ;
        image_del(extr_line);
        extracted[i+1] -= spec_w*sky_estim ;
    }
	extracted[0] = pos ;
	
	/* Free filtered image */
	image_del(filtered) ;
    
	/* If display option is specified */
	if (display) {
		to_plot = double3_new(in->lx);
		for (i=0 ; i<in->lx ; i++) {
			to_plot->x[i] = (double)(wave[0]+wave[1]*(i+1)+wave[2]*(i+1)*(i+1)+
                    wave[3]*(i+1)*(i+1)*(i+1));
			to_plot->y[i] = (double)extracted[i+1] ;
		}
		gnuplot_plot_once("Extracted spectrum",
						  "lines",
						  "wavelength",
						  "spectrum",
						  to_plot->x,
						  to_plot->y,
						  to_plot->n);
		double3_del(to_plot) ;
	}

	/* Return */
	return extracted ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	find the relations: (star_position(LR_spectrum_position) and 
            star_position(MR_spectrum_position))
  @param	pos	array of positions
  @param	nb_pos	number of positions
  @param	display	flag to display results
  @return	coefficients of two 2nd degree polynomials
 */
/*----------------------------------------------------------------------------*/
static double ** sttr_compute_corres(
		double	**	pos,
		int			nb_pos,
		int			display)
{
	double			**	corr_table ;	
	double3			*	to_fit ;
	int					nb_valid_points ;
	gnuplot_ctrl    *   handle ;
	char				cmd[FILENAMESZ] ;
	
	int					i ;

	/* Allocate correspondance table (2 polynomials) and to_fit */
	corr_table = malloc(2*sizeof(double*)) ;
	to_fit = double3_new(nb_pos);

	/* Correspondance for LR */

	/* Fill to_fit */
	nb_valid_points = 0 ;
	for (i=0 ; i<nb_pos ; i++) {
		if ((pos[0][i] > 0) && (pos[1][nb_pos-i-1] >0)) {
			to_fit->x[nb_valid_points] = pos[0][i] ;
			to_fit->y[nb_valid_points] = pos[1][nb_pos-i-1] ;
			nb_valid_points++ ;
		}
	}

	/* At least three points to fit a 2nd degree poly */
	if (nb_valid_points < 3) {
		e_error("not enough detections to create the correspondance table") ;
		double3_del(to_fit) ;
		free(corr_table) ;
		return NULL ;
	}

	/* Compute the fit */
	to_fit->n = nb_valid_points ;
	if ((corr_table[0]=fit_1d_poly(2, to_fit, NULL)) == NULL) {
		e_error("cannot fit a polynomial") ;
		double3_del(to_fit) ;
		free(corr_table) ;
		return NULL ;
	}

	if (display) {
		handle = gnuplot_init() ;
		gnuplot_setstyle(handle, "points") ;
		gnuplot_set_xlabel(handle, "Y_imaging") ;
		gnuplot_set_ylabel(handle, "Y_spec_LR") ;
		gnuplot_plot_xy(handle,
    		            to_fit->x,
    		            to_fit->y,
    		            to_fit->n,
    	        	    "Correspondance Imaging-LR") ;
		sprintf(cmd, "replot %g+%g*x+%g*x*x\n", corr_table[0][0],
			corr_table[0][1], corr_table[0][2]) ;
		printf("press enter to continue\n") ;
		while (getchar() != '\n') {}
		gnuplot_cmd(handle, cmd) ;
		printf("press enter to continue\n") ;
		while (getchar() != '\n') {}
		gnuplot_close(handle) ;
	}

	
	/* Correspondance for MR	 */
	
	/* Fill to_fit */
	nb_valid_points = 0 ; 
	for (i=0 ; i<nb_pos ; i++) {
		if ((pos[0][i] > 0) && (pos[2][i] > 0)) {
			to_fit->x[nb_valid_points] = pos[0][i] ;
			to_fit->y[nb_valid_points] = pos[2][i] ;
			nb_valid_points++ ;
		}
	}

	/* At least three points to fit a 2nd degree poly */
    if (nb_valid_points < 3) {
        e_error("not enough detections to create the correspondance table") ;
        double3_del(to_fit) ;
    	free(corr_table[0]) ;
		free(corr_table) ;
        return NULL ;
    }

	/* Compute the fit */
	to_fit->n = nb_valid_points ;
	if ((corr_table[1]=fit_1d_poly(2, to_fit, NULL)) == NULL) {
		e_error("cannot fit a polynomial") ;
		double3_del(to_fit) ;
		free(corr_table[0]) ;
		free(corr_table) ;
		return NULL ;
	}
	
	if (display) {
		handle = gnuplot_init() ;
		gnuplot_setstyle(handle, "points") ;
		gnuplot_set_xlabel(handle, "Y_imaging") ;
		gnuplot_set_ylabel(handle, "Y_spec_MR") ;
		gnuplot_plot_xy(handle,
    		            to_fit->x,
    		            to_fit->y,
    		            to_fit->n,
    	        	    "Correspondance Imaging-MR") ;
		sprintf(cmd, "replot %g+%g*x+%g*x*x\n", corr_table[1][0],
			corr_table[1][1], corr_table[1][2]) ;
		printf("press enter to continue\n") ;
		while (getchar() != '\n') {}
		gnuplot_cmd(handle, cmd) ;
		printf("press enter to continue\n") ;
		while (getchar() != '\n') {}
		gnuplot_close(handle) ;
	}


	/* Free and return */
	double3_del(to_fit) ;
	return corr_table ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Write the out fits file
  @param	outname		output name
  @param	nb_lines	number of lines in the output table
  @param	nb_col		number of columns in the output table
  @param	col_labs	names of columns
  @param	key			key to access the PRO CATG keyword	
  @param    recipe_id   Recipe ID
  @param	out_table	data to write in the table
  @param	inname		input file name
  @return	-1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int sttr_write_tables(
		char			*	outname,
		int					nb_lines,
		int					nb_col,
		char			**	col_labs,
		procat      		key,
        char            *   recipe_id,
		double			**	out_table,
		char			*	inname)
{
	qfits_header	*	fh ;
	qfits_table	    *   table ;
	qfits_col       *   col ;
	framelist		*	lnames ;
	int					i ;
   
	/* Write the output qfits_table table (informations) */
	table = qfits_table_new(outname, QFITS_BINTABLE, -1, nb_col, nb_lines) ;
	col = table->col ;
	for (i=0 ; i<table->nc ; i++) {
		qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, col_labs[i],
                " ", " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
		col++ ;
	}

	/* Get the input files names */
	if ((lnames=framelist_load(inname)) == NULL) {
		e_error("cannot read the ascii input file") ;
		return -1 ;
	}
	
    /* WRITE THE OUTPUT FILE */
	/* Read the input header */
    if ((fh = qfits_header_read(lnames->name[0])) == NULL) {
        e_error("in writing the output fits file") ;
        qfits_table_close(table) ;
    	framelist_del(lnames) ;
        return -1 ;
    }

	/* Prepare it for table output */
    if (isaac_header_for_table(fh) == -1) {
        e_error("in writing the output fits file") ;
        qfits_header_destroy(fh) ;
    	framelist_del(lnames) ;
        qfits_table_close(table) ;
        return -1 ;
    }

	/* Write the PRO keywords in the header */
    if (isaac_pro_fits(fh,
                        outname,
                        "REDUCED",
                        NULL,
                        key,
                        "OK",
                        recipe_id,
                        lnames->n,
						lnames,
						NULL) == -1) {
        e_error("in writing PRO keywords in output file") ;
        qfits_header_destroy(fh) ;
   		framelist_del(lnames) ;
        qfits_table_close(table) ;
        return -1 ;
    }

	/* Write the HISTORY keywords with the input file names */
	if (isaac_add_files_history(fh, lnames) == -1) {
        e_warning("cannot write HISTORY keywords in out file") ;
    }
    framelist_del(lnames) ;
   
	/* Write the file on disk */
	if (qfits_save_table_hdrdump((void**)out_table, table, fh) == -1) {
		e_error("cannot write file: %s", outname) ;
		qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
		return -1 ;
	}
	qfits_header_destroy(fh) ;
	qfits_table_close(table) ;

	e_comment(0, "File [%s] produced", outname) ;
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Write the out fits file
  @param	inname	input file name
  @param	outname	output file name
  @param	nb_coeffs	number of coefficients
  @param	out_table	data to write in the output table
  @param    ascii_file	input ascii file
  @param    pro_catg_val    PRO CATG value to write
  @return	i1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int sttr_write_poly2d(
		char		    *	inname,
		char		    *	outname,
		int			    	nb_coeffs,
		double		    **	out_table,
		char		    *	ascii_file,
        procat              pro_catg_val)
{
	qfits_header	*	fh ;
	qfits_table		*   table ;
    qfits_col       *   col ;
	framelist		*	lnames ;
	char                cval[80] ;
	char			*	res ;
    instrument_t        ins ;

	int					i ;
	
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

   /* Write the output qfits_table table (informations) */
	table = qfits_table_new(outname, QFITS_BINTABLE, -1, 3, nb_coeffs) ;
	col = table->col ;
    for (i=0 ; i<table->nc ; i++) {
		qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, " ", " ",
                " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
		col++ ;
	}
	col = table->col ;
	sprintf(col->tlabel, "Degree_of_x") ;
	col++ ;	
	sprintf(col->tlabel, "Degree_of_y") ;
	col++ ;	
	sprintf(col->tlabel, "poly2d_coef") ;

	/* Get the input files names */
    if ((lnames=framelist_load(ascii_file)) == NULL) {
        e_error("cannot read the ascii input file") ;
        return -1 ;
    }

    /* WRITE THE OUTPUT FILE */
    /* Read the input header */
    if ((fh = qfits_header_read(lnames->name[0])) == NULL) {
        e_error("in writing the output fits file") ;
        qfits_table_close(table) ;
        framelist_del(lnames) ;
        return -1 ;
    }

    /* Prepare it for table output */
    if (isaac_header_for_table(fh) == -1) {
        e_error("in writing the output fits file") ;
        qfits_header_destroy(fh) ;
        framelist_del(lnames) ;
        qfits_table_close(table) ;
        return -1 ;
    }

    /* Write the PRO keywords in the header */
    if (isaac_pro_fits(fh,
                        outname,
                        "REDUCED",
                        NULL,
                        pro_catg_val,
                        "OK",
                        "spec_tec_startrace",
                        lnames->n,
						lnames,
						NULL) == -1) {
        e_error("in writing PRO keywords in output file") ;
        qfits_header_destroy(fh) ;
        framelist_del(lnames) ;
        qfits_table_close(table) ;
        return -1 ;
    }

    /* Write the HISTORY keywords with the input file names */
    if (isaac_add_files_history(fh, lnames) == -1) {
        e_warning("cannot write HISTORY keywords in out file") ;
    }
    framelist_del(lnames) ;
	if ((res = pfits_get(ins, inname, "resolution")) != NULL) { 
		sprintf(cval, "INS.GRAT.NAME= %s", res) ;
		qfits_header_add(fh, "HISTORY", cval, NULL, NULL) ;
	}	

	/* Write the file on disk */
	if (qfits_save_table_hdrdump((void**)out_table, table, fh) == -1) {
		e_error("cannot write file: %s", outname) ;
		qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
		return -1 ;
	}
	qfits_table_close(table) ;
	qfits_header_destroy(fh) ;
	
	e_comment(0, "File [%s] produced", outname) ;
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Classify the input frames in 3 cubes
  @param	inname	input ASCII list
  @param    mode    instrument mode
  @return	3 cubes (1 for images, 1 for LR, 1 for MR) 
 */
/*----------------------------------------------------------------------------*/
static cube_t ** sttr_read_input(
        char    *   inname,
        int         mode)
{
	cube_t		*	in ;
	cube_t		**	classified_cubes ;
	framelist	*	flist ;
	char            im_type[80],
                    lr_type[80],
                    mr_type[80] ;
    instrument_t    ins ;                
    int				i,
					j ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

    if (mode == MODE_HAWAI) {                
        /* If SW : use SWI1 SWS1-LR SWS1-MR  */
        sprintf(im_type, "SWI1") ;
        sprintf(lr_type, "SWS1-LR") ;
        sprintf(mr_type, "SWS1-MR") ;
    } else if (mode == MODE_ALLADIN) {
        /* If LW : use LWI3 LWS3-LR LWS3-MR  */
        sprintf(im_type, "LWI3") ;
        sprintf(lr_type, "LWS3-LR") ;
        sprintf(mr_type, "LWS3-MR") ;
    } else {
        e_error("Unrecognized mode - abort") ;
        return NULL ;
    }
                    
	/* Load the input file as a cube */
	if ((in = cube_load(inname)) == NULL) {
		e_error("cannot load the input file: [%s]", inname) ;
		return NULL ;
	}

	/* The number of files as to be a multiple of 3 */
	if (in->np % 3) {
		e_error("the number of input files is not a multiple of 3: %d", in->np);
		cube_del(in) ;
		return NULL ;		
	}
	
	/* Classify the input cube -> three cubes */
	classified_cubes = malloc(3*sizeof(cube_t*)) ;
	for (i=0 ; i<3 ; i++) {
        classified_cubes[i] = cube_new(in->lx, in->ly, (int)in->np/3);
    }


     /* Distribute planes in classified output cubes  */
    for (i=0 ; i<3 ; i++) {
		for (j=0 ; j<in->np/3 ; j++) {
			classified_cubes[i]->plane[j] = in->plane[((in->np)/3)*i+j] ;
            in->plane[((in->np)/3)*i+j] = NULL ;
        }
    }
    cube_del_shallow(in);

	/* Verification of data types through the header */
	/* Load the file names */
	if ((flist=framelist_load(inname)) == NULL) {
		e_warning("cannot load the filenames - skip header verification") ;
	} else {
		/* Verify that the first cube contains imaging data */
		for (i=0 ; i<flist->n/3 ; i++) {
			if (!strcmp(pfits_get(ins, flist->name[i], "mode"), im_type)) {
				e_comment(1, "verif. image %d -> imaging mode", i+1) ;
			} else {
				e_error("image %d -> NOT imaging mode - aborting", i+1) ;
				framelist_del(flist);
				for (i=0 ; i<3 ; i++) {
					cube_del(classified_cubes[i]) ;
				}
				free(classified_cubes) ;
				return NULL ;
			}
		}
	
		/* Verify that the second cube contains LR spectroscopic data */
		for (i=flist->n/3 ; i<2*flist->n/3 ; i++) {
			if (!strcmp(pfits_get(ins, flist->name[i], "mode"), lr_type)) {
				e_comment(1, "verif. image %d -> spectroscopic mode (LR)", i+1);
			} else {
				e_error("image %d -> NOT spectroscopic mode (LR) - aborting",
						i+1) ;
				framelist_del(flist);
				for (i=0 ; i<3 ; i++) {
					cube_del(classified_cubes[i]) ;
				}
				free(classified_cubes) ;
				return NULL ;
			}
		}
	
	
		/* Verify that the first cube contains MR spectroscopic data */
		for (i=2*flist->n/3 ; i<flist->n ; i++) {
			if (!strcmp(pfits_get(ins, flist->name[i], "mode"), mr_type)) {
				e_comment(1, "verif. image %d -> spectroscopic mode (MR)", i+1);
			} else {
				e_error("image %d -> NOT spectroscopic mode (MR) - aborting",
						i+1) ;
				framelist_del(flist);
				for (i=0 ; i<3 ; i++) {
					cube_del(classified_cubes[i]) ;
				}
				free(classified_cubes) ;
				return NULL ;
			}
		}
		framelist_del(flist);
	}
	return classified_cubes ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Write the PAF file for startrace
  @param	outname	output file name
  @param	inimage_name	one input image name (for header reference)
  @param	corr_il1 a (see doc field)
  @param	corr_il2 b (see doc field)
  @param	corr_il3 c (see doc field)
  @param	corr_im1 A (see doc field)
  @param	corr_im2 B (see doc field)
  @param	corr_im3 C (see doc field)
  @param	dist_lr Array with the 2d distortion (LR) polynomial coefficients
  @param	dist_mr Array with the 2d distortion (MR) polynomial coefficients
  @param    mse_lr      Fit mean square error in LR
  @parma    mse_mr      Fit mean square error in MR
  @param    pro_catg_val    PRO CATG value to write
  @return	-1 in error case, 0 otherwise
  star_pos = a+b*LR_spec_pos+c*LR_spec_pos*LR_spec_pos
  star_pos = A+b*MR_spec_pos+C*MR_spec_pos*MR_spec_pos
 */
/*----------------------------------------------------------------------------*/
static int sttr_write_paffile(
		char	        *	outname,
		char	        *	inimage_name,
		double		        corr_il1,
		double		        corr_il2,
		double		        corr_il3,
		double		        corr_im1,
		double		        corr_im2,
		double		        corr_im3,
		double	        **	dist_lr,
		double	        **	dist_mr,
        double              mse_lr,
        double              mse_mr,
        procat              pro_catg_val)
{
	FILE	    *	paf ;
	char		    pafname[FILENAMESZ] ;
	char	    *	mjd_obs ;
	char	    *	strvar ;
    instrument_t    ins ;
	
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

	sprintf(pafname, "%s.paf", get_rootname(outname)) ;
	paf = qfits_paf_print_header( pafname,
							"ISAAC/startrace",
							"Star trace recipe results",
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
            fprintf(paf, "INSTRUME \"%s\" \n", strvar) ;
        }
        /* TPL.ID  */
        strvar = pfits_get(ins, inimage_name, "templateid") ;
        if (strvar != NULL) {
            fprintf(paf, "TPL.ID  \"%s\" \n", strvar) ;
        }
        /* TPL.NEXP */
        strvar = pfits_get(ins, inimage_name, "numbexp") ;
        if (strvar != NULL) {
            fprintf(paf, "TPL.NEXP  %s \n", strvar) ;
        }
        /* DPR.CATG */
        strvar = pfits_get(ins, inimage_name, "dpr_catg") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.CATG  \"%s\" \n", strvar) ;
        }
        /* DPR.TYPE */
        strvar = pfits_get(ins, inimage_name, "dpr_type") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.TYPE  \"%s\" \n", strvar) ;
        }
        /* DPR.TECH */
        strvar = pfits_get(ins, inimage_name, "dpr_tech") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.TECH  \"%s\" \n", strvar) ;
        }
		/* Add PRO.CATG */
        fprintf(paf, "PRO.CATG \"%s\" ;# Product category\n",
                pfits_getprokey(ins, pro_catg_val)) ;
        /* Add the date */
        fprintf(paf, "DATE-OBS \"%s\" ;# Date\n",
                pfits_get(ins, inimage_name, "date_obs")) ;
		/* QC.CORR_IL1 */
		fprintf(paf, "QC.CORR_IL1  %g \n", corr_il1) ;
		/* QC.CORR_IL2 */
		fprintf(paf, "QC.CORR_IL2  %g \n", corr_il2) ;
		/* QC.CORR_IL3 */
		fprintf(paf, "QC.CORR_IL3  %g \n", corr_il3) ;
		/* QC.CORR_IM1 */
		fprintf(paf, "QC.CORR_IM1  %g \n", corr_im1) ;
		/* QC.CORR_IM2 */
		fprintf(paf, "QC.CORR_IM2  %g \n", corr_im2) ;
		/* QC.CORR_IM3 */
		fprintf(paf, "QC.CORR_IM3  %g \n", corr_im3) ;
		/* QC.DISTLR1 */
		fprintf(paf, "QC.DISTLR1  %g \n", dist_lr[2][0]) ;
		/* QC.DISTLRX */
		fprintf(paf, "QC.DISTLRX  %g \n", dist_lr[2][1]) ;
		/* QC.DISTLRY */
		fprintf(paf, "QC.DISTLRY  %g \n", dist_lr[2][2]) ;
		/* QC.DISTLRXY */
		fprintf(paf, "QC.DISTLRXY %g \n", dist_lr[2][3]) ;
		/* QC.DISTLRXX */
		fprintf(paf, "QC.DISTLRXX %g \n", dist_lr[2][4]) ;
		/* QC.DISTLRYY */
		fprintf(paf, "QC.DISTLRYY %g \n", dist_lr[2][5]) ;
		/* QC.DISTMR1  */
		fprintf(paf, "QC.DISTMR1  %g \n", dist_mr[2][0]) ;
		/* QC.DISTMRX  */
		fprintf(paf, "QC.DISTMRX  %g \n", dist_mr[2][1]) ;
		/* QC.DISTMRY  */
		fprintf(paf, "QC.DISTMRY  %g \n", dist_mr[2][2]) ;
		/* QC.DISTMRXY */
		fprintf(paf, "QC.DISTMRXY %g \n", dist_mr[2][3]) ;
		/* QC.DISTMRXX */
		fprintf(paf, "QC.DISTMRXX %g \n", dist_mr[2][4]) ;
		/* QC.DISTMRYY */
		fprintf(paf, "QC.DISTMRYY %g \n", dist_mr[2][5]) ;
		/* QC.FITMSELR */
		fprintf(paf, "QC.FITMSELR %g \n", mse_lr) ;
		/* QC.FITMSEMR */
		fprintf(paf, "QC.FITMSEMR %g \n", mse_mr) ;
	
		fclose(paf) ;
	}

	e_comment(2, "file [%s] produced", pafname) ;
	return 0 ;
}


