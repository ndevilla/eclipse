/*----------------------------------------------------------------------------*/
/**
   @file    jtypes.h
   @author  N. Devillard
   @date    March 2002
   @version	$Revision: 1.29 $
   @brief   Jitter data types
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jtypes.h,v 1.29 2002/12/05 10:11:55 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/05 10:11:55 $
	$Revision: 1.29 $
*/

#ifndef _JTYPES_H_
#define _JTYPES_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "eclipse.h"
#include "pfitspro.h"

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

typedef enum jframe_type {
    type_obj=0,
    type_sky,
    type_hc,
    type_rej,
    type_subtracted
} jframe_type ;

typedef enum jsky_method {
    skymethod_auto        = 0,
    skymethod_combine     = 1,
    skymethod_combine_mc  = 2,
    skymethod_medianframe = 3
} jsky_method ;


typedef enum jalgo_status {
    ALGO_NOTREACHED = 0,
    ALGO_OK         = 1,
    ALGO_FAILED     =-1,
    ALGO_SKIPPED    = 2
} jalgo_status ;

typedef struct reject_zone_t {
    /* Number lines or columns to reject */
    int         bottom ;
    int         top ;
    int         left ;
    int         right ;
} reject_zone_t ;

typedef struct jitter_frame_t {
    /* FITS file name */
    char        name[FILENAMESZ];
    /* Plane number in frame for NAXIS3>1 */
    int         pnum ;
    /* Extension number in file (0=main) */
    int         xtnum ;

    /* Image information */
    image_t *   image ;
    jframe_type type ;
    char    *   docatg;

    double      off_x ;
    double      off_y ;

    double      off_cor_x ;
    double      off_cor_y ;
    double      off_dist ;

    double      off_err_x ;
    double      off_err_y ;

    /* Background information */
    double      skyval ;

} jitter_frame_t ;


/*
  Jitter imaging blackboard container

  This structure holds all information related to the jitter imaging
  routine. It is used as a container for the flux of ancillary data,
  computed values, and algorithm status. Pixel flux is separated from
  the blackboard.
  */

typedef struct jitter_config_t {

    /* Input data */
    instrument_t            data_type;
    char                    in_name[FILENAMESZ];
    jitter_frame_t      *   frame ;
    int                     nframes ;
    reject_zone_t           zone ;
    long                    total_pixin ;
    int                     lx ;
    int                     ly ;

    /* Instrument setup */
    instrument_t            algo ;

    /* Pre-processing */
    int     preproc_active ;
    int     preproc_oddeven ;
    int     preproc_fiftyhertz ;

    /* Calibrations */
    int     dark_sub ;
    char    dark_name[FILENAMESZ];
    int     ff_div ;
    char    ff_name[FILENAMESZ];
    int     badpix_rep ;
    char    badpixmap[FILENAMESZ];

    /* Sky estimation */
    int     sky_active ;
    int     sky_ispresent ;
    int     sky_outdiff ;

    jsky_method sky_method ;
    jsky_method sky_method_used ;

    /* Sky filter settings */
    int     skyfilter_minframes ;
    int     skyfilter_rejhw ;
    int     skyfilter_rejmin ; 
    int     skyfilter_rejmax ;
    int     skyfilter_sepquad ;

    /* Shift and add */
    int     saa_active ;

    /* Shift and add: object source */
    enum {
        objsource_auto=0,
        objsource_file
    } saa_objsource ;

    /* Shift and add: automatic object source */
    enum {
        detectim_diff=0,
        detectim_first,
        detectim_invalid
    } saa_detectim ;

    double  saa_detectk ;
    int     saa_detectminp ;
    int     saa_detectmaxp ;
    int     saa_detectoutf ;

    /* Shift and add: file object source */
    char    saa_objfile[FILENAMESZ];

    /* Shift and add: list of x-correlation places */
    int       saa_xcorrp_n ;
    double  * saa_xcorrp_x ;
    double  * saa_xcorrp_y ;


    /* Shift and add: offset source */
    enum {
        offsource_unknown=0,
        offsource_header,
        offsource_file,
        offsource_blind
    } saa_offsource ;

    /* Shift and add: file offsets */
    char    saa_offfilename[FILENAMESZ];

    /* Shift and add: x-correlation configuration */
    int     saa_xcorractive ;
    int     saa_xcorrsx ;
    int     saa_xcorrsy ;
    int     saa_xcorrhx ;
    int     saa_xcorrhy ;

    /* Shift and add: 3d filtering for stacking */
    int     saa_3drejmin ;
    int     saa_3drejmax ;
    int     saa_union ;

    /* Post-processing */
	int		pproc_active ;
	int		pproc_rowmediansub ;
	int		pproc_startviewer ;
	char	pproc_viewer[ASCIILINESZ];
	
    /* Saving results */
	char	output_basename[FILENAMESZ];

    /* Final image */
    image_t * final ;

    /* Algo status */
	jalgo_status	status_load ;
	jalgo_status	status_calib ;
	jalgo_status	status_sky ;
	jalgo_status	status_saa ;
	jalgo_status	status_postproc ;
	jalgo_status	status_save ;

} jitter_config_t ;

#endif
