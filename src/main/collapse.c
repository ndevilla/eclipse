/*----------------------------------------------------------------------------*/
/**
   @file    collapse.c
   @author  N. Devillard
   @date    Sept 1999
   @version	$Revision: 1.26 $
   @brief   Collapse an image to a single line
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: collapse.c,v 1.26 2002/11/19 10:50:23 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 10:50:23 $
	$Revision: 1.26 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define OPT_DIRECTION		1000
#define OPT_GNUPLOT			1001
#define OPT_MEDIAN			1002
#define OPT_REJBORDER		1003
#define OPT_OUTFITS			1010
#define OPT_UNCOLLAPSE		1011

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "image collapse along X or Y" ;
static image_t * uncollapse_line(image_t * line, int width);
static void plot_collapsed(image_t * col, char * name);
static void save_collapsed(image_t * in, char * name);
 
/*-----------------------------------------------------------------------------
                                Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    image_t *   image_in ;
	image_t	*	one_line ;
	image_t	*	uncol ;
    char    	name_i[FILENAMESZ+1];
    char      	name_o[FILENAMESZ+1];
    int        	c ;
	int			direction ;
	int			plot_output ;
	int			median_collapse ;
	int			rej_lo, rej_hi ;
	int			fits_output ;
	int			uncollapse_width ;

    /* Initialize */
	direction = 0 ;
	plot_output = 0 ;
	median_collapse = 0 ;
	rej_lo = 0 ;
	rej_hi = 0 ;
	fits_output = 0 ;
	uncollapse_width = 0 ;

    if (argc<2) usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"dir", 	1, 0, OPT_DIRECTION},
            {"gnuplot",	0, 0, OPT_GNUPLOT},
            {"median",	0, 0, OPT_MEDIAN},
            {"reject",	1, 0, OPT_REJBORDER},

			{"fits",    0, 0, OPT_OUTFITS},
			{"uncollapse",  1, 0, OPT_UNCOLLAPSE},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhd:fgmr:u:",
                        long_options,
                        &option_index) ;
        if (c==-1)
            break ;

        switch(c) {
            /* Standard option: display license undocumented option */
            case OPT_LICENSE:
            case 'L':
                eclipse_display_license() ;
                return 0 ;
            /* Standard option : help */
            case OPT_HELP:
            case 'h':
                usage(argv[0]) ;
                break ;
            /* Standard option: version */
            case OPT_VERSION:
                print_eclipse_version() ;
                return 0 ;

		    /* Local option: collapse direction */
			case OPT_DIRECTION:
            case 'd':
                if (optarg[0]=='y') direction=0 ;
                else if (optarg[0]=='x') direction=1 ;
                else {
                    e_error("undefined direction: [%s]", optarg);
                    return -1 ;
                }
                break;
            /* Local option: plot output */
			case OPT_GNUPLOT:
			case 'g':
                plot_output = 1 ;
                break ;
            /* Local option: median collapse */
			case OPT_MEDIAN:
			case 'm':
			    median_collapse = 1 ;
			    break ;
		    /* Local option: reject borders */
			case OPT_REJBORDER:
			case 'r':
			    sscanf(optarg, "%d %d", &rej_lo, &rej_hi);
			    break ;
		    /* Local option: save as a 1d FITS file */
			case OPT_OUTFITS:
			case 'f':
			    fits_output = 1 ;
			    break ;
		    /* Local option: uncollapse */
			case OPT_UNCOLLAPSE:
			case 'u':
			    uncollapse_width = atoi(optarg) ;
			    break ;
            default:
                usage(argv[0]) ;
                break ;
        }
    }

	/*
	 * Set FITS output to 0 when uncollapse is activated, since the output
	 * can only be a FITS file anyway.
	 */
	if (uncollapse_width > 0) fits_output=0 ;

	/* Initialize eclipse environment */
	eclipse_init();

    /* Retrieve arguments:
     * argc - optind is the number of remaining arguments
     * argv[optind] is the first argument which is no option
     * nor option argument.
     */
    if ((argc-optind) < 1) {
        e_error("missing arguments: input name") ;
        return(-1) ;
    }

    /* After the options, there must be at least an input name  */
    strncpy(name_i, argv[optind++], FILENAMESZ) ;
	if ((argc-optind)<1) {
		strncpy(name_o, get_rootname(name_i), FILENAMESZ-10);
		if (uncollapse_width) strcat(name_o, "_unc.fits");
		else strcat(name_o, "_line");
		if (fits_output) strcat(name_o, ".fits");
	} else {
		strncpy(name_o, argv[optind], FILENAMESZ);
	}

    /* Load requested image */
	e_comment(0, "loading image [%s]", name_i);
    if ((image_in = image_load(name_i)) == NULL) {
        e_error("in loading image [%s]: aborting", name_i) ;
        return -1 ;
    }

	if (uncollapse_width) {
		e_comment(0, "uncollapsing image...");
		uncol = uncollapse_line(image_in, uncollapse_width);
		image_del(image_in);
		if (uncol==NULL) {
			e_error("un-collapsing the input line");
			return -1 ;
		}
		e_comment(1, "created new image [%d x %d]", uncol->lx, uncol->ly);
		image_save_fits_hdrcopy(uncol, name_o, name_i, BPP_DEFAULT);
		e_comment(1, "saved as [%s]", name_o);
		image_del(uncol);
	} else {
		e_comment(0, "collapsing image");
		if (median_collapse) {
			one_line = image_collapse_median(image_in,
											 direction,
											 rej_lo,
											 rej_hi);
		} else {
			one_line = image_collapse(image_in, direction);
		}
		image_del(image_in) ;
        
        if (one_line==NULL) return -1 ;

		if (fits_output) {
			e_comment(0, "saving FITS result [%s]", name_o);
			image_save_fits_hdrcopy(one_line, name_o, name_i, BPP_DEFAULT);
		} else {
			e_comment(0, "saving ASCII result [%s]", name_o);
			save_collapsed(one_line, name_o);
		}

		if (plot_output) {
			e_comment(0, "plotting output");
			plot_collapsed(one_line, name_i);
		}
		image_del(one_line);
	}

    if (debug_active()) xmemory_status() ;
    return 0 ;
}

static void usage(char *pname)
{
    hello_world(pname, prog_desc) ;
	printf(
"use : %s [options] <in> [out]\n"
"options are:\n"
"\t-d y or --dir y                  vertical collapse (default)\n"
"\t-d x or --dir x                  horizontal collapse\n"
"\t-g or --gnuplot                  get gnuplot output\n"
"\t-m or --median                   median collapse\n"
"\t-r 'lo hi' or --reject 'lo hi'   reject lo and hi pixels\n"
"\n"
"\t-f or --fits                     save output to FITS instead of ASCII\n"
"\t-u or --uncollapse <width>       to reverse a collapse\n"
"\n\n", pname) ;
    exit(0) ;
}

static image_t * uncollapse_line(image_t * line, int width)
{
	image_t		*	uncol ;
	int				pos ;
	pixelvalue		val ;
	int				i, j ;

	if (line==NULL) return NULL ;
	if (width<1) return NULL ;

	if ((line->lx!=1) && (line->ly!=1)) {
		e_error("input is not a line but a 2d image: aborting");
		return NULL ;
	}

	/* Make two cases depending on the direction */
	if (line->lx==1) {
		/* One single column in input */
		uncol = image_new(width, line->ly);
		for (j=0 ; j<uncol->ly ; j++) {
			pos = j * uncol->lx ;
			val = line->data[j] ;
			for (i=0 ; i<uncol->lx ; i++) {
				uncol->data[pos+i] = val ;
			}
		}
	} else {
		/* One single line in input */
		uncol = image_new(line->lx, width);
		for (j=0 ; j<uncol->ly ; j++) {
			pos = j * uncol->lx ;
			memcpy(uncol->data+pos, line->data, line->lx*sizeof(pixelvalue));
		}
	}
	return uncol ;
}

static void plot_collapsed(image_t * col, char * name)
{
	gnuplot_ctrl	*	g ;
	double			*	list_x ;
	double			*	list_y ;
	int					npix ;
	int					i ;

	/* Store input line or column into x and y */
	npix = col->lx * col->ly ;
	list_x = malloc(npix * sizeof(double));
	list_y = malloc(npix * sizeof(double));
	for (i=0 ; i<npix ; i++) {
		list_x[i] = (double)i ;
		list_y[i] = (double)col->data[i];
	}

	g = gnuplot_init();
	gnuplot_setstyle(g, "impulses");
	gnuplot_set_xlabel(g, "pixel position");
	gnuplot_set_ylabel(g, "collapsed value");

	gnuplot_plot_xy(g, list_x, list_y, npix, name);
	free(list_x);
	free(list_y);

	printf("press ENTER to quit ");
	while (getchar()!='\n') {}
	return ;
}

static void save_collapsed(image_t * line, char * name)
{
	FILE	*	out ;
	int			i ;

	out = fopen(name, "w");
	if (out==NULL) {
		e_error("cannot create file [%s]: aborting save", name);
		return ;
	}
	fprintf(out, "#\n");
	fprintf(out, "# PixelPos\tSum\n");
	fprintf(out, "#\n");
	for (i=0 ; i<(line->lx * line->ly) ; i++) {
		fprintf(out, "%d\t%g\n", i+1, (double)line->data[i]);
	}
	fclose(out);
	return ;
}
