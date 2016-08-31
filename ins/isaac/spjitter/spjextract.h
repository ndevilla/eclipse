/*----------------------------------------------------------------------------*/
/**
   @file    spjextract.h
   @author  Y. Jung
   @date    Jan. 2003
   @version	$Revision: 1.2 $
   @brief   Spectroscopy jitter spectrum extraction
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjextract.h,v 1.2 2003/01/09 12:40:41 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/09 12:40:41 $
	$Revision: 1.2 $
*/

#ifndef _SPJEXTRACT_H_
#define _SPJEXTRACT_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a spectrum from a combined image
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_extract(spjitter_config_t * spjc) ;

#endif
