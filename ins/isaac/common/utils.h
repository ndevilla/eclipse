/*----------------------------------------------------------------------------*/
/*
   @author  N. Devillard
   @date    August 1998
   @version	$Revision: 1.10 $
   @brief   ISAAC various utilities
*/
/*----------------------------------------------------------------------------*/

/*
 $Id: utils.h,v 1.10 2003/10/24 08:29:38 llundin Exp $
 $Author: llundin $
 $Date: 2003/10/24 08:29:38 $
 $Revision: 1.10 $
 */

#ifndef _UTILS_H_
#define _UTILS_H_

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"

/*-----------------------------------------------------------------------------
						Function ANSI C prototypes 
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out the slit width
  @param    filename    ISAAC fits file name
  @return   the slit width in pixels or -1 in error case
 */
/*----------------------------------------------------------------------------*/
double isaac_get_slitwidth(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out all header offsets for a frame list.
  @param    filename    Name of the ASCII list to parse.
  @return   1 newly allocated double3 array.

  This function calls iteratively get_cumoffsetx and get_cumoffsety on each
  file name in the input ASCII frame list, and stores the results
  into a newly allocated double3 array. If an error occurs, this function
  returns NULL.
 */
/*----------------------------------------------------------------------------*/
double3 * isaac_get_offsets(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Read the central wl in header and convert it in Angstroms
  @param    filename    ISAAC FITS file name
  @return   central wavelength in angstroms, -1 in error case
 */
/*----------------------------------------------------------------------------*/
double isaac_get_central_wavelength(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    find out for a given ISAAC file, if the Argon lamp was active
  @param    filename    ISAAC FITS file name
  @return   1 if the lamp is active, 0 if not, -1 in error case
  Based on the status of keyword INS.LAMP1.ST
 */
/*----------------------------------------------------------------------------*/
int isaac_is_argon_lamp_active(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    find out for a given ISAAC file, if the Xenon lamp was active
  @param    filename    ISAAC FITS file name
  @return   1 if the lamp is active, 0 if not, -1 in error case
  Based on the status of keyword INS.LAMP2.ST
 */
/*----------------------------------------------------------------------------*/
int isaac_is_xenon_lamp_active(char * filename) ;

#endif
