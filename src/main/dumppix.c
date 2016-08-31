/*----------------------------------------------------------------------------*/
/**
   @file    dumppix.c
   @author  N. Devillard
   @date    Sept 29, 1997
   @version	$Revision: 1.17 $
   @brief   Pixel dump to stdout or gnuplot
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: dumppix.c,v 1.17 2002/11/19 13:34:52 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 13:34:52 $
	$Revision: 1.17 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
                                Function prototypes
 -----------------------------------------------------------------------------*/

static void dump_pixels(image_t *, int, int, int) ;
static void usage(char *pname) ;
static char prog_desc[] = "dump pixels to stdout" ;

/*-----------------------------------------------------------------------------
                                    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    cube_t	*	in ;
    char        inname[FILENAMESZ+1];
    int         c ;
	int			samples ;
	int			x, y ;
	int			i ;

    if (argc<2) usage(argv[0]) ;

    /* Initialize */
	x = -1 ;
	y = -1 ;
	samples = 1 ;

    /* Command line parsing by getopt(3c) */
    while ((c = getopt(argc, argv, "Ls:x:y:")) != EOF) {
        switch(c) {
            /* Standard option: display license (not documented in usage)   */
            case 'L':
                eclipse_display_license() ;
                return 0 ;
			case 's':
			    samples = (int)atoi(optarg) ;
			    break ;
			case 'x':
			    x = (int)atoi(optarg);
			    if (y>0) {
			    	e_error("only one of -x or -y at a time");
			    	return -1 ;
			    }
			    break ;
			case 'y':
                y = (int)atoi(optarg);
			    if (x>0) {
				    e_error("only one of -x or -y at a time");
				    return -1 ;
		    	}
		    	break ;
            default:
                usage(argv[0]) ;
                break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

    /*
	 * Get arguments:
     * argc - optind is the number of remaining arguments argv[optind] is the 
     * first argument which is no option nor option argument.
	 */

    if ((argc-optind) < 1) {
        e_error("missing arguments") ;
        return -1 ;
    }

    /* After the options, there must be at least an input name  */
    strncpy(inname, argv[optind++], FILENAMESZ) ;

    /* Load requested cube */
    if ((in = cube_load(inname)) == NULL) {
        e_error("in loading cube [%s]: aborting", inname) ;
        return -1 ;
    }

	for (i=0 ; i<in->np ; i++) {
		if (in->np > 1) printf("# Plane %d\n", i+1) ;
		dump_pixels(in->plane[i], samples, x, y) ;
	}
    cube_del(in) ;

    if (debug_active()) xmemory_status() ;
    return 0 ;
}

static void usage(char * pname)
{
    hello_world(pname, prog_desc) ;
	printf(
"use : %s [options] in\n"
"options are :\n"
"\t[-x col]    extract 1 column only (x in 1..NAXIS1)\n"
"\t[-y lin]    extract 1 line only   (y in 1..NAXIS2)\n"
"\t[-s rate]   change the sample rate\n"
"\n\n", pname) ;
    exit(0) ;
}

static void dump_pixels(
        image_t *   in,
	    int			samples,
	    int			x,
	    int			y)
{
    int i, j ;

	/* One column only */
	if (x>0) {
		i=x-1 ;
		for (j=0 ; j<in->ly ; j+=samples) {
			printf("%d %d %g\n", i+1, j+1, in->data[i+j*in->lx]);
		}
		return ;
	}

	/* One line only */
	if (y>0) {
		j=y-1 ;
		for (i=0 ; i<in->lx ; i+=samples) {
			printf("%d %d %g\n", i+1, j+1, in->data[i+j*in->lx]);
		}
		return ;
	}

	/* The whole image */
	for (j=0 ; j<in->ly ; j+=samples) {
		for (i=0 ; i<in->lx ; i+=samples) {
			printf("%d %d %g\n", i+1, j+1, in->data[i+j*in->lx]) ;
		}
		printf("\n") ;
	}
	return ;
}
