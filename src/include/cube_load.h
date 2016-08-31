/*-------------------------------------------------------------------------*/
/**
   @file    cube_load.h
   @author  Nicolas Devillard
   @date    May 1999
   @version $Revision: 1.26 $
   @brief   cube loading
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: cube_load.h,v 1.26 2001/12/03 10:51:56 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2001/12/03 10:51:56 $
    $Revision: 1.26 $
*/

#ifndef _CUBE_LOAD_H_
#define _CUBE_LOAD_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "comm.h"
#include "static_sz.h"
#include "xmemory.h"
#include "cube_defs.h"
#include "image_handling.h"
#include "file_handling.h"
#include "framelist.h"
#include "qfits.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Load a cube from disk.
  @param    filename    Name of the file to load.
  @return   1 newly allocated cube object (NULL if error).

  This is just a wrapper around supported file formats. The given file
  name is first checked to see if it is FITS (in which case
  cube_load_fits is called), then checked to see if it is an ASCII
  list.

  Further formats might be added later on.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_load(char * filename) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Load a cube from the current image displayed in RTD
  @return   1 newly allocated cube containing 1 image.

  This function initiates a connection to the current RTD session
  running for the current user, gets the pixels which are currently
  displayed there, builds a cube containing a single plane and returns
  it.

  If any error occurs, this function returns NULL. The returned cube
  must be deallocated using cube_del. Notice that the rtd session is
  opened/closed by the function, so no side-effect occurs.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_load_rtd(void) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Load a cube from a FITS file on disk.
  @param    filename    Name of the FITS file to load.
  @return   1 newly allocated cube object (NULL if error).
  Reads a cube in from a FITS file on disk.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_load_fits(char * filename) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Load cube from a list defined as a char **.
  @param    filenames   Array of strings containing the file names.
  @param    nfiles      Number of file names in the list.
  @return   1 newly allocated cube object.

  This function takes in input a list of strings, like (argc,argv) and
  loads a cube from it. File names are expected to refer to FITS
  files. These files might be 2 or 3 dimensional, they are all
  expected to share the same image size.

  Example: if the input list contains

  \begin{verbatim}
  image1.fits   # An image
  image2.fits   # An image
  cube1.fits    # A data cube with 20 planes
  \end{verbatim}

  Then the returned cube will have 22 planes (the first 2 plus the 20
  from the cube).

 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_load_strings(char ** filenames, int nfiles) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Load a cube from an ASCII list file.
  @param    listname    Name of the ASCII list file.
  @return   1 newly allocated cube object.

  This function takes in input the name of a file, supposed to contain
  a list of frame names. File names corresponding to single frames are
  loaded as single frames. If a file name corresponds to a cube, all
  frames of this cube are loaded into separate planes in the returned
  cube. See cube_load_strings() for more information.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_load_framelist(char * listname) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief        Get some file information about a FITS file.
  @param    filename    Name of the file to parse.
  @return   1 new cube_info structure, NULL if error.

  This function reads in a FITS file and fills up a cube_info struct
  informing about the file structure. The returned structure must be
  freed using free().
  If the cube has an inconsistent declaration, an error is triggered
  and NULL is returned.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_info * cube_getinfo(char *filename) ;
/* </python> */


#endif
