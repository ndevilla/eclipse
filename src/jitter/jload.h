/*----------------------------------------------------------------------------*/
/**
   @file    jload.h
   @author
   @date    March 2002
   @version	$Revision: 1.8 $
   @brief   Jitter data loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jload.h,v 1.8 2002/07/15 13:23:14 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/07/15 13:23:14 $
	$Revision: 1.8 $
*/

#ifndef _JLOAD_H_
#define _JLOAD_H_

#include "jtypes.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Load the data
  @param    ininame     Name of the input ini file
  @return   a jitter configuration object 
  This function creates a jitter configuration object that contains the data 
  loaded into it.
 */
/*----------------------------------------------------------------------------*/
jitter_config_t * jitter_load(char * ininame) ;

#endif
