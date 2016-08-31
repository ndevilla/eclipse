/*-------------------------------------------------------------------------*/
/**
   @file    jcalib.h
   @author
   @date    March 2002
   @version	$Revision: 1.4 $
   @brief   Jitter calibration handling.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: jcalib.h,v 1.4 2002/04/19 09:04:15 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 09:04:15 $
	$Revision: 1.4 $
*/

#ifndef _JCALIB_H_
#define _JCALIB_H_

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Apply calibrations to type_obj frames
  @param    jc  Jitter configuration object
  @return   0 if ok, -1 otherwise
  This function loads the dark flatfield and bad pixel map specified and apply
  the corrections to the object frames.
 */
/*----------------------------------------------------------------------------*/
int jitter_calibration(jitter_config_t * jc) ;

#endif
