/*----------------------------------------------------------------------------*/
/**
   @file    distortion.h
   @author  Y. Jung
   @date    Feb 2001
   @version	$Revision: 1.9 $
   @brief   ISAAC distortion utilities
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: distortion.h,v 1.9 2004/12/01 14:47:32 yjung Exp $
	$Author: yjung $
	$Date: 2004/12/01 14:47:32 $
	$Revision: 1.9 $
*/

#ifndef _DISTORTION_H_
#define _DISTORTION_H_

/*-----------------------------------------------------------------------------
						Define
 -----------------------------------------------------------------------------*/

#define ISAAC_ARC_SATURATION    20000

/*-----------------------------------------------------------------------------
						Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the distortion
  @param    in  input image
  @param    xmin
  @param    ymin    The region of interest
  @param    xmax
  @param    ymax
  @param    auto_dark_sub   Flag to automatically subtract the dark
  @param    arcs    array with arcs positions
  @return   a 2-D polynomial of size 6 double
  
  This function is ISAAC-specific. It attempts to detect a dark ramp and
  subtract it if found. See compute_distortion for a generic version.
 */
/*----------------------------------------------------------------------------*/
poly2d * isaac_compute_distortion(
        image_t    *   in,
        int             xmin,
        int             ymin,
        int             xmax,
        int             ymax,
        int             auto_dark_sub,
        int         *   nb_arcs,
        double      **  arcs) ;

#endif




