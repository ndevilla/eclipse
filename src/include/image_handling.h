/*-------------------------------------------------------------------------*/
/**
   @file    image_handling.h
   @author  Nicolas Devillard
   @date    Aug 17, 1995
   @version $Revision: 1.23 $
   @brief   image data structure handling routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: image_handling.h,v 1.23 2001/12/03 10:51:56 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2001/12/03 10:51:56 $
    $Revision: 1.23 $
*/

#ifndef _IMAGE_HANDLING_H_
#define _IMAGE_HANDLING_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "local_types.h"
#include "xmemory.h"
#include "cube_handling.h"
#include "qfits.h"
#include "history.h"
#include "image_io.h"
#include "image_stats.h"

/*---------------------------------------------------------------------------
  						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocate an image structure and pixel buffer for an image.
  @param    size_x  Size in x
  @param    size_y  Size in y
  @return   1 newly allocated image.
 
  Allocates both space for the image structure and the pixel buffer. The
  returned pixel buffer is always seen as if it were in memory.
 
  The returned image must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_new(
        int     size_x,
        int     size_y) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Free memory associated to an image object.
  @param    d   Image to destroy.
  @return   void

  Frees all memory associated to an image.
 */
/*--------------------------------------------------------------------------*/
void image_del(image_t * d) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the size of an image in bytes.
  @param    im  Image to examine.
  @return   size of the image in memory in bytes, as an int.

  This function computes the size taken in memory by the given image, in
  bytes, and returns it as an int. If a problem occurs during computation
  (e.g. because of inconsistent structure values), this function returns
  -1.
 
  This function is useful to declare the size of a given image to a garbage
  collector, for example. Since it depends on the local implementation
  chosen by the compiler, it is likely that identical image sizes might
  return different values. This is only useful to know the amount of memory
  taken up by one image in the current program.
 */
/*--------------------------------------------------------------------------*/
int image_get_bytesize(image_t * im) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Copy an image.
  @param    src_img     Source image.
  @return   1 newly allocated image.

  Copy an image into a new image object. The pixel buffer is also copied.
  The returned image must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_copy(image_t * src_img) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Build an image from a shared memory segment.
  @param    shmid       Id of the shared memory segment to use.
  @param    offset      Offset from segment start.
  @param    lx          Image size in X.
  @param    ly          Image size in Y.
  @param    bpp         Bytes per pixel a la FITS (8,16,32,-32,-64).
  @return   1 newly allocated image.

  This function expects a shared memory ID and basic image info.
  It will attach itself to the segment and copy the segment contents
  over to a new image structure, which is then returned.

  This is expected to work with programs that allocate images in
  shared memory like RTD (VLT software).
 */
/*--------------------------------------------------------------------------*/
image_t * image_from_shmem(
        int shmid,
        int offset,
        int lx,
        int ly,
        int bpp) ;

#endif
