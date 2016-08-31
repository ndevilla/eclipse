/*----------------------------------------------------------------------------*/
/**
   @file    image_rtd.h
   @author  Nicolas Devillard
   @date    July 2001
   @version $Revision: 1.6 $
   @brief   image handling from an RTD session
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: image_rtd.h,v 1.6 2003/02/21 14:30:47 yjung Exp $
    $Author: yjung $
    $Date: 2003/02/21 14:30:47 $
    $Revision: 1.6 $
*/

#ifndef _IMAGE_RTD_H_
#define _IMAGE_RTD_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "image_handling.h"
#include "doubles.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the current image displayed in RTD.
  @return   1 newly allocated image.

  This function connects to the current RTD session running as the same
  user, and retrieves all pixels associated to the image currently being
  displayed. The returned object is a copy of all pixels accessed from
  RTD, it must be deallocated using image_del().

  If no RTD session can be found or an error occurs, this function
  returns NULL.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * rtd_image_get(void) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Display an image on the current RTD session.
  @param    im      Image to be displayed
  @return   int 0 if Ok, anything else otherwise.

  This function will dump an image into the current RTD session running
  for the same user. If anything wrong happens, it returns a non-zero
  value.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int rtd_image_put(image_t * im) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Plot points on the current RTD session
  @param    pts     List of points to plot
  @return   int 0 if Ok, anything else otherwise.

  This function will draw little circles around every given position
  on the current RTD session.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int rtd_point_plot(double3 * pts) ;
/* </python> */

#endif
