/*----------------------------------------------------------------------------*/
/**
   @file    detect_ks.h
   @author  N. Devillard
   @date    June 2001
   @version $Revision: 1.6 $
   @brief   Object detection with kappa-sigma clipping
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: detect_ks.h,v 1.6 2003/02/20 13:31:13 yjung Exp $
    $Author: yjung $
    $Date: 2003/02/20 13:31:13 $
    $Revision: 1.6 $
*/

#ifndef _DETECT_KS_H_
#define _DETECT_KS_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "local_types.h"
#include "doubles.h"
#include "detect.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Locate objects in an image according to a kappa-sigma clipping.
  @param    ref         Reference image.
  @param    kappa       Kappa for kappa-sigma clipping.
  @param    smear_flag  Request image smearing before applying detection.
  @return   the detected object
 */
/*----------------------------------------------------------------------------*/
detected * detected_ks_engine(
        image_t  * ref,
        double     kappa,
        int        smear_flag) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the brigthest stars in an image with the ks method
  @param    image1  image
  @param    nbobjs  number of objects to detect
  @param    kappa   kappa for detection
  @return   the stars position
 */
/*----------------------------------------------------------------------------*/
double3 * detected_ks_brightest_stars(
        image_t *   image1,
        int         nbobjs,
        double      kappa) ;

#endif
