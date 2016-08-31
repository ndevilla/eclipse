
/*----------------------------------------------------------------------------
   
   File name 	:	local_types.h
   Author 		:	Nicolas Devillard
   Created on	:	Nov 27, 1995
   Description	:	all shared local types for eclipse

 ---------------------------------------------------------------------------*/

/*
	$Id: local_types.h,v 1.47 2003/01/14 11:53:17 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/14 11:53:17 $
	$Revision: 1.47 $
 */

#ifndef _ECLIPSE_TYPES_H_
#define _ECLIPSE_TYPES_H_


/*----------------------------------------------------------------------------
   								Includes
 *--------------------------------------------------------------------------*/

#include <sys/types.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "cube_defs.h"

/*----------------------------------------------------------------------------
   								Defines
 *--------------------------------------------------------------------------*/

/*
 * this makes use of "inline" invisible to compilers that do not support it
 */

#ifndef inline
#define inline
#endif


/*----------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

typedef unsigned char BYTE ;

/* defined in limits.h, redefined here for portability  */

#define LONG32_MIN  (long32)(-2147483647-1) 
#define LONG32_MAX  (long32)(2147483647)
#define ULONG32_MAX (ulong32)(4294967295)

#define SHRT16_MIN  (short16)(-32768)
#define SHRT16_MAX  (short16)(32767)
#define USHRT16_MAX (ushort16)(65535)



/*-------------------------------------------------------------------------*/
/**
  @brief	Overall pixel type used in the library.

  This is the generic pixel type used everywhere in the library. In no
  circumstance should any assumption be made on the actual numeric type
  used to store pixels. In particular, pixels could be changed to integers
  or floating-point values with large precision.

  Any numeric operation involving pixels must make use of casts (most
  generally to double).

  Currently, the default pixel type is 'float'. This can be changed to
  'double' by defining the symbol DOUBLEPIX at compile-time.
 */
/*-------------------------------------------------------------------------*/
/* <python> */
#ifdef DOUBLEPIX
typedef double	pixelvalue ;
#else
typedef float	pixelvalue ;
#endif
/* </python> */



/*-------------------------------------------------------------------------*/
/**
  @brief	The eclipse image structure.

  This structure is fairly simple and should be easy to interface with
  other image processing source code. It basically holds the size of the
  image in x and y and a pointer to a pixel buffer. The pixel buffer is
  1-dimensional for performance reasons. To access the pixel located at
  position (i,j), use:

  \begin{verbatim}
  pix = image->data[i+j*image->lx];
  \end{verbatim}
 */
/*-------------------------------------------------------------------------*/

typedef struct _image_
{
	/* Size of the image in x and y */
    int             	lx, ly ;
	/* Pointer to pixel buffer as a 1d buffer */
    pixelvalue      *	data ;
} image_t ;
 


/*-------------------------------------------------------------------------*/
/**
  @brief	A struct to hold image statistics.

  This structure is meant to hold various statistical parameters about an
  image. It might be enhanced to contain more values in the future.
 */
/*-------------------------------------------------------------------------*/
typedef struct _image_stats_
{
    pixelvalue  min_pix ;
    pixelvalue  max_pix ;
    double	    avg_pix ;
	pixelvalue	median_pix ;
    double      stdev ;
    double      energy ;
    double      flux ;
    double      absflux ;

    int         min_x, min_y ;
    int         max_x, max_y ;
	int			npix ;
} image_stats ;



/*-------------------------------------------------------------------------*/
/**
  @brief	A struct to hold various FITS header information.

  This structure holds information read or computed from a FITS file
  header, they are only useful for the FITS pixel reader to get incoming
  pixels into memory.

  This structure should be extended in the future to handle extensions.
 */
/*-------------------------------------------------------------------------*/
typedef struct _cube_info_ 
{
    int     lx ;
    int     ly ;
    int     n_im ;
    int     ptype ;
    int		headersize ;
    double  b_scale ;
    double  b_zero ;
} cube_info ;



/*-------------------------------------------------------------------------*/
/**
  @brief	Type of a pixel in a binary map.

  This type makes a pixel in a binary map an abstract type. The only valid
  assumptions about this type are that it can take only two values
  (PIXELMAP_0 and PIXELMAP_1) that can be used as the numbers 0 and 1
  (through proper casts).
 */
/*-------------------------------------------------------------------------*/
typedef unsigned char binpix ;



/*-------------------------------------------------------------------------*/
/**
  @brief	A binary image.

  This type identifies an image which pixels are of type 'binpix' (i.e.
  binary pixels). These images have special associated operators, they
  usually cannot be mixed with "normal" images except in specific
  functions.

  The fields define the size in X and Y and a pixel buffer, and also an
  integer indicating how many pixels are set to 1 in the image. Every
  function operating on pixel maps must update this field whenever
  relevant.
 */
/*-------------------------------------------------------------------------*/
typedef struct _pixelmap_
{
    int			lx, ly ;
    int			ngoodpix ;
    binpix	*	data ;
} pixelmap ;




/*-------------------------------------------------------------------------*/
/**
  @brief	A data cube.

  This structure holds a data cube, i.e. a list of images of same size in X
  and Y. It does not contain any pixel information, only pointers to
  image_t structures.
 */
/*-------------------------------------------------------------------------*/

typedef struct _cube_
{
	/* Cube size in X, Y, Z */
    int			lx, ly ;
    int			np ;
	/* Pointers to image zones     		*/
    image_t	**	plane ;
} cube_t ;
 
#endif 
