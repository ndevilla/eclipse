

/*-------------------------------------------------------------------------*/
/**
   @file	chronos.c
   @author	N. Devillard
   @date	Feb 1998
   @version	$Revision
   @brief	Timing related routines

   This module offers some functionalities to measure the execution
   time of a program or program part.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: chronos.c,v 1.9 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.9 $
 */

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "chronos.h"

/*----------------------------------------------------------------------------
  							Function codes
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

double eclipse_cpu_timing(int mode, int npix)
{
	double 			total, 
					perpix ;
	static double 	acc_total = 0,
					acc_perpix = 0 ;
	static clock_t 	chrono ;				

	if (mode == START_CLOCK) {
		chrono = clock() ;
		return 0.0 ;
	}

	if (mode == STOP_CLOCK) {
		total = (double)(clock() - chrono) / (double)CLOCKS_PER_SEC ;
		acc_total += total ;
		if (npix >= 0) {
			perpix = (double)total * 1e6 / (double)npix ;
			acc_perpix += perpix ;
			printf("\ttotal %4.2f\tpixel %4.2f\t%6.2f\n",
					total,
					perpix,
					1000.0/perpix) ; 
		} else {
			printf("\telapsed %4.2f sec\n", total) ;
		}
		fflush(stdout) ;
		return 0.0 ;
	}

	if (mode == ACC_CLOCK) {
		printf("\n-------------------------------") ;
		printf("---------------------------------\n");
		if (npix >= 0) {
			printf("total: %4.2f (s)\tpixel %4.2f (us)\t%4.2f (kpix/s)\n",
					acc_total,
					acc_perpix,
					1000.0/acc_perpix) ;
		} else {
			printf("total elapsed time: %4.2f sec\n", acc_total) ;
		}
		return acc_total ;
	}

	return 0.0 ;
}
/* vim: set ts=4 et sw=4 tw=75 */
