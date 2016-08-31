/*----------------------------------------------------------------------------*/
/**
   @file    wavelength.h
   @author  Y. Jung
   @date    July 2000
   @version	$Revision: 1.14 $
   @brief   ISAAC common function to handle with wl calibration
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: wavelength.h,v 1.14 2003/11/10 14:42:32 llundin Exp $
	$Author: llundin $
	$Date: 2003/11/10 14:42:32 $
	$Revision: 1.14 $
*/

#ifndef _WAVELENGTH_H_
#define _WAVELENGTH_H_

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Estimate the instrument wavelength range.
  @param    filename    input file name
  @param    poly_deg    polynomial degree
  @return   1 newly allocated array of poly_deg+1 polynomial coefficients
  
  From a physical model of the instrument, find out the wavelength range
  associated to a given instrument configuration. The returned coefficients
  are such as wave = c[0] + c[1] * pix + ... + c[poly_deg] * pix^poly_deg 

  Instrument configuration informations are fetched from the FITS header.

  Used by arc, respfunc, startrace, wavecal, sw_spjitter, lw_spjitter
 */
/*----------------------------------------------------------------------------*/
double * isaac_get_disprel_estimate(
        char    *   filename,
        int         poly_deg) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Determine the order according to filter, grating and wl
  @param    image_name  input image name
  @return   order, -1 in error case 
 */
/*----------------------------------------------------------------------------*/
int isaac_find_order(char * image_name) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Determine presence of thermal background according to instr. setting
  @param    image_name  input image name
  @return   1 if yes, 0, when not, -1 in error case 

 */
/*----------------------------------------------------------------------------*/
int isaac_has_thermal(char * image_name) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    ISAAC physical model
  @param    lambda_c    central wavelength
  @param    objective   slit used
  @param    resolution  resolution (medium or low)
  @param    nbpix       number of pixels to calibrate
  @return   wavelengths array (index i <-> pixel nb i+1)
    
  This module determines the dispersion relation of ISAAC for the different 
  configurations of objectives, gratings, detectors, and central wavelengths.

  Assumed Optical configuration:
   
    Focal lens of objectives for short- and long-wavelength objective (at 77 K):
        S1 = f/1.75, S2 = f/3.25, L1 = f/1.56, L2 = f/4.77,  L3 = f/9.98
    Pupil size: 100 mm
    Pixel size: 18.5 microns (SW), 27 microns (LW)
    Gratings:   low resolution, 40 gr/mm, entering at about 5 degrees
                medium resolution, 210 gr/mm, entering at about 23 degrees
    Beam difference: 2.72 degrees
 */
/*----------------------------------------------------------------------------*/
double * isaac_physical_model(
        double      lambda_c,
        char    *   objective,
        char    *   resolution,
        int         nbpix) ;

#endif



