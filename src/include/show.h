
/*-------------------------------------------------------------------------*/
/**
  @file     show.h
  @author   Y. Jung
  @date     Mar 2001
  @version  $Revision: 1.5 $
  @brief    All-in-one interface to display images or plot signals.

  This interface is sitting on top of other interfaces to display
  tools, allowing to send display orders without having to care about
  the way it is truly handled behind the scenes.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: show.h,v 1.5 2001/10/19 08:15:05 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 08:15:05 $
	$Revision: 1.5 $
*/

#ifndef _SHOW_H_
#define _SHOW_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*---------------------------------------------------------------------------
   							Functions prototypes
 ---------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/**
  @brief    Starts an image viewer to display an image.
  @param    image_name  Name of the image to display (FITS file).
  @param    viewer_name Viewer name (rtd, xv ...)
  @return   void
  
  This function launches an image viewer (described in "viewer_name")
  on the file described in "image_name". The viewer is launched in
  background.

  Example:

  @code
  show_image("result1.fits", "rtd");
  show_image("result2.fits", "saoimage -fits");
  @endcode


 */
/*--------------------------------------------------------------------------*/
/* <python> */
void show_image(char * image_name, char * viewer_name) ;
/* </python> */

/*-------------------------------------------------------------------------*/
/**
  @brief    Plot a signal with gnuplot
  @param    x           X coordinate values
  @param    y           Y coordinate values
  @param    nbpoints    Nb of values
  @param    xlabel      Label for X axis
  @param    ylabel      Label for Y axis
  @return   void

  This functions sends a 2d curve to plot to gnuplot. It will fail if
  no DISPLAY environment variable is set (working on console).

  This functions expects the user to type <ENTER> to end the display,
  so cannot be used in pipeline mode.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
void plot_signal(
        double      *   x,
        double      *   y,
        int             nbpoints,
        char        *   xlabel,
        char        *   ylabel) ;
/* </python> */

#endif
