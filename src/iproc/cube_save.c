/*-------------------------------------------------------------------------*/
/**
   @file	cube_save.c
   @author	Nicolas Devillard
   @date	May 1999
   @version	$Revision: 1.48 $
   @brief	save cubes to FITS format
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: cube_save.c,v 1.48 2002/02/05 13:35:23 rengelin Exp $
	$Author: rengelin $
	$Date: 2002/02/05 13:35:23 $
	$Revision: 1.48 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <string.h>
#include "cube_save.h"
#include "qfits.h"

/*---------------------------------------------------------------------------
							Static variables
 ---------------------------------------------------------------------------*/

static int fits_bpp_save = BPP_DEFAULT ;

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Set default pixel depth for all consecutive cube writes.
  @param	bpp		Integer value indicating which FITS bpp to use.
  @return	int indicating the previous value before the change.

  Call this function to change the default pixel depth used to save
  cubes to FITS files. Once this function is called, all consecutive
  cube saves to FITS will use this pixel depth. The function returns
  the previous pixel depth before the change.

  Reminder: possible FITS pixel depths are 8, 16, 32, -32 and -64.
  If any other value is given, this function does nothing and
  returns 0.
 */
/*--------------------------------------------------------------------------*/
int cube_set_fits_bpp(int bpp)
{
	int	prev ;

	if ((bpp!= BPP_8_UNSIGNED) &&
		(bpp!= BPP_16_SIGNED) &&
		(bpp!= BPP_32_SIGNED) &&
		(bpp!= BPP_IEEE_FLOAT) &&
		(bpp!= BPP_IEEE_DOUBLE)) {
		return 0 ;
	}
	prev = fits_bpp_save ;
	fits_bpp_save = bpp ;
	return prev ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Save a cube to disk in FITS format.
  @param	to_save		Cube to save.
  @param	filename	Output file name.
  @param	hs			History to dump into the output FITS header.
  @return	int 0 if Ok, -1 otherwise

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  Prefer cube_save_fits_hdrcopy to conserve headers, this version
  only outputs with a minimal header.
 */
/*--------------------------------------------------------------------------*/
int cube_save_fits_wh(cube_t * to_save, char * filename, history * hs)
{
	qfits_header*	fh ;
	FILE		*	out ;
	char			cval[80];
    int				i ;
	char		*	md5hash ;
	char			md5card[81];

	/* Sanity checks */
    if ((to_save == NULL) || (filename==NULL)) return -1 ;
    if ((to_save->lx > MAX_COLUMN_NUMBER) ||
        (to_save->lx < 1) ||
        (to_save->ly > MAX_LINE_NUMBER) ||
        (to_save->ly < 1) ||
        (to_save->np > MAX_IMAGE_NUMBER) ||
        (to_save->np < 1)) {
        e_error("invalid cube size [%dx%dx%d]: cannot save",
				to_save->lx,
				to_save->ly,
				to_save->np);
        return -1 ;
    }

	/* Make a default FITS header for this cube */
	fh = qfits_header_default();
	/* BITPIX */
	sprintf(cval, "%d", fits_bpp_save);
	qfits_header_add(fh, "BITPIX", cval, "Bits per pixel", NULL);
	/* NAXIS */
	if (to_save->np > 1) {
		qfits_header_add(fh, "NAXIS", "3", "File dimension", NULL);
	} else {
		qfits_header_add(fh, "NAXIS", "2", "File dimension", NULL);
	}
	/* NAXIS1 NAXIS2 (NAXIS3) */
	sprintf(cval, "%d", to_save->lx);
	qfits_header_add(fh, "NAXIS1", cval, "Size in x", NULL);
	sprintf(cval, "%d", to_save->ly);
	qfits_header_add(fh, "NAXIS2", cval, "Size in y", NULL);
	if (to_save->np>1) {
		sprintf(cval, "%d", to_save->np);
		qfits_header_add(fh, "NAXIS3", cval, "Number of planes", NULL);
	}
    /* 
     * Adding BSCALE and BZERO keywords for Workcid compatibility
     * eclipse works internally without scaling and offset, so the
     * values shall always be BSCALE=1 and BZERO=0.
     */
	qfits_header_add(fh, "BSCALE", "1.0", "pixel scale factor", NULL);
	qfits_header_add(fh, "BZERO",  "0.0", "pixel offset", NULL);
	/* Add eclipse signature */
	qfits_header_add(fh, "ECLIPSE", "1", "created by eclipse", NULL);
	qfits_header_add(fh, "ORIGIN", "eclipse", "created by eclipse", NULL);

	/* Add data MD5 signature placeholder*/
	qfits_header_add(fh, "DATAMD5", "'0'",  "MD5 checksum", NULL);

    /* Write the history object into header */
	if (hs!=NULL)
		history_addfits(hs, fh);

    /* Ouput header to file */
	if (!strcmp(filename, "STDOUT")) {
		out = stdout ;
	} else {
		out = fopen(filename, "w");
	}
	if (out==NULL) {
		e_error("writing to file [%s]", filename);
		qfits_header_destroy(fh);
		return -1 ;
	}
	qfits_header_dump(fh, out);
	if (out!=stdout)
		fclose(out);
	qfits_header_destroy(fh);

    /* Convert planes one by one and copy them into Fits structure  */
    for (i=0 ; i<to_save->np ; i++) {
		if (to_save->np>1)
			compute_status("converting plane", i, to_save->np, 3) ;
        if (cube_fits_appendimage( filename,
											to_save->plane[i],
											fits_bpp_save)!=0){
            e_error("cannot append plane %d to file [%s]: aborting save",
					i+1, filename) ;
            return -1 ;
        }
    }
	/* Zero-pad the FITS file */
    qfits_zeropad(filename) ; 

	/* Add MD5 signature (not for STDOUT) */
	if (strcmp(filename, "STDOUT")) {
		md5hash = qfits_datamd5(filename);
		if (md5hash==NULL) {
			e_error("computing MD5 signature for output file %s", filename);
			return -1 ;
		}
		sprintf(md5card, "DATAMD5 = '%s' / MD5 checksum", md5hash);
		qfits_replace_card(filename, "DATAMD5", md5card);
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Save a cube to disk in FITS format.
  @param	to_save		Cube to save.
  @param	filename	Output file name.
  @return	int 0 if Ok, -1 otherwise

  Saves a cube to disk in FITS format. The given file name may include
  a complete path like '/data/output/result.fits', or be a simple name
  like 'result.fits'.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.

  Prefer cube_save_fits_hdrcopy to conserve headers, this version
  only outputs with a minimal header.
 */
/*--------------------------------------------------------------------------*/
int cube_save_fits(cube_t * to_save, char * filename)
{
	return cube_save_fits_wh(to_save, filename, NULL);
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Save a cube to disk in FITS format, using a provided header.
  @param	to_save		Cube to save.
  @param	filename	Output file name.
  @param	fh			FITS header to insert in output file.
  @return	int 0 of Ok, -1 otherwise

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
int cube_save_fits_hdrdump(  
		cube_t			*	to_save,
		char			*	filename,
		qfits_header	*	fh)
{
    int			i ;
	FILE	*	out ;
	char		cval[80];
	int			use_default_hdr ;
	char	*	md5hash ;
	char		md5card[81];

    
    /* Error handling : test entry  */
    if (to_save==NULL || filename==NULL) return -1 ;
    if ((to_save->lx > MAX_COLUMN_NUMBER) ||
        (to_save->lx < 1) ||
        (to_save->ly > MAX_LINE_NUMBER) ||
        (to_save->ly < 1) ||
        (to_save->np > MAX_IMAGE_NUMBER) ||
        (to_save->np < 1)) {
        e_error("invalid cube size [%dx%dx%d]: cannot save",
				to_save->lx,
				to_save->ly,
				to_save->np);
        return -1 ;
    }

	if (fh==NULL) {
		fh = qfits_header_default();
		use_default_hdr=1 ;
	} else {
		use_default_hdr=0 ;
	}

	/* Modify main entries to reflect this cube */
	/* BITPIX */
	qfits_header_del(fh, "BITPIX");
	sprintf(cval, "%d", fits_bpp_save);
	qfits_header_add(fh, "BITPIX", cval, "bits per pixel", NULL);

	/* NAXIS, NAXIS1, NAXIS2, NAXIS3 */
	qfits_header_del(fh, "NAXIS");
	qfits_header_del(fh, "NAXIS1");
	qfits_header_del(fh, "NAXIS2");
	qfits_header_del(fh, "NAXIS3");
	qfits_header_del(fh, "DATAMD5");

	if (to_save->np > 1) {
		qfits_header_add_after(fh, "BITPIX", "NAXIS", "3", "data cube", NULL);
		sprintf(cval, "%d", to_save->lx);
		qfits_header_add_after(fh, "NAXIS",  "NAXIS1", cval, "x size", NULL);
		sprintf(cval, "%d", to_save->ly);
		qfits_header_add_after(fh, "NAXIS1", "NAXIS2", cval, "y size", NULL);
		sprintf(cval, "%d", to_save->np);
		qfits_header_add_after(fh, "NAXIS2", "NAXIS3", cval, "z size", NULL);
	} else {
		qfits_header_add_after(fh, "BITPIX","NAXIS","2","single image",NULL);
		sprintf(cval, "%d", to_save->lx);
		qfits_header_add_after(fh, "NAXIS",  "NAXIS1", cval, "x size", NULL);
		sprintf(cval, "%d", to_save->ly);
		qfits_header_add_after(fh, "NAXIS1", "NAXIS2", cval, "y size", NULL);
	}

	qfits_header_mod(fh, "BSCALE", "1.0", "pixel scale factor");
	qfits_header_mod(fh, "BZERO",  "0.0", "pixel value offset");

	/* Add data MD5 signature placeholder*/
	qfits_header_add(fh, "DATAMD5", "'0'",  "MD5 checksum", NULL);

	if (!strcmp(filename, "STDOUT")) {
		out = stdout;
	} else {
		out = fopen(filename, "w");
	}
	if (out==NULL) {
		e_error("writing to file [%s]", filename);
		return -1 ;
	}
	qfits_header_dump(fh, out);
	if (out!=stdout)
		fclose(out);

	if (use_default_hdr) {
		qfits_header_destroy(fh);
	}

    /* Convert planes one by one and copy them into Fits structure  */
    for (i=0 ; i<to_save->np ; i++) {
		if (to_save->np>1)
			compute_status("converting plane", i, to_save->np, 3) ;
        if (cube_fits_appendimage( filename,
											to_save->plane[i],
											fits_bpp_save)!=0){
            e_error("cannot append plane %d to file [%s]: aborting save",
					i+1, filename) ;
            return -1 ;
        }
    }
    qfits_zeropad(filename) ; 
	/* Add MD5 signature */
	md5hash = qfits_datamd5(filename);
	if (md5hash==NULL) {
		e_error("computing MD5 signature for output file %s", filename);
		return -1 ;
	}
	sprintf(md5card, "DATAMD5 = '%s' / MD5 checksum", md5hash);
	qfits_replace_card(filename, "DATAMD5", md5card);
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Save a cube to disk, copying the header from another file.
  @param	to_save		Cube to save.
  @param	filename	Output file name.
  @param	ref_file	Name of a reference file to use for header.
  @param	hs			History to add to the output header.
  @return	int 0 if Ok, -1 if error occurred.

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
int cube_save_fits_hdrcopy_wh(
		cube_t	*	to_save, 
		char	*	filename,
		char	*	ref_file,
		history	*	hs)
{
    qfits_header*	fh ;	
	char		*	ref_name ;
	int				header_ok ;
	int				status ;

    /* Error handling : test entry  */
    if (to_save==NULL || filename==NULL) return -1 ;
	header_ok=1 ;
	ref_name=NULL ;
	if (ref_file==NULL) {
		header_ok=0 ;
	} else if (!strcmp(ref_file, "RTD")) {
		header_ok=0 ;
	} else if (is_fits_file(ref_file)==1) {
		ref_name = ref_file;
	} else if (is_ascii_list(ref_file)==1) {
		ref_name = framelist_firstname(ref_file);
		if (ref_name==NULL) {
			e_error("getting a valid FITS file name out of list %s", ref_file);
			header_ok=0 ;
		}
	} else {
		e_error("cannot find reference FITS header out of file %s", ref_file);
		header_ok=0 ;
	}

	fh=NULL ;
	if (ref_name!=NULL) {
		fh = qfits_header_read(ref_name);
		if (fh==NULL) {
			e_error("reading header from [%s]", ref_name);
			header_ok = 0 ;
		}
	}
	
	if (header_ok) {
		/* Add history cards */
		if (hs!=NULL) {
			history_addfits(hs, fh);
		}
		status = cube_save_fits_hdrdump(to_save, filename, fh);
		qfits_header_destroy(fh);
	} else {
		e_warning("saving cube with default (empty) header");
		status = cube_save_fits_wh(to_save, filename, hs);
	}
	return status ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Save a cube to disk, copying the header from another file.
  @param	to_save		Cube to save.
  @param	filename	Output file name.
  @param	ref_file	Name of a reference file to use for header.
  @return	int 0 if Ok, -1 if error occurred.

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
int cube_save_fits_hdrcopy(
		cube_t	*	to_save, 
		char	*	filename,
		char	*	ref_file)
{
	return cube_save_fits_hdrcopy_wh(to_save, filename, ref_file, NULL);
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Append image data to a file.
  @param	filename	Output file name.
  @param	appended	Image data to append.
  @param	pixtype		Pixel type to use when dumping to file.
  @return	int 0 if Ok, -1 otherwise.

  This function appends pixel data from an image into a given file, in
  the requested pixel type. No padding is done after the data have
  been dumped to the file.

  If the given file name is 'STDOUT' (without quotes), data will be
  dumped on the process standard out stream.
 */
/*--------------------------------------------------------------------------*/
int cube_fits_appendimage(
		char           *	filename,
		image_t        *	appended,
		int             	pixtype)
{
	qfitsdumper	qd ;

    /* Error handling : check entries   */
    if (appended==NULL || filename==NULL) return -1 ;

	/* Set parameters for qfits dumper */
	qd.filename  = filename ;
	qd.npix      = appended->lx * appended->ly ;
#ifdef DOUBLEPIX
	qd.ptype     = PTYPE_DOUBLE ;
#else
	qd.ptype	 = PTYPE_FLOAT ;
#endif
	qd.fbuf      = appended->data ;
	qd.out_ptype = pixtype ;

	if (qfits_pixdump(&qd)!=0) {
		e_error("cannot save buffer to file %s", filename);
		return -1 ;
	}

    return 0 ;
}

