/*----------------------------------------------------------------------------*/
/**
   @file    spjtypes.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.6 $
   @brief   Spectroscopic jitter data types
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjtypes.h,v 1.6 2003/11/19 12:02:52 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/19 12:02:52 $
	$Revision: 1.6 $
*/

#ifndef _SPJTYPES_H_
#define _SPJTYPES_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "isaacp_lib.h"
#include "eclipse.h"
#include "pfitspro.h"

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

typedef enum spjframe_type {
    type_obj=0,
    type_averaged,
    type_hc,
    type_rej,
    type_subtracted,
    type_combined
} spjframe_type ;

typedef enum spjoff_source {
    offsets_unknown=0,
    offsets_header,
    offsets_file,
    offsets_blind
} spjoff_source ;

typedef enum spjdiff_meth {
    diff_all=0,
    diff_half,
    diff_unknown
} spjdiff_meth ;

typedef enum spjcomb_meth {
    combine_unknown=0,
    combine_median,
    combine_rejection,
    combine_linear
} spjcomb_meth ;
 
typedef enum spjalgo_status {
    NOTREACHED = 0,
    OK         = 1,
    FAILED     =-1,
    SKIPPED    = 2
} spjalgo_status ;

typedef struct spjitter_frame_t {
    /* FITS file name */
    char                name[FILENAMESZ];
    /* Plane number in frame for NAXIS3>1 */
    int                 pnum ;
    /* Extension number in file (0=main) */
    int                 xtnum ;

    /* Image information */
    image_t         *   image ;
    spjframe_type       type ;
    char            *   docatg;
    
    double              offset ;
    int                 cube_id ;

} spjitter_frame_t ;

/*
  Spectroscopic jitter blackboard container

  This structure holds all information related to the spjitter routine. 
  It is used as a container for the flux of ancillary data, computed values, 
  and algorithm status. Pixel flux is separated from the blackboard.
*/
typedef struct spjitter_config_t {

    /* Input data */
    instrument_t            data_type;
    char                    in_name[FILENAMESZ];
    spjitter_frame_t    *   frame ;
    int                     nframes ;
    int                     nobjframes ;
    long                    total_pixin ;
    int                     lx ;
    int                     ly ;
    image_t             *   sky_lines ;
    
    /* Instrument setup */
    instrument_t            algo ;

    /* Calibrations */
    int                     cal_arc_active ;
    char                    cal_arc_name[FILENAMESZ];
    int                     cal_startrace_active ;
    char                    cal_startrace_name[FILENAMESZ];
    int                     cal_spflat_active ;
    char                    cal_spflat_name[FILENAMESZ];

    /* Classification */
    int                     divided_by_flat ; 
    spjoff_source           offsets_source ;
    char                    offsets_file[FILENAMESZ];
    int                     nb_classified_cubes ;

    /* Wavelength calibration */
    int                     wavecal_active ;

    int                     wavecal_arc_active ;
    char                    wavecal_arcfile[FILENAMESZ] ;
    
    int                     wavecal_discard_hi ;
    int                     wavecal_discard_lo ;
    int                     wavecal_discard_le ;
    int                     wavecal_discard_ri ;
    
    int                     wavecal_nb_coeff ;
    computed_disprel    *   wavecal_disprel ;
    
    /* Differences */
    spjdiff_meth            diff_method ;
        
    /* Correction of the distortion */
    int                     distortion_active ;
    
    int                     auto_dark_subtraction ;
    
    int                     distor_xmin ;
    int                     distor_ymin ;  
    int                     distor_xmax ;
    int                     distor_ymax ;
    
    /* Combination */
    double              *   main_offset_diff ;
    int                     circular_shift ;
    int                     refine_offsets ;
    spjcomb_meth            combine_method ;
    double                  average_hi_rejection ;
    double                  average_lo_rejection ;
    image_t             *   combined ;

    /* Extraction of the brightest spectrum */
    int                     spectrum_extr_active ;

    int                     detect_bad_left ;
    int                     detect_bad_right ; 
    int                     detect_bad_top ;
    int                     detect_bad_bot ;

    int                     spectrum_detected ;

    int                     spectrum_position ;
    int                     spectrum_width ;
    int                     res_sky_hi_width ;
    int                     res_sky_lo_width ;
    int                     res_sky_hi_dist ;
    int                     res_sky_lo_dist ;
    int                     apply_filter ; 
     
    int                     spectrum_extracted ;
     
    double              *   extracted_values ;
    double              *   extr_x_coordinate ;
    double              *   sky_signal ;

    /* Saving the output */
    char                    output_basename[FILENAMESZ];
    int                     output_startviewer ;
    char                    output_viewer[ASCIILINESZ];
    int                     output_gnuplot ;
    int                     output_statusreport ;

    /* Algorithm status */
    spjalgo_status          status_load ;
    spjalgo_status          status_classification ;
    spjalgo_status          status_wavecal_arc ;
    spjalgo_status          status_wavecal_sky ;
    spjalgo_status          status_wavecal_done ;
    spjalgo_status          status_differences ;
    spjalgo_status          status_disto_slit_curv ;
    spjalgo_status          status_disto_startrace ;
    spjalgo_status          status_combination ;
    spjalgo_status          status_extraction ;
    spjalgo_status          status_save ;
     
} spjitter_config_t ;

#endif
