/*----------------------------------------------------------------------------*/
/**
   @file    spjload.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.2 $
   @brief   Spectroscopy jitter data loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjload.h,v 1.2 2002/12/20 14:56:00 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/20 14:56:00 $
	$Revision: 1.2 $
*/

#ifndef _SPJLOAD_H_
#define _SPJLOAD_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Load the data
  @param    ininame     Name of the input ini file
  @return   a spectroscopic jitter configuration object 

  This function creates a spjitter configuration object that contains the data 
  loaded into it.
 */
/*----------------------------------------------------------------------------*/
spjitter_config_t * spjitter_load(char * ininame) ;

#endif
