/*-------------------------------------------------------------------------*/
/**
   @file    detect_sq.h
   @author  Yves Jung
   @date    June 2001
   @version $Revision: 1.5 $
   @brief   Object detection with square filter method
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: detect_sq.h,v 1.5 2001/12/12 16:19:48 yjung Exp $
    $Author: yjung $
    $Date: 2001/12/12 16:19:48 $
    $Revision: 1.5 $
*/

#ifndef _DETECT_SQ_H_
#define _DETECT_SQ_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "local_types.h"
#include "doubles.h"
#include "detect.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Locate objects in an image according to the square method.
  @param    ref     Reference image
  @param    hx      Square half-size in X.
  @param    hy      Square half-size in Y.
  @return   the detected object
 */
/*--------------------------------------------------------------------------*/
detected * detected_sq_engine(
        image_t     *   ref,
        int             hx,
        int             hy) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Detect the brigthest stars in an image with the sq method
  @param    image1  image
  @param    nbobjs  number of objects to detect
  @param    hx      half square size x
  @param    hy      half square size y
  @return   the stars position
 */
/*--------------------------------------------------------------------------*/
double3 * detected_sq_brightest_stars(
        image_t *   image1,
        int         nbobjs,
        int         hx,
        int         hy) ;

#endif

