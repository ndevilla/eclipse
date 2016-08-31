/*----------------------------------------------------------------------------*/
/**
   @file    spjengine.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.4 $
   @brief   Main engine for spjitter command
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjengine.c,v 1.4 2003/01/10 16:13:19 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/10 16:13:19 $
	$Revision: 1.4 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "spjtypes.h"
#include "spjload.h"
#include "spjclassif.h"
#include "spjconfig.h"
#include "spjcalib.h"
#include "spjsaa.h"
#include "spjextract.h"
#include "spjsave.h"

/*-----------------------------------------------------------------------------
                                Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Main spjitter recipe engine
  @param    ininame     Name of the input ini file.
  @return   long giving the total number of processed pixels

  This is the main engine for the spjitter algorithm.
  The returned value indicates the number of pixels received in input,
  or -1 if an error occurred.
 */
/*----------------------------------------------------------------------------*/
#define NPARTS 10
long spjitter_engine(char * ininame)
{
    spjitter_config_t   *   spjc ;
    long                    total_pixin ;
    int                     p ;
    time_t                  local_t ;

    p=0 ;
	e_comment(0, "---> STARTING SPJITTER ENGINE") ;
	time(&local_t) ;
	e_comment(0, "%s", ctime(&local_t)) ;
	e_comment(0, "pid is %ld", (long)getpid());

	/* Load data */
	p++ ;
	e_comment(0, "---> part %d of %d: loading data", p, NPARTS) ;
    spjc = spjitter_load(ininame);
    if (spjc==NULL) return -1 ;
    total_pixin = spjc->total_pixin ;

    /* Data classification */
    p++ ;
    e_comment(0, "---> part %d of %d: data classification", p, NPARTS) ;
    if (spjitter_classif(spjc) != 0) {
        e_error("applying classification: aborting");
        spjitter_config_del(spjc);
        return -1 ;
    }

    /* Flatfield correction */
    p++ ;
    e_comment(0, "---> part %d of %d: flatfielding", p, NPARTS) ;
    if (spjitter_flatfield(spjc) != 0) {
        e_error("applying flatfielding: aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }

    /* Shift and average classified cubes */
    p++ ;
    e_comment(0, "---> part %d of %d: average cubes", p, NPARTS) ;
    if (spjitter_averaging(spjc) != 0) {
        e_error("averaging cubes - aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }
    
    /* Wavelength calibration */
    p++ ;
    e_comment(0, "---> part %d of %d: wavelength calibration", p, NPARTS) ;
    if (spjitter_wlcalib(spjc) != 0) {
        e_error("wavelength calibration - aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }
    
    /* Compute differences */
    p++ ;
    e_comment(0, "---> part %d of %d: differences computation", p, NPARTS) ;
    if (spjitter_differences(spjc) != 0) {
        e_error("differences computation - aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }
    
    /* Distortion correction */
    p++ ;
    e_comment(0, "---> part %d of %d: distortion correction", p, NPARTS) ;
    if (spjitter_distortion(spjc) != 0) {
        e_error("distortion correction - aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }

    /* Frames combination */
    p++ ;
    e_comment(0, "---> part %d of %d: frames combination", p, NPARTS) ;
    if (spjitter_combine(spjc) != 0) {
        e_error("frames combination - aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }

    /* Spectrum extraction */
    p++ ;
    e_comment(0, "---> part %d of %d: spectrum extraction", p, NPARTS) ;
    if (spjitter_extract(spjc) != 0) {
        e_warning("spectrum extraction failed") ;
    }

    /* Save products */
    p++ ;
    e_comment(0, "---> part %d of %d: save products", p, NPARTS) ;
    if (spjitter_save(spjc) != 0) {
        e_error("saving products - aborting") ;
        spjitter_config_del(spjc);
        return -1 ;
    }
    
    /* Free data */
    spjitter_config_del(spjc);

	e_comment(0, "---> STOPPING SPJITTER ENGINE") ;
	time(&local_t) ;
	e_comment(0, "%s", ctime(&local_t));

	return total_pixin ;
}
