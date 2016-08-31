/*----------------------------------------------------------------------------*/
/**
   @file    spjsaa.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.3 $
   @brief   Spectroscopy jitter shift and add utilities
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjsaa.h,v 1.3 2003/01/09 12:40:41 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/09 12:40:41 $
	$Revision: 1.3 $
*/

#ifndef _SPJSAA_H_
#define _SPJSAA_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    shift and average each classified cube to one image 
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_averaging(spjitter_config_t * spjc) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Check the mode and call the right difference function
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_differences(spjitter_config_t * spjc) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Combine (shift and add) all type_obj frames
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_combine(spjitter_config_t * spjc) ;

#endif
