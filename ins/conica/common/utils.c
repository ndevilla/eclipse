
/*----------------------------------------------------------------------------
   
   File name    :   utils.c
   Author       :	N. Devillard
   Created on   :	July 2000 	
   Description  :	CONICA various utilities 

 ---------------------------------------------------------------------------*/

/*
 $Id: utils.c,v 1.4 2002/01/07 12:48:21 ndevilla Exp $
 $Author: ndevilla $
 $Date: 2002/01/07 12:48:21 $
 $Revision: 1.4 $
 */

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include "utils.h"

/*----------------------------------------------------------------------------
						Function ANSI C code 
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	apply dark subtraction and ff division, replace bad pixels
  @param	in	pointer to allocated cube
  @param	ff_name	flat field name
  @param	dark_name	dark name
  @return	badpix_name	bad pixel name
  The input cube is always modified, to ensure that the
  returned value is read/write.
 */
/*--------------------------------------------------------------------------*/
void conica_ff_dark_badpix_handling(
		cube_t	**	in,
		char	*	ff_name,
		char	*	dark_name,
		char	*	badpix_name)
{
	image_t		*	dark ;
	image_t		*	ff ;
	pixelmap	*	badpix ;
	int				has_dark,
					has_ff ;

	dark = NULL ;
	ff   = NULL ;

	/* see if dark was given and can be found */
	/* note: further tests should be done to ensure consistency, too */
	if (dark_name && *dark_name) {
		if ((dark = image_load(dark_name)) == NULL) {
			e_error("cannot load dark file [%s]", dark_name) ;
			has_dark = 0 ;
		} else if ((dark->lx != (*in)->lx) || (dark->ly != (*in)->ly)) {
			e_error("incompatible sizes for dark and co_jitter cube") ;
			e_error("dark image size is [%d x %d]", dark->lx, dark->ly);
			has_dark = 0 ;
			image_del(dark) ;
		} else has_dark = 1 ;
	} else has_dark = 0 ;

	/* see if flat-field was given and can be found */
	/* note: further tests shouls be done to ensure consistency, too */
	if (ff_name && *ff_name) {
		if ((ff = image_load(ff_name)) == NULL) {
			e_error("cannot load flat-field file [%s]", ff_name) ;
			has_ff = 0 ;
		} else if ((ff->lx != (*in)->lx) || (ff->ly != (*in)->ly)) {
			e_error("incompatible sizes for flat-field and co_jitter cube") ;
			e_error("flat-field image size is [%d x %d]", ff->lx, ff->ly);
			image_del(ff) ;
			has_ff = 0 ;
		} else has_ff = 1 ;
	} else has_ff = 0 ;

	/* If no input was provided, return the input cube untouched */
	if ((has_dark==0) && (has_ff==0)) {
		e_comment(1, "flat-field division and dark subtraction skipped");
		return ;
	}

	/* Only dark was provided */
	if ((has_dark==1) && (has_ff==0)) {
		e_comment(1, "applying dark subtraction") ;
		cube_sub_im((*in), dark) ;
		image_del(dark) ;
		e_comment(1, "no flat-field provided: skipping") ;
	}
	
	/* Only flat-field was provided */
	if ((has_dark==0) && (has_ff==1)) {
		e_comment(1, "no dark provided: skipped") ;
		e_comment(1, "applying flat-field division") ;
		cube_div_im((*in), ff) ;
		image_del(ff) ;
	}

	/* Both dark and flat-field have been provided */
	if ((has_dark==1) && (has_ff==1)) {
		e_comment(1, "applying dark subtraction and flat-field division");
		cube_subdiv_im((*in), dark, ff) ;
		image_del(dark) ;
		image_del(ff) ;
	}

	/* Apply now bad pixel correction if needed */
    if (badpix_name && *badpix_name) {
        badpix = pixelmap_load(badpix_name) ;
        if (badpix == NULL) {
            e_error("cannot load bad pixel map [%s]: skipping", badpix_name) ;
        } else {
            e_comment(1, "applying dead pixel correction") ;
            cube_clean_deadpix(*in, badpix) ;
            pixelmap_del(badpix) ;
        }
    } else {
        e_comment(1, "bad pixel replacement: skipped") ;
    }

	return ;
}

