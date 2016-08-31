/*----------------------------------------------------------------------------*/
/**
   @file    speedtest.c
   @author  N. Devillard
   @date    June 4, 1997
   @version	$Revision: 1.19 $
   @brief   speed tests
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: speedtest.c,v 1.19 2002/11/20 14:44:59 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 14:44:59 $
	$Revision: 1.19 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define	START_CLOCK			0
#define STOP_CLOCK			1
#define ACC_CLOCK			2	

/*-----------------------------------------------------------------------------
                                Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "speed benchmark for image processing" ;
void cpu_speed_tests(int size) ;
 
/*-----------------------------------------------------------------------------
                                    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int				c ;
	int				size = 1024 ;

	if (argc>1) {
		if (!strcmp(argv[1], "-help")) usage(argv[0]) ;
	}

    while ((c = getopt(argc, argv, "L")) != EOF)
        switch(c) {
            /* Standard option: display license (not documented in usage)   */
			case 'L':
                eclipse_display_license() ;
                return 0 ;
        }

	eclipse_init();
	printf("CPU tests:\n") ;
	cpu_speed_tests(size) ;

	if (debug_active()) xmemory_status();
	return 0 ;

}

void cpu_speed_tests(int size)
{
	image_t		*	in ;
	image_t		*	out ;
	cube_t		*	fft ;
	image_stats	*	stats ;
	double			param[6] ;
	float			acc_time ;

	printf("test name       \t(s)\t\t(us)\t\t(kpix)/s\n") ;
	printf("\n----------------------------------------") ;
	printf("------------------------\n");
	
	/* generate an image to work with	*/
	printf("noise generate..") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, size*size) ;
	in = image_gen_random_uniform(size, size, -100.0, 100.0) ; 
	eclipse_cpu_timing(STOP_CLOCK, size*size) ;

	/* test : 3x3 filtering	*/
	printf("filtering 3x3...") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, size*size) ;
	out = image_filter3x3(in, image_filter_getkernel("mean3",0,0)) ;
	eclipse_cpu_timing(STOP_CLOCK, size*size) ;
	image_del(out) ;

	/* test : 5x5 filtering	*/
	printf("filtering 5x5...") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, size*size) ;
	out = image_filter5x5(in, image_filter_getkernel("mean5",0,0)) ;
	eclipse_cpu_timing(STOP_CLOCK, size*size) ;
	image_del(out) ;

	/* test : median filtering	*/
	printf("median filter...") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, size*size) ;
	out = image_filter_median(in) ;
	eclipse_cpu_timing(STOP_CLOCK, size*size) ;
	image_del(out) ;

	/* test : statistics */
	printf("statistics......") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, size*size) ;
	stats = image_getstats(in) ; 
	eclipse_cpu_timing(STOP_CLOCK, size*size) ;
	free(stats) ;

	/* test : fft	*/
	printf("fft.............") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, size*size) ;
	fft = image_fft(in, NULL, FFT_FORWARD) ;
	eclipse_cpu_timing(STOP_CLOCK, size*size) ;
	cube_del(fft) ;

	/* test : resampling	*/
	param[0] = param[4] = 2.0 ;
	param[1] = param[2] = param[3] = param[5] = 0.0 ;
	printf("zoom by 2.......") ;
	fflush(stdout) ;
	eclipse_cpu_timing(START_CLOCK, 4*size*size) ;
	out = image_warp_linear(in, param, "default") ;
	eclipse_cpu_timing(STOP_CLOCK, 4*size*size) ;
	image_del(out) ;

	image_del(in) ;

	acc_time = eclipse_cpu_timing(ACC_CLOCK, size*size) ;
	printf("CPU power: %4.2f\n", 10000.0/acc_time);
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
	exit(0) ;
}
