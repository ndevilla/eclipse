/*----------------------------------------------------------------------------*/
/**
   @file    jsky.h
   @author  N. Devillard
   @date    March 2002
   @version	$Revision: 1.5 $
   @brief   Jitter sky estimation/subtraction
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jsky.h,v 1.5 2002/04/19 11:30:27 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 11:30:27 $
	$Revision: 1.5 $
*/

#ifndef _JSKY_H_
#define _JSKY_H_

/*-----------------------------------------------------------------------------
                                Functions prototype
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter sky estimation and correction
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 if error occurred.
 */
/*----------------------------------------------------------------------------*/
int jitter_sky(jitter_config_t * jc) ;

#endif
