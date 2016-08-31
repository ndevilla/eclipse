
/*-------------------------------------------------------------------------*/
/**
   @file	image_io.h
   @author	Nicolas Devillard
   @date	Nov 2001
   @version	$Revision: 1.4 $
   @brief	Image loading/saving methods
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: image_io.h,v 1.4 2002/06/28 12:09:15 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/06/28 12:09:15 $
	$Revision: 1.4 $
*/
#ifndef _IMAGE_IO_H_
#define _IMAGE_IO_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "cube_handling.h"
#include "image_rtd.h"
#include "qfits.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Alias image_load() to image_loadext().

  This alias makes it easier to load an image in the main FITS part
  (xtnum=0).
 */
/*-------------------------------------------------------------------------*/
#define image_load(f)   image_loadext(f,0,0,1)

/*-------------------------------------------------------------------------*/
/**
  @brief	Alias image_load_nomap() to image_loadext().

  This alias makes it easier to load an image in the main FITS part
  (xtnum=0). This version ensures no mapping is performed, i.e. the
  returned pixels are guaranteed to be the ones in the initial file.
 */
/*-------------------------------------------------------------------------*/
#define image_load_nomap(f)   image_loadext(f,0,0,0)

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Load an image from a file.
  @param    filename    Name of the image to load.
  @param    xtnum       Extension to consider for load (0=main)
  @param    pnum        Plane number in extension
  @param    map         File mapping request flag.
  @return   1 newly allocated image.

  Load a FITS image from a file. Returns NULL in case of error.
  The returned image must be deallocated using image_del().

  xtnum is the extension to consider, or 0 for the main data section.
  pnum is the plane number to consider in the requested extension.
  It is useful when NAXIS3 is more than 1. pnum runs from 0 to NAXIS3-1,
  i.e. to load the first image in the main data section you would do:

  map is a flag that indicates whether you want the image mapped
  from the input file (i.e. the pointer you get results from an
  mmap call), or if you prefer to get an image copy (result
  of malloc+fread). Mapping an image is always faster and saves
  memory resources, but has side-effects: if the file is mapped
  several times, any change brought to it in one part of the code
  will be seen in all other mappings.

  @code
  image_t * im = image_loadext("a.fits", 0, 0, 0);
  @endcode
 */
/*--------------------------------------------------------------------------*/
image_t * image_loadext(char *filename, int xtnum, int pnum, int map);

/*-------------------------------------------------------------------------*/
/**
  @brief	Save an image to a FITS file on disk.
  @param	to_save			Image to save.
  @param	filename		Name of the output file.
  @param	pixel_type		Pixel type to use in output.
  @param	hs				History to be appended to FITS header.
  @return	void

  Saves an image structure to a FITS file on disk. Request the pixel type
  you want in output, but beware of losses in precision! Saving a
  floating-point image to an integer buffer will kill some pixel values.

  Possible pixel types are:
  \begin{itemize}
  \item {\tt BPP_8_UNSIGNED}
  \item {\tt BPP_16_SIGNED}
  \item {\tt BPP_32_SIGNED}
  \item {\tt BPP_IEEE_FLOAT}
  \item {\tt BPP_IEEE_DOUBLE}
  \item {\tt BPP_DEFAULT}
  \end{itemize}

  The safe bet is always to use BPP_DEFAULT as output pixel type, which
  always corresponds to the pixel type in memory, i.e. you are sure to keep
  always the pixel precision. You could also always save in double
  precision, but that simply doubles the size you need on disk...
 */
/*--------------------------------------------------------------------------*/
void image_save_fits_wh(
		image_t    	*	to_save,
	    char       	*	filename,
	    int         	pixel_type,
		history		*	hs);

/*-------------------------------------------------------------------------*/
/**
  @brief	Save an image to a FITS file on disk.
  @param	to_save			Image to save.
  @param	filename		Name of the output file.
  @param	pixel_type		Pixel type to use in output.
  @return	void

  Saves an image structure to a FITS file on disk. Request the pixel type
  you want in output, but beware of losses in precision! Saving a
  floating-point image to an integer buffer will kill some pixel values.

  Possible pixel types are:
  \begin{itemize}
  \item {\tt BPP_8_UNSIGNED}
  \item {\tt BPP_16_SIGNED}
  \item {\tt BPP_32_SIGNED}
  \item {\tt BPP_IEEE_FLOAT}
  \item {\tt BPP_IEEE_DOUBLE}
  \item {\tt BPP_DEFAULT}
  \end{itemize}

  The safe bet is always to use BPP_DEFAULT as output pixel type, which
  always corresponds to the pixel type in memory, i.e. you are sure to keep
  always the pixel precision. You could also always save in double
  precision, but that simply doubles the size you need on disk...
 */
/*--------------------------------------------------------------------------*/
void image_save_fits(
		image_t    *	to_save,
	    char       *	filename,
	    int         	pixel_type);

/*-------------------------------------------------------------------------*/
/**
  @brief	Save an image to a FITS file with a given FITS header.
  @param	to_save			Image to save.
  @param	filename		Name of the image on disk.
  @param	fh				FITS header to insert in the output file.
  @param	pixel_type		Output image pixel type in the FITS file.
  @return	void

  Save an image to a FITS file, with a given FITS header.
  See image_save_fits() for possible pixel types.
 */
/*--------------------------------------------------------------------------*/
void image_save_fits_hdrdump(
		image_t      *	to_save,
	    char         *	filename,
		qfits_header *	fh,
	    int         	pixel_type);

/*-------------------------------------------------------------------------*/
/**
  @brief	Save an image to a FITS file, copying the header from another
  			file and adding HISTORY entries.
  @param	to_save			Image to save.
  @param	filename		Name of the output file.
  @param	ref_filename	Name of a reference FITS file to use.
  @param	pixel_type		Output pixel type.
  @param	hs				History to be appended to FITS header.
  @return	void

  Save an image to a FITS file on disk, using the FITS header from another
  file as a reference and adding the appropriate HISTORY fields as
  provided. The header from the reference file is read into memory, and the
  relevant keywords are updated to reflect the image to save, then the new
  header and the image pixels are dumped to the output file.

  It is Ok to provide a NULL history object if you do not want to add any
  HISTORY field in output.
  
  See image_save_fits() for possible pixel types.
 */
/*--------------------------------------------------------------------------*/
void image_save_fits_hdrcopy_wh(
		image_t *	to_save,
	    char    *	filename,
	    char    *	ref_filename,
	    int        	pixel_type,
		history	*	hs);

/*-------------------------------------------------------------------------*/
/**
  @brief	Save an image to a FITS file, copying the header from another
  			file.
  @param	to_save			Image to save.
  @param	filename		Name of the output file.
  @param	ref_filename	Name of a reference FITS file to use.
  @param	pixel_type		Output pixel type.
  @return	void

  Save an image to a FITS file on disk, using the FITS header from another
  file as a reference. The header from the reference file is read into
  memory, and the relevant keywords are updated to reflect the image to
  save, then the new header and the image pixels are dumped to the output
  file.
  
  See image_save_fits() for possible pixel types.
 */
/*--------------------------------------------------------------------------*/
void image_save_fits_hdrcopy(
		image_t	*	to_save,
    	char	*	filename,
    	char	*	ref_filename,
    	int			pixel_type);

#endif
