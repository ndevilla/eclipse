/*-------------------------------------------------------------------------*/
/**
   @file    intimage.h
   @author  Nicolas Devillard
   @date    July 2000
   @version $Revision: 1.8 $
   @brief   Image object containing integer pixels
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: intimage.h,v 1.8 2001/10/26 14:33:42 yjung Exp $
    $Author: yjung $
    $Date: 2001/10/26 14:33:42 $
    $Revision: 1.8 $
*/

#ifndef _INTIMAGE_H_
#define _INTIMAGE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "local_types.h"
#include "xmemory.h"

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Integer pixel type

  This type is guaranteed to have positive and negative values, and at
  least 16 bits/pel. Do not make any assumption further than that, though.
  The pixel type might have to be changed to support more bits/pixel in
  the future.
 */
/*--------------------------------------------------------------------------*/
typedef short intpix ;


/*-------------------------------------------------------------------------*/
/**
  @brief	Integer image.

  An integer image is just like a normal image, except that its pixel
  buffer contains 'intpix' elements instead of 'pixelvalue'. The intpix
  type should always be considered abstract up to the following
  assumptions:

  \begin{itemize}
  \item The pixel type can take only integer values.
  \item Integer pixels are signed, and at least coded on 16 bits.
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
typedef struct _intimage_ {
	int			lx ;
	int			ly ;
	intpix	*	data ;
} intimage ;


/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Integer image constructor.
  @param    lx      Size in x.
  @param    ly      Size in y.
  @return   1 newly allocated intimage.

  Allocates the main pointer and the pixel buffer, and returns the newly
  allocated object. The returned image must be deallocated using
  intimage_del().
 */
/*--------------------------------------------------------------------------*/
intimage * intimage_new(
        int lx,
        int ly) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Integer image destructor.
  @param    img     Integer image to deallocate.
  @return   void

  Deallocates the main pointer and the pixel buffer.
 */
/*--------------------------------------------------------------------------*/
void intimage_del(intimage * img) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Integer image loading from a FITS file.
  @param    filename    Name of the file to load.
  @return   1 newly allocated intimage.

  The loading is outsourced to the standard FITS image loader. Only FITS
  integer types are supported.
 */
/*--------------------------------------------------------------------------*/
intimage * intimage_load(char * filename) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Save an intimage to disk as an integer FITS file.
  @param    img         Integer image to save.
  @param    filename    Output file name.
  @return   void

  Save an intimage to disk as an integer FITS file. The number of bits per
  pixel is determined by the resolution of the intpix type.
 */
/*--------------------------------------------------------------------------*/
void intimage_save(
        intimage    *   img,
        char        *   filename) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    add 1 in a defined zone
  @param    img     Integer image (modified !)
  @param    xpos    x coordinate of the zone start
  @param    ypos    y coordinate of the zone start
  @param    xsize   x size of the zone
  @param    ysize   y size of the zone
  @return   0 if ok, -1 otherwise
    
  Input image is modified. The coordinates respect C convention: first 
  pixel is (0,0).

 */
/*--------------------------------------------------------------------------*/
int intimage_increment_zone(
        intimage    *   img,
        int             xpos,
        int             ypos,
        int             xsize,
        int             ysize) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Labelize a pixel map into an intimage
  @param    map         Pixelmap to labelize.
  @param    maxlabel    Returned number of labels found in map.
  @return   1 newly allocated intimage.

  This function labelizes all blobs in a pixelmap. All 4-neighbour
  connected zones set to 1 in the input pixel map will end up in
  the return intimage as zones where all pixels are set to the same
  (unique for this blob in this image) label.

  A non-recursive flood-fill is applied to label the zones. The flood-fill
  is dimensioned by the number of lines in the image, and the maximal
  number of lines possibly covered by a blob.
 */
/*--------------------------------------------------------------------------*/
intimage * intimage_labelize_pixelmap(
        pixelmap    *   map,
        int         *   maxlabel) ;

#endif
