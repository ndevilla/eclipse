
/*----------------------------------------------------------------------------
   
   File name 	:	ado_utils.c
   Author 		:	Nicolas Devillard
   Created on	:	Feb 29, 1996	
   Rewritten    :   April 1999
   Description	:	all these routines are Adonis specific 

 ---------------------------------------------------------------------------*/

/*

 $Id: ado_utils.c,v 1.20 2002/07/31 14:02:01 ndevilla Exp $
 $Author: ndevilla $
 $Date: 2002/07/31 14:02:01 $
 $Revision: 1.20 $

 */

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "ado_utils.h"


/*----------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define TIME_INFO_SIZE	(16)
#define ONE_MEG			(int)1048576


/*----------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

typedef struct _ADONIS_TIME_STAMP_ {

	/*
	 * Time stamp format before 05 oct 1998
	 */
	unsigned int		hh;
	unsigned int		mm;
	unsigned int		ss;
	unsigned int		day;
	unsigned int		DD;
	unsigned int		MM;
	unsigned int		YY;
	unsigned int		tick;
	unsigned int		rate;

	/*
	 * Time stamp format after 05 oct 1998
	 */
	unsigned long	secmid;
	unsigned long	julian;

} adonis_time_stamp ;


typedef enum _ADONIS_STAMP_FORMAT_ {
	adonis_stamp_pre05oct98,
	adonis_stamp_pos05oct98,
	adonis_stamp_unknown
} adonis_stamp_format ;

/* 8 days in a week? Last one stands for unknown day... you never know...	*/
static char DayOfTheWeek[8][20] = {	
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT",
	"-"
} ;


/*----------------------------------------------------------------------------
  						Function private to this module
 ---------------------------------------------------------------------------*/

static adonis_stamp_format get_adonis_stamp_format(unsigned char * tinfo) ;
static void
adonis_decode_time_stamp(
	unsigned char     * tinfo,
	adonis_time_stamp * stamp,
	adonis_stamp_format stamp_f);


static cube_t *
get_planes_with_pattern(
    cube_t  *   cube_in,
    char    *   pattern,
    int         im_per_step,
    int         plane_type
);


static int copy_file(char *src, char *dest);
static int copy_file_n_bytes(FILE *, FILE *, int);


/*----------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Copy a file, overwrite destination if needed.
  @param    src     Name of the source file to copy.
  @param    dest    Name of the destination file to copy.
  @return   int 0 if Ok, anything else in case of error.
 
  Copy a file using fopen, fread and fwrite. This will overwrite the
  destination file if it already exists.
 */
/*--------------------------------------------------------------------------*/
 
static int copy_file(char *src, char *dest)
{
    FILE    *in, *out ;
    char    *buffer ;
    int      size ;
 
    if ((size = filesize(src)) <= 0) {
        e_error("wrong size file in input (%ld): aborting", size) ;
        return 1 ;
    }
 
    /* Open source file */
    if ((in = fopen(src, "r")) == NULL) {
        e_error("cannot open file [%s]: aborting copy", src) ;
        return 1 ;
    }
 
    /* Remove any already existing output file by the same name */
    if (file_exists(dest) == 1) {
        remove(dest) ;
    }
 
    /* Open dest file for output */
    if ((out = fopen(dest, "w")) == NULL) {
        e_error("cannot create file [%s]: aborting copy", dest) ;
        fclose(in) ;
        return 1 ;
    }
 
    /*
     * Allocate buffer:
     * If the file size is smaller than ONE_MEG, do it in one go,
     * or else do it meg by meg.
     */
 
    buffer = malloc(ONE_MEG);
    if (size > ONE_MEG) {
        size = size % ONE_MEG ;
        while (fread(buffer, 1, ONE_MEG, in) == ONE_MEG)
            fwrite(buffer, 1, ONE_MEG, out) ;
    } else {
        size = fread(buffer, 1, ONE_MEG, in) ;
    }
 
    if ((fwrite(buffer, 1, size, out)) != size) {
        e_error("cannot write %ld bytes to file %s (disk full?) aborting",
                size, dest) ;
    }
    fclose(in) ;
    fclose(out) ;
    free(buffer) ;
    return 0 ;
} 



/*-------------------------------------------------------------------------*/
/**
  @brief    Copy n bytes from file src to file dest.
  @param    src     FILE pointer to the source file to copy from.
  @param    dest    FILE pointer to the destination file to copy to.
  @param    nbytes  How many bytes to transfer between files.
  @return   int 0 if Ok, anything else in case of error.
 
  Copy n bytes from one file to another using fread and fwrite. The
  two input FILE pointers myst be pointing to opened files.
 */
/*--------------------------------------------------------------------------*/
 
static int copy_file_n_bytes(FILE * dest, FILE * src, int nbytes)
{
    int          actbytes ;
    char        *buf ;
 
    buf = malloc(nbytes) ;
    actbytes = fread(buf, 1, nbytes, src) ;
    if (actbytes < nbytes) {
        e_error("unexpected EOF") ;
        free(buf) ;
        return 1 ;
    }
    actbytes = fwrite(buf, 1, nbytes, dest) ;
    free(buf) ;
    if (actbytes < nbytes) {
        e_error("write error") ;
        return 1 ;
    }
    return 0 ;
} 


/*---------------------------------------------------------------------------
   Function	:	check_band_consistency()
   In 		:	2 FITS file names
   Out 		:	boolean
   Job		:	Checks if 2 given cubes are taken in the same band
   Notice	:	returns
  				1 if both cubes have the same content in band keyword
  				0 otherwise.
  				A message is displayed if no match.
 ---------------------------------------------------------------------------*/

int 
check_band_consistency(
	char	*file1,
	char	*file2)
{
	int			check ;
	char	*	cval1 ;
	char	*	cval2 ;
			
	/* Get the value associated with the Adonis keyword to describe	*/
	/* the IR band used for acquisition.							*/

	cval1 = qfits_query_hdr(file1, "OP_FILT");
	cval2 = qfits_query_hdr(file2, "OP_FILT");

	check = 0 ;
	if (cval1!=NULL && cval2!=NULL) {
		if (!strcmp(cval1, cval2)) {
			check = 1 ;
		} 
	}
	return check ;
}



/*----------------------------------------------------------------------------
   Function	:	reduce_separated_cube()
   In 		:	object, sky, flat, badpixelmap, output file names
   Out 		:	void
   Job		:	cleans out an object cube (Adonis)
   Notice	:
  
  	The algorithm is the following:
  	1. average the sky
  	2. subtract the average sky from the object
    3. average the result
  	4. flat-field and correct the bad pixels
 ---------------------------------------------------------------------------*/

void
reduce_separated_cube(
	char	*object,
	char	*sky,
	char	*flat,
	char	*bpm,
	char	*out,
	int		 flag_avg
)
{
	cube_t		*	c_object,
				*	c_sky,
				*	c_flat ;
	image_t		*	i_avg_sky,
				*	i_object ;
	pixelmap	*	bad_pixelmap ;

	/* Error handling: test entries	*/

	if (is_fits_file(object) != 1) {
		e_error("cannot open file [%s]: aborting", object) ;
		return ;
	}

	if (is_fits_file(sky) != 1) {
		e_error("cannot open file [%s]: aborting", sky) ;
		return ;
	}

	/*
	 * First step: load the sky and average it
	 */

	c_sky = cube_load(sky) ;
	i_avg_sky = cube_avg_linear(c_sky) ;
	cube_del(c_sky) ;

	/* Now load object and subtract averaged sky from it	*/
	c_object = cube_load(object) ;
	cube_sub_im(c_object, i_avg_sky);

	/* Now average the object to one plane if requested*/
	if (flag_avg == 1) {
		i_object = cube_avg_linear(c_object) ;
		cube_del(c_object) ;
		c_object = cube_from_image(i_object) ;
		image_del(i_object) ;
	}

	/* If the flat-field exists, load it and divide results by flat-field	*/
	if (file_exists(flat) == 1) {
		c_flat = cube_load(flat) ;
		cube_op(&c_object, c_flat, '/') ;
		cube_del(c_flat) ;
	} else {
		e_warning("No flat fielding done") ;
	}

	/* If the bad pixel map exists, load it and correct bad pixels	*/
	if (file_exists(bpm) == 1) {
		bad_pixelmap = pixelmap_load(bpm) ;
		cube_clean_deadpix(c_object, bad_pixelmap) ;
		pixelmap_del(bad_pixelmap) ;
	} else {
		e_warning("No bad pixel correction") ;
	}

	/* Save the result and bye bye	*/
	cube_save_fits_hdrcopy(c_object, out, object);
	cube_del(c_object) ;
	return ;
}



/*----------------------------------------------------------------------------
   Function	:	reduce_packed_cube()
   In 		:	packed cube name, flat, bad pixel map, output name
   Out 		:	void
   Job		:	cleans out a packed cube (Adonis)
   Notice	:	quite specific to Adonis
  
   The algorithm is the following:
   For each (object, sky) acquisition cycle:
   	1. extract the sky, average it
  	2. subtract it from each object plane in same cycle
  	3. flat-field the result
  	4. correct for dead pixels
  	5. append the results to output cube
 ---------------------------------------------------------------------------*/


void
reduce_packed_cube(
	char	*packed,
	char	*flat,
	char	*bpm,
	char	*out,
	int		 flag_avg
)
{
	cube_t		*	one_cycle,
				*	c_obj,
				*	c_sky,
				*	out_cube ;
	image_t		*	i_sky,
				*	i_obj,
				*	i_flat ;

	pixelmap	*bad_pixelmap ;
	int			ncycles,
				im_per_step,
				im_per_cycle ;
	int			i, j ;
	int			np_out ;
	int			begin, end ;
	char		pattern[MAX_IMAGE_NUMBER+1] ;
	int			nim_out ;

	/* error handling: test entries	*/
	if (is_fits_file(packed) != 1) {
		e_error("cannot open file [%s]: aborting", packed) ;
		return ;
	}

	/* get the cycle organization	*/
	get_cycle_organization(packed, &ncycles, &im_per_step, pattern) ;
	im_per_cycle = (int)strlen(pattern) * im_per_step ;
	if (flag_avg == 1) {
		nim_out = ncycles ;
	} else {
		nim_out = ncycles * im_per_step ;
	}

	out_cube = NULL ;
	np_out = 0 ;
	/* Process cycle per cycle	*/
	for (i=0 ; i<ncycles ; i++) {
		e_comment(0, "cycle reduction: %d of %d", i+1, ncycles) ;
		begin = 1 + i * im_per_cycle ;
		end = begin + im_per_cycle - 1  ;
		/* Load one cycle	*/
		one_cycle = extract_cube_from_cube(packed, begin, end) ;
		/* Extract the sky from this cycle	*/
		c_sky = get_planes_with_pattern(one_cycle, pattern, im_per_step, 0) ;
		/* Average the sky	*/
		i_sky = cube_avg_linear(c_sky) ; 
		cube_del(c_sky) ;

		/* Extract the object from this cycle	*/
		c_obj = get_planes_with_pattern(one_cycle, pattern, im_per_step, 1) ;
		cube_del(one_cycle) ;

		/* subtract averaged sky from object	*/
		cube_sub_im(c_obj, i_sky);
		image_del(i_sky);

		/* Average resulting cube if requested	*/
		if (flag_avg == 1) {
			i_obj = cube_avg_linear(c_obj) ;
			cube_del(c_obj) ;
			c_obj = cube_from_image(i_obj);
			image_del(i_obj);
		}

		if (out_cube==NULL) {
			out_cube = cube_new(c_obj->lx, c_obj->ly, nim_out);
		}
		/* append results to output cube */
		for (j=0 ; j<c_obj->np ; j++) {
			out_cube->plane[np_out] = c_obj->plane[j] ;
			np_out ++ ;
		}
	}

	/* If the flat-field exists, load it and divide output cube by it	*/
	if (file_exists(flat) == 1) {
		e_comment(0, "flat fielding cube") ;
		i_flat = image_load(flat) ;
		cube_div_im(out_cube, i_flat);
		image_del(i_flat) ;
	} else {
		e_warning("No flat fielding done") ;
	}

	/* If the bad pixel map exists, load it and correct bad pixels	*/
	if (file_exists(bpm) == 1) {
		e_comment(0, "clearing out bad pixels...") ;
		bad_pixelmap = pixelmap_load(bpm) ;
		cube_clean_deadpix(out_cube, bad_pixelmap) ;
		pixelmap_del(bad_pixelmap) ;
	} else {
		e_warning("No bad pixel correction") ;
	}

	/* Save the result and bye bye	*/
	cube_save_fits_hdrcopy(out_cube, out, packed) ;
	cube_del(out_cube) ;
	return ;
}



/*----------------------------------------------------------------------------
   Function :   get_planes_with_pattern()
   In       :   1 cube, pattern string, im. per cycle step binary value
   Out      :   1 cube
   Job      :   extract plane in a cube according to a pattern
                The input cube contains ONLY 1 PATTERN!
   Notice   :   a pattern is a character string, e.g. "1001" describing
                how object/sky are organized, 1 meaning object, 0 sky.
                The binary value is an integer, use 0 to extract skies,
                1 to extract objects.
 ---------------------------------------------------------------------------*/
 
static cube_t *
get_planes_with_pattern(
    cube_t  *   cube_in,
    char    *   pattern,
    int         im_per_step,
    int         plane_type
)
{
    cube_t * cube_out ;
    int      i, j ;
    int      nim_out ;
    int      nrun ;
 
    /* Error handling: test entries */
    if (cube_in == NULL) {
        e_error("cannot extract pattern from NULL cube: aborting") ;
        return NULL ;
    }
 
    if (pattern == NULL) {
        e_error("undefined pattern: cannot extract from cube") ;
        return NULL ;
    }
 
    if ((plane_type != 0) && (plane_type != 1)) {
        e_error("need a 0 or a 1 to identify planes to extract: aborting") ;
        e_error("given pattern is: [%s]", pattern) ;
        return NULL ;
    }
 
    if (im_per_step < 1) {
        e_error("cycle step is wrong: %d, should be positive", im_per_step) ;
        return NULL ;
    }
 
    /*
     * Count how many output planes will be produced
     */
 
    nim_out = 0 ;
    for (i=0 ; i<(int)strlen(pattern) ; i++) {
        if ((int)(pattern[i]-'0') == plane_type) {
            nim_out ++ ;
        }
    }
 
    nim_out *= im_per_step ;
    cube_out = cube_new(cube_in->lx, cube_in->ly, nim_out) ;

    /* Go through the whole pattern */
    nrun = 0 ;
    for (i=0 ; i<(int)strlen(pattern) ; i++) {
        if ((int)(pattern[i] - '0') == plane_type) {
            for (j=0 ; j<im_per_step ; j++) {
                cube_out->plane[nrun] =
                    image_copy(cube_in->plane[i*im_per_step+j]) ;
                nrun ++ ;
            }
        }
    }
    if (nrun!=nim_out) {
        e_error("counting planes: aborting");
        cube_del(cube_out);
        cube_out = NULL ;
    }
    return(cube_out) ;
}


/*----------------------------------------------------------------------------
   Function :   extract_cube_from_cube()
   In       :   1 cube, first and last plane #
   Out      :   1 cube
   Job      :   extract a continuous cube from another cube
   Notice   :   plane #: limits are inclusive
                plane # go from 1 to n_planes
 ---------------------------------------------------------------------------*/
 
cube_t *
extract_cube_from_cube(
    char    *   cubename,
    int         p_begin,
    int         p_end)
{
	cube_t		*	loaded ;
    cube_t      *   ext_cube ;
	int				np ;
	int				i ;

	loaded = cube_load(cubename);
	if (loaded==NULL)
		return NULL ;
 
    if ((p_end < 0) || (p_end > loaded->np)) {
        p_end = loaded->np  ;
	}
 
    if (p_begin<1)
		p_begin = 1 ;

	np = (p_end - p_begin) + 1 ;
    ext_cube = cube_new(loaded->lx, loaded->ly, np);
 
	for (i=0 ; i<np ; i++) {
		ext_cube->plane[i] = image_copy(loaded->plane[p_begin-1+i]) ;
	}
	cube_del(loaded);

    return ext_cube ;
}


/*----------------------------------------------------------------------------
   Function	:	get_cycle_organization()
   In 		:	cube name
   Out 		:	# of cycles, # of image per cycle step, pattern
   Job		:	extracts the cycle organization from a cube
   Notice	:	SPECIFIC TO ADONIS
 ---------------------------------------------------------------------------*/

void
get_cycle_organization(	
	char	*	packed, 
	int		*	ncycles, 
	int		*	im_per_step, 
	char	*	pattern)
{
	char	*	cval ;

	*ncycles = *im_per_step = 0 ;
	pattern[0] = (char)0 ;

	/* Get number of cycles	*/
	cval = qfits_query_hdr(packed, "OJ_N_SEQ");
	if (cval!=NULL)
		*ncycles = atoi(cval);

	cval = qfits_query_hdr(packed, "OJ_N_IMA");
	if (cval!=NULL)
		*im_per_step = atoi(cval);

	cval = qfits_query_hdr(packed, "OB_CYCL");
	if (cval!=NULL)
			strcpy(pattern, cval);

	return ;
}



/*----------------------------------------------------------------------------
   Function	:	adonis_reformat_fits()
   In 		:	FITS file name, force flag.
   Out 		:	error code: 0 if ok, 1 otherwise
   Job		:	reformat a FITS file (Adonis mostly specific)
   Notice	:	if working in the same directory, files are overwritten
 ---------------------------------------------------------------------------*/

int
adonis_reformat_fits(char *filename, int force_flag)
{
    char        		outname[FILENAMESZ] ;
	char				newval[80];
	char			*	cval ;
    int		     		overwrite_flag ;
    int         		status ;
    unsigned char 	*	timeinfo ;
	qfits_header	*	fh ;	
	cube_info		*	fileinfo ;
	int					is_already_reformatted ;

	/* Is it truly a FITS file ?	*/
	if (is_fits_file(filename) != 1) {
		e_error("file %s is not recognized as FITS", filename) ;
		return 1 ;
	}

	/* Has the file been reformatted already? */
	if (qfits_query_hdr(filename, "ECLIPSE")!=NULL) {
		is_already_reformatted = 1 ;
	} else {
		is_already_reformatted = 0 ;
	}

	if (!force_flag && is_already_reformatted) {
		e_warning("file [%s] already reformatted", filename);
		return 1 ;
	}

	/* Check if acquisition was aborted	*/
	if (qfits_query_hdr(filename, "WARNING")!=NULL) {
		e_warning("acquisition was aborted for file %s", filename);
	}

    /*
     * Are we working in the same directory? Is it necessary to create
     * a temporary file?
     */
 
    overwrite_flag = 0 ;
    strcpy(outname, filename) ;
 
    /* See if a file in the current directory has the same name */
    if (file_exists(get_basename(outname))) {
        /*
         * Overwriting is required: outputs are directed to a temporary
         * file, then from the temporary file to the current one.
         */
        e_warning("overwriting file %s", filename) ;
        overwrite_flag = 1 ;
        sprintf(outname, "tmp-%06d.fits", (int)getpid()) ;
    }
 
	/* First step: Load the header	*/
    fh = qfits_header_read(filename) ;
    if (fh == NULL) {
        e_error("cannot read FITS header for file %s: aborting", filename) ;
        return 1 ;
    }

	/* Correct OB_CYCL keyword: should be a string */
	cval = qfits_header_getstr(fh, "OB_CYCL");
	if (cval!=NULL) {
		sprintf(newval, "'%s'", cval) ;
		qfits_header_mod(fh, "OB_CYCL", newval, "Cycle [ nnnn ] star=1 sky=0");
	}

	/*
	 * Add keyword to indicate reformatting has been done.
	 * Touch all cards in the header.
	 */
	if (!is_already_reformatted) {
		qfits_header_add(fh, "ECLIPSE", "1", "processed with eclipse", NULL);
	}
	qfits_header_touchall(fh);

    /* Get file information */
	fileinfo = cube_getinfo(filename) ;
	if (fileinfo == NULL) {
        e_error("problems in getting info from fits header") ;
        return 1 ;
    }

	/* Check the size to see if it is consistent with the file size */
	check_fits_size(filename, fileinfo) ;
    if (fileinfo->ly == 129 || fileinfo->ly==257) {
        /* Read time info and write it into the header  */
        timeinfo = read_time_info(filename, fileinfo) ;
        add_timeinfo_to_fits_hdr(fh, timeinfo, fileinfo->n_im) ;
		free(timeinfo) ;
    }
 
    /*
	 * Now transfer data from the original file to the new one
     * If time info is present, it will be discarded.
     * The value of NAXIS2 is then modified to the new one
     * Checks are made on the file size, NAXIS3 is eventually
     * modified to its real value if smaller than declared.
	 */
 
    status = transfer_data(filename, outname, fh, fileinfo) ;
	free(fileinfo) ;
	qfits_header_destroy(fh);
    if (status != 0) {
        e_error("while transfering data: aborting") ;
        return status ;
    }
 
    /*
     * If overwriting, data has been written to a temporary file,
     * transfer it to current directory.
     */
     if (overwrite_flag == 1) {
        /*
         * Copy data from tmp to current directory, overwriting previous
         * file eventually.
         */
        status = copy_file(outname, filename) ;
        /* Remove temporary file    */
        remove(outname) ;
    }
    return status ;
}

/*----------------------------------------------------------------------------
   Function	:	transfer_data()
   In 		:	filename in, filename out, FITS header, file info
   Out 		:	error code 0 if ok, 1 otherwise
   Job		:	transfer data contained in FITS files
   Notice	:	takes care to remove inconsistencies
  				does not copy time information if present (Adonis only)
 ---------------------------------------------------------------------------*/

int
transfer_data(
    char        *	inname,
    char        *	outname,
	qfits_header*	fh,
	cube_info	*	fileinfo
)
{
    FILE    *	in,
			*	out ;
    int   		linesize, plane_size;
	int			p ;
	char		cval[80];
	int			timeflag ;

	/* Error handling: test entries	*/
	if (inname == NULL || outname == NULL || fileinfo == NULL || fh == NULL) {
		e_error("transferring data");
		return 1 ;
	}

	timeflag=0 ;
	if (fileinfo->ly==129 || fileinfo->ly==257) {
		timeflag=1 ;
		fileinfo->ly -- ;
	}
    plane_size = fileinfo->lx * 
				 fileinfo->ly * 
				 BYTESPERPIXEL(fileinfo->ptype) ;

	linesize =  fileinfo->lx * BYTESPERPIXEL(fileinfo->ptype) ;
 
    /* Modify FITS header */

	sprintf(cval, "%d", fileinfo->ly);
	qfits_header_mod(fh, "NAXIS2", cval, "y axis");
	sprintf(cval, "%d", fileinfo->n_im);
	qfits_header_mod(fh, "NAXIS3", cval, "number of planes");
 
    /* Open in and out files for plane write    */
    if ((in = fopen(inname, "r")) == NULL) {
        e_error("cannot read %s : aborting\n", inname) ;
		return 1 ;
    }
    fseek(in, fileinfo->headersize, SEEK_SET) ;
    if ((out = fopen(outname, "a")) == NULL) {
        e_error("cannot append %s : aborting\n", outname) ;
        fclose(in) ;
        return 1 ;
    }
	
	/* Dump FITS header */
	qfits_header_dump(fh, out);
 
    /* Now repeat: read plane in, write plane out   */
    for (p=0 ; p<fileinfo->n_im ; p++) {
        copy_file_n_bytes(out, in, plane_size) ;
        if (timeflag) {
            fseek(in, linesize, SEEK_CUR) ;
		}
    }
    fclose(in) ;
    fclose(out) ;
    qfits_zeropad(outname) ;
    return 0;
}
 
/*----------------------------------------------------------------------------
   Function	:	check_fits_size()
   In 		:	filename, lx, ly, n_im, pixel type, header size
   Out 		:	1 if declared size matches actual size, 0 if not
  				-1 if error occurred
   Job		:	checks out if the declared size matches the actual size
   Notice	:
 ---------------------------------------------------------------------------*/
                             
int
check_fits_size(
    char    	*	filename,
	cube_info	*	fileinfo
)
{
    int	actual_size,
        declared_size,
        actual_blocks,
        declared_blocks,
        actual_planes,
        plane_size ;

	/* Error handling: test entries	*/

	if (filename == NULL) {
		e_error("no provided file name: cannot check FITS size") ;
		return(-1) ;
	}

	if (fileinfo == NULL) {
		e_error("no provided info: cannot check FITS size") ;
		return(-1) ;
	}

    /* Compute and get actual and declared size */
    plane_size = fileinfo->lx * 
				 fileinfo->ly * 
				 BYTESPERPIXEL(fileinfo->ptype) ;
    actual_size = filesize(filename) ;
	if (actual_size==0) {
		e_error("NULL file size") ;
		return(-1) ;
	}

	/* Compute number of FITS blocks needed to contain all the planes	*/
	declared_blocks = fileinfo->n_im * plane_size ; 
	if (declared_blocks % FITS_BLOCK_SIZE) {
		declared_blocks = 1 + declared_blocks / FITS_BLOCK_SIZE ;
	} else {
		declared_blocks = declared_blocks / FITS_BLOCK_SIZE ;
	}

	/* Add up header blocks	*/
	declared_blocks += (fileinfo->headersize) / FITS_BLOCK_SIZE ;

	/* This should be the file size	*/
    declared_size = declared_blocks * FITS_BLOCK_SIZE ;

	if (actual_size != declared_size) {
		e_warning("file size is %ld, should be %ld", actual_size, declared_size) ;
	}

    /* Transform it into a number of FITS blocks.   */
    if (actual_size % FITS_BLOCK_SIZE)
        actual_blocks = 1 + (actual_size / FITS_BLOCK_SIZE) ;
    else
        actual_blocks = actual_size / FITS_BLOCK_SIZE ;
 
    actual_planes = (actual_size - fileinfo->headersize) / plane_size ;
 
    if (declared_blocks != actual_blocks) {
        e_warning("file %s has inconsistent size:", filename) ;
        e_warning("found %ld blocks instead of %ld declared in header",
                actual_blocks, declared_blocks) ;
        if (actual_planes < fileinfo->n_im)
        {
            fileinfo->n_im = (int)actual_planes ;
            e_warning("lowering declared # of planes for consistency") ;
        } else {
			e_warning("trusting declared value: truncating file") ;
		}
        return 0 ;
    }    
    return 1 ;
}
 


/*----------------------------------------------------------------------------
   Function	:	read_time_info()
   In 		:	file name, FITS header of this file
   Out 		:	char * buffer, containing time information
   Job		:	retrieves raw time information written in the planes
   Notice	:	specific to Adonis
 ---------------------------------------------------------------------------*/
 
unsigned char *
read_time_info(char *filename, cube_info *fileinfo)
{
    FILE    		*	in ;
    unsigned char	*	t_info ;
    int     			i ;
    int		 			plane_size ;
    int		 			offset ;

	/* error handling: test entries	*/

	if (filename == NULL) {
		e_error("no provided file name: aborting time info read") ;
		return NULL ;
	}

	if (fileinfo == NULL) {
		e_error("no provided cube information: aborting time info read") ;
		return NULL ;
	}

	/* Open input file	*/
    if ((in = fopen(filename, "r")) == NULL) {
        e_error("cannot open file %s: aborting", filename) ;
        return NULL ;
    }

	/* Compute plane size in bytes	*/
    plane_size = fileinfo->lx * 
				 fileinfo->ly * 
				 BYTESPERPIXEL(fileinfo->ptype) ;

    /* Allocate memory for returned data    */

    t_info = calloc(fileinfo->n_im, TIME_INFO_SIZE) ;
    if (t_info==NULL) {
        e_error("in allocating space for time info: aborting") ;
        return NULL ;
    }

	/* Now loop on the file to extract all time info	*/
    for (i=0 ; i<fileinfo->n_im ; i++) {
        /* Compute offset to next time information */
        offset = fileinfo->headersize + (i+1)*plane_size - 
				 fileinfo->lx * BYTESPERPIXEL(fileinfo->ptype);
        fseek(in, offset, SEEK_SET) ;
        /* Read time info into memory   */
        if (fread(	t_info+i*TIME_INFO_SIZE,
					1,
					TIME_INFO_SIZE,
					in)<TIME_INFO_SIZE)
		{
			e_error("while reading time information: aborting") ;
			free(t_info) ;
			fclose(in) ;
			return NULL ;
		}
    }

    fclose(in) ;
    return t_info ;
}

 
/*----------------------------------------------------------------------------
   Function	:	add_timeinfo_to_fits_hdr()
   In 		:	FITS header, list of time info, # of infos in the list 
   Out 		:	void
   Job		:	adds up time info into FITS header
   Notice	:	specific to Adonis
 ---------------------------------------------------------------------------*/


void
add_timeinfo_to_fits_hdr(
	qfits_header	*	fh,
	unsigned char	*	timeinfo,
	int					n_info
)
{
	int					i ;
	char				cval[80];

	adonis_time_stamp	stamp ;
	adonis_stamp_format	stamp_f ;

	/* error handling: test entries	*/
	if (fh == NULL || timeinfo==NULL || n_info<1) return ;

	stamp_f = get_adonis_stamp_format(timeinfo) ;
	if (stamp_f == adonis_stamp_unknown) {
		e_error("unknown time stamp format: discarding time information");
		return ;
	}

	/* Print out format in a HISTORY keyword	*/
	qfits_header_add(fh, "HISTORY",
					"following are the acquisition dates", NULL, NULL);

	switch (stamp_f) {
		case adonis_stamp_pre05oct98:
		qfits_header_add(fh, "HISTORY",
						"plane day day.month.year hh:mm:ss tick (rate)",
						NULL, NULL);
		break ;

		case adonis_stamp_pos05oct98:
		qfits_header_add(fh, "HISTORY",
						"plane day julian seconds tick (rate)",
						NULL, NULL);
		break ;

		default:
		break ;
	}

	for (i=0 ; i<n_info ; i++) {
		/*
		 * This decodes the time information... 
		 * According to OS-9 manuals
		 */
		adonis_decode_time_stamp(timeinfo+i*TIME_INFO_SIZE, &stamp, stamp_f) ;
		switch (stamp_f) {
			case adonis_stamp_pre05oct98:
			sprintf(cval,
					"%04d: %s %02d.%02d.%04d %02d:%02d:%02d %02d (%02d)",
					i+1,
					DayOfTheWeek[stamp.day],
					stamp.DD,
					stamp.MM,
					stamp.YY,
					stamp.hh,
					stamp.mm,
					stamp.ss,
					stamp.tick,
					stamp.rate) ;
			break ;

			case adonis_stamp_pos05oct98:
			sprintf(cval,
					"%04d: %s %ld %ld %02d (%02d)",
					i+1,
					DayOfTheWeek[stamp.day],
					stamp.julian,
					stamp.secmid,
					stamp.tick,
					stamp.rate) ;
			break ;

			default:
			break ;
		}
		qfits_header_add(fh, "HISTORY", cval, NULL, NULL);
	}
	return ;
}


static void
adonis_decode_time_stamp(
	unsigned char 	  * tinfo,
	adonis_time_stamp * stamp,
	adonis_stamp_format stamp_f)
{
	switch (stamp_f) {
		case adonis_stamp_pre05oct98:
		/*
		 * Adonis time stamp format before 05 oct 1998
		 * Source: Francois Lacombe <Francois.Lacombe@obspm.fr>
		 *
		 * byte 00: unused
		 * byte 01: hour
		 * byte 02: minute
		 * byte 03: second
		 * byte 04-05: year
		 * byte 06: month
		 * byte 07: day
		 * byte 08-09-10: unused
		 * byte 11: weekday
		 * byte 12 & 13: tickrate
		 * byte 14 & 15: curtick
		 */

		stamp->hh  = (unsigned int)tinfo[1] ;
		stamp->mm  = (unsigned int)tinfo[2] ;
		stamp->ss  = (unsigned int)tinfo[3] ;

		stamp->day = (unsigned int)tinfo[11] ;
		stamp->DD  = (unsigned int)tinfo[7] ;
		stamp->MM  = (unsigned int)tinfo[6] ;
		stamp->YY  = (unsigned int)tinfo[4]<<8 | (unsigned int)tinfo[5] ; 

		stamp->rate = (unsigned int)tinfo[12]<<8 | (unsigned int)tinfo[13] ;
		stamp->tick = (unsigned int)tinfo[14]<<8 | (unsigned int)tinfo[15] ;
		break ;


		case adonis_stamp_pos05oct98:
		/*
		 * Adonis time stamp format after 05 oct 1998
		 * Source: Francois Lacombe <Francois.Lacombe@obspm.fr>
		 *
		 * byte 0-1-2-3 : seconds since midnight (long)
		 * byte 4-5-6-7 : julian day number (long)
		 * byte 8-9 : unused
		 * byte 10-11 : day of the week (short)
		 * byte 12-13 : tickrate (ticks per sec)
		 * byte 14-15 : curent tick number
		 */

		stamp->secmid = (unsigned long)tinfo[0]<<24 |
						(unsigned long)tinfo[1]<<16 |
						(unsigned long)tinfo[2]<<8  |
						(unsigned long)tinfo[3] ;
		stamp->julian = (unsigned long)tinfo[4]<<24 |
						(unsigned long)tinfo[5]<<16 |
						(unsigned long)tinfo[6]<<8  |
						(unsigned long)tinfo[7] ;
		stamp->day    = (unsigned int)tinfo[10]<<8  |
						(unsigned int)tinfo[11] ;
		stamp->rate   = (unsigned int)tinfo[12]<<8  |
						(unsigned int)tinfo[13] ;
		stamp->tick   = (unsigned int)tinfo[14]<<8  |
						(unsigned int)tinfo[15] ;
		break ;

		default:
		memset(stamp, sizeof(adonis_time_stamp), 0) ;
		break ;
	}
	return ;
}

static adonis_stamp_format
get_adonis_stamp_format(unsigned char * tinfo)
{
	adonis_stamp_format		stamp_f ;
	adonis_time_stamp		stamp ;

	adonis_decode_time_stamp(tinfo, &stamp, adonis_stamp_pre05oct98) ;
	if (stamp.hh>24 || stamp.ss>59 || stamp.day>6 ||
		stamp.DD>31 || stamp.MM>12 || stamp.YY>2100) {
		/*
		 * Not in Pre 05oct98 format: try out Post 05oct98
		 */
		stamp_f = adonis_stamp_pos05oct98 ;
	} else {
		stamp_f = adonis_stamp_pre05oct98 ;
	}
	return stamp_f ;
}

