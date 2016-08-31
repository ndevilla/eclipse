
/*---------------------------------------------------------------------------
	
    File name 	:	ado_pl.c
	Author 		:	Nicolas Devillard
	Created on	:	Sept 2, 1996
	Description	:	Basic Adonis data reduction pipeline

 *--------------------------------------------------------------------------*/

/*

 $Id: ado_pl.c,v 1.3 2001/01/24 13:43:50 ndevilla Exp $
 $Author: ndevilla $
 $Date: 2001/01/24 13:43:50 $
 $Revision: 1.3 $

 */

/*---------------------------------------------------------------------------
 * 								Includes
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"
#include "ado_utils.h"

static void usage(char *pname) ;
static char prog_desc[] = "simple Adonis pipeline" ;

 
/*---------------------------------------------------------------------------
 * 								Main
 *--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	char		object[FILENAMESZ],
				sky[FILENAMESZ],
				flat[FILENAMESZ],
				bpm[FILENAMESZ],
				output[FILENAMESZ] ;
	int			c ;
	int			mode = CALIBRATION_UNKNOWN ;
	int			flag_avg = 0 ;

	object[0] = sky[0] = flat[0] = (char)0 ;
	(void)strcpy(bpm, "badpixmap") ;
	(void)strcpy(output, "calib.fits") ;
	
    /* 
	 * Command line parsing by getopt()
	 * See man page for getopt(3c) and implementation in ./pipes
	 */
    while ((c = getopt(argc, argv, "LvDM:a:p:o:s:f:b:1")) != EOF)
        switch(c)
        {
		/* Standard option: display license (not documented in usage)	*/
			case 'L':
			eclipse_display_license() ;
			return 0 ;

		/* Cube name for packed (chopped) cubes */
			case 'p':
			(void)strcpy(object, optarg) ;
			mode = CALIBRATION_PACKED ;
			break ;

		/* Cube name for object cubes	*/
			case 'a':
			(void)strcpy(object, optarg) ;
			mode = CALIBRATION_SEPARATED ;
			break ;

		/* Output name	*/
			case 'o':
			(void)strcpy(output, optarg) ;
			break ;

		/* Cube name for sky cube */
			case 's':
			(void)strcpy(sky, optarg) ;
			mode = CALIBRATION_SEPARATED ;
			break ;

		/* Cube name for flat cube */
			case 'f':
			(void)strcpy(flat, optarg) ;
			break ;

		/* Bad pixel map name	*/
			case 'b':
			(void)strcpy(bpm, optarg) ;
			break ;

		/* Averaging flag */
			case '1':
			flag_avg = 1 ;
			break ;

            default:
            usage(argv[0]) ;
            break ;
        }

	/* Initialize eclipse environment */
	eclipse_init();

	switch (mode) {
		case CALIBRATION_PACKED:
		reduce_packed_cube(object, flat, bpm, output, flag_avg) ;
		break ;

		case CALIBRATION_SEPARATED:
		reduce_separated_cube(object, sky, flat, bpm, output, flag_avg) ;
		break ;

		default:
		e_error("unkown calibration scheme: aborting") ;
		break ;
	}

	if (debug_active())
		xmemory_status() ;
	return(0) ;
}

/*
 * This function only gives the usage for the program
 */

static void
usage(char *pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s\n", pname) ;
	printf("\t[-p filename] to give a packed cube name\n") ;
	printf("\t[-a filename] to give an object cube name\n") ;
	printf("\t[-o filename] to give an output file name\n") ;
	printf("\t[-s filename] to give a sky cube name\n") ;
	printf("\t[-f filename] to give a gain map name\n") ;
	printf("\t[-b filename] to give a bad pixel map name") ;
	printf(" (default: badpixmap)\n") ;
	printf("\t[-1] to request an average object for each cycle\n") ;
	printf("\n\n") ;
	exit(0) ;
}
