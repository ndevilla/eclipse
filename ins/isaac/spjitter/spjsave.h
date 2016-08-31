/*----------------------------------------------------------------------------*/
/**
   @file    spjsave.h
   @author  Y. Jung
   @date    Jan. 2003
   @version	$Revision: 1.2 $
   @brief   Spectroscopy jitter saving utilities
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjsave.h,v 1.2 2003/01/09 12:40:41 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/09 12:40:41 $
	$Revision: 1.2 $
*/

#ifndef _SPJSAVE_H_
#define _SPJSAVE_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    saving function
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_save(spjitter_config_t * spjc) ;

#endif
