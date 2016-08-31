/*----------------------------------------------------------------------------*/
/**
   @file    spjengine.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.2 $
   @brief   Main spjitter engine.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjengine.h,v 1.2 2002/12/20 14:56:00 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/20 14:56:00 $
	$Revision: 1.2 $
*/

#ifndef _SPJITTER_ENGINE_H_
#define _SPJITTER_ENGINE_H_

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/
 
/*----------------------------------------------------------------------------*/
/**
  @brief    Main spjitter recipe engine
  @param    ininame     Name of the input ini file.
  @return   long giving the total number of processed pixels
  This is the main engine for the spjitter algorithm.
  The returned value indicates the number of pixels received in input,
  or -1 if an error occurred.
 */
/*----------------------------------------------------------------------------*/
long spjitter_engine(char * ininame) ;

#endif
