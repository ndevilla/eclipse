/*-------------------------------------------------------------------------*/
/**
   @file	do_catg.h
   @author	Y. Jung
   @date	Apr 2002
   @version	$Revision: 1.3 $
   @brief   DO_CATG keywords	
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: do_catg.h,v 1.3 2003/02/18 12:53:29 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/18 12:53:29 $
	$Revision: 1.3 $
*/

#ifndef _PRODUCTS_H_
#define _PRODUCTS_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "insid.h"

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

typedef enum docat {
    docat_invalid = 0,    /* Reserve an invalid product label */
    
    docat_imag_flat,
    docat_imag_dark,
    docat_imag_badpix,
    docat_imag_detlin_coeff_A,
    docat_imag_detlin_coeff_B,
    docat_imag_detlin_coeff_C,
    docat_spec_arc,
    docat_spec_sttr,
    docat_spec_flat,
    
    docat_end             /* Reserve for end of loops */
} docat ;

typedef struct docat_list_t {
    docat   label ;
    char *  value ;
    char *  descr ;
} docat_list_t ;

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Get docat associated to a string.
  @param	ins		Instrument ID
  @param	value   Product category value as found in DO.CATG (2nd column)
  @return	A category label

  This function expects a string as read from e.g. the second column of a 
  reduction block, and converts it to a valid category label.
  The returned label is docat_invalid (0) if the string has no known 
  association.
 */
/*--------------------------------------------------------------------------*/
docat pfits_getdocat_label(instrument_t ins, char * value) ;

/*-------------------------------------------------------------------------*/
/**
  @brief	Get string associated to a label.
  @param    ins     Instrument ID
  @param    label   DO CATG label
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string.
  The returned string is statically allocated so do not free it or modify it.
 */
/*--------------------------------------------------------------------------*/
char * pfits_getdocat_value(instrument_t ins, docat label) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Get description string associated to a label 
  @param    ins     Instrument ID
  @param    label   CO CATG label 
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string that describes the associated product.
  The returned string is statically allocated so do not free it or modify it.
 */
/*--------------------------------------------------------------------------*/
char * pfits_getdocat_descr(instrument_t ins, docat label) ;

#endif
