
/*-------------------------------------------------------------------------*/
/**
  @file		show.c
  @author	Y. Jung
  @date		Mar 2001
  @version	$Revision: 2.6 $
  @brief	All-in-one interface to display images or plot signals.

  This interface allows to send an image or a plot to display by an
  external process.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: show.c,v 2.6 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 2.6 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "comm.h"
#include "gnuplot_i.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Starts an image viewer to display an image.
  @param    image_name 	Name of the image to display (FITS file).
  @param	viewer_name	Viewer name (rtd, xv ...)
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
void show_image(
		char	*	image_name,
		char	*	viewer_name)
{
    char    launch_cmd[1024] ;
    char    viewer[1024] ;
     
	/* Start an image viewer */
    e_comment(1, "now spawning image viewer...") ;
    sprintf(viewer, "%s &", viewer_name);
    sprintf(launch_cmd, viewer, image_name) ;
    e_comment(1, "%s", launch_cmd) ;
    system(launch_cmd) ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Plot a signal with gnuplot
  @param   	x			X coordinate values
  @param	y			Y coordinate values
  @param	nbpoints	Nb of values
  @param	xlabel		Label for X axis
  @param	ylabel		Label for Y axis
  @return   void

  This functions sends a 2d curve to plot to gnuplot. It will fail if
  no DISPLAY environment variable is set (working on console).

  This functions expects the user to type <ENTER> to end the display,
  so cannot be used in pipeline mode.
 */
/*--------------------------------------------------------------------------*/
void plot_signal(
		double		*	x,
		double		*	y,
		int				nbpoints,
		char		*	xlabel,
		char		*	ylabel)
{
    gnuplot_ctrl    *   handle ;

    if (getenv("DISPLAY")==NULL) {
        e_warning("DISPLAY variable is not set: cannot launch gnuplot");
        return ;
    }

    /* Plot the signal */
    handle = gnuplot_init() ;
    gnuplot_setstyle(handle, "lines") ;
    gnuplot_set_xlabel(handle, xlabel) ;
    gnuplot_set_ylabel(handle, ylabel) ;
    gnuplot_plot_xy(handle, x, y, nbpoints, ylabel) ; 
    printf("press enter to quit\n") ;
    while (getchar() != '\n') {}
    gnuplot_close(handle) ;
    return ;
}

/* vim: set ts=4 et sw=4 tw=75 */
