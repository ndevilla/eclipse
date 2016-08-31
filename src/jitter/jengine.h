/*----------------------------------------------------------------------------*/
/**
   @file    jengine.h
   @author  N. Devillard
   @date    March 2002
   @version	$Revision: 1.2 $
   @brief   Main jitter engine.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jengine.h,v 1.2 2002/04/19 09:16:18 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 09:16:18 $
	$Revision: 1.2 $
*/

#ifndef _JITTER_ENGINE_H_
#define _JITTER_ENGINE_H_

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/
 
/*----------------------------------------------------------------------------*/
/**
  @brief    Main jitter recipe engine
  @param    ininame     Name of the input ini file.
  @return   long giving the total number of processed pixels

  This is the main engine for the jitter algorithm.
  The returned value indicates the number of pixels received in input,
  or -1 if an error occurred.
 */
/*----------------------------------------------------------------------------*/
long jitter_engine(char * ininame) ;

#endif
