/*----------------------------------------------------------------------------*/
/**
   @file	cube_load.c
   @author	Nicolas Devillard
   @date	May 1999
   @version	$Revision: 1.65 $
   @brief	cube loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: cube_load.c,v 1.65 2003/04/14 11:14:24 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/14 11:14:24 $
	$Revision: 1.65 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "qfits.h"
#include "cube_load.h"
#include "image_rtd.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a cube from disk.
  @param	filename	Name of the file to load.
  @return	1 newly allocated cube object (NULL if error).

  This is just a wrapper around supported file formats. The given file
  name is first checked to see if it is FITS (in which case
  cube_load_fits is called), then checked to see if it is an ASCII
  list.

  Further formats might be added later on.
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_load(char * filename)
{
    struct stat     sta ;
	cube_t		*	loaded ;

    if (stat(filename, &sta)!=0) {
        e_error("no such file: %s", filename);
        return NULL ;
    }
    if (sta.st_size<1) {
        e_error("empty file: %s", filename);
        return NULL ;
    }
	if (!strcmp(filename, "RTD")) {
		loaded = cube_load_rtd() ;
	} else if (is_fits_file(filename)==1) {
		loaded = cube_load_fits(filename);
	} else if (is_ascii_list(filename)==1) {
		loaded = cube_load_framelist(filename);
	} else {
		e_error("unsupported format for file [%s]", filename);
		loaded = NULL ;
	}
	return loaded ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a cube from the current image displayed in RTD
  @return	1 newly allocated cube containing 1 image.

  This function initiates a connection to the current RTD session
  running for the current user, gets the pixels which are currently
  displayed there, builds a cube containing a single plane and returns
  it.

  If any error occurs, this function returns NULL. The returned cube
  must be deallocated using cube_del. Notice that the rtd session is
  opened/closed by the function, so no side-effect occurs.
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_load_rtd(void)
{
	cube_t	*	loaded ;
	image_t	*	rtd_image ;

	rtd_image = rtd_image_get();
	if (rtd_image==NULL) return NULL ;

	loaded = cube_new(rtd_image->lx, rtd_image->ly, 1);
	loaded->plane[0] = rtd_image ;
	return loaded ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a cube from a FITS file on disk.
  @param	filename	Name of the FITS file to load.
  @return	1 newly allocated cube object (NULL if error).
  Reads a cube in from a FITS file on disk.
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_load_fits(char * filename)
{
    cube_t     *	loaded_cube ;
    image_t    *	one_plane ;
    int         	i ; 
	qfitsloader		ql ;


	/* Initialize a FITS loader */
	ql.filename = filename ;
	ql.xtnum    = 0 ;
	ql.pnum     = 0 ;
    ql.map      = 1 ;
#ifdef DOUBLEPIX
	ql.ptype	= PTYPE_DOUBLE ;
#else
	ql.ptype    = PTYPE_FLOAT ;
#endif

	if (qfitsloader_init(&ql)!=0) {
		return NULL ;
	}

    /* Create cube and fill up information fields */
    loaded_cube = cube_new(ql.lx, ql.ly, ql.np);
	/* Loop on all planes   */
	for (i=0 ; i<ql.np ; i++) {
        compute_status("loading cube", i, ql.np, 1);
		ql.pnum = i ;
		if (qfits_loadpix(&ql)!=0) {
			e_error("loading plane %d from file %s", ql.pnum+1, ql.filename);
			cube_del(loaded_cube);
			return NULL ;
		}
		one_plane = malloc(sizeof(image_t));
		one_plane->lx = ql.lx ;
		one_plane->ly = ql.ly ;
#ifdef DOUBLEPIX
		one_plane->data = (pixelvalue*)ql.dbuf ;
#else
		one_plane->data = (pixelvalue*)ql.fbuf ;
#endif

		loaded_cube->plane[i] = one_plane ;
	}
    return loaded_cube ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Load cube from a list defined as a char **.
  @param	filenames	Array of strings containing the file names.
  @param	nfiles		Number of file names in the list.
  @return	1 newly allocated cube object.

  This function takes in input a list of strings, like (argc,argv) and
  loads a cube from it. File names are expected to refer to FITS
  files. These files might be 2 or 3 dimensional, they are all
  expected to share the same image size.

  Example: if the input list contains

  \begin{verbatim}
  image1.fits	# An image
  image2.fits	# An image
  cube1.fits    # A data cube with 20 planes
  \end{verbatim}

  Then the returned cube will have 22 planes (the first 2 plus the 20
  from the cube).

 */
/*----------------------------------------------------------------------------*/
cube_t * cube_load_strings(char ** filenames, int nfiles)
{
    cube_t     *	loaded_cube ;
    image_t    *	one_plane ;
	int				i, j ;
    int             np ;
	qfitsloader*	ql ;

	/* Test entries */
	if ((nfiles<1) || (filenames==NULL)) {
		return NULL ;
	}

	/* If there is a single file, outsource the job to cube_load_fits() */
	if (nfiles==1) {
		if (filenames[0] == NULL) return NULL ;
		loaded_cube = cube_load_fits(filenames[0]) ;
		return loaded_cube ;
	}

    /* Initialize a qfitsloader object per input file. */
    ql = malloc(nfiles * sizeof(qfitsloader));
    for (i=0 ; i<nfiles ; i++) {
        /* Init with first plane to get image info */
        ql[i].filename = filenames[i] ;
        ql[i].xtnum    = 0 ;
        ql[i].pnum     = 0 ;
        ql[i].map      = 1 ;
        ql[i].ptype    = PTYPE_FLOAT ;
        if (qfitsloader_init(&(ql[i]))!=0) {
            free(ql);
            return NULL ;
        }
    }

    /* Check that all input planes have the same size. */
    /* Accumulate total number of planes. */
    np=ql[0].np ;
    for (i=1 ; i<nfiles ; i++) {
        if (ql[i].lx!=ql[0].lx || ql[i].ly!=ql[0].ly) {
            e_error("incompatible plane sizes in list");
            free(ql);
            return NULL ;
        }
        np += ql[i].np ;
    }

	/* Create output cube */
	loaded_cube = cube_new(ql[0].lx, ql[0].ly, np);

	/* Loop on all planes */
    np=0 ;
	for (i=0 ; i<nfiles ; i++) {
        compute_status("loading framelist...", i, nfiles, 1);
        
        for (j=0 ; j<ql[i].np ; j++) {
            ql[i].pnum = j ;
#ifdef DOUBLEPIX
            ql[i].ptype	= PTYPE_DOUBLE ;
#else
            ql[i].ptype    = PTYPE_FLOAT ;
#endif
            if (qfits_loadpix(&ql[i])!=0) {
                cube_del(loaded_cube);
                free(ql);
                return NULL ;
            }
            one_plane = malloc(sizeof(image_t));
            one_plane->lx = ql[i].lx ;
            one_plane->ly = ql[i].ly ;
#ifdef DOUBLEPIX
            one_plane->data = (pixelvalue*)ql[i].dbuf ;
#else
            one_plane->data = (pixelvalue*)ql[i].fbuf ;
#endif

            loaded_cube->plane[np] = one_plane ;
            np++ ;
        }
    }
    free(ql);
    return loaded_cube ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a cube from an ASCII list file.
  @param	listname	Name of the ASCII list file.
  @return	1 newly allocated cube object.

  This function takes in input the name of a file, supposed to contain
  a list of frame names. File names corresponding to single frames are
  loaded as single frames. If a file name corresponds to a cube, all
  frames of this cube are loaded into separate planes in the returned
  cube. See cube_load_strings() for more information.
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_load_framelist(char * listname)
{
	cube_t		*	loaded ;
	framelist	*	flist ;


	/* Load the list of all file names contained in the list */
	flist = framelist_load(listname);
	if (flist==NULL) {
		e_error("reading ASCII list [%s]: aborting load", listname);
		return NULL ;
	}

	/* Outsource the loading to cube_load_strings() */
	loaded = cube_load_strings(flist->name, flist->n) ;
	framelist_del(flist);
	return loaded ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief		Get some file information about a FITS file.
  @param	filename	Name of the file to parse.
  @return	1 new cube_info structure, NULL if error.

  This function reads in a FITS file and fills up a cube_info struct
  informing about the file structure. The returned structure must be
  freed using free().
  If the cube has an inconsistent declaration, an error is triggered
  and NULL is returned.
 */
/*----------------------------------------------------------------------------*/
cube_info * cube_getinfo(char *filename)
{
	char		*	sval ;
    int          	naxes ;     
	int				naxis[3];
	int				ptype ;
	double			b_scale, b_zero ;
	int				headersize ;
    cube_info    *	fileinfo ;

    /* Check file existence */
    if (file_exists(filename) != 1) {
        e_error("file %s not found", filename) ;
        return NULL ;
    }   

    /* Get header size  */
	if (qfits_get_hdrinfo(filename, 0, NULL, &headersize)!=0) {
		e_error("reading FITS header for file %s", filename);
		return NULL ;
	}

    /* find the number of axes  */
	naxes = 0 ;
	sval = qfits_query_hdr(filename, "NAXIS");
	if (sval!=NULL) {
		naxes = atoi(sval);
	} else {
		e_error("missing key in header: NAXIS");
		return NULL ;
	}

    if  ((naxes < 1) || (naxes > 3))    {
        e_error("cannot handle cube with %d axes", naxes) ;
        return NULL ;
    }

    /*
     * Cubes may not have less than one dimension or more
	 * than 3 dimensions. One dimensional cubes are processed as
	 * single-lined images.
     */

    /* Read the dimension value for each axis  */
    /* NAXIS1 is always present */
	naxis[0]=0;
	sval = qfits_query_hdr(filename, "NAXIS1");
	if (sval!=NULL) {
		naxis[0] = atoi(sval);
	} else {
		e_error("missing key in header: NAXIS1");
        return NULL ;
	}
    if ((naxis[0]< 1) || (naxis[0] > MAX_COLUMN_NUMBER)) {
        e_error("cannot process cube with NAXIS1=%d", naxis[0]) ;
        return NULL ;
    }

	/* Now get second axis size value. */
    if (naxes == 1) {
        naxis[1]=1 ;
    } else {
        /* Read value and assign ly */
        naxis[1]= 0 ;
		sval = qfits_query_hdr(filename, "NAXIS2");
		if (sval!=NULL) {
			naxis[1]=atoi(sval);
		} else {
			e_error("missing key in header: NAXIS2");
			return NULL ;
		}
    }
    if ((naxis[1] < 1) || (naxis[1] > MAX_LINE_NUMBER)) {
        e_error("cannot process cube with NAXIS2=%d", naxis[1]);
        return NULL ;
    }

    /* For 3D cubes, get the third dimension size   */
    if (naxes <= 2) {
        naxis[2]=1 ;
    } else /* It is a 3D cube   */ {
		naxis[2]=0 ;
		sval = qfits_query_hdr(filename, "NAXIS3");
		if (sval!=NULL) {
			naxis[2] = atoi(sval);
		} else {
			e_error("missing key in header: NAXIS3");
			return NULL ;
		}
        if ((naxis[2] < 1) || (naxis[2] > MAX_IMAGE_NUMBER)) {
            e_error("cannot process cube with NAXIS3=%d", naxis[2]);
            return NULL ;
        }
        /* Third axis size : OK */
    }

    /* Test data type   */
    ptype=0 ;
	sval = qfits_query_hdr(filename, "BITPIX");
	if (sval!=NULL) {
		ptype = atoi(sval);
	} else {
		e_error("missing key in header: BITPIX");
		return NULL ;
	}
	if (BYTESPERPIXEL(ptype) == 0) {
        e_error("cannot process cube with BITPIX=%s", sval) ;
        return NULL ;
    }

    /* Get BSCALE keyword, or assign 1.0 if not present */
	b_scale=1.0 ;
	sval = qfits_query_hdr(filename, "BSCALE");
	if (sval!=NULL) {
		b_scale = atof(sval);
	}

    /* Get BZERO keyword, or assign 0.0 if not present  */
	b_zero=0.0 ;
	sval = qfits_query_hdr(filename, "BZERO");
	if (sval!=NULL) {
		b_zero=atof(sval);
	}

	fileinfo = malloc(sizeof(cube_info));
	fileinfo->lx = naxis[0];
	fileinfo->ly = naxis[1];
	fileinfo->n_im = naxis[2];
	fileinfo->ptype = ptype ;
	fileinfo->headersize = headersize ;
	fileinfo->b_scale = b_scale ;
	fileinfo->b_zero = b_zero ;

    return fileinfo ;
}

