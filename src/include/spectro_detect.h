/*-------------------------------------------------------------------------*/
/**
   @file    spectro_detect.h
   @author  Thomas Rogon
   @date    October 1999
   @version $Revision: 1.14 $
   @brief   spectroscopy routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: spectro_detect.h,v 1.14 2001/11/07 16:06:13 yjung Exp $
    $Author: yjung $
    $Date: 2001/11/07 16:06:13 $
    $Revision: 1.14 $
*/

#ifndef _SPECTRO_DETECT_H_
#define _SPECTRO_DETECT_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "eclipse.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define SPEC_PARAM_WINDOWSIZE			32
#define SPEC_PARAM_MINGOODPIX			100
#define SPEC_PARAM_THRESHFACT           1.0
#define SPEC_MIN_LEN_FACT               5    /* fraction of image length */
#define SPEC_SHADOW_FACT                10.0 /* Negative spectrum intensity*/
#define SPEC_MAXWIDTH					48
#define SPEC_PARAM_MEDIAN_SIZE			10

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Criteria for spectrum detection 

  This enum is used internally by find_brightest_spectrum().
 */
/*--------------------------------------------------------------------------*/
typedef enum SPEC_DETECT_MODE_T{
	/* 2 shadows above and below true spectrum */
	EQUALLY_SPACED_SHADOW_SPECTRA, 
	/* 1 shadow at specified distance from spectrum */
	ONE_SHADOW_SPECTRUM,	
	/* Do not search for shadow */
	NO_SHADOW_SPECTRUM	
} spec_detect_mode_t;


/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/**
  @brief    Finds the brightest spectrum in an image 
  @param    in      spectral image with spectra conforming to the other param.
  @param    main_offset_diff the characteristic diff. bet. pos. and neg. spec.
  @param    spec_detect_mode the spectrum detection method 
  @param    min_bright      min. bright. required for a spec. to be detected.
  @return The coordinates of the found spectrum, NULL if error.

  Finds the brightest spectrum in an image by collapsing the image
  orthogonally to the spectrum orientation.
  Spectra are assumed to be horizontal.
  C-style convention for coordinates is used, i.e.  top,left=0,0.
*/
/*---------------------------------------------------------------------------*/
double3 * find_brightest_spectrum_1d(
        image_t         *   in,
        int                 main_offset_diff,
        spec_detect_mode_t  spec_detect_mode,
        double              min_bright) ;

#endif
