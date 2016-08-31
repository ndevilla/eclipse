/*----------------------------------------------------------------------------*/
/**
   @file	lwload.h
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.3 $
   @brief	ISAAC LW cube loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: lwload.h,v 1.3 2002/06/03 14:21:12 yjung Exp $
	$Author: yjung $
	$Date: 2002/06/03 14:21:12 $
	$Revision: 1.3 $
*/

#ifndef _LWLOAD_H_
#define _LWLOAD_H_

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Load a list of files into a cube.
  @param    flist   Name of framelist to load
  @return   1 pointer to newly allocated cube.

  This function hides the fact that ISAAC LW data may come as list of 
  single-frame or double-frame (NAXIS3=2) files. If the input list designates 
  single-frame files, they are all loaded into a cube. If the list designates 
  double-frame files, each pair of frame is loaded, frame 2 subtracted from 
  frame 1 and the result stored into the returned cube.
  The returned cube must be deallocated using one of the cube deallocators.
 */
/*----------------------------------------------------------------------------*/
cube_t * isaac_loadcube(framelist * flist) ;

#endif
