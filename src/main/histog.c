/*----------------------------------------------------------------------------*/
/**
   @file    histog.c
   @author  N. Devillard
   @date    Oct 1st, 1997
   @version	$Revision: 1.22 $
   @brief   Histogram plotting
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: histog.c,v 1.22 2002/11/20 10:38:55 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 10:38:55 $
	$Revision: 1.22 $
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

static void usage(char * pname) ;
static void plot_histogram(histogram * h, int ascii_plot);

/*-----------------------------------------------------------------------------
                                     Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    image_t     *   in ;
    char        *   name_i ;
    int             c ;
	int			    bins ;
	int			    ascii_plot = 1 ;
	int			    cumulative = 0 ;
	double		    d[2] ;
	pixelvalue	    min, max ;
    histogram   *   h ;

    if (argc<2) usage(argv[0]) ;

    /* Initialize */
	bins = -1 ;
	min = MIN_PIX_VALUE ;
	max = MAX_PIX_VALUE ;

    while ((c = getopt(argc, argv, "Lb:gci:")) != EOF)
        switch(c) {
            /* Standard option: display license (not documented in usage)   */
            case 'L':
                eclipse_display_license() ;
                return 0 ;
			case 'b':
		    	bins = (int)atoi(optarg) ;
		    	break ;
			case 'g':
                ascii_plot = 0 ;
	    		break ;
			case 'c':
		    	cumulative = 1 ;
			    break ;
			case 'i':
    			sscanf(optarg, "%lg %lg", d, d+1) ;
	    		min = (pixelvalue)d[0] ;
		    	max = (pixelvalue)d[1] ;
			    break ;
            default:
                usage(argv[0]) ;
                break ;
        }

	/* Initialize eclipse environment */
	eclipse_init();

    /* Get arguments    */
    if ((argc-optind) < 1) {
        e_error("missing argument: image name") ;
        return -1 ;
    }

    /* After the options, there must be at least an input name  */
    name_i = argv[optind] ;

    /* Load requested image */
    if ((in = image_load(name_i)) == NULL) {
        e_error("in loading image [%s]: aborting", name_i) ;
        return -1 ;
    }

    /* Set number of bins to the image size in X if not user-set */
    if (bins<0) bins = in->lx ;

    /* Compute histogram or cumulative histogram */
    if (cumulative==0) h = histogram_compute(in, bins, min, max);
    else h = histogram_compute_cumulative(in, bins, min, max);
    
    /* Delete input image, not needed past that point */
    image_del(in);
    if (h==NULL) {
        e_error("computing histogram: aborting");
        return -1 ;
    }

    /* Print out some info about the histogram */
    fprintf(stdout,
            "#\n"
            "# Histogram for image: %s\n"
            "# Bins=%d min=%g max=%g binsize=%g\n"
            "#\n",
            name_i, h->nbin, h->min, h->max, h->binsize);

    /* Plot histogram to stdout or gnuplot session */
    plot_histogram(h, ascii_plot);

    histogram_del(h);
    if (debug_active()) xmemory_status() ;
    return 0 ;
}

static void usage(char * pname)
{
    hello_world(pname, "image histogram") ;
    printf( "use : %s [options] in\n", pname) ;
	printf( "-g           for gnuplot output\n"
            "-c           for cumulative histogram\n"
            "-i 'min max' to declare a working interval\n"
            "-b <nbins>   to change number of bins\n"
            "\n\n");
    exit(0) ;
}

static void plot_histogram(histogram * h, int ascii_plot)
{
	gnuplot_ctrl *	handle ;

    if (ascii_plot) {
        histogram_dump(h, stdout);
    } else {
        handle = gnuplot_init();
        if (handle==NULL) {
            e_error("cannot open gnuplot session: aborting");
            return ;
        }
        if (h->nbin<500) {
            gnuplot_setstyle(handle, "boxes");
        } else {
            gnuplot_setstyle(handle, "impulses");
        }
        histogram_plot(h, handle);
        printf("press <ENTER> to continue\n");
        while (getchar()!='\n') {}
		gnuplot_close(handle) ;
    }
	return ;
}
