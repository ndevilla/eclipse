/*-------------------------------------------------------------------------*/
/**
   @file    extraction.h
   @author  Nicolas Devillard
   @date    Mar 25, 1996
   @version $Revision: 1.15 $
   @brief   data extraction from a cube
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: extraction.h,v 1.15 2001/10/26 14:30:35 yjung Exp $
    $Author: yjung $
    $Date: 2001/10/26 14:30:35 $
    $Revision: 1.15 $
*/

#ifndef _EXTRACTION_H_
#define _EXTRACTION_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h" 
#include "file_handling.h"
#include "image_handling.h"
#include "cube_handling.h"

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Extract a rectangular zone from a cube into another cube.
  @param    cube_in     Input cube
  @param    loleft_x    Lower left X coordinate
  @param    loleft_y    Lower left Y coordinate
  @param    upright_x   Upper right X coordinate
  @param    upright_y   Upper right Y coordinate
  @return   1 newly allocated cube.
 
  The input coordinates define the extracted region by giving the
  coordinates of the lower left and upper right corners (inclusive).
 
  Coordinates must be provided in the FITS convention: lower left
  corner of the image is at (1,1), x growing from left to right,
  y growing from bottom to top.
 
  The same rectangle is extracted from each plane in the input cube,
  and appended to the output cube.
 
  The returned cube contains pixel copies of the input pixels. It must be
  freed using cube_del().
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_getvig(
        cube_t *    cube_in,
        int         loleft_x,
        int         loleft_y,
        int         upright_x,
        int         upright_y) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Extract a rectangular zone from an image into another image.
  @param    image_in    Input image
  @param    loleft_x    Lower left X coordinate
  @param    loleft_y    Lower left Y coordinate
  @param    upright_x   Upper right X coordinate
  @param    upright_y   Upper right Y coordinate
  @return   1 newly allocated image.
 
  The input coordinates define the extracted region by giving the
  coordinates of the lower left and upper right corners (inclusive).
 
  Coordinates must be provided in the FITS convention: lower left
  corner of the image is at (1,1), x growing from left to right,
  y growing from bottom to top.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * image_getvig(
        image_t    *    image_in,
        int             loleft_x,
        int             loleft_y,
        int             upright_x,
        int             upright_y) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Extract a row from an image.
  @param    image1      Image to process.
  @param    row_num     Row number.
  @return   1 newly allocated array of pixelvalues.
 
  Extracts a row of pixels from an image. The row number goes from 0 to
  ly-1. The returned array must be freed using free().
 */
/*--------------------------------------------------------------------------*/
/* <python> */
pixelvalue * image_getrow(
        image_t *  image1,
        int         row_num) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Extract a column from an image.
  @param    image       Image to process
  @param    col_num     Column number.
  @return   1 newly allocated array of pixelvalues.
 
  Extracts a column of pixels from an image. The column number goes from
  0 to lx-1. The returned array must be freed using free().
 */
/*--------------------------------------------------------------------------*/
/* <python> */
pixelvalue * image_getcol(
        image_t *   image1,
        int         col_num) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Extract a time line along the z-axis of a cube.
  @param    cube1       Cube to process.
  @param    pos         Position on the detector.
  @return   1 newly allocated image containing one line only.
 
  Extract a line of pixels along the z-axis of a cube. All pixels lying
  on the same detector position are extracted in each plane. A new array
  of pixelvalues is returned, containing as many pixels as planes in the
  input cube. This array is stored into an image containing a single line.
 
  The detector position must be provided as a single number understood
  as i + j*lx, where (i,j) is the position on the detector, in the C
  coordinate convention (i runs from 0 to lx-1, j runs from 0 to ly-1).
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_get_z(
        cube_t  *   cube1,
        int         pos) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Create a new cube containing less planes, according to a list.
  @param    cube1       Input cube.
  @param    planes      List of plane indexes to copy.
  @param    np          Number of indexes in the list.
  @return   1 newly allocated cube.

  Create a new cube by extracting from the input cube only the planes
  selected in the list of indexes. Indexes run from 0 to np-1 (incl).

  The returned cube copies the planes of the input cube. It must be freed
  using cube_del().
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_copy_planes(
        cube_t  *   cube1,
        int     *   planes,
        int         np) ;
/* </python> */

#endif
