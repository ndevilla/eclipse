/*----------------------------------------------------------------------------*/
/**
   @file	image_rtd.c
   @author	Nicolas Devillard
   @date	July 2001
   @version	$Revision: 1.9 $
   @brief	image handling from an RTD session
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: image_rtd.c,v 1.9 2005/03/02 10:40:29 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/02 10:40:29 $
	$Revision: 1.9 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "xmemory.h"
#include "image_handling.h"
#include "doubles.h"
#include "rtd_i.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

static char * rtd_swapfilename(void) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Get the current image displayed in RTD.
  @return	1 newly allocated image.

  This function connects to the current RTD session running as the same user, 
  and retrieves all pixels associated to the image currently being displayed. 
  The returned object is a copy of all pixels accessed from RTD, it must be 
  deallocated using image_del().
  If no RTD session can be found or an error occurs, this function returns NULL.
 */
/*----------------------------------------------------------------------------*/
image_t * rtd_image_get(void)
{
	int			lx, ly, bpp ;
	int			shmid ;
	char	*	rtd_answer ;
	int			rtd_status ;
	int			offs, length, size ;

	/* Open a connection to the current rtd session */
	if (rtd_connect(0,0,0)!=0) {
		e_error("connecting to rtd: cannot obtain image");
		return NULL ;
	}

	/* Get image size in X */
	rtd_status = rtd_send("width", &rtd_answer);
	if (rtd_status!=0) {
		e_error("failed to get image width: cannot obtain image from rtd");
        e_error("rtd says: %s", rtd_answer);
		rtd_disconnect();
		return NULL ;
	}
	lx = atoi(rtd_answer);

	/* Get image size in Y */
	rtd_status = rtd_send("height", &rtd_answer);
	if (rtd_status!=0) {
		e_error("failed to get image height: cannot obtain image from rtd");
        e_error("rtd says: %s", rtd_answer);
		rtd_disconnect();
		return NULL ;
	}
	ly = atoi(rtd_answer);

	/* Get bits per pixel */
	rtd_status = rtd_send("bitpix", &rtd_answer);
	if (rtd_status!=0) {
		e_error("failed to get image bitpix: cannot obtain image from rtd");
        e_error("rtd says: %s", rtd_answer);
		rtd_disconnect();
		return NULL ;
	}
	bpp = atoi(rtd_answer);

	/* Get shared memory segment ID */
	rtd_status = rtd_send("shm get data", &rtd_answer);
	if (rtd_status!=0) {
		e_error("failed to get image data: cannot obtain image from rtd");
        e_error("rtd says: %s", rtd_answer);
		rtd_disconnect();
		return NULL ;
	}
	sscanf(rtd_answer, "%d %d %d %d", &shmid, &offs, &length, &size);
	rtd_disconnect();

	return image_from_shmem(shmid, offs, lx, ly, bpp) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Display an image on the current RTD session.
  @param	im		Image to be displayed
  @return	int 0 if Ok, anything else otherwise.

  This function will dump an image into the current RTD session running for the
  same user. If anything wrong happens, it returns a non-zero value.
 */
/*----------------------------------------------------------------------------*/
int rtd_image_put(image_t * im)
{
	int		status ;
	char * 	answ ;
	char	cmd[FILENAMESZ];
	char *	rtd_dfile ;

	if (im==NULL) return -1 ;

	/* Open a connection to the current rtd session */
	if (rtd_connect(0,0,0)!=0) {
		e_error("connecting to rtd: cannot display image");
		return -1 ;
	}

	/* Retrieve a valid RTD-eclipse exchange file name */
	rtd_dfile = rtd_swapfilename();
	if (rtd_dfile==NULL) {
		rtd_disconnect();
		return -1 ;
	}
	image_save_fits(im, rtd_dfile, BPP_DEFAULT);
	sprintf(cmd, "config -file %s", rtd_dfile);
	status = rtd_send(cmd, &answ);
	rtd_disconnect();
	if (status!=0) {
		e_error("failed to send image data: cannot display image");
        e_error("rtd says: %s", answ);
		return -1 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Plot points on the current RTD session
  @param	pts		List of points to plot
  @return	int 0 if Ok, anything else otherwise.

  This function will draw little circles around every given position on the 
  current RTD session.
 */
/*----------------------------------------------------------------------------*/
#define RTD_GETCANVAS \
"remotetcl {set eclipse_c [[[itcl_info objects -class ::rtd::Rtd] "\
"component image] get_canvas]}"

#define RTD_CREATEOVAL \
"remotetcl {$eclipse_c create oval [expr $eclipse_x-5] [expr $eclipse_y-5] "\
"[expr $eclipse_x+5] [expr $eclipse_y+5] -outline green -width 1}"

int rtd_point_plot(double3 * pts)
{
	int			status ;
	char 	*	answ ;
	char		cmd[128] ;
	int			i ;

	/* Open a connection to the current rtd session */
	if (rtd_connect(0,0,0)!=0) {
		e_error("connecting to rtd: cannot display image");
		return -1 ;
	}

	/* Get canvas name */
	status = rtd_send(RTD_GETCANVAS, &answ);
	if (status!=0) {
		e_error("failed to get canvas ID: aborting point plot");
        e_error("rtd says: %s", answ);
		rtd_disconnect();
		return -1 ;
	}
        
	/* Convert all point coordinates to canvas reference */
	for (i=0 ; i<pts->n ; i++) {
		sprintf(cmd, "convert coords %d %d image eclipse_x eclipse_y canvas",
				1+(int)pts->x[i],
				1+(int)pts->y[i]);
		status = rtd_send(cmd, &answ);
		/* Draw a circle around each point */
		status = rtd_send(RTD_CREATEOVAL, &answ);
		/* Write out coordinates */
		sprintf(cmd, "remotetcl {$c create text $x $y "
					 "-text \"(%d,%d)\" -fill green}",
					 (int)pts->x[i],
					 (int)pts->y[i]);
		status = rtd_send(cmd, &answ);
	}
	rtd_disconnect();
	return 0 ;
}

static char * rtd_swapfilename(void)
{
	static char	swapname[FILENAMESZ];
	char		cwd[FILENAMESZ];

    if (getcwd(cwd, FILENAMESZ-1)==NULL) {
        e_error("getting current working directory name");
        return NULL ;
    }
    sprintf(swapname, "%s/eclipse-rtd.swp", cwd);
	return swapname ;
}

