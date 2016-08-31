/*----------------------------------------------------------------------------*/
/**
   @file    jsaa.h
   @author
   @date    March 2002
   @version	$Revision: 1.4 $
   @brief   Jitter shift-and-add
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jsaa.h,v 1.4 2002/04/19 11:20:36 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 11:20:36 $
	$Revision: 1.4 $
*/

#ifndef _JSAA_H_
#define _JSAA_H_

/*-----------------------------------------------------------------------------
                            Functions prototype
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Apply shit-and-add to input set of frames.
  @param    jc  Current jitter config.
  @return   int 0 if Ok, -1 if error occured.

  This part includes:
  - Blind offset search 
  - X-correlation:
    - xcorr object detection
    - xcorrelation to refine offsets
  - Shifting and adding frames
  Blind offset search, cross-correlation are only carried out if required.
 */
/*----------------------------------------------------------------------------*/
int jitter_saa(jitter_config_t * jc) ;

#endif
