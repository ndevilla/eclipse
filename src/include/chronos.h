
/*-------------------------------------------------------------------------*/
/**
   @file    chronos.h
   @author  N. Devillard
   @date    Feb 1998
   @version $Revision
   @brief   Timing related routines

   This module offers some functionalities to measure the execution
   time of a program or program part.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: chronos.h,v 1.6 2001/10/17 08:12:21 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 08:12:21 $
	$Revision: 1.6 $
 */

#ifndef _CHRONOS_H_
#define _CHRONOS_H_

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <time.h>


/*----------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Start clock signal */
#define	START_CLOCK			0
/** Stop clock signal */
#define STOP_CLOCK			1
/** Print accumulated time */
#define ACC_CLOCK			2	


/*----------------------------------------------------------------------------
  						Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Timer handling for benchmarking purposes.
  @param    mode    is one of: START_CLOCK, STOP_CLOCK or ACC_CLOCK
  @param    npix    Number of crunched pixels (optional)
  @return   double indicating an elapsed time in seconds.
 
  This routine is useful to benchmark the time spent in an image
  processing routine. It is used in three steps:
 
  - Start the timer by calling @c eclipse_cpu_timing(START_CLOCK)
  - Stop the timer by calling @c eclipse_cpu_timing(STOP_CLOCK)
  - Print out the accumulated total elapsed time by calling
  @c eclipse_cpu_timing(ACC_CLOCK)
 
  It is possible to start/stop the clock several times in the same
  program. The last call should be done with @c ACC_CLOCK to print the
  total elapsed time.
 
  If the number of crunched pixels is passed to the routines, further
  speed estimations will be given in kilopixels/second on stdout. If
  you do not know this value, feed -1 as the number of crunched
  pixels.
 
  Example:
 
  @code
  int npix=0 ;
  printf("entering first number crunching module\n");
  eclipse_cpu_timing(START_CLOCK, npix);
  npix += number_crunching1();
  eclipse_cpu_timing(STOP_CLOCK, npix);
 
  printf("entering second number crunching module\n");
  eclipse_cpu_timing(START_CLOCK, npix);
  npix += number_crunching2();
  eclipse_cpu_timing(STOP_CLOCK, npix);
 
  printf("end results:\n");
  eclipse_cpu_timing(ACC_CLOCK, npix);
  @endcode
 */
/*--------------------------------------------------------------------------*/
double eclipse_cpu_timing(int mode, int npix) ;


#endif
