
/*---------------------------------------------------------------------------
   
   File name 	:	wfip_lib.c
   Author 		:	N. Devillard
   Created on	:	18 May 2000
   Description	:	WFI library tilities

 *--------------------------------------------------------------------------*/

/*
	$Id: wfip_lib.c,v 1.21 2003/03/18 15:16:20 yjung Exp $
	$Author: yjung $
	$Date: 2003/03/18 15:16:20 $
	$Revision: 1.21 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "wfip_lib.h"
#include "qfits.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

int wfi_split(char * name_i, char * name_o, int xtnum)
{
	qfits_header*	h_main ;
	qfits_header*	h_ext ;
	int				i ;
	int				n_ext ;
	char			ext_name_o[FILENAMESZ];
	FILE		*	extension ;
	int				data_beg, data_size ;
	int				naxis1, naxis2, bitpix ;
	int			*	xts ;
	int				nxts ;
	char		*	fdata ;
	size_t			fsize ;

	/* Sanity checks */
	if (is_fits_file(name_i)!=1) {
		e_error("cannot find file [%s]", name_i);
		return -1 ;
	}

	/* Read main header */
	e_comment(0, "reading main header");
	h_main = qfits_header_read(name_i);
	if (h_main==NULL) {
		e_error("reading main header from [%s]", name_i);
		return -1 ;
	}

	/* Find out how many extensions are in the file */
	e_comment(0, "finding number of extensions");
	n_ext = qfits_query_n_ext(name_i);
	if (n_ext<1) {
		e_error("no extension found in [%s]", name_i);
		return -1 ;
	}
	e_comment(0, "[%d] extensions found in file", n_ext);

	if (xtnum<1) {
		nxts = n_ext ;
		xts = malloc(nxts * sizeof(int)) ;
		for (i=0 ; i<nxts ; i++) {
			xts[i]=i+1 ;
		}
	} else {
		nxts = 1 ;
		xts = malloc(nxts * sizeof(int)) ;
		xts[0] = xtnum ;
	}

	/* Go extension after extension */
	for (i=0 ; i<nxts ; i++) {
		/* Read extension header */
		e_comment(1, "reading extension [%d]", xts[i]);
		h_ext = qfits_header_readext(name_i, xts[i]);
		if (h_ext==NULL) {
			e_error("reading extension header #%d", xts[i]);
			continue;
		}
		/* Compute data size in bytes */
		naxis1 = qfits_header_getint(h_ext, "NAXIS1", 0);
		naxis2 = qfits_header_getint(h_ext, "NAXIS2", 0);
		bitpix = qfits_header_getint(h_ext, "BITPIX", 0);

		data_size = naxis1 * naxis2 * BYTESPERPIXEL(bitpix);
		if (data_size<1) {
			e_error("cannot determine data size in bytes in ext[%d]",xts[i]);
			qfits_header_destroy(h_ext);
			continue ;
		}

		/* Output new header to file */
		sprintf(ext_name_o, "%s_%02d.fits", name_o, xts[i]);
		if ((extension=fopen(ext_name_o, "w"))==NULL) {
			e_error("cannot output to file [%s]", ext_name_o);
			continue ;
		}
		qfits_header_dump(h_main, extension);
		qfits_header_dump(h_ext, extension);
		qfits_header_destroy(h_ext);

		/* Find out pixels in main belonging to this extension */
		if (qfits_get_datinfo(name_i, xts[i], &data_beg, NULL)!=0) {
			e_error("getting offset to extension %d", xts[i]);
			continue ;
		}
		/* Map input file */
		fdata = falloc(name_i, 0, &fsize);
		if (fdata==NULL) {
			e_error("mapping input file [%s]", name_i);
			continue ;
		}
		/* Dump pixels from one file to the other */
		e_comment(1, "copying extension [%d]", xts[i]);
		fwrite(fdata+data_beg, 1, data_size, extension);

		/* Close file pointers */
		free(fdata);
		fclose(extension);

		/* zero-padding */
		qfits_zeropad(ext_name_o);
	}
	qfits_header_destroy(h_main);
	free(xts);
	return 0 ;
}


/*
 * Loads a WFI list into a cube_t structure.
 */
cube_t * wfi_cube_load(char * filename, int xtnum)
{
	cube_t	  *	loaded ;
	image_t  * wfi_frame ;
	framelist *	flist ;
	int			i ;
	int		  *	exts ;
	int			err ;
	int			single_frames ;

	/* Check entries */
	if (filename==NULL) return NULL ;

	/* Load framelist */
	flist = framelist_load(filename);
	if (flist==NULL) {
		e_error("cannot load frame list [%s]", filename);
		return NULL ;
	}
	/* Get number of extensions contained in each frame */
	exts = malloc(flist->n * sizeof(int));
	for (i=0 ; i<flist->n ; i++) {
		exts[i] = qfits_query_n_ext(flist->name[i]);
	}

	/* Check consistency */
	err=0 ;
	single_frames = 1;
	for (i=1 ; i<flist->n ; i++) {
		/* Check that all files have the same number of extensions */
		if (exts[i]!=exts[0]) {
			e_error("inconsistent input data set");
			err++ ;
		}
		/* Check that the requested extension (if any) is present */
		if (xtnum>0) {
			if (xtnum>exts[i]) {
				e_error("inconsistent input data set");
				err++;
			}
			if (exts[i]>0) {
				single_frames = 0 ;
			}
		}
	}
	/* Process errors */
	if (err) {
		e_error("%d errors occurred during reading", err);
		for (i=0 ; i<flist->n ; i++) {
			e_error("frame [%s] has %d extensions", flist->name[i], exts[i]);
		}
		framelist_del(flist);
		free(exts);
		return NULL ;
	}
	free(exts);


	/* A list of single frames */
	err=0 ;
	if (single_frames) {
		loaded = cube_load_strings(flist->name, flist->n);
		if (loaded == NULL) {
			e_error("loading framelist [%s]", filename);
			err++ ;
		}
	} else {
	/* A list of whole WFI frames */
		/* Load first image */
		wfi_frame = wfi_load_ext(flist->name[i], xtnum) ;
		if (wfi_frame==NULL) {
			e_error("cannot load frame [%s][%d]", flist->name[i], xtnum);
			err++ ;
		}
		loaded = cube_new(wfi_frame->lx, wfi_frame->ly, flist->n);
		loaded->plane[0] = wfi_frame ;
		for (i=1 ; i<flist->n ; i++) {
			wfi_frame = wfi_load_ext(flist->name[i], xtnum);
			if (wfi_frame == NULL) {
				e_error("cannot load frame [%s][%d]", flist->name[i], xtnum);
				err++ ;
			}
			loaded->plane[i] = wfi_frame ;
		}
	}
	/* Process errors */
	if (err) {
		e_error("an error occurred during loading: aborting");
		if (loaded!=NULL) {
			cube_del(loaded) ;
		}
		loaded = NULL ;
	}
	framelist_del(flist);
	return loaded ;
}


image_t * wfi_load_ext(char * filename, int xtnum)
{
	qfitsloader		ql ;
	image_t		*	ext_load ;

	ql.filename = filename ;
	ql.xtnum    = xtnum ;
	ql.pnum     = 1 ;
	ql.map      = 1 ;
#ifdef DOUBLEPIX
	ql.ptype    = PTYPE_DOUBLE ;
#else
	ql.ptype    = PTYPE_FLOAT ;
#endif

	if (qfitsloader_init(&ql)!=0) {
		return NULL ;
	}

	ext_load = malloc(sizeof(image_t));
	ext_load->lx = ql.lx ;
	ext_load->ly = ql.ly ;
#ifdef DOUBLEPIX
	ext_load->data = (pixelvalue*)ql.dbuf ;
#else
	ext_load->data = (pixelvalue*)ql.fbuf ;
#endif

	return ext_load ;
}



int wfi_is_extension(char * filename)
{
	return qfits_query_n_ext(filename) ;
}



image_t * wfi_overscan_correction(
	image_t	*	wfi_frame,
	int			*	prescan_x,
	int			*	overscan_x,
	int			*	rej_int,
	int			*	crop_reg
)
{
	image_t	*	cropped_frame ;
	pixelvalue		filtered ;
	pixelvalue	*	bias_lin ;
	int				scan_width ;
	int				i, j, k ;
	pixelvalue	*	pix ;

    /* Sanity tests with input parameters */
    scan_width = (prescan_x[1]-prescan_x[0]+1) +
                 (overscan_x[1]-overscan_x[0]+1) ;
 
    if ((rej_int[0]+rej_int[1])>=scan_width) {
        e_error("in rejection parameters: rejecting too many pixels");
        return NULL ;
    }
 
    if ((crop_reg[0]>=crop_reg[1])||(crop_reg[2]>=crop_reg[3])) {
        e_error("in crop region definition");
        return NULL ;
    }

 
    /* Bring coordinates back to C convention */
    prescan_x[0]-- ; prescan_x[1]-- ; overscan_x[0]-- ; overscan_x[1]-- ;
 
    /* Make out bias column */
    bias_lin = malloc(scan_width * sizeof(pixelvalue));

    /* Loop on all lines */
    for (j=0 ; j<wfi_frame->ly ; j++) {
        pix = wfi_frame->data + j*wfi_frame->lx ;
        k=0 ;
        for (i=prescan_x[0] ; i<=prescan_x[1] ; i++) {
            bias_lin[k++] = pix[i];
        }
        for (i=overscan_x[0] ; i<=overscan_x[1] ; i++) {
            bias_lin[k++] = pix[i] ;
        }
        filtered =
            function1d_average_reject(bias_lin,
                                      scan_width,
                                      rej_int[0],
                                      rej_int[1]);
        for (i=0 ; i<wfi_frame->lx ; i++) {
            pix[i] -= filtered ;
        }
    }
    free(bias_lin);

    /* Bring coordinates back to FITS convention */
    prescan_x[0]++ ; prescan_x[1]++ ; overscan_x[0]++ ; overscan_x[1]++ ;

    /* Extract cropped region */
    cropped_frame = image_getvig(wfi_frame,
                                            crop_reg[0],
                                            crop_reg[2],
                                            crop_reg[1],
                                            crop_reg[3]);


	return cropped_frame ;
}



/* There are 4 quadrants in a frame */
#define WFI_NQUAD		4

int wfi_gradient_check(
	image_t	*	wfi_frame,
	double			max_grad_level
)
{
	double	avg_frame ;
	double	avg_quad[WFI_NQUAD] ;
	int		i ;
	int		gradient_ok ;

	/* Compute average for the whole frame */
	avg_frame = image_getmean(wfi_frame);
	if (fabs(avg_frame)<1e-6) {
		e_warning("frame has zero average: cannot compute gradient check");
		return 1 ;
	}

	/* Compute average value inside each quadrant */
	/* Lower left quadrant */
	avg_quad[0] = image_getmean_vig(
							wfi_frame,
							1,
							wfi_frame->lx/2,
							1,
							wfi_frame->ly/2);
	/* Lower right quadrant */
	avg_quad[1] = image_getmean_vig(
							wfi_frame,
							1+wfi_frame->lx/2,
							wfi_frame->lx,
							1,
							wfi_frame->ly/2);
	/* Upper left quadrant */
	avg_quad[2] = image_getmean_vig(
							wfi_frame,
							1,
							wfi_frame->lx/2,
							1+wfi_frame->ly/2,
							wfi_frame->ly);
	/* Upper right quadrant */
	avg_quad[3] = image_getmean_vig(
							wfi_frame,
							1+wfi_frame->lx/2,
							wfi_frame->lx,
							1+wfi_frame->ly/2,
							wfi_frame->ly);
	
	e_comment(1, "average/quadrants: [%g] %4.2f %4.2f %4.2f %4.2f",
				avg_frame,
				avg_quad[0],
				avg_quad[1],
				avg_quad[2],
				avg_quad[3]);

	/* Compute ratio */
	gradient_ok = 1 ;
	for (i=0 ; i<WFI_NQUAD ; i++) {
		avg_quad[i] = (avg_frame - avg_quad[i]) / avg_frame ;
		if (avg_quad[i] > max_grad_level) {
			e_warning("quadrant ratio above limit (%g)", max_grad_level);
			gradient_ok = 0 ;
		}
	}

	return gradient_ok ;
}

