/*----------------------------------------------------------------------------*/
/**
   @file    spjcalib.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.3 $
   @brief   Spectroscopy jitter data calibration
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjcalib.h,v 1.3 2003/01/09 12:40:41 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/09 12:40:41 $
	$Revision: 1.3 $
*/

#ifndef _SPJCALIB_H_
#define _SPJCALIB_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Correct the flatfield on type_obj frames
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_flatfield(spjitter_config_t * spjc) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the wavelength calibration with different methods   
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_wlcalib(spjitter_config_t * spjc) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute and correct the distortion for type_obj frames  
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_distortion(spjitter_config_t * spjc) ;

#endif
