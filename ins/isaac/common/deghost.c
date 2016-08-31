/*----------------------------------------------------------------------------*/
/**
   @file    deghost.c
   @author  N. Devillard
   @date    January 2001
   @version	$Revision: 1.10 $
   @brief   ISAAC deghosting routines
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: deghost.c,v 1.10 2002/12/10 15:11:38 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/10 15:11:38 $
	$Revision: 1.10 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "deghost.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define GHOST_SCALE			1.35e-5
#define GHOST_ALGO_ID		"14 Apr 1999"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

static image_t * isaac_deghost_image(image_t *) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	remove ISAAC electrical ghost from an ISAAC image
  @param	filename	input FITS image
  @param	force_flag	force flag
  @return	0 if ok, -1 otherwise
  New keywords are added to the header.
  GHOSTREM = 1
  GHOSTVER = 'Revision: x.y'
  If GHOSTREM is present and equal to 1, and force_flag is 0, no action
  is performed on the file.
 */
/*----------------------------------------------------------------------------*/
int isaac_ghost_removal(
	    char 	*   inname,
	    int		    force_flag)
{
	image_t	    *	in ;
	image_t	    *	deghosted ;
	qfits_header*	fh ;
	char			name_out[1024] ;
	char		*	romode ;
    instrument_t    ins ;

	/* Bulletproof entries */
	if (inname==NULL) return -1 ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	
    /* Check if file has already been corrected */
	if (!force_flag) {
		if (qfits_query_hdr(inname, "GHOSTREM")!=NULL) {
			e_comment(0, "ghost already removed from [%s]", inname);
			return -1 ;
		}
	}

	/* Check out readout mode */
	if (!force_flag) {
		romode = pfits_get(ins, inname, "detector_readout_mode");
		if (romode==NULL) {
			e_error("cannot determine readout mode for [%s]", inname);
			return -1 ;
		} else {
			if (strcmp(romode, "NonDest") && strcmp(romode, "DoubleCorr")) {
				e_error("invalid readout mode for frame [%s]", inname);
				e_error("DET.MODE.NAME == \"%s\"", romode);
				e_error("should be \"NonDest\" or \"DoubleCorr\"");
				return -1 ;
			}
		}
	}

	/* Load input image */
	in = image_load(inname);
	if (in==NULL) {
		e_error("removing ghost: cannot read file [%s]", inname) ;
		return -1 ;
	}

	/* Apply ghost removal */
	deghosted = isaac_deghost_image(in);
	image_del(in);
	if (deghosted==NULL) {
		e_error("removing ghost: aborting");
		return -1 ;
	}

	/* Prepare output */
	strcpy(name_out, get_basename(inname));
	fh = qfits_header_read(inname);
	if (fh==NULL) {
		e_error("removing ghost: cannot read file [%s]", inname);
		return -1 ;
	}
	qfits_header_add(fh, "GHOSTREM", "1", "ISAAC ghost removed", NULL);
	qfits_header_add(fh, "GHOSTVER", GHOST_ALGO_ID,
					"ghost removal algorithm ID", NULL);
	if (file_exists(name_out)) {
		e_warning("overwriting file [%s]", name_out);
	}
	/* Save results */
	image_save_fits_hdrdump(deghosted,
								name_out,
								fh,
								BPP_DEFAULT);
	qfits_header_destroy(fh);
	image_del(deghosted);
	e_comment(0, "ghost removed from [%s]", name_out);
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Remove the ghost from an ISAAC frame.
  @param	in		Image to process.
  @return	1 newly allocated image or NULL if error occurred.
  This functions applies the ghost removal algorithm for ISAAC frames.
 */
/*----------------------------------------------------------------------------*/
static image_t * isaac_deghost_image(image_t * in)
{
	image_t	*	deghosted ;
	int				i, j ;
	double		*	sum_line ;
	double		*	new_line ;
	pixelvalue	*	start_in ;
	pixelvalue	*	start_out ;

	if (in==NULL) return NULL ;
	
	/* Sum all pixels along the lines */
	sum_line = malloc(in->ly * sizeof(double)) ;
	for (j=0 ; j<in->ly ; j++) {
		sum_line[j] = 0.00 ;
		start_in = in->data + j*in->lx ;
		for (i=0 ; i<in->lx ; i++) {
			sum_line[j] += (double)start_in[i] ; 
		}
	}

	/* Create the correction line */
	new_line = malloc(in->ly * sizeof(double));
	for (i=0 ; i<(in->ly/2) ; i++) {
		new_line[i] =
		new_line[i+in->ly/2] =
			(sum_line[i] + sum_line[i+in->ly/2]) * GHOST_SCALE ;
	}
	free(sum_line);

	/* Subtract correction value from all pixels in each line */
	deghosted = image_new(in->lx, in->ly) ;
	for (j=0 ; j<in->ly ; j++) {
		start_in = in->data + j*in->lx ;
		start_out = deghosted->data + j*deghosted->lx ;
		for (i=0 ; i<in->lx ; i++) {
			start_out[i] = start_in[i] - (pixelvalue)new_line[j];
		}
	}
	free(new_line);
	return deghosted ;
}

