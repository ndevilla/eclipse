/*----------------------------------------------------------------------------*/
/**
   @file    fits_rw.h
   @author  N. Devillard
   @date    Mar 2000
   @version $Revision: 1.11 $
   @brief   FITS header reading/writing.
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: fits_rw.h,v 1.11 2004/06/03 15:42:06 yjung Exp $
    $Author: yjung $
    $Date: 2004/06/03 15:42:06 $
    $Revision: 1.11 $
*/

#ifndef FITS_RW_H
#define FITS_RW_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "fits_std.h"
#include "fits_h.h"

/*-----------------------------------------------------------------------------
						Function ANSI prototypes
 -----------------------------------------------------------------------------*/

/* <dox> */
/*----------------------------------------------------------------------------*/
/**
  @brief    Read a FITS header from a file to an internal structure.
  @param    filename    Name of the file to be read
  @return   Pointer to newly allocated qfits_header or NULL in error case.

  This function parses a FITS (main) header, and returns an allocated
  qfits_header object. The qfits_header object contains a linked-list of
  key "tuples". A key tuple contains:

  - A keyword
  - A value
  - A comment
  - An original FITS line (as read from the input file)

  Direct access to the structure is not foreseen, use accessor
  functions in fits_h.h

  Value, comment, and original line might be NULL pointers.
 */
/*----------------------------------------------------------------------------*/
qfits_header * qfits_header_read(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Read a FITS header from a 'hdr' file.
  @param    filename    Name of the file to be read
  @return   Pointer to newly allocated qfits_header or NULL in error case

  This function parses a 'hdr' file, and returns an allocated qfits_header 
  object. A hdr file is an ASCII format were the header is written with a 
  carriage return after each line. The command dfits typically displays 
  a hdr file.
 */
/*----------------------------------------------------------------------------*/
qfits_header * qfits_header_read_hdr(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Read a FITS header from a 'hdr' string
  @param    hdr_str         String containing the hdr file
  @param    nb_char         Number of characters in the string
  @return   Pointer to newly allocated qfits_header or NULL in error case

  This function parses a 'hdr' string, and returns an allocated qfits_header 
  object. 
 */
/*----------------------------------------------------------------------------*/
qfits_header * qfits_header_read_hdr_string(
        unsigned char   *   hdr_str,
        int                 nb_char) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Read an extension header from a FITS file.
  @param    filename    Name of the FITS file to read
  @param    xtnum       Extension number to read, starting from 0.
  @return   Newly allocated qfits_header structure.

  Strictly similar to qfits_header_read() but reads headers from
  extensions instead. If the requested xtension is 0, this function
  calls qfits_header_read() to return the main header.

  Returns NULL in case of error.
 */
/*----------------------------------------------------------------------------*/
qfits_header * qfits_header_readext(char * filename, int xtnum) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Pad an existing file with zeros to a multiple of 2880.
  @param    filename    Name of the file to pad.
  @return   void

  This function simply pads an existing file on disk with enough zeros
  for the file size to reach a multiple of 2880, as required by FITS.
 */
/*----------------------------------------------------------------------------*/
void qfits_zeropad(char * filename) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Identify if a file is a FITS file.
  @param    filename name of the file to check
  @return   int 0, 1, or -1

  Returns 1 if the file name looks like a valid FITS file. Returns
  0 else. If the file does not exist, returns -1.
 */
/*----------------------------------------------------------------------------*/
int is_fits_file(char *filename) ;
/* </dox> */

#ifdef __cplusplus
}
#endif

#endif
/* vim: set ts=4 et sw=4 tw=75 */
