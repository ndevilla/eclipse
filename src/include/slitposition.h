/*----------------------------------------------------------------------------*/
/**
   @file    extraction.h
   @author  Y. Jung
   @date    Mar 28 2003
   @version $Revision: 1.2 $
   @brief   slit position analysis
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: slitposition.h,v 1.2 2003/03/28 14:46:17 yjung Exp $
    $Author: yjung $
    $Date: 2003/03/28 14:46:17 $
    $Revision: 1.2 $
*/

#ifndef _SLITPOSITION_H_
#define _SLITPOSITION_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doubles.h"
#include "image_handling.h"

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the slit position, detect its ends, extract a thin image
            containing only the slit and find its edges
  @param    inimage input image
  @param    slit_max_width maximum slit width
  @param    slit_angle  pointer to the angle horizontal-slit
  @param    slit_length pointer to the slit length
  @return   ptr to 3 double3 objects.

  This function can be used for vertical slits.
  This function returns 3 double3 objects:
  - Left or Lower edge of the slit
  - Center of the slit
  - Right or Upper edge of the slit
  NB: Coordinates use FITS convention.
 */
/*----------------------------------------------------------------------------*/
double3 ** slitpos_analysis(
        image_t     *   inimage,
        int             slit_max_width,
        double      *   slit_angle,
        int         *   slit_length) ;

#endif
