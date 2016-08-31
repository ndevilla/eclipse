/*-------------------------------------------------------------------------*/
/**
   @file    cube_save.h
   @author  Nicolas Devillard
   @date    May 1999
   @version $Revision: 1.21 $
   @brief   save cubes to FITS format
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: cube_save.h,v 1.21 2001/12/03 10:51:56 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2001/12/03 10:51:56 $
    $Revision: 1.21 $
*/

#ifndef _CUBE_SAVE_H_
#define _CUBE_SAVE_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "comm.h" 
#include "xmemory.h"
#include "cube_defs.h"
#include "image_handling.h"
#include "file_handling.h"
#include "qfits.h"
#include "history.h"

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Set default pixel depth for all consecutive cube writes.
  @param    bpp     Integer value indicating which FITS bpp to use.
  @return   int indicating the previous value before the change.

  Call this function to change the default pixel depth used to save
  cubes to FITS files. Once this function is called, all consecutive
  cube saves to FITS will use this pixel depth. The function returns
  the previous pixel depth before the change.

  Reminder: possible FITS pixel depths are 8, 16, 32, -32 and -64.
  If any other value is given, this function does nothing and
  returns 0.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_set_fits_bpp(int bpp) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Save a cube to disk in FITS format.
  @param    to_save     Cube to save.
  @param    filename    Output file name.
  @param    hs          History to dump into the output FITS header.
  @return   int 0 if Ok, -1 otherwise

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  Prefer cube_save_fits_hdrcopy to conserve headers, this version
  only outputs with a minimal header.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_save_fits_wh(cube_t * to_save, char * filename, history * hs) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Save a cube to disk in FITS format.
  @param    to_save     Cube to save.
  @param    filename    Output file name.
  @return   int 0 if Ok, -1 otherwise

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  Prefer cube_save_fits_hdrcopy to conserve headers, this version
  only outputs with a minimal header.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_save_fits(cube_t * to_save, char * filename) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Save a cube to disk in FITS format, using a provided header.
  @param    to_save     Cube to save.
  @param    filename    Output file name.
  @param    fh          FITS header to insert in output file.
  @return   int 0 of Ok, -1 otherwise

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  The provided FITS header will be dumped into the output file, after
  having been modified to reflect the cube properties: NAXIS, BITPIX,
  NAXIS1, NAXIS2 and NAXIS3 (if it exists) will have the values
  corresponding the cube size.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_save_fits_hdrdump(
        cube_t          *   to_save,
        char            *   filename,
        qfits_header    *   fh) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Save a cube to disk, copying the header from another file.
  @param    to_save     Cube to save.
  @param    filename    Output file name.
  @param    ref_file    Name of a reference file to use for header.
  @param    hs          History to add to the output header.
  @return   int 0 if Ok, -1 if error occurred.

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  The output header will be loaded from another file (ref_file),
  modified to reflect the cube properties (NAXIS, BITPIX, etc.),
  possibly extended with HISTORY cards from the history object, and
  then dumped in output.

  The reference file may also be the name of an ASCII list. In that
  case, the FITS header used for reference is the one of the first
  FITS file found in the ASCII list.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_save_fits_hdrcopy_wh(
        cube_t  *   to_save,
        char    *   filename,
        char    *   ref_file,
        history *   hs) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Save a cube to disk, copying the header from another file.
  @param    to_save     Cube to save.
  @param    filename    Output file name.
  @param    ref_file    Name of a reference file to use for header.
  @return   int 0 if Ok, -1 if error occurred.

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  The output header will be loaded from another file (ref_file),
  modified to reflect the cube properties (NAXIS, BITPIX, etc.) and
  then dumped in output.

  The reference file may also be the name of an ASCII list. In that
  case, the FITS header used for reference is the one of the first
  FITS file found in the ASCII list.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_save_fits_hdrcopy(
        cube_t  *   to_save,
        char    *   filename,
        char    *   ref_file) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Append image data to a file.
  @param    filename    Output file name.
  @param    appended    Image data to append.
  @param    pixtype     Pixel type to use when dumping to file.
  @return   int 0 if Ok, -1 otherwise.

  This function appends pixel data from an image into a given file, in
  the requested pixel type. No padding is done after the data have
  been dumped to the file.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_fits_appendimage(
        char           *    filename,
        image_t        *    appended,
        int                 pixtype) ;
/* </python> */

#endif
