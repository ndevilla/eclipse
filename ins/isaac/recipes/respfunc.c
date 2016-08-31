/*----------------------------------------------------------------------------*/
/**
   @file    respfunc.c
   @author  Y. Jung
   @date    May 2002
   @version	$Revision: 1.9 $
   @brief   Response function
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: respfunc.c,v 1.9 2004/02/17 09:52:20 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/17 09:52:20 $
	$Revision: 1.9 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"
#include "irstd.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define F0_BAND_Z       2250
#define F0_BAND_SZ      1780
#define F0_BAND_J       1600
#define F0_BAND_H       1020
#define F0_BAND_K       657
#define F0_BAND_SL      252
#define F0_BAND_M       164

#define CENT_WL_BAND_Z  0.9
#define CENT_WL_BAND_SZ 1.06
#define CENT_WL_BAND_J  1.25
#define CENT_WL_BAND_H  1.65
#define CENT_WL_BAND_K  2.2
#define CENT_WL_BAND_SL 3.78
#define CENT_WL_BAND_M  4.78

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int respfunc_engine(char *, char *, int, int, int, int, int, int, 
		double, double, double, double, int, double, int) ;
static int respfunc_write_tables(char *, char *, int, int, char **, procat, 
        double **) ;

/*-----------------------------------------------------------------------------
                                Main 
 -----------------------------------------------------------------------------*/
int isaac_respfunc_main(void * dict)
{
	dictionary	*	d ;
		
	char		*	tmp_string ;
	
	int				spec_width ;
	int				sky_lo_dist,
					sky_hi_dist,
					sky_lo_width,
					sky_hi_width ;
	int				display ;
	double			disp_coef1,
					disp_coef2,
					disp_coef3,
					disp_coef4 ;
	int				filter ;
    double          magnitude ;
    int             temperature ;
	int				items ;
	
	char			argname[10] ;
	char    	*	name_i ;
    char    	*	name_o ;
    int     		nfiles ;

	int				errors ;
	int				i ;
	 
	d = (dictionary*)dict ;
   	/* Get options */
	spec_width   = dictionary_getint(d, "arg.width", 15) ;
	sky_lo_dist  = dictionary_getint(d, "arg.sky_dist_lo", 200) ;
	sky_hi_dist  = dictionary_getint(d, "arg.sky_dist_hi", 200) ;
	sky_lo_width = dictionary_getint(d, "arg.sky_width_lo", 20) ;
	sky_hi_width = dictionary_getint(d, "arg.sky_width_hi", 20) ;
	display      = dictionary_getint(d, "arg.display", 0) ;
	tmp_string   = dictionary_get(d, "arg.wavelength", NULL) ;
	if (tmp_string == NULL) {
		disp_coef1 = -1 ;
		disp_coef2 = -1 ;
		disp_coef3 = -1 ;
		disp_coef4 = -1 ;
	} else {
		items = sscanf(tmp_string, "%lg %lg %lg %lg", 
                &disp_coef1, &disp_coef2, &disp_coef3, &disp_coef4) ;
		if (items != 4) {
			disp_coef1 = -1 ;
			disp_coef2 = -1 ;
			disp_coef3 = -1 ;
			disp_coef4 = -1 ;
		}
	}
	filter       = dictionary_getint(d, "arg.filter", 0) ;	
    tmp_string   = dictionary_get(d, "arg.star_infos", NULL) ;
    if (tmp_string == NULL) {
        magnitude   = -1.0 ;
        temperature = -1 ;
    } else {
        items = sscanf(tmp_string, "%lg %d", &magnitude, &temperature) ;
        if (items != 2) {
            magnitude   = -1.0 ;
            temperature = -1 ;
        }
    }
	
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
		
		/* Call the main computing function. */
    	errors += respfunc_engine(name_i,
                            		name_o,
									spec_width,
									sky_lo_dist,
                        		    sky_hi_dist,
									sky_lo_width,
                        		    sky_hi_width,
                            		display,
                            		disp_coef1,
                            		disp_coef2,
                            		disp_coef3,
                            		disp_coef4,
                            		filter,
                                    magnitude,
                                    temperature) ;
		free(name_o) ;
	}
	return errors ;
}

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Flux calibration using standard star observation (see man page)
  @param	image_name		combined image created by {\it spjitter}
  @param	outname			out table name
  @param	spec_width	spectrum width
  @param	res_sky_lo_dist	distance spectrum-sky under the spectrum
  @param	res_sky_hi_dist	distance spectrum-sky above the spectrum
  @param	res_sky_lo_width	residual sky width (under the spec.)
  @param	res_sky_hi_width	residual sky width (above the spec.)
  @param	display	flag to use display mode
  @param	disp_coef1	first dispersion coefficient
  @param	disp_coef2	second dispersion coefficient
  @param	disp_coef3	third dispersion coefficient
  @param	disp_coef4	fourth dispersion coefficient
  @param	filter	flag to filter or not the image before extraction
  @param    mag     Magnitude of the star (-1 to use catalogs)
  @param    temp    Temperature of the star (-1 to use the catalogs)
  @return	-1 in error case 
 */
/*----------------------------------------------------------------------------*/
static int respfunc_engine(
		char	*	image_name,
		char	*	outname,
		int			spec_width,
		int			res_sky_lo_dist,
		int			res_sky_hi_dist,
		int			res_sky_lo_width,
		int			res_sky_hi_width,
		int			display,
		double		disp_coef1,
		double		disp_coef2,
		double		disp_coef3,
		double		disp_coef4,
		int			filter_flag,
        double      mag,
        int         temp)
{
	image_t			*	combined ;
	image_t			*	filtered ;
	image_t			*	extr_line ;
	double			*	disprel ;
	int					npoints ;
	int					slit_length ;
	double3			*	position ;
	int 				spec_pos ;
	int					low_side,
						up_side ;
	int					sky_pos[4] ;
	double			*	extracted ;
	double			*	wavelength ;	
	double			*	res_sky ;
	double			*   extr_corr ;
	double				median_1,
						median_2 ;
	pixelvalue			sky_estim ;
	char			*	sval ;
	double				ra,
						dec ;
	irstd			*	refstar ;
	double				dit ;
	isaac_filter_id		f_id;
	int					temperature ;
	double				magnitude ;
	double				cent_wl ;
	int					f0 ;
	double			*	conversion ;
	double			*	efficiency_curve ;
	double				avg_disp ;
	double			*	bb_flux_norm ;
	double				scaling ;
	double			*	bb_phot_norm ;
	double				factor ;
    ir_waveband         band ;
	
	char				name[FILENAMESZ] ;
	char			**	col_names ;
	double			**	out_table ;

    char            *   val ;
    procat              pro_effi,
                        pro_conv,
                        pro_extr,
                        pro_back ;
    instrument_t         ins ;                    

	/* Const */
	double				h ;
	double				k ;
	double				c ;
	double				surface ;

	int					i ;

	/* Const */
	h = 6.62e-34 ;
	k = 1.38e-23 ;
	c = 3e8 ;
	surface = PI_NUMB * 400 * 400 ;

    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;
    
    /* The PRO CATG keys written in output depend on the arm */
    /* Default */
    pro_effi = pro_conv = pro_extr = pro_back = procat_invalid ;
    if ((val = pfits_get(ins, image_name, "arm")) != NULL) {
        if (toupper(val[0])=='S') {
            pro_effi = procat_spec_sw_resp_effi ;
            pro_conv = procat_spec_sw_resp_conv ;
            pro_extr = procat_spec_sw_resp_extr ;
            pro_back = procat_spec_sw_resp_back ;
        } else if (toupper(val[0])=='L') {
            pro_effi = procat_spec_lw_resp_effi ;
            pro_conv = procat_spec_lw_resp_conv ;
            pro_extr = procat_spec_lw_resp_extr ;
            pro_back = procat_spec_lw_resp_back ;
        }
    }

	/* Load the input combined image */
	if ((combined=image_load(image_name)) == NULL) {
		e_error("cannot load combined image") ;
		return -1 ;
	}
	npoints = combined->lx ;
	slit_length = combined->ly ;

	/* Wavelength calibration */
	if (disp_coef1 + 1 > 1e-4) {
		/* The dispersion relation is given on the command line	 */
		disprel = malloc(4*sizeof(double)) ;
		disprel[0] = disp_coef1 ;
		disprel[1] = disp_coef2 ;
		disprel[2] = disp_coef3 ;
		disprel[3] = disp_coef4 ;
	} else if ((pfits_get(ins, image_name, "hist_disp1")!=NULL) 
			&& (pfits_get(ins, image_name, "hist_disp2")!=NULL) 
			&& (pfits_get(ins, image_name, "hist_disp3")!=NULL) 
			&& (pfits_get(ins, image_name, "hist_disp4")!=NULL) 
			&& ((double)atof(pfits_get(ins, image_name, "hist_disp1")
                    +strlen("DISPCOE1="))!=0.0)
			&& ((double)atof(pfits_get(ins, image_name, "hist_disp2")
                    +strlen("DISPCOE2="))!=0.0)
			&& ((double)atof(pfits_get(ins, image_name, "hist_disp3")
                    +strlen("DISPCOE3="))!=0.0)
			&& ((double)atof(pfits_get(ins, image_name, "hist_disp4")
                    +strlen("DISPCOE4="))!=0.0)) {
		/* Try to get the dispersion relation from the combined image header */
		disprel = malloc(4*sizeof(double)) ;
		disprel[0] = (double)atof(pfits_get(ins, image_name, "hist_disp1")
                +strlen("DISPCOE1=")) ;
		disprel[1] = (double)atof(pfits_get(ins, image_name, "hist_disp2")
                +strlen("DISPCOE2=")) ;
		disprel[2] = (double)atof(pfits_get(ins, image_name, "hist_disp3")
                +strlen("DISPCOE3=")) ;
		disprel[3] = (double)atof(pfits_get(ins, image_name, "hist_disp4")
                +strlen("DISPCOE4=")) ;
	} else {
		/* Calibrate with the physical model */
		if ((disprel=isaac_get_disprel_estimate(image_name, 3)) == NULL) {
			e_error("cannot compute the wavelength calibration") ;
			image_del(combined) ;
			return -1 ;
		}
	}
	
	/* Detect the brightest spectrum */
	if ((position=find_brightest_spectrum_1d(combined,
											0,
											NO_SHADOW_SPECTRUM,
											0)) == NULL) {
		e_error("no detected spectrum") ;
		image_del(combined) ;
		free(disprel) ;
		return -1 ;
	}
	spec_pos = (int)(position->y[0]) ;
	double3_del(position) ;
	
	/* Extract the spectrum */
	
	/* Extraction parameters */
	/* Spectrum position */
	low_side = (int)(spec_pos - (spec_width/2)) ;
	up_side  = low_side + spec_width ;
	if ((low_side < 1) || (up_side > combined->ly)) {
		e_error("spectrum position out of the image - aborting") ;
		image_del(combined) ;
		free(disprel) ;
		return -1 ;
	}
	/* Sky position */
	sky_pos[1] = (int)(spec_pos - res_sky_lo_dist) ;
	sky_pos[0] = (int)(sky_pos[1] - res_sky_lo_width) ;
	sky_pos[2] = (int)(spec_pos + res_sky_hi_dist) ;
	sky_pos[3] = (int)(sky_pos[2] + res_sky_hi_width) ;

	/* Allocate arrays */
	wavelength = malloc(npoints * sizeof(double)) ;
	extracted  = malloc(npoints * sizeof(double)) ;
	res_sky    = malloc(npoints * sizeof(double)) ;
	extr_corr  = malloc(npoints * sizeof(double)) ;

	if (filter_flag == 1) {
		/* Filter the combined image */
		if ((filtered=image_filter_median(combined)) == NULL) {
			e_warning("cannot filter the combined image") ;
			filtered = image_copy(combined) ;
		} else {
			e_comment(1, "filter image before extraction") ;
		}
	} else {
		filtered = image_copy(combined) ;
	}
	image_del(combined) ;

	/* Extract the spectrum and get rid of the residual sky */
	for (i=0 ; i<npoints ; i++) {
		/* Estimate the SKY */
		if (((sky_pos[0] < 1) || (res_sky_lo_width == 0)) &&
			((sky_pos[3] <= slit_length) && (res_sky_hi_width > 0))) {
			median_1 = image_getmedian_vig(filtered,
                                                i+1,
                                                sky_pos[2],
                                                i+1,
                                                sky_pos[3]) ;
			sky_estim = median_1 ;
		} else if (((sky_pos[3] > slit_length) || (res_sky_hi_width == 0)) &&
			((sky_pos[0] > 0) && (res_sky_lo_width > 0))) {
	
			
			
			median_1 = image_getmedian_vig(filtered,
												i+1,
												sky_pos[0],
												i+1,
												sky_pos[1]) ;
			sky_estim = median_1 ;
		} else if ((res_sky_lo_width != 0)
				&& (res_sky_hi_width != 0)
				&& (sky_pos[0] > 0)
				&& (sky_pos[3] <= slit_length)) {
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
		/* Estimate the spectrum         */
		if ((extr_line = image_getvig(filtered,
												i+1,
												low_side,
												i+1,
												up_side)) == NULL) {
			e_error("error in line extraction - aborting") ;
			image_del(filtered) ;
			free(disprel) ;
			return -1 ;
		}
		
		extracted[i] = (double)image_getsumpix(extr_line) ;
		image_del(extr_line);
		res_sky[i] = (double)(sky_estim * spec_width) ;
		extr_corr[i] = extracted[i] - res_sky[i] ;
		wavelength[i] = (double)(disprel[0] + disprel[1]*(i+1) + 
                disprel[2]*(i+1)*(i+1) + disprel[3]*(i+1)*(i+1)*(i+1)) ;
		
	}
	avg_disp=(wavelength[npoints-1] - wavelength[0])/npoints ;
	image_del(filtered) ;
	free(disprel) ;
	free(extracted) ;

	/* Plot the spectrum */
	if (display) {
		gnuplot_plot_once("Extracted spectrum",
						  "lines",
						  "wavelength",
						  "spectrum",
						  wavelength,
						  extr_corr,
						  npoints);
	}
	
	/* Write the extracted spectrum in FITS table */
	sprintf(name, "%s_extr.tfits", outname) ;
	col_names = malloc(2*sizeof(char*)) ;
	col_names[0] = malloc(FILENAMESZ * sizeof(char)) ;
	col_names[1] = malloc(FILENAMESZ * sizeof(char)) ;
	sprintf(col_names[0], "Wavelength") ;
	sprintf(col_names[1], "Extracted_spec") ;
	out_table = malloc(2*sizeof(double*)) ;
	out_table[0] = wavelength ;
	out_table[1] = extr_corr ;
	if (respfunc_write_tables(image_name,
							name,
							npoints,
							2,
							col_names,
							pro_extr,
							out_table) == -1) {
		e_warning("cannot write the extraction table") ;
	}
	free(col_names[0]) ;
	free(col_names[1]) ;
	free(col_names) ;
	free(out_table) ;	

	/* Plot the background */
	if (display) {
		gnuplot_plot_once("Sky Background",
						  "lines",
						  "wavelength",
						  "background",
						  wavelength,
						  res_sky,
						  npoints);
	}
	
	/* Write the background in FITS table */
	sprintf(name, "%s_back.tfits", outname) ;
	col_names = malloc(2*sizeof(char*)) ;
	col_names[0] = malloc(FILENAMESZ * sizeof(char)) ;
	col_names[1] = malloc(FILENAMESZ * sizeof(char)) ;
	sprintf(col_names[0], "Wavelength") ;
	sprintf(col_names[1], "Background") ;
	out_table = malloc(2*sizeof(double*)) ;
	out_table[0] = wavelength ;
	out_table[1] = res_sky ;
	if (respfunc_write_tables(image_name,
							name,
							npoints,
							2,
							col_names,
							pro_back,
							out_table) == -1) {
		e_warning("cannot write the background table") ;
	}
	free(col_names[0]) ;
    free(col_names[1]) ;
    free(col_names) ;
    free(out_table) ;
	free(res_sky) ;
	
	/* Get the used filter  */
	if ((sval=pfits_get(ins, image_name, "filter")) == NULL) {
		e_error("cannot get filter from file [%s]", image_name) ;
		free(wavelength) ;
		free(extr_corr) ;
		return -1 ;
	}
	f_id = isaac_get_filterid(sval);

	/* Get the DIT */
	if ((sval=pfits_get(ins, image_name, "dit"))== NULL) {
		e_error("cannot get dit from file [%s]", image_name) ;
		free(wavelength) ;
		free(extr_corr) ;
		return -1 ;
	}
	dit = (double)atof(sval) ;

    /* Get star magnitude and temperature */
    if ((mag > 0) && (temp > 0)) {
        /* Either magnitude and temperature are provided ... */
        e_comment(2, "Use user provided magnitude (%g) and temperature (%d)",
                mag, temp);
        magnitude = mag ;
        temperature = temp ;
    } else {
        /* ... or they are read from the database */
        /* Identify standard star */
            
        /* Find RA and DEC */
        if ((sval=pfits_get(ins, image_name, "ra")) == NULL) {
            e_error("cannot get RA from header") ;
            free(wavelength) ;
            free(extr_corr) ;
            return -1 ;
        }
        ra = (double)atof(sval);
        
        if ((sval=pfits_get(ins, image_name, "dec")) == NULL) {
            e_error("cannot get DEC from header") ;
            free(wavelength) ;
            free(extr_corr) ;
            return -1 ;
        }
        dec = (double)atof(sval) ;
       
        /* Find the closest standard star whose magnitude is known */
        e_comment(2, "getting standard star from database...");
      
        /* Different cases according filter */
        switch (isaac_associate_filter(f_id)) {
            case isaac_filter_z:  
            case isaac_filter_sz: 
            case isaac_filter_j:  
            case isaac_filter_jblock:  
                band = WAVEBAND_J ;
                break ;
            case isaac_filter_sh: 
                band = WAVEBAND_H ;
                break ;
            case isaac_filter_sk: 
                band = WAVEBAND_K ;
                break ;
            case isaac_filter_sl: 
                band = WAVEBAND_L ;
                break ;
            case isaac_filter_m:  
                band = WAVEBAND_M ;
                break ;
            default:
            e_error("unsupported band : [%s]", isaac_get_filtername(f_id)) ;
            free(wavelength) ;
            free(extr_corr) ;
            return -1 ;
        }
        
        if ((refstar=irstd_get_star_magnitude(ra, dec, band, 
                        &magnitude)) == NULL) {
            e_error("standard star not found") ;
            free(wavelength) ;
            free(extr_corr) ;
            return -1 ;
        }

        /* Get the star temperature */
        if ((temperature=
                    irstd_get_star_temperature((char*)refstar->sptype))==-1) {
            e_error("cannot get the star temperature") ;
            free(wavelength) ;
            free(extr_corr) ;
            return -1 ;
        }
    }
    
	/* Different cases according filter */
	switch (isaac_associate_filter(f_id)) {
		case isaac_filter_z:
		f0        = F0_BAND_Z ;
		cent_wl   = CENT_WL_BAND_Z ;
		break ;

		case isaac_filter_sz:
		f0        = F0_BAND_SZ ;
		cent_wl   = CENT_WL_BAND_SZ ;
		break ;

		case isaac_filter_j:
		case isaac_filter_jblock:
		f0        = F0_BAND_J ;
		cent_wl   = CENT_WL_BAND_J ;
		break ;

		case isaac_filter_sh:
		f0        = F0_BAND_H ;
		cent_wl   = CENT_WL_BAND_H ;
		break ;

		case isaac_filter_sk:
		f0        = F0_BAND_K ;
		cent_wl   = CENT_WL_BAND_K ;
		break ;

        case isaac_filter_sl:
        f0        = F0_BAND_SL ;
        cent_wl   = CENT_WL_BAND_SL ;
        break ;

        case isaac_filter_m:
        f0        = F0_BAND_M ;
        cent_wl   = CENT_WL_BAND_M ;
        break ;

		default:
		e_error("unsupported band : [%s]", isaac_get_filtername(f_id)) ;
		free(wavelength) ;
		free(extr_corr) ;
		return -1 ;
	}

	/* Set factor and scaling */
	scaling = 3e-13 * dit * avg_disp * f0 * pow(10,(-magnitude/2.5)) /
		pow(cent_wl,2) ; 
			
	factor = (3e-22 / (h*c)) * dit * surface * avg_disp * f0 * 
		pow(10,(-magnitude/2.5)) / (1e4 * cent_wl) ;
	
	/* Allocate conversion and efficiency curve */
	conversion = malloc(npoints * sizeof(double)) ;
	efficiency_curve = malloc(npoints * sizeof(double)) ;
	bb_flux_norm = malloc(npoints * sizeof(double)) ;
	bb_phot_norm = malloc(npoints * sizeof(double)) ;
	
	for (i=0 ; i<npoints ; i++) {
		bb_flux_norm[i] = pow(cent_wl*1e-6,5) * 
            (exp(h*c/(cent_wl*1e-6*k*temperature))-1) /
			(pow(wavelength[i]*1e-10,5) *
			 (exp(h*c/(wavelength[i]*1e-10*k*temperature))-1)) ;
			 
		bb_phot_norm[i] = pow(cent_wl*1e-6,4) * 
            (exp(h*c/(cent_wl*1e-6*k*temperature))-1) /
			(pow(wavelength[i]*1e-10,4) *
             (exp(h*c/(wavelength[i]*1e-10*k*temperature))-1)) ;
		
		conversion[i] = extr_corr[i] / (bb_flux_norm[i] * scaling) ; 
		efficiency_curve[i] = extr_corr[i] / (bb_phot_norm[i] * factor) ;
	}
	free(bb_flux_norm) ;
	free(bb_phot_norm) ;
	free(extr_corr) ;

	/* Plot the conversion file */
	if (display) {
		gnuplot_plot_once("Conversion file",
						  "lines",
						  "wavelength",
						  "conversion",
						  wavelength,
						  conversion,
						  npoints);
	}
	
	/* Write the conversion function in FITS table */
	sprintf(name, "%s_conversion.tfits", outname) ;
	col_names = malloc(2*sizeof(char*)) ;
	col_names[0] = malloc(FILENAMESZ * sizeof(char)) ;
	col_names[1] = malloc(FILENAMESZ * sizeof(char)) ;
	sprintf(col_names[0], "Wavelength") ;
	sprintf(col_names[1], "Conversion") ;
	out_table = malloc(2*sizeof(double*)) ;
	out_table[0] = wavelength ;
	out_table[1] = conversion ;
	if (respfunc_write_tables(image_name,
							name,
							npoints,
							2,
							col_names,
							pro_conv,
							out_table) == -1) {
		e_warning("cannot write the conversion table") ;
	}
	free(col_names[0]) ;
    free(col_names[1]) ;
    free(col_names) ;
    free(out_table) ;
	free(conversion) ;

	/* Plot the efficiency curve */
	if (display) {
		gnuplot_plot_once("Efficiency curve",
						  "lines",
						  "wavelength",
						  "efficiency",
						  wavelength,
						  efficiency_curve,
						  npoints);
	}
	
	/* Write the efficiency curve in FITS table */
	sprintf(name, "%s_efficiency.tfits", outname) ;
	col_names = malloc(2*sizeof(char*)) ;
	col_names[0] = malloc(FILENAMESZ * sizeof(char)) ;
	col_names[1] = malloc(FILENAMESZ * sizeof(char)) ;
	sprintf(col_names[0], "Wavelength") ;
	sprintf(col_names[1], "Efficiency") ;
	out_table = malloc(2*sizeof(double*)) ;
	out_table[0] = wavelength ;
	out_table[1] = efficiency_curve ;
	if (respfunc_write_tables(image_name,
							name,
							npoints,
							2,
							col_names,
							pro_effi,
							out_table) == -1) {
		e_warning("cannot write the background table") ;
	}
	free(col_names[0]) ;
    free(col_names[1]) ;
    free(col_names) ;
    free(out_table) ;
	free(efficiency_curve) ;
	
	/* Free and return */
	free(wavelength) ;
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Write the out FITS table
  @param	infilename	input file name
  @param	outname		output name
  @param	nb_lines	number of lines
  @param	nb_col		number of columns
  @param	col_labs	columns titles
  @param	key			key to the PRO CATG keyword 
  @param	out_table	data to write in table
  @return	-1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int respfunc_write_tables(
		char			*	infilename,
        char    		*   outname,
        int     		    nb_lines,
        int     		    nb_col,
        char    		**  col_labs,
        procat      		key,
        double  		**  out_table)
{
	framelist		*	lnames ;
	
	qfits_header	*	fh ;
    qfits_table	    *   table ;
    qfits_col       *   col ;

    int                 i ;

    /* Write the output qfits_table table (informations) */
	table = qfits_table_new(outname, QFITS_BINTABLE, -1, nb_col, nb_lines) ;
    col = table->col ;
    for (i=0 ; i<table->nc ; i++) {
		qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, col_labs[i],
                " ", " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
        col++ ;
    }

	/* WRITE THE OUTPUT FITS TABLE */
	/* Read the input header */
    if ((fh = qfits_header_read(infilename)) == NULL) {
        e_error("in writing the output fits file") ;
        qfits_table_close(table) ;
        return -1 ;
    }

    /* Prepare it for table output */
	if (isaac_header_for_table(fh) == -1) {
        e_error("in writing the output fits file") ;
        qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
        return -1 ;
    }

	/* Create framelist */
	lnames = framelist_new(1) ;
	lnames->name[0] = strdup(infilename) ;
	
	/* Write the PRO keywords in the header */
    if (isaac_pro_fits(fh,
                        outname,
                        "REDUCED",
                        NULL,
                        key,
                        "OK",
                        "spec_tec_resp",
                        1,
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
	qfits_table_close(table) ;
    qfits_header_destroy(fh) ;

	e_comment(0, "File [%s] produced", outname) ;
    return 0 ;
}



