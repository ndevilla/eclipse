/*----------------------------------------------------------------------------*/
/**
   @file    detect.h
   @author  Y.Jung
   @date    May 2001
   @version $Revision: 1.19 $
   @brief   Object detection in an astronomical image
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: detect.h,v 1.19 2003/02/21 09:23:07 yjung Exp $
    $Author: yjung $
    $Date: 2003/02/21 09:23:07 $
    $Revision: 1.19 $
*/

#ifndef _DETECT_H_
#define _DETECT_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "local_types.h"
#include "intimage.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

/* Default for kappa-sigma detection */
#define DETECTED_KAPPA          2.0

/* Defaults for square detection */
#define DETECTED_SQHX			10
#define DETECTED_SQHY			10

/* Defaults for fine positioning radiuses */
#define DETECTED_FPOS_STAR      10.0
#define DETECTED_FPOS_INT       15.0
#define DETECTED_FPOS_EXT       20.0

/* Defaults for photometry radiuses */
#define DETECTED_PHOT_STAR      10.0
#define DETECTED_PHOT_INT       15.0
#define DETECTED_PHOT_EXT       20.0

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	detected object

  This object contains various informations on detected objects in an image. 
  All coordinates are stored useing C convention (first pixel is (0, 0))
 */
/*----------------------------------------------------------------------------*/
typedef struct _detected_ {

	/* Number of detected objects in the structure */
	int			nbobj ;

	/* Object positions */
	double	*	x ;
	double	*	y ;

	/* Morphological data, not always filled */
	int		*	obj_nbpix ;
	int		*	bottom_x ;
	int		*	bottom_y ;
	int		*	top_x ;
	int		*	top_y ;
	int		*	left_x ;
	int		*	left_y ;
	int		*	right_x ;
	int		*	right_y ;
	int		*	min_x ;
	int		*	min_y ;
	int		*	max_x ;
	int		*	max_y ;
	double	*	min_i ;
	double	*	max_i ;
	double	*	obj_mean ;
	double	*	obj_stdev ;
	double	*	obj_median ;

	/* Object central fine positions (weighted gravity centers) */
	double	*	fine_x ;
	double	*	fine_y ;

	/* Objects FWHM */
	double	*	fwhm_x ;
	double	*	fwhm_y ;
	double		fwhm_medx ;
	double		fwhm_medy ;
	double		fwhm_meda ;


	/* Object photometry */
	double	*	obj_flux ;
	double	*	obj_background ;

} detected ;

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "detect_ks.h"
#include "detect_sq.h"

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Constructor for detected object.
  @return   1 newly allocated detected object.

  This function only allocates the main pointer. No information is stored in 
  there yet. The returned object must be deleted using detected_del().
 */
/*----------------------------------------------------------------------------*/
detected * detected_new(void) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Destructor for detected object.
  @param    det     Object to delete.
  @return   void

  This function deallocates all possibly allocated arrays inside the given 
  object, then deallocates the main pointer.
 */
/*----------------------------------------------------------------------------*/
void detected_del(detected * det) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Dump a detected object to an opened file pointer.
  @param    det     Detected object to dump
  @param    fp      Opened file pointer, ready to receive data
  @return   void

  This function dumps all informations contained into a detected object, to the
  passed (opened) file pointer. It is Ok to pass stdout or stderr. If the 
  object is unallocated or contains nothing, this function does nothing.
 */
/*----------------------------------------------------------------------------*/
void detected_dump(detected * det, FILE * fp) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    kappasigma detection and objects statistics computation.
  @param    in      Image to examine.
  @param    kappa   kappa for detection
  @return   a detected object   

  This function will detect astronomical objects in the image and fill up a
  detected structure accordingly. This version uses default parameters for 
  all settings.
 */
/*----------------------------------------------------------------------------*/
detected * detected_ks_withstats(
        image_t     *   in,
        double          kappa) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    squares-method object detection and statistics computation.
  @param    in      Image to examine.
  @param    hx      Halfsize of the square in X direction 
  @param    hy      Halfsize of the square in Y direction 
  @return   a detected object   

  This function will detect astronomical objects in the image and fill up a
  detected structure accordingly. This version uses default parameters for 
  all settings.
 */
/*----------------------------------------------------------------------------*/
detected * detected_sq_withstats(
        image_t     *   in,
        int             hx,
        int             hy) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute object statistics for all labelled objects in an image.
  @param    ref     Reference image.
  @param    lab     Label image.
  @param    nb      Number of objects
  @return   a detected object
 */
/*----------------------------------------------------------------------------*/
detected * detected_compute_objstat(
        image_t     *   ref,
        intimage    *   lab,
        int             nb) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute fine positioning for all detected objects.
  @param    det         Detected structure to fill up.
  @param    ref         Reference image.
  @param    fpos_star   Radius for star.
  @param    fpos_int    Internal radius for background.
  @param    fpos_ext    External radius for background.
  @return   int 0 if Ok, -1 otherwise.
 */
/*----------------------------------------------------------------------------*/
int detected_compute_finepos(
        detected    *   det,
        image_t     *   ref,
        double          fpos_star,
        double          fpos_int,
        double          fpos_ext) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the background value around an object
  @param    in          input image 
  @param    x_pos       Object x position
  @param    y_pos       Object y position
  @param    rad1        small radius
  @param    rad2        big radius
  @return   the background value    
 */
/*----------------------------------------------------------------------------*/
double detected_compute_background(
        image_t     *   in,
        int             x_pos,
        int             y_pos,
        double          rad1,
        double          rad2) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute FWHM of all objects in a detected structure.
  @param    det     Detected structure to fill up.
  @param    ref     Image to examine.
  @return   void

  This function computes the FWHM for all objects contained into the passed 
  detected structure. It writes the results into the detected structure, 
  assuming the FWHM fields have already been allocated.
  It also computes the median FWHM of all objects in the image.
 */
/*----------------------------------------------------------------------------*/
int detected_compute_fwhm(detected * det, image_t * ref) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute image quality
  @param    det     Detected structure to examine.
  @param    pscale  Pixel scale in arcsec/pixel
  @param    srange  Seeing range in arcsec as an array of 2 doubles.
  @return   1 double, negative if error occurred.

  This function tries to estimate the image quality in an image.

  This function expects a detected structure with filled FWHM fields (fwhm_x, 
  fwhm_y), a pixel scale in arcsec/pixel and possibly a seeing range in arcsec,
  given as an array of 2 doubles (may be NULL).

  The algorithm is the following:

  - Reject all measurements for which fwhm_x differs from fwhm_y by more than 
    a pre-set threshold (see SEEING_FWHM_VAR).
  - Reject all measurements for which FWHM is outside of the given seeing range.
  - Return the median of the remaining fwhm_a values.

  The provided seeing range may be NULL, in which case a default seeing range 
  of 0.1 to 5 arcseconds will be used.
  This function returns a negative value in case of error.
 */
/*----------------------------------------------------------------------------*/
double detected_compute_iq(
        detected    *   det,
        double          pscale,
        double      *   srange) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute photometry of all objects in an image.
  @param    det         Detected struct
  @param    ref         Image to examine
  @param    phot_star   Photometry: star radius
  @param    phot_int    Photometry: internal background radius
  @param    phot_ext    Photometry: external background radius
  @return   void

  This function computes the photometry for each object declared into the 
  detected structure. It assumes the photometry arrays in the input detected 
  object to be already allocated.
 */
/*----------------------------------------------------------------------------*/
int detected_compute_phot(
        detected    *   det,
        image_t     *   ref,
        double          phot_star,
        double          phot_int,
        double          phot_ext) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Convert a detected object in a double3 object
  @param    det     detected object 
  @return   the double3 object  
 */
/*----------------------------------------------------------------------------*/
double3 * detected2double3(detected * det) ;

#endif
