/*-------------------------------------------------------------------------*/
/**
   @file    corner.h
   @author  N. Devillard
   @date    Nov 2000
   @version $Revision: 1.3 $
   @brief   Corner detector
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: corner.h,v 1.3 2001/10/26 14:30:35 yjung Exp $
    $Author: yjung $
    $Date: 2001/10/26 14:30:35 $
    $Revision: 1.3 $
*/

#ifndef _CORNER_H_
#define _CORNER_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

#include "image_handling.h"
#include "doubles.h"

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Detect corners in an image.
  @param    in      Input image.
  @param
  @return   1 newly allocated list of pixel positions.

  This function applies a corner detector to an image and returns a list of
  pixel positions where corners have been located.
 */
/*--------------------------------------------------------------------------*/
image_t * image_detect_corners(image_t * in) ;

#endif
