/*----------------------------------------------------------------------------*/
/**
   @file    pixelmaps.h
   @author  Nicolas Devillard
   @date    Mar 03, 1997
   @version $Revision: 1.21 $
   @brief   pixel map handling
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: pixelmaps.h,v 1.21 2003/04/24 14:33:28 yjung Exp $
    $Author: yjung $
    $Date: 2003/04/24 14:33:28 $
    $Revision: 1.21 $
*/

#ifndef _PIXEL_MAPS_H_
#define _PIXEL_MAPS_H_

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "xmemory.h"
#include "cube_defs.h"
#include "cube_handling.h"
#include "cube2image.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define PIXELMAP_1	((binpix)1)
#define PIXELMAP_0	((binpix)0)

/*-----------------------------------------------------------------------------
  						Function ANSI C prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Allocate a new pixel map.
  @param    lx  Size in x.
  @param    ly  Size in y.
  @return   1 newly allocated pixelmap.

  Allocates space to hold a pixelmap and its pixel buffer. All pixels in
  the buffer are set to PIXELMAP_1.

  The returned object must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelmap * pixelmap_new(int lx, int ly) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the number of PIXELMAP_1    
  @param    Pointer to a pixelmap struct.
  @return   int
 */
/*----------------------------------------------------------------------------*/
int pixelmap_getselected(pixelmap * map) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the size in bytes of a pixelmap structure in memory.
  @param    Pointer to a pixelmap struct.
  @return   int, size of the struct and associated fields in bytes

  This function computes approximately the size occupied by a pixelmap
  struct in memory. It takes into account both the size of the struct
  itself and the size of the associated memory zone (pixels).

  It is meant to be used with a garbage collector, to declare the size
  in bytes for future collection.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int pixelmap_getbytesize(pixelmap * map) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Deallocates a pixelmap.
  @param    p   Pixel map to deallocate.
  @return   void

  Deallocates pixel buffer and structure associated to a pixel map.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
void pixelmap_del(pixelmap * p) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief        Make a copy of a pixel map.
  @param        in      Original pixelmap.
  @return       1 newly allocated pixelmap.

  Make a copy of a pixelmap. Copies both the structure and the pixel array.
  The returned object must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelmap * pixelmap_copy(pixelmap * in) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Threshold an image to a pixel map.
  @param    in      Image to threshold.
  @param    lo_cut  Lower bound for threshold.
  @param    hi_cut  Higher bound for threshold.
  @return   1 newly allocated pixelmap.

  Create a pixel map from an image. All pixels outside of the provided
  bounds will produce a PIXELMAP_0 in the output pixel map, all other
  pixels (i.e. within bounds) will produce a PIXELMAP_1 in the output map.

  The returned map must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelmap * image_threshold2pixelmap(
    image_t    *    in,
    double          lo_cut,
    double          hi_cut) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Update a pixel map with another.
  @param    p1  Pixel map to update.
  @param    p2  Pixel map to consider to update p1.
  @return   void

  Updates the first pixel map with the second, i.e. all pixels set to zero
  in the second map are also set to zero in the first map.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
void pixelmap_update(pixelmap * p1, pixelmap * p2) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Dump a pixel map to disk in FITS format.
  @param    p           Pixel map to dump.
  @param    filename    Name of the output FITS file.
  @return   void

  Dumps a pixel map to disk as a FITS file in 8 bits/pel format. The header
  is minimal.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
void pixelmap_dump(pixelmap * p, char * filename) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Load a pixel map from disk into memory.
  @param    filename    Name of the file to load.
  @return   1 newly allocated pixelmap.

  Loads a pixel map from a FITS file into a pixelmap object. A pixel map
  on disk can be any integer FITS image (BITPIX is 8, 16 or 32). Any
  non-zero pixel will be read as PIXELMAP_1, any zero pixel will be read as
  PIXELMAP_0.

  The returned map must be deallocated using pixelmap_del().
 */
/*----------------------------------------------------------------------------*/
/* <python> */
pixelmap * pixelmap_load(char * filename) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Convert a pixelmap to an image object.
  @param    p   Pixel map to be promoted.
  @return   1 newly allocated image.

  Convert a pixel map to a valid image. The returned object is a newly
  allocated image which must be deallocated using image_del().

  PIXELMAP_0 is converted to (pixelvalue)0 and PIXELMAP_1 is converted to
  (pixelvalue)1.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
image_t * pixelmap_2_image(pixelmap * p) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Update the ngoodpix field in a pixelmap structure.
  @param    p   Pixel map to update.
  @return   void

  Update the ngoodpix field in a pixelmap structure, by simply counting
  the number of pixels set to PIXELMAP_1.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
void pixelmap_updatecount(pixelmap * p) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a binary AND between two pixel maps.
  @param    p1  First operand.
  @param    p2  Second operand.
  @return   int 0 if Ok, anything else otherwise.

  Modifies the first input pixel map to contain the result of the operation
  p1 AND p2.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int pixelmap_binary_AND(pixelmap * p1, pixelmap * p2) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a binary OR between two pixel maps.
  @param    p1  First operand.
  @param    p2  Second operand.
  @return   int 0 if Ok, anything else otherwise.

  Modifies the first input pixel map to contain the result of the operation
  p1 OR p2.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int pixelmap_binary_OR(pixelmap * p1, pixelmap * p2) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a binary XOR between two pixel maps.
  @param    p1  First operand.
  @param    p2  Second operand.
  @return   int 0 if Ok, anything else otherwise.

  Modifies the first input pixel map to contain the result of the operation
  p1 XOR p2.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int pixelmap_binary_XOR(pixelmap * p1, pixelmap * p2) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a binary NOT on a pixel map.
  @param    p1  Pixel map to modify.
  @return   int 0 if Ok, anything else otherwise.

  Modifies the input pixel map to contain the result of the operation
  NOT p1.
 */
/*----------------------------------------------------------------------------*/
/* <python> */
int pixelmap_binary_NOT(pixelmap * p1) ;
/* </python> */

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological erosion with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise

  This function performs a binary erosion and modifies the input pixel map
  to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_erosion(pixelmap * in) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological erosion with a user-defined kernel.
  @param    mi      Input pixel map.
  @param    mk      User-specified binary kernel.
  @return   int 0 if Ok, -1 else.
 
  This function performs a binary erosion using a user-defined kernel.
  The input pixel map is modified.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_erosion_k(pixelmap * mi, pixelmap * mk) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological dilation with a user-defined kernel.
  @param    mi      Input pixel map.
  @param    mk      User-specified binary kernel.
  @return   int 0 if Ok, -1 else.
 
  This function performs a binary dilation using a user-defined kernel.
  The input pixel map is modified.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_dilation_k(pixelmap * mi, pixelmap * mk) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Performs a morphological dilation with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise

  This function performs a binary dilation and modifies the input pixel map
  to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_dilation(pixelmap * in) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Perform a morphological closing with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise.
 
  This function performs a binary morphological closing and modifies the
  input pixel map to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_closing(pixelmap *in) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Perform a morphological opening with a 3x3 kernel.
  @param    in  Input pixel map.
  @return   int 0 if Ok, -1 otherwise.
 
  This function performs a binary morphological opening and modifies the
  input pixel map to store the result of the operation.
 */
/*----------------------------------------------------------------------------*/
int pixelmap_morpho_opening(pixelmap *in) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a rectangular zone from an pixelmap into another pixelmap.
  @param    map_in      Input pixelmap
  @param    loleft_x    Lower left X coordinate
  @param    loleft_y    Lower left Y coordinate
  @param    upright_x   Upper right X coordinate
  @param    upright_y   Upper right Y coordinate
  @return   1 newly allocated pixelmap.
 
  The input coordinates define the extracted region by giving the
  coordinates of the lower left and upper right corners (inclusive).
 
  Coordinates must be provided in the FITS convention: lower left
  corner of the pixelmap is at (1,1), x growing from left to right,
  y growing from bottom to top.
 */
/*----------------------------------------------------------------------------*/
pixelmap * pixelmap_getvig(
        pixelmap    *   map_in,
        int             loleft_x,
        int             loleft_y,
        int             upright_x,
        int             upright_y) ;

#endif
