/*----------------------------------------------------------------------------*/
/**
   @file	products.h
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.19 $
   @brief	Product key/name handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: products.h,v 1.19 2004/02/09 15:25:28 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 15:25:28 $
	$Revision: 1.19 $
*/

#ifndef _PRODUCTS_H_
#define _PRODUCTS_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "insid.h"

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

typedef enum procat {
    procat_invalid = 0,    /* Reserve an invalid product label */

    procat_imag_sw_flat_interce,
    procat_imag_sw_flat_errmap,
    procat_imag_sw_flat_result,
    procat_imag_sw_flat_badpix,
    procat_imag_sw_flat_qc,
    procat_imag_lampflat_result,
    procat_imag_lampflat_qc,

    procat_imag_jitter_qc,
    procat_imag_sw_jitter_result,
    procat_imag_sw_jitter_diff,

    procat_imag_detlin_coeff_Q,
    procat_imag_detlin_coeff_A,
    procat_imag_detlin_coeff_B,
    procat_imag_detlin_coeff_C,
    procat_imag_detlin_coeff_D,
    procat_imag_detlin_QC,
    procat_imag_detlin_limit,
    procat_imag_detlin_bpm,

    procat_imag_lw_jitter_result,

    procat_imag_zpoint_qc,
    procat_imag_zpoint_result,

    procat_imag_illum,

    procat_imag_bg,

    procat_spec_sw_arc_qc,
    procat_spec_sw_arc_coef,
    procat_spec_sw_arc_corr,

    procat_spec_sw_jitter_comb,
    procat_spec_sw_jitter_qc,
    procat_spec_sw_jitter_extr,

    procat_spec_sw_resp_effi,
    procat_spec_sw_resp_conv,
    procat_spec_sw_resp_extr,
    procat_spec_sw_resp_back,

    procat_spec_sw_flat,
    procat_spec_sw_flat_qc,

    procat_spec_sw_sttr_extract,
    procat_spec_sw_sttr_corresp,
    procat_spec_sw_sttr_disto,
    procat_spec_sw_sttr_qc,
    procat_spec_sw_sttr_correct,
    procat_spec_sw_sttr_shape,
    procat_spec_sw_sttr_pos,

    procat_spec_lw_arc_qc,
    procat_spec_lw_arc_coef,
    procat_spec_lw_arc_corr,

    procat_spec_lw_jitter_comb,
    procat_spec_lw_jitter_qc,
    procat_spec_lw_jitter_extr,

    procat_spec_lw_resp_effi,
    procat_spec_lw_resp_conv,
    procat_spec_lw_resp_extr,
    procat_spec_lw_resp_back,

    procat_spec_lw_flat,
    procat_spec_lw_flat_qc,

    procat_spec_lw_sttr_extract,
    procat_spec_lw_sttr_corresp,
    procat_spec_lw_sttr_disto,
    procat_spec_lw_sttr_qc,
    procat_spec_lw_sttr_correct,
    procat_spec_lw_sttr_shape,
    procat_spec_lw_sttr_pos,

    procat_spec_slitpos_qc,
    procat_spec_slitpos_table,

    procat_dark_ron,
    procat_dark_result,
    procat_dark_hot,
    procat_dark_dev,
    procat_dark_cold,
    
    procat_focus,

    procat_qc_strehl,

    procat_end             /* Reserve for end of loops */
} procat ;

typedef struct prodlist_t {
    procat  cat ;
    char *  key ;
    char *  desc ;
} prodlist_t ;


/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Get procat associated to a string.
  @param	ins		Instrument ID
  @param	key		Product category key as found in PRO.CATG
  @return	A category label.

  This function expects a string as read from e.g. the PRO.CATG keyword in
  a PAF or FITS header, and converts it to a valid category label.
  The returned label is invalid_product_key (0) if the string has
  no known association.
 */
/*----------------------------------------------------------------------------*/
procat pfits_getprocat(instrument_t ins, char * key);

/*----------------------------------------------------------------------------*/
/**
  @brief	Get string associated to a procat.
  @param    ins     Instrument ID
  @param    cat     Pro category (label)
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string to be placed e.g. in the PRO.CATG
  keyword of an output PAF or FITS file. The returned string
  is statically allocated so do not free it or modify it.
 */
/*----------------------------------------------------------------------------*/
char * pfits_getprokey(instrument_t ins, procat cat);

/*----------------------------------------------------------------------------*/
/**
  @brief    Get description string associated to a procat.
  @param    ins     Instrument ID
  @param    cat     Pro category (label)
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string that describes the associated product.
  The returned string is statically allocated so do not free it or modify
  it.
 */
/*----------------------------------------------------------------------------*/
char * pfits_getprodesc(instrument_t ins, procat cat);


#endif
