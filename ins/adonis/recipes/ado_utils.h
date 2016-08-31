
/*----------------------------------------------------------------------------
   
   File name 	:	ado_utils.h
   Author 		:	Nicolas Devillard
   Created on	:	Feb 29, 1996	
   Description	:	all these routines are Adonis specific 

 ---------------------------------------------------------------------------*/

/*

 $Id: ado_utils.h,v 1.3 2001/10/22 11:33:09 ndevilla Exp $
 $Author: ndevilla $
 $Date: 2001/10/22 11:33:09 $
 $Revision: 1.3 $

 */

#ifndef _ADO_UTILS_H_
#define _ADO_UTILS_H_


/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eclipse.h"


/*----------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/


#define KW_ADONIS_IR_BAND                       "OP_FILT"
#define KW_ADONIS_RIGHT_ASCENSION               "OJ_ALPHA"
#define KW_ADONIS_DECLINATION                   "OJ_DELTA"
#define KW_ADONIS_EPOCH                         "OJ_EPOCH"
#define KW_ADONIS_OBS_MODE                      "OB_MODE"

/*
 * These are specific to compute the theoretical PSF on the 3.6m
 * the primary and secondary mirror are the effective diameter and
 * obstruction, not the theoretical ones!
 */

#define PRIMARY_3_60        (3.47)
#define SECONDARY_3_60      (1.66)
#define LAMBDA_0_3_60       (2.20)
#define D_LAMBDA_3_60       (0.30)
#define PIXSCALE_3_60       (0.05)


#define CALIBRATION_UNKNOWN		0
#define CALIBRATION_PACKED		1
#define CALIBRATION_SEPARATED	2



/*----------------------------------------------------------------------------
  						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


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
	char	*file2) ;





/*----------------------------------------------------------------------------
   Function :   reduce_separated_cube()
   In       :   object, sky, flat, badpixelmap, output file names
   Out      :   void
   Job      :   cleans out an object cube (Adonis)
   Notice   :   quite specific to Adonis
  
    The algorithm is the following:
    1. average the sky
    2. subtract the average sky from the object
    3. average the result (if flag_avg is 1)
    4. flat-field and correct the bad pixels
 ---------------------------------------------------------------------------*/

void
reduce_separated_cube(
    char    *object,
    char    *sky,
    char    *flat,
    char    *bpm,
    char    *out,
	int	     flag_avg
) ;


/*----------------------------------------------------------------------------
   Function :   reduce_packed_cube()
   In       :   packed cube name, flat, bad pixel map, output name
   Out      :   void
   Job      :   cleans out a packed cube (Adonis)
   Notice   :   quite specific to Adonis
  
   The algorithm is the following:
   For each (object, sky) acquisition cycle:
    1. extract the sky, average it
    2. subtract it from each object plane in same cycle
    2b. average the result if flag_avg is 1
    3. flat-field the result
    4. correct for dead pixels
    5. append the results to output cube
 ---------------------------------------------------------------------------*/


void
reduce_packed_cube(
    char    *packed,
    char    *flat,
    char    *bpm,
    char    *out,
	int      flag_avg
) ;


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
    int         p_end);


/*----------------------------------------------------------------------------
   Function :   get_cycle_organization()
   In       :   cube name
   Out      :   # of cycles, # of image per cycle step, pattern
   Job      :   extracts the cycle organization from a cube
   Notice   :   SPECIFIC TO ADONIS
 ---------------------------------------------------------------------------*/

void
get_cycle_organization(
    char    *packed,
    int     *ncycles,
    int     *im_per_step,
    char    *pattern
) ;



/*----------------------------------------------------------------------------
   Function	:	adonis_reformat_fits()
   In 		:	FITS file name, force flag.
   Out 		:	error code: 0 if ok, 1 otherwise
   Job		:	reformat a FITS file (Adonis mostly specific)
   Notice	:	if working in the same directory, files are overwritten
 ---------------------------------------------------------------------------*/

int
adonis_reformat_fits(char *filename, int force_flag) ;


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
) ;


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
) ;



/*----------------------------------------------------------------------------
   Function	:	read_time_info()
   In 		:	file name, FITS header of this file
   Out 		:	char * buffer, containing time information
   Job		:	retrieves raw time information written in the planes
   Notice	:	specific to Adonis
 ---------------------------------------------------------------------------*/
 
unsigned char *
read_time_info(char *filename, cube_info *fileinfo) ;


 
/*----------------------------------------------------------------------------
   Function	:	add_timeinfo_to_fits_hdr()
   In 		:	FITS header, timeinfo, fileinfo
   Out 		:	void
   Job		:	adds up time info into FITS header
   Notice	:	specific to Adonis
 ---------------------------------------------------------------------------*/


void
add_timeinfo_to_fits_hdr(
	qfits_header		*	fh,
	unsigned char		*	timeinfo,
	int						n_info
) ;

#endif
