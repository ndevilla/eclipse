/*----------------------------------------------------------------------------*/
/**
   @file    rename.c
   @author  Y. Jung
   @date    May 2002
   @version	$Revision: 1.13 $
   @brief   ISAAC renaming recipe
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: rename.c,v 1.13 2004/02/09 15:18:46 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 15:18:46 $
	$Revision: 1.13 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"
#include "irstd.h"
#include "calendar.h"
#include "pfits.h"
/* Statistics on files */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define ISAAC_CHANGE_DAY_HOUR   18

#define RENAME_LOW_RES          1000
#define RENAME_MED_RES          1001

#define RENAME_TECH_IMA         1002
#define RENAME_TECH_SPEC        1003

#define RENAME_TYPE_STD         1004
#define RENAME_TYPE_OBS         1005

#define RENAME_CORRELATED       1006
#define RENAME_UNCORRELATED     1007

#define RENAME_A_ASCII_DECIMAL  65
#define RENAME_NB_OF_LETTERS    26

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int rename_engine(char *, int, int, char *) ;
static char * rename_compute_product_name(char *, int, int, char *) ;

static int rename_get_resolution(char * filename) ;
static int rename_get_dpr_tech(char * filename) ;
static int rename_get_dpr_type(char * filename) ;

static char * rename_get_dit(char * filename) ;
static char * rename_get_filter(char * filename) ;
static char * rename_get_central_wavelength(char * filename) ;
static char * rename_get_optical_id(char * filename) ;
static char * rename_get_obs_id(char * filename) ;
static char * rename_get_target(char * filename) ;
static int rename_dark_romode(char * filename) ;

static char * rename_compose_name(char *, char *, char *, char *) ;

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int isaac_rename_main(void * dict)
{
	dictionary	*	d ;
	
    int             archive ;
    int             origfile ;
    char        *   ref_name ;
	char			argname[10] ;
	char    	*	name_i ;
    int     		nfiles ;
	
	int				errors ;
	int				i ;
	 
	d = (dictionary*)dict ;
   
    /* Get options */
    archive = dictionary_getint(d, "arg.archive", 0) ;
    origfile = dictionary_getint(d, "arg.origfile", 0) ;
    ref_name = dictionary_get(d, "arg.ref_name", NULL) ;

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
		
		/* Call the main computing function. */
    	errors += rename_engine(name_i, archive, origfile, ref_name) ;
	}
	return errors ;
}

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/
static int rename_engine(
        char    *   in,
        int         archive,
        int         origfile,
        char    *   ref_name)
{
    char        *   new_name ;
    char            line[81] ;
    
    /* Test inputs */
    if (ref_name != NULL) {
        if ((archive==1) || (origfile==1)) {
            e_error("ARCHIVE or ORIGFILE opt cannot be used with REF") ;
            return -1 ;
        }
    }

    if ((archive==1) && (origfile==1)) {
        e_error("ARCHIVE and ORIGFILE options cannot be used together") ;
        return -1 ;
    }

    /* Find the new name */
    if ((new_name = rename_compute_product_name(in, 
                    archive, 
                    origfile, 
                    ref_name)) == NULL) {
        e_warning("File %s not renamed", in) ;
        return -1 ;
    }

    /* Rename the file */
    e_comment(0, "%35s RENAMED IN %s", in, new_name) ;
    if (rename(in, new_name)) {
        e_comment(1, "cannot rename %s in %s", in, new_name) ;
        return -1 ;
    }

    /* Update PIPEFILE keyword in the renamed file if it is FITS */
    if (is_fits_file(new_name)) {
        keytuple2str(line, "PIPEFILE", get_basename(new_name), 
                "pipeline filename") ;
        qfits_replace_card(new_name, "PIPEFILE", line) ;
    }

    /* Free and return */
    free(new_name) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out the new file name by checking the header
  @param    filename    old file name
  @param	archive		Flag to use the archive name for renaming
  @param	origfile	Flag to use the origfile for renaming
  @param	ref_name	File used in option to compute a new_name (optional)
  @return   The new file name
  The rules used to compute the new file name are implemented here.
 */
/*----------------------------------------------------------------------------*/
static char * rename_compute_product_name(
		char 	* 	filename, 
		int 		archive,
		int			origfile,
		char	*	ref_name)
{
	char		        reffile[FILENAMESZ] ;
    char   	        *   pro_catg ;
    instrument_t        ins ;
    char   	        *   date ;
    char   		        valid_date[6] ;
    char   		        tmp_year[4] ;
    int    		        hour,
                        day,
            	        month,
            	        year ;
	int			        res ;			
	int			        dpr_tech ;
	int			        dpr_type ;
	char	        *	dit ;
	char	        *	filter ;
	char	        *	target ;
	char	        *	obs_id ;
	char	        *	optical_id ;
	char	        *	wl ;
	char	        *	lamp ;
    char                first[FILENAMESZ] ;
    char                second[FILENAMESZ] ;
	char		        extension[5] ;
    char            *	final_name ;
    char            *   sval ;

    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;

	/* Which file is used to compute the new name */
    if (ref_name == NULL) strcpy(reffile, filename) ;
    else strcpy(reffile, ref_name) ;

    /* Test the reference file */
	if ((!qfits_is_paf_file(reffile)) && (!is_fits_file(reffile))) {
		e_error("The reference file has to be FITS or PAF") ;
		return NULL ;
	}
	
	e_comment(0, "Find the new name of: %s", filename) ;

	/* Use Arcfile */
	if (archive) {
		final_name = pfits_get(ins, reffile, "arcfile") ;
		if (final_name == NULL) 
			e_error("cannot read ARCFILE from the header") ;
		else final_name = strdup(final_name) ;
		return final_name ;
	}

	/* Use Origfile */
	if (origfile) {
		final_name = pfits_get(ins, reffile, "origfile") ;
		if (final_name == NULL) 
			e_error("cannot read ORIGFILE from the header") ;
		else final_name = strdup(final_name) ;
		return final_name ;
	}
	
    /* Read the date */
    if ((date = pfits_get(ins, reffile, "date_obs")) == NULL) {
        e_error("cannot read DATE-OBS keyword in the header - abort") ;
        return NULL ;
    }
    if (sscanf(date, "%4d-%2d-%2dT%2d:%*s", &year, &month, &day, &hour) == 0) {
		e_error("cannot parse the DATE-OBS keyword - abort") ;
		return NULL ;
	}
	
    /* Test if the valid date is yesterday or today */
    if (hour < ISAAC_CHANGE_DAY_HOUR) calendar_getprev(&day, &month, &year) ;

    /* Get only 2 digits for the year: 2001 -> 01 */
    sprintf(tmp_year, "%04d", year) ;
    sscanf(tmp_year, "%*2d%2d", &year) ;

    /* Write down the valid date */
    sprintf(valid_date, "%02d%02d%02d", year, month, day) ;

    /* GET THE FIRST PART OF THE NEW NAME */
    
	/* Get the PRO CATG value of the product */
    if ((pro_catg = pfits_get(ins, reffile, "pro_catg")) == NULL) {
        e_error("cannot read PRO CATG keyword in the header - abort") ;
        return NULL ;
    }
    switch (pfits_getprocat(ins, pro_catg)) {
        case procat_spec_sw_arc_coef:
        case procat_spec_sw_arc_qc:
            res = rename_get_resolution(reffile) ;
			if (res == RENAME_LOW_RES) 
				sprintf(first, "IS_SSAL_%6s", valid_date) ;
            else if (res == RENAME_MED_RES) 
				sprintf(first, "IS_SSAM_%6s", valid_date) ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_spec_lw_arc_coef:
        case procat_spec_lw_arc_qc:
            res = rename_get_resolution(reffile) ;
			if (res == RENAME_LOW_RES) 
				sprintf(first, "IS_LSAL_%6s", valid_date) ;
            else if (res == RENAME_MED_RES) 
				sprintf(first, "IS_LSAM_%6s", valid_date) ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_dark_result:
        case procat_dark_ron:
            sval = pfits_get(ins, reffile, "arm");
            if (sval==NULL) {
                e_error("cannot determine SW/LW for %s", reffile);
                return NULL ;
            }
			switch (toupper(sval[0])) {

                /* SW mode */
				case 'S':
					dpr_tech = rename_get_dpr_tech(reffile) ;
					if (dpr_tech == RENAME_TECH_IMA) 
						sprintf(first, "IS_SIDK_%6s", valid_date) ;
					else if (dpr_tech == RENAME_TECH_SPEC) 
						sprintf(first, "IS_SSDK_%6s", valid_date) ;
					else {
						e_error("DPR TECH keyword not recognized") ;
						return NULL ;
					}
					break ;

                /* LW mode */
				case 'L':
					switch (rename_dark_romode(reffile)) {
						case RENAME_CORRELATED:
						sprintf(first, "IS_LGDD_%s", valid_date) ;
						break ;

						case RENAME_UNCORRELATED:
						sprintf(first, "IS_LGDU_%s", valid_date) ;
						break ;

						default:
						return NULL ;
						break ;
					}
					break ;
				default:
					e_error("cannot recognize the mode") ;
					return NULL ;
			}
			break ;
        case procat_spec_sw_flat:
        case procat_spec_sw_flat_qc:
			res = rename_get_resolution(reffile) ;
            if (res == RENAME_LOW_RES) 
				sprintf(first, "IS_SSFL_%6s", valid_date) ;
            else if (res == RENAME_MED_RES) 
				sprintf(first, "IS_SSFM_%6s", valid_date) ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_spec_lw_flat:
        case procat_spec_lw_flat_qc:
			res = rename_get_resolution(reffile) ;
            if (res == RENAME_LOW_RES) 
				sprintf(first, "IS_LSFL_%6s", valid_date) ;
            else if (res == RENAME_MED_RES) 
				sprintf(first, "IS_LSFM_%6s", valid_date) ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_imag_illum:
            sprintf(first, "IS_SIIL_%6s", valid_date) ;
            break ;
        case procat_imag_sw_jitter_result:
        case procat_imag_jitter_qc:
            sprintf(first, "IS_SIJC_%6s", valid_date) ;
            break ;
        case procat_spec_sw_resp_extr:
        case procat_spec_sw_resp_back:
        case procat_spec_sw_resp_conv:
        case procat_spec_sw_resp_effi:
			res = rename_get_resolution(reffile) ;
            if (res == RENAME_LOW_RES) 
				sprintf(first, "IS_SSRL_%6s", valid_date) ;
            else if (res == RENAME_MED_RES) 
				sprintf(first, "IS_SSRM_%6s", valid_date) ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_spec_lw_resp_extr:
        case procat_spec_lw_resp_back:
        case procat_spec_lw_resp_conv:
        case procat_spec_lw_resp_effi:
			res = rename_get_resolution(reffile) ;
            if (res == RENAME_LOW_RES) 
				sprintf(first, "IS_LSRL_%6s", valid_date) ;
            else if (res == RENAME_MED_RES) 
				sprintf(first, "IS_LSRM_%6s", valid_date) ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_spec_sw_jitter_comb:
        case procat_spec_sw_jitter_extr:
        case procat_spec_sw_jitter_qc:
            res = rename_get_resolution(reffile) ;
			dpr_type = rename_get_dpr_type(reffile) ;
			if (dpr_type == RENAME_TYPE_STD) {
                if (res == RENAME_LOW_RES) 
					sprintf(first, "IS_SSSL_%6s", valid_date) ;
                else if (res == RENAME_MED_RES) 
					sprintf(first, "IS_SSSM_%6s", valid_date) ;
                else {
                    e_error("Resolution not recognized") ;
                    return NULL ;
                }
            } else if (dpr_type == RENAME_TYPE_OBS) {
                if (res == RENAME_LOW_RES) 
					sprintf(first, "IS_SSCL_%6s", valid_date) ;
                else if (res == RENAME_MED_RES) 
					sprintf(first, "IS_SSCM_%6s", valid_date) ;
                else {
                    e_error("Resolution not recognized") ;
                    return NULL ;
                }
            } else {
                e_error("Observation type not recognized") ;
                return NULL ;
            }
            break ;
        case procat_spec_lw_jitter_comb:
        case procat_spec_lw_jitter_extr:
        case procat_spec_lw_jitter_qc:
            res = rename_get_resolution(reffile) ;
			dpr_type = rename_get_dpr_type(reffile) ;
			if (dpr_type == RENAME_TYPE_STD) {
                if (res == RENAME_LOW_RES) 
					sprintf(first, "IS_LSSL_%6s", valid_date) ;
                else if (res == RENAME_MED_RES) 
					sprintf(first, "IS_LSSM_%6s", valid_date) ;
                else {
                    e_error("Resolution not recognized") ;
                    return NULL ;
                }
            } else if (dpr_type == RENAME_TYPE_OBS) {
                if (res == RENAME_LOW_RES) 
					sprintf(first, "IS_LSCL_%6s", valid_date) ;
                else if (res == RENAME_MED_RES) 
					sprintf(first, "IS_LSCM_%6s", valid_date) ;
                else {
                    e_error("Resolution not recognized") ;
                    return NULL ;
                }
            } else {
                e_error("Observation type not recognized") ;
                return NULL ;
            }
            break ;
        case procat_imag_lw_jitter_result:
			sprintf(first, "IS_LIJC_%6s", valid_date) ;
			break ;
        case procat_spec_sw_sttr_pos:
        case procat_spec_sw_sttr_shape:
        case procat_spec_sw_sttr_corresp:
        case procat_spec_sw_sttr_disto:
        case procat_spec_sw_sttr_extract:
        case procat_spec_sw_sttr_qc:
            sprintf(first, "IS_SSST_%6s", valid_date) ;
            break ;
        case procat_spec_lw_sttr_pos:
        case procat_spec_lw_sttr_shape:
        case procat_spec_lw_sttr_corresp:
        case procat_spec_lw_sttr_disto:
        case procat_spec_lw_sttr_extract:
        case procat_spec_lw_sttr_qc:
            sprintf(first, "IS_LSST_%6s", valid_date) ;
			break ;
        case procat_imag_sw_flat_result:
        case procat_imag_sw_flat_badpix:
        case procat_imag_sw_flat_interce:
        case procat_imag_sw_flat_errmap:
            sprintf(first, "IS_SITF_%6s", valid_date) ;
            break ;
        case procat_imag_zpoint_result:
        case procat_imag_zpoint_qc:
			sprintf(first, "IS_GIZP_%6s", valid_date) ;
			break ;
        case procat_imag_bg:
        case procat_spec_sw_arc_corr:
        case procat_spec_lw_arc_corr:
        case procat_imag_sw_jitter_diff:
        case procat_spec_slitpos_table:
        case procat_spec_slitpos_qc:
        case procat_spec_sw_sttr_correct:
        case procat_spec_lw_sttr_correct:
        	e_comment(1, "File registered but not supported") ;
			return NULL ;
			break ;
		default:
            e_error("PRO CATG key not recognized") ;
            return NULL ;
            break ;
    }

    /* GET THE SECOND PART OF THE NEW NAME */

	/* Get the PRO CATG value of the product */
    if ((pro_catg = pfits_get(ins, reffile, "pro_catg")) == NULL) {
        e_error("cannot read PRO CATG keyword in the header - abort") ;
        return NULL ;
    }
    switch (pfits_getprocat(ins, pro_catg)) {
        case procat_spec_sw_arc_coef:
        case procat_spec_sw_arc_qc:
        case procat_spec_lw_arc_coef:
        case procat_spec_lw_arc_qc:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
            if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
                free(filter) ;
                return NULL ;
            }
            if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
                free(filter) ;
                free(optical_id) ;
                return NULL ;
            }
			if ((lamp = pfits_get(ins, reffile, "pro_catalog")) == NULL) {
				free(filter) ;
				free(optical_id) ;
				free(wl) ;
				return NULL ;
			}
            sprintf(second, "_%s_%s_%s_%s", filter, optical_id, wl, lamp) ;
			free(filter) ;
            free(optical_id) ;
            free(wl) ;
			break ;
        case procat_dark_result:
			if ((dit = rename_get_dit(reffile)) == NULL) return NULL ;
            sprintf(second, "_DIT=%s", dit) ;
			free(dit) ;
            break ;
        case procat_dark_ron:
			sprintf(second, "_RON") ;
			break ;
        case procat_spec_sw_flat:
        case procat_spec_sw_flat_qc:
        case procat_spec_lw_flat:
        case procat_spec_lw_flat_qc:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
				free(filter) ;
				return NULL ;
			}
			if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
				free(filter) ;
				free(optical_id) ;
				return NULL ;
			}
			sprintf(second, "_%s_%s_%s", filter, optical_id, wl) ;
			free(filter) ;
			free(optical_id) ;
			free(wl) ;
            break ;
        case procat_imag_illum:
            if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			sprintf(second, "_%s", filter) ;
			free(filter) ;
            break ;
        case procat_imag_sw_jitter_result:
        case procat_imag_lw_jitter_result:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			if ((obs_id = rename_get_obs_id(reffile)) == NULL) {
				free(filter) ;
				return NULL ;
			}
            sprintf(second, "_%s_%s", filter, obs_id) ;
			free(filter) ;
			free(obs_id) ;
            break ;
        case procat_imag_jitter_qc:
        	if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			if ((obs_id = rename_get_obs_id(reffile)) == NULL) {
				free(filter) ;
				return NULL ;
			}
            sprintf(second, "_%s_%s_qc", filter, obs_id) ;
			free(filter) ;
			free(obs_id) ;
			break ;
        case procat_spec_sw_resp_extr:
        case procat_spec_lw_resp_extr:
            if ((target = rename_get_target(reffile)) == NULL) return NULL ;
			if ((filter = rename_get_filter(reffile)) == NULL) {
				free(target) ;
				return NULL ;
			}
			if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				return NULL ;
			}
			if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				free(optical_id) ;
				return NULL ;
			}
			sprintf(second, "_%s_%s_%s_%s_extr", target,filter,optical_id,wl);
			free(target) ;
			free(filter) ;
			free(optical_id) ;
			free(wl) ;
			break ;
        case procat_spec_sw_resp_back:
        case procat_spec_lw_resp_back:
            if ((target = rename_get_target(reffile)) == NULL) return NULL ;
            if ((filter = rename_get_filter(reffile)) == NULL) {
				free(target) ;
				return NULL ;
			}
			if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				return NULL ;
			}
			if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				free(optical_id) ;
				return NULL ;
			}
			sprintf(second, "_%s_%s_%s_%s_back",target,filter,optical_id,wl);
			free(target) ;
			free(filter) ;
			free(optical_id) ;
			free(wl) ;
            break ;
        case procat_spec_sw_resp_conv:
        case procat_spec_lw_resp_conv:
            if ((target = rename_get_target(reffile)) == NULL) return NULL ;
            if ((filter = rename_get_filter(reffile)) == NULL) {
				free(target) ;
				return NULL ;
			}
			if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				return NULL ;
			}
			if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				free(optical_id) ;
				return NULL ;
			}
			sprintf(second, "_%s_%s_%s_%s_conversion", target, filter, 
					optical_id, wl) ;
			free(target) ;
			free(filter) ;
			free(optical_id) ;
			free(wl) ;
            break ;
        case procat_spec_sw_resp_effi:
        case procat_spec_lw_resp_effi:
            if ((target = rename_get_target(reffile)) == NULL) return NULL ;
            if ((filter = rename_get_filter(reffile)) == NULL) {
				free(target) ;
				return NULL ;
			}
			if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				return NULL ;
			}
			if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
				free(target) ;
				free(filter) ;
				free(optical_id) ;
				return NULL ;
			}
			sprintf(second, "_%s_%s_%s_%s_efficiency", target, filter, 
					optical_id, wl) ;
			free(target) ;
			free(filter) ;
			free(optical_id) ;
			free(wl) ;
            break ;
        case procat_spec_sw_jitter_comb:
        case procat_spec_sw_jitter_extr:
        case procat_spec_sw_jitter_qc:
        case procat_spec_lw_jitter_comb:
        case procat_spec_lw_jitter_extr:
		case procat_spec_lw_jitter_qc:
            dpr_type = rename_get_dpr_type (reffile) ;
			if (dpr_type == RENAME_TYPE_STD) {
				if ((target = rename_get_target(reffile))==NULL) return NULL ;
				sprintf(second, "_%s", target) ;
				free(target) ;
			} else second[0] = (char)0 ;   
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			if ((optical_id = rename_get_optical_id(reffile)) == NULL) {
				free(filter) ;
				return NULL ;
			}
			if ((wl = rename_get_central_wavelength(reffile)) == NULL) {
				free(filter) ;
				free(optical_id) ;
				return NULL ;
			}
			if ((obs_id = rename_get_obs_id(reffile)) == NULL) {
				free(filter) ;
				free(optical_id) ;
				free(wl) ;
				return NULL ;
			}
			sprintf(second, "%s_%s_%s_%s_%s", second, filter, optical_id, wl,
					obs_id) ;
			free(filter) ;
			free(optical_id) ;
			free(wl) ;
			free(obs_id) ;
            break ;
        case procat_spec_sw_sttr_pos:
        case procat_spec_lw_sttr_pos:
            sprintf(second, "_positions") ;
            break ;
		case procat_spec_sw_sttr_qc:
		case procat_spec_lw_sttr_qc:
			sprintf(second, "_qc") ;
			break ;
		case procat_spec_sw_sttr_shape:
        case procat_spec_lw_sttr_shape:
            sprintf(second, "_shapes") ;
            break ;
        case procat_spec_sw_sttr_disto:
        case procat_spec_lw_sttr_disto:
			res = rename_get_resolution(reffile) ;
			if (res == RENAME_LOW_RES) sprintf(second, "_poly2d_LR") ;
            else if (res == RENAME_MED_RES) sprintf(second, "_poly2d_MR") ;
            else {
                e_error("Resolution not recognized") ;
                return NULL ;
            }
            break ;
        case procat_spec_sw_sttr_corresp:
        case procat_spec_lw_sttr_corresp:
            sprintf(second, "_corresp") ;
            break ;
        case procat_spec_sw_sttr_extract:
        case procat_spec_lw_sttr_extract:
            sprintf(second, "_extracted") ;
            break ;
        case procat_imag_sw_flat_result:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			sprintf(second, "_%s", filter) ;
			free(filter) ;
			break ;
        case procat_imag_sw_flat_badpix:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			sprintf(second, "_%s_badpix", filter) ;
			free(filter) ;
            break ;
        case procat_imag_sw_flat_interce:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			sprintf(second, "_%s_intercept", filter) ;
			free(filter) ;
            break ;
        case procat_imag_sw_flat_errmap:
			if ((filter = rename_get_filter(reffile)) == NULL) return NULL ;
			sprintf(second, "_%s_errmap", filter) ;
			free(filter) ;
            break ;
		case procat_imag_zpoint_result:
			if ((target = rename_get_target(reffile)) == NULL) return NULL ;
			if ((filter = rename_get_filter(reffile)) == NULL) {
				free(target) ;
				return NULL ;
			}
			sprintf(second, "_%02d_%s_%s_check", hour, target, filter) ;
			free(target) ;
			free(filter) ;
			break ;
		case procat_imag_zpoint_qc:
			if ((target = rename_get_target(reffile)) == NULL) return NULL ;
			if ((filter = rename_get_filter(reffile)) == NULL) {
				free(target) ;
				return NULL ;
			}
			sprintf(second, "_%02d_%s_%s", hour, target, filter) ;
			free(target) ;
			free(filter) ;
			break ;
		case procat_imag_bg:
		case procat_spec_sw_arc_corr:
        case procat_spec_lw_arc_corr:
        case procat_imag_sw_jitter_diff:
        case procat_spec_slitpos_table:
		case procat_spec_slitpos_qc:
        case procat_spec_sw_sttr_correct:
        case procat_spec_lw_sttr_correct:
        	e_comment(1, "File registered but not supported") ;
			return NULL ;
			break ;
        default:
            e_error("PRO CATG key not recognized") ;
            return NULL ;
            break ;
    }

    /* GET THE EXTENSION OF THE NEW NAME */
	if (ref_name != NULL) {
		sprintf(extension, get_extname(filename)) ;
	} else {
		if (qfits_is_paf_file(filename)) sprintf(extension, "paf") ;
		else if (qfits_is_table(filename, 1)) sprintf(extension, "tfits") ;
		else if (is_fits_file(filename)) sprintf(extension, "fits") ;
		else {
			e_error("File type not recognized - abort") ;
			return NULL ;
		}
	}

	/* Use first and second to compose the final file name	 */
	final_name = rename_compose_name(filename, first, second, extension) ;

    return final_name ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get the resolution from a FITS header	
  @param	filename	FITS file name
  @return	int RENAME_LOW_RES or RENAME_MED_RES, -1 in error case		
 */
/*----------------------------------------------------------------------------*/
static int rename_get_resolution(char * filename) 
{
	char	    *	static_char ;
	char		    reso[32] ;
    instrument_t    ins ;
    
    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;

	if ((static_char = pfits_get(ins, filename, "resolution")) == NULL) {
		e_error("cannot get resolution") ;
		return -1 ;
	}
	strcpy(reso, strlwc(static_char)) ;
	if (reso[0] == 'l')      return RENAME_LOW_RES ; 
	else if (reso[0] == 'm') return RENAME_MED_RES ;
	else                     return -1 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the observation technic from a FITS header	
  @param	filename	FITS file name
  @return	int RENAME_TECH_IMA or RENAME_TECH_SPEC, -1 in error case		
 */
/*----------------------------------------------------------------------------*/
static int rename_get_dpr_tech(char * filename) 
{
	char	    *	static_char ;
    instrument_t    ins ;
	char		    tech[32] ;

    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;
    
	if ((static_char = pfits_get(ins, filename, "dpr_tech")) == NULL) {
		e_error("cannot get dpr tech") ;
		return -1 ;
	}
	strcpy(tech, static_char) ;
	if (!strcmp(tech, "IMAGE")) return RENAME_TECH_IMA ; 
	else if (!strcmp(tech, "SPECTRUM")) return RENAME_TECH_SPEC ;
	else return -1 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the observation type from a FITS header	
  @param	filename	FITS file name
  @return	int RENAME_TYPE_STD or RENAME_TYPE_OBS, -1 in error case		
 */
/*----------------------------------------------------------------------------*/
static int rename_get_dpr_type(char * filename) 
{
	char	    *	static_char ;
    instrument_t    ins ;
	char		    type[32] ;

    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;

	if ((static_char = pfits_get(ins, filename, "dpr_type")) == NULL) {
		e_error("cannot get dpr type") ;
		return -1 ;
	}
	strcpy(type, static_char) ;
	if (!strcmp(type, "STD"))         return RENAME_TYPE_STD ; 
	else if (!strcmp(type, "OBJECT")) return RENAME_TYPE_OBS ;
	else                              return -1 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get the DIT from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static char * rename_get_dit(char * filename)
{
	char	    *	static_char ;
	char	    *	dit ;
	int			    int_chars ;
	int			    dec_chars ;		
	int			    tot_chars ;		
    instrument_t    ins ;
	
	int			i ;

	/* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	i = 0 ;
	int_chars = dec_chars = 0 ;

	if ((static_char = pfits_get(ins, filename, "dit")) == NULL) {
		e_error("cannot get DIT") ;
		return NULL ;
	}
	dit = strdup(static_char) ;

	/* Truncate the 0s at the end */
	while (dit[i] != '\0') {
		if (dit[i] == '.') {
			dit[i] = '_' ;
			i++ ;
			while (dit[i] != '\0') {
				if (dit[i] != '0') dec_chars = i - int_chars ;
				i++ ;
			}
			break ;
		} else int_chars ++ ;
		i++ ;
	}

	if (dec_chars == 0) tot_chars = int_chars ;
	else tot_chars = int_chars + dec_chars + 1 ;
	dit[tot_chars] = '\0' ;
	
	return dit ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the filter from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static char * rename_get_filter(char * filename)
{
	char	    *	static_char ;
	char	    *	filter ;
    instrument_t    ins ;
	int			i = 0 ;
	
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	
    if ((static_char = pfits_get(ins, filename, "filter")) == NULL) {
		e_error("cannot get filter") ;
		return NULL ;
	}
	filter = strdup(static_char) ;
	while (filter[i] != '\0') {
		if (filter[i] == '.') filter[i] = '_' ;
		i++ ;
	}
	return filter ;

}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the central wavelength from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static char * rename_get_central_wavelength(char * filename)
{
	double		wl_double ;
	char	*	wl_char ;

	if ((wl_double = isaac_get_central_wavelength(filename)) == -1) {
		e_error("cannot get central wavelength") ;
		return NULL ;
	}
	wl_char = malloc(32 * sizeof(char)) ;
	if (wl_double < 10000) {
		sprintf(wl_char, "0%3f", wl_double) ;
		/* Keek only the 3 first digits  */
		wl_char[3] = '\0' ;
	} else {
		sprintf(wl_char, "%7.1f", wl_double) ;
		/* Keek only the 3 first digits  */
		wl_char[3] = '\0' ;
	}
	
	return wl_char ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the optical ID from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static char * rename_get_optical_id(char * filename)
{
	char	    *	static_char ;
	char	    *	opti ;
    instrument_t    ins ;
	int			    i = 0 ;
    
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	
	if ((static_char = pfits_get(ins, filename, "optical_id")) == NULL) {
		e_error("cannot get observation ID") ;
		return NULL ;
	}
	opti = strdup(static_char) ;

	if (!strcmp(opti, "slit_0.3_tilted")) {
		sprintf(opti, "s03t") ;
		return opti ;
	} else if (!strcmp(opti, "slit_1")) {
		 sprintf(opti, "sl1") ;
		 return opti ;
	} else if (!strcmp(opti, "slit_0.6_tilted")) {
		sprintf(opti, "s06t") ;
		return opti ;
	} else if (!strcmp(opti, "slit_2")) {
		sprintf(opti, "sl2") ;
		return opti ;
	} else if (!strcmp(opti, "slit_1.5")) {
        sprintf(opti, "sl15") ;
        return opti ;
    } else if (!strcmp(opti, "slit_0.8")) {
        sprintf(opti, "sl08") ;
        return opti ;
    }

	while (opti[i] != '\0') {
		if (opti[i] == '.') opti[i] = '_' ;
		i++ ;
	}
	return opti ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get the OBS TARG NAME from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static char * rename_get_target(char * filename)
{
	char	    *	static_char ;
	char	    *	star1 ;
	char	    *	star2 ;
	int			    name_pos ;
    instrument_t    ins ;
	int			    i, j ;	

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

	if ((static_char = pfits_get(ins, filename, "obs_targ_name")) == NULL) {
		e_error("cannot get target") ;
		return NULL ;
	}
	
	/* Find position of the name: abc/def/ghi => ghi is the right name */
	name_pos = strlen(static_char) - 1 ;
	while ((name_pos >= 0) && (static_char[name_pos] != '/')) name_pos -- ;
	/* Get the good name */
	star1 = strdup(static_char+name_pos+1) ;
	i = 0 ;
	while ((star1[i] != '/') && (star1[i] != '\0')) i++ ;
	star1[i] = '\0' ;
	
	/* Get rid of '-' in the name	 */
	star2 = strdup(star1) ;
	j=0 ;
	for (i=0 ; i<strlen(star1) ; i++) {
		if (star1[i] != '-') {
			star2[j] = star1[i] ;
			j++ ;
		}
	}
	free(star1) ;	
	star2[j] = '\0' ;
	return star2 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the observation ID from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static char * rename_get_obs_id(char * filename)
{
	char	    *	static_char ;
    instrument_t    ins ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	
    if ((static_char = pfits_get(ins, filename, "obs_id")) == NULL) {
		e_error("cannot get observation ID") ;
		return NULL ;
	}
	return strdup(static_char) ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get the observation ID from a FITS header
  @param	filename	FITS file name
  @return	allocated string (to be freed...)
 */
/*----------------------------------------------------------------------------*/
static int rename_dark_romode(char * filename)
{
	char            *   s ;
    instrument_t       ins ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    
	if ((s=pfits_get(ins, filename, "romode_name")) == NULL) {
		e_error("cannot get readout mode name") ;
		return -1 ;
	}
	if (!strcmp(s, "UncorrHighBias"))     return RENAME_UNCORRELATED ;
	if (!strcmp(s, "UncorrHighBiasCal"))  return RENAME_UNCORRELATED ;
	if (!strcmp(s, "DoubleCorrHighBias")) return RENAME_CORRELATED ;
	if (!strcmp(s, "DoubleCorrLowBias"))  return RENAME_CORRELATED ;
	if (!strcmp(s, "DoubleCorrLowBiasCal"))  return RENAME_CORRELATED ;
	if (!strcmp(s, "DoubleCorrHighBiasCal")) return RENAME_CORRELATED ;

	return -1 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compose the final output name	
  @param	filename	Old file name
  @param	first	First part of the final name
  @param	second	Second part of the final name
  @param	extension extension of the file
  @return	allocated string (to be freed...)

  If the file firstAsecond exists, try firstBsecond, firstCsecond ...
 */
/*----------------------------------------------------------------------------*/
static char * rename_compose_name(
		char	*	filename,
		char	*	first,
		char	*	second,
		char	*	extension)
{
	int				letter ;
	char		*	outname ;
	char			tmp_name[FILENAMESZ] ;
	char			dir_name[FILENAMESZ] ;
	struct	stat	f1 ;
	struct	stat	f2 ;

	/* Initialize */
	letter = RENAME_A_ASCII_DECIMAL ;
	
	sprintf(dir_name, get_dirname(filename)) ;
	
	if (!strcmp(dir_name, ".")) {
		sprintf(tmp_name, "%s%c%s.%s", first, (char)letter, second, extension) ;
	} else {	
		sprintf(tmp_name, "%s/%s%c%s.%s", dir_name, first, (char)letter, 
				second, extension) ;
	}

	while ((file_exists(tmp_name) == 1) 
			&& (letter < RENAME_A_ASCII_DECIMAL + RENAME_NB_OF_LETTERS)) {
		/* Old file name = New file name */
		stat(tmp_name, &f1);
		stat(filename, &f2);
		if (f1.st_ino == f2.st_ino) {
			e_comment(1, "The file name is already correct") ;
			return NULL ;
		}
		letter++ ;
		e_comment(1, "%s already exists", tmp_name) ;
		if (!strcmp(dir_name, ".")) {
			sprintf(tmp_name, "%s%c%s.%s", first, (char)letter, second, 
					extension) ;
		} else {	
			sprintf(tmp_name, "%s/%s%c%s.%s", dir_name, first, (char)letter, 
					second, extension) ;
		}
	}
	if (letter == RENAME_A_ASCII_DECIMAL + RENAME_NB_OF_LETTERS) return NULL ;
	outname = strdup(tmp_name) ;
	return outname ;
}

