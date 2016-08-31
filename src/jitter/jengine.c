/*----------------------------------------------------------------------------*/
/**
   @file    jengine.c
   @author  N. Devillard
   @date    Sep 1997
   @version	$Revision: 1.5 $
   @brief   Main engine for jitter command
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jengine.c,v 1.5 2002/04/19 09:16:18 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 09:16:18 $
	$Revision: 1.5 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "jtypes.h"
#include "jconfig.h"
#include "jload.h"
#include "jcalib.h"
#include "jsky.h"
#include "jsaa.h"
#include "jpproc.h"
#include "jsave.h"

/*-----------------------------------------------------------------------------
                                Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Main jitter recipe engine
  @param    ininame     Name of the input ini file.
  @return   long giving the total number of processed pixels

  This is the main engine for the jitter algorithm.
  The returned value indicates the number of pixels received in input,
  or -1 if an error occurred.
 */
/*----------------------------------------------------------------------------*/
#define NPARTS 6
long jitter_engine(char * ininame)
{
    jitter_config_t   *   jc ;
    long                  total_pixin ;
    int                   p ;
    time_t                local_t ;

    p=0 ;
	e_comment(0, "---> STARTING JITTER ENGINE") ;
	time(&local_t) ;
	e_comment(0, "%s", ctime(&local_t)) ;
	e_comment(0, "pid is %ld", (long)getpid());

	/*
	 * Load data
	 */
	p++ ;
	e_comment(0, "---> part %d of %d: loading data", p, NPARTS) ;
    jc = jitter_load(ininame);
    if (jc==NULL) {
        return -1 ;
    }
    total_pixin = jc->total_pixin ;

	/*
	 * Apply dark subtraction/flat-field division/bad pixel replacement. 
	 */
	p++ ;
	e_comment(0, "---> part %d of %d: calibrations", p, NPARTS) ;
    if (jitter_calibration(jc)!=0) {
        e_error("applying calibrations: aborting");
        jitter_config_del(jc);
        return -1 ;
    }
	
	/*
	 * Apply sky background subtraction.
	 */
	p++ ;
	e_comment(0, "---> part %d of %d: sky estimation/subtraction", p, NPARTS) ;
    if (jitter_sky(jc)!=0) {
		e_error("applying background subtraction: aborting") ;
		jitter_config_del(jc);
		return -1 ;
	}

    /*
     * Apply shift-and-add
     */
	p++ ;
	e_comment(0, "---> part %d of %d: shift and add", p, NPARTS);
    if (jitter_saa(jc)!=0) {
		e_error("applying shift-and-add: aborting");
        jitter_config_del(jc);
		return -1 ;
	}
	
	/*
	 * Optional post-processing features will be inserted here
	 */
	p++ ;
	e_comment(0, "---> part %d of %d: post-processing", p, NPARTS) ;
	if (jitter_postproc(jc)!=0) {
		e_error("applying post-processing: aborting") ;
        jitter_config_del(jc);
        return -1 ;
	}

	/*
	 * Save results
	 */
	p++ ;
	e_comment(0, "---> part %d of %d: saving output data", p, NPARTS);
    if (jitter_save(jc)!=0) {
        e_error("saving results to disk: aborting");
        jitter_config_del(jc);
        return -1 ;
    }

    /* If requested: launch an image viewer on the result */
    jitter_viewer(jc);

    /* Free data */
    jitter_config_del(jc);

	e_comment(0, "---> STOPPING JITTER ENGINE") ;
	time(&local_t) ;
	e_comment(0, "%s", ctime(&local_t));

	return total_pixin ;
}
