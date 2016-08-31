/*-------------------------------------------------------------------------*/
/**
   @file    pixel_handling.h
   @author  Nicolas Devillard
   @date    March 04, 1997
   @version $Revision: 1.8 $
   @brief   Functions processing arrays of pixels
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: pixel_handling.h,v 1.8 2001/10/26 14:33:42 yjung Exp $
    $Author: yjung $
    $Date: 2001/10/26 14:33:42 $
    $Revision: 1.8 $
*/

#ifndef _PIXEL_HANDLING_H_
#define _PIXEL_HANDLING_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "xmemory.h"
#include "local_types.h"

/*---------------------------------------------------------------------------
  							Function ANSI prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Compare two pixelvalues.
  @param    pix1    First pixel value.
  @param    pix2    Second pixel value.
  @return   1 if (pix1>pix2), -1 otherwise.

  This function is meant to be used with qsort().
 */
/*--------------------------------------------------------------------------*/
int pixel_compare(
        const void * pix1,
        const void * pix2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Sort an array of pixels by increasing pixelvalue.
  @param    pix_arr     Array to sort.
  @param    npix        Number of pixels in the array.
  @return   void

  Optimized implementation of a fast pixel sort. The input array is
  modified.
 */
/*--------------------------------------------------------------------------*/
void pixel_qsort(
        pixelvalue  *   pix_arr,
        int             npix) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Convert an array from pixelvalue to double.
  @param    arr     Input pixelvalue array.
  @param    n       Number of values in the array.
  @return   1 newly allocated array of doubles.

  Convert an array of pixelvalues to a newly allocated array of doubles.
  The returned array must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
double * pixel2double_array(
        pixelvalue  *   arr,
        int             n) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Convert an array from double to pixelvalue.
  @param    arr     Input double array.
  @param    n       Number of values in the array.
  @return   1 newly allocated array of pixelvalues.

  Convert an array of doubles to a newly allocated array of pixelvalues.
  The returned array must be deallocated using free().
 */
/*--------------------------------------------------------------------------*/
pixelvalue * double2pixel_array(
        double  *   arr,
        int         n) ;

#endif
