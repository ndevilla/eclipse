/*----------------------------------------------------------------------------*/
/**
   @file    deghost.h
   @author  N. Devillard
   @date    January 2001
   @version	$Revision: 1.4 $
   @brief   ISAAC deghosting routine
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: deghost.h,v 1.4 2003/01/20 14:42:08 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/20 14:42:08 $
	$Revision: 1.4 $
*/

#ifndef _DEGHOST_H_
#define _DEGHOST_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    remove ISAAC electrical ghost from an ISAAC image
  @param    filename    input FITS image
  @param    force_flag  force flag
  @return   0 if ok, -1 otherwise
  New keywords are added to the header.
  GHOSTREM = 1
  GHOSTVER = 'Revision: x.y'
  If GHOSTREM is present and equal to 1, and force_flag is 0, no action
  is performed on the file.
 */
/*----------------------------------------------------------------------------*/
int isaac_ghost_removal(
        char    *   inname,
        int         force_flag) ;

#endif
