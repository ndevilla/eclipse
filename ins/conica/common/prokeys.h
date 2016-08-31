
/*---------------------------------------------------------------------------
   
   File name 	:	prokeys.h
   Author 		:	Y. Jung
   Created on	:	July 2000
   Description	:	CONICA common fonctions to write (in) produced files

 *--------------------------------------------------------------------------*/

/*
	$Id: prokeys.h,v 1.5 2002/07/31 14:30:49 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/07/31 14:30:49 $
	$Revision: 1.5 $
*/

#ifndef _CO_UTILS_FILES_H_
#define _CO_UTILS_FILES_H_

#include "pfitspro.h"

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Create a first version of an output image header
  @param    fh      input fits header
  @return   0 if ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int conica_header_for_image(qfits_header * fh);

/*-------------------------------------------------------------------------*/
/**
  @brief    Create a first version of an output table header
  @param    fh      input fits header
  @return   0 if ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int conica_header_for_table(qfits_header * fh);

/*-------------------------------------------------------------------------*/
/**
  @brief    Add HISTORY entries in the FITS header 
  @param    fh      Fits header
  @param    filenames   list of files to write
  @return   0 if ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int conica_add_files_history(
        qfits_header    *   fh,
        framelist       *   filenames) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Fill up a given FITS header with PRO keyword
  @param    fh              fits header in which values are written
  @param    filename        produced file name
  @param    pro_type        product type
  @param    pro_redlevel    product reduction level
  @param    pro_catg        product category key
  @param    pro_status      product status
  @param    pro_rec_id      
  @param    pro_datancom    number of reduced files
  @param    rawfiles        list of input raw files
  @param    calibfiles      list of calibration files
  @return   0 if ok, -1 otherwise

  DFS only. See the DICB dictionaries to have details on the keywords.
 */
/*--------------------------------------------------------------------------*/
int conica_pro_fits(
        qfits_header    *   fh,
        char            *   pipefile,
        char            *   pro_type,
        char            *   pro_redlevel,
        procat              pro_catg,
        char            *   pro_status,
        char            *   pro_rec_id,
        int                 pro_datancom,
        framelist       *   rawfiles,
        framelist       *   calibfiles) ;

#endif






