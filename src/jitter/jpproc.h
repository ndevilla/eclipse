/*----------------------------------------------------------------------------*/
/**
   @file    jpproc.h
   @author
   @date    March 2002
   @version	$Revision: 1.4 $
   @brief   Jitter post-processing
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jpproc.h,v 1.4 2002/04/19 11:15:22 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 11:15:22 $
	$Revision: 1.4 $
*/

#ifndef _JPPROC_H_
#define _JPPROC_H_

/*-----------------------------------------------------------------------------
                                Functions prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter post-processing
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 if error occurred.

  Apply various post-processing filters to output data.
 */
/*----------------------------------------------------------------------------*/
int jitter_postproc(jitter_config_t * jc) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    View jitter results
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 otherwise

  Launch viewer command requested in ini file to see the results
  of the jitter run.
 */
/*----------------------------------------------------------------------------*/
int jitter_viewer(jitter_config_t * jc) ;

#endif
