/*----------------------------------------------------------------------------*/
/**
   @file    classif.h
   @author  Y. Jung
   @date    July 2000
   @version $Revision: 1.9 $ 
   @brief   ISAAC common functions for frames classification
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: classif.h,v 1.9 2003/01/13 09:41:17 yjung Exp $
    $Author: yjung $
    $Date: 2003/01/13 09:41:17 $
    $Revision: 1.9 $
*/

#ifndef _CLASSIF_H_
#define _CLASSIF_H_

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    compare the grating, the slit and the wl in input files headers 
  @param    file1   first file
  @param    file2   second file
  @return   -1 in error case, 0 for different settings, 1 for equal settings

  Used in sp_flat and arc
 */
/*----------------------------------------------------------------------------*/
int compare_settings(
        char    *   file1,
        char    *   file2) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Filter half-cycle frames out of a frame list.
  @param    flist   Framelist object to purge.
  @return   int 0 if Ok, -1 otherwise.
  This function processes a framelist object to remove any half-cycle
  frame. These frames are only expected in long-wavelength mode and they
  can be detected in two ways. Either the frame type in the framelist
  contains the word 'half' (case-insensitive), or the frame name
  corresponds to a FITS file that is referenced as a half-cycle frame.

  Used in zpoint
 */
/*----------------------------------------------------------------------------*/
int isaac_lw_filter_halfcycle(framelist ** flist) ;

#endif





