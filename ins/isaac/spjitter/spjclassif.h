/*----------------------------------------------------------------------------*/
/**
   @file    spjclassif.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.2 $
   @brief   Spectroscopy jitter data classification
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjclassif.h,v 1.2 2002/12/20 14:53:14 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/20 14:53:14 $
	$Revision: 1.2 $
*/

#ifndef _SPJCLASSIF_H_
#define _SPJCLASSIF_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the spectroscopic jitter input data classification
  @param    spjc    sjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_classif(spjitter_config_t * spjc) ;

#endif
