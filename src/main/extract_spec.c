/*----------------------------------------------------------------------------*/
/**
   @file    extract_spec.c
   @author  Y. Jung
   @date    Aug, 5th 2002
   @version	$Revision: 1.5 $
   @brief   Extract a spectrum
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: extract_spec.c,v 1.5 2003/12/02 12:46:00 yjung Exp $
	$Author: yjung $
	$Date: 2003/12/02 12:46:00 $
	$Revision: 1.5 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   	  							Defines
 -----------------------------------------------------------------------------*/

#define OPT_SPEC_POS        1000
#define OPT_SPEC_WIDTH      1001
#define OPT_SKY_DIST        1002
#define OPT_SKY_WIDTH       1003
#define OPT_NOGRAPH         1004

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "Spectrum extraction" ;
static int extract_spectrum_engine(char	*, char	*, int, int, int, int, int) ;
static int extract_spectrum_write(char *, int, pixelvalue **) ;

/*-----------------------------------------------------------------------------
  								    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int			c ;
	char		name_i[FILENAMESZ+1],
				name_o[FILENAMESZ+1] ;
    int         spec_width ;
    int         sky_dist ;
    int         sky_width ;
    int         spec_pos ;
    int         no_graph ;

	if (argc<2) usage(argv[0]) ;

    /* Initialize */
	name_i[0] = name_o[0] = 0 ; 
    spec_pos = -1 ;
    no_graph = 0 ;
    sky_width = 30 ;
    sky_dist = 10 ;
    spec_width = 10 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"spec_p", 1, 0, OPT_SPEC_POS},
            {"spec_w", 1, 0, OPT_SPEC_WIDTH},
            {"sky_d",  1, 0, OPT_SKY_DIST},
            {"sky_w",  1, 0, OPT_SKY_WIDTH},
            {"nograph",0, 0, OPT_NOGRAPH},

            {"in",      1, 0, OPT_INPUT},
            {"out",     1, 0, OPT_OUTPUT},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhi:o:p:w:d:W:n",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c)
        {

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

		/* Local options: input and output file names */

			case OPT_INPUT:
			case 'i':
			strncpy(name_i, optarg, FILENAMESZ) ;
			break ;

			case OPT_OUTPUT:
			case 'o':
			strncpy(name_o, get_rootname(optarg), FILENAMESZ) ;
			break ;

		/* Local options: extraction parameters */

			case OPT_SPEC_POS:
			case 'p':
            spec_pos = (int)atoi(optarg) ;
            break ;

			case OPT_SPEC_WIDTH:
			case 'w':
            spec_width = (int)atoi(optarg) ;
            break ;
            
			case OPT_SKY_DIST:
			case 'd':
            sky_dist = (int)atoi(optarg) ;
            break ;
			
            case OPT_SKY_WIDTH:
			case 'W':
            sky_width = (int)atoi(optarg) ;
            break ;
			
            case OPT_NOGRAPH:
			case 'n':
            no_graph = 1 ;
            break ;
            
            default:
            usage(argv[0]) ;
            break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

    /* Test if an input file has been specified */
	if (name_i[0] == 0) {
		e_error("no input file name provided, use the -i option");
		return -1 ;
	}

	if (name_o[0] == 0) {
		strncpy(name_o, get_basename(get_rootname(name_i)), FILENAMESZ);
	}
	
	e_comment(1, "input     : %s", name_i);
	e_comment(1, "output    : %s", name_o);

	extract_spectrum_engine(name_i, name_o, spec_pos, spec_width, sky_dist,
            sky_width, no_graph) ;
	
    if (debug_active()) xmemory_status() ;
    return 0 ;
}

/*-----------------------------------------------------------------------------
  					        Functions code
 -----------------------------------------------------------------------------*/
static void usage(char *pname)
{
	hello_world(pname, prog_desc) ; 
	printf("use: %s -i infile [-o outfile] [options]\n", pname) ;
	printf(
		"Options are:\n"
		"\n");
	printf(
		"\t--spec_w or -w\n"
        "\t\tto specify the spectrum width.\n"
		"\t--spec_p or -p\n"
        "\t\tto specify the spectrum position in pixels.\n"
		"\t--sky_d or -d\n"
        "\t\tto specify the distance to the sky part.\n"
		"\t--sky_w or -W\n"
        "\t\tto specify the size of the sky window\n"
		"\t--nograph or -n\n"
        "\t\tnot to have any display.\n"
		"\n");
	exit(0) ;
}

static int extract_spectrum_engine(
        char			*	name_i,
        char			*	name_o,
        int                 spec_pos,
        int                 spec_width,
        int                 sky_dist,
        int                 sky_width,
        int                 nograph)
{
    image_t         *   inimage;
    double3         *   det_pos ;
    pixelvalue      *   array[2] ;
    double          *   plot_x,
                    *   plot_y ;
    image_t         *   extr_line ;
    int                 low_side, up_side ;
    int                 spec_length, 
                        slit_length ;
    int                 sky_pos[4] ;
    pixelvalue          sky_estim ;
    double              median_1 ;
    double              median_2 ;
    char                outtablename[1024] ;

    int                 i ;

    /* Load the input image */
    if ((inimage = image_load(name_i)) == NULL) {
        e_error("in loading image [%s] - aborting", name_i) ;
        return -1 ;
    }

    /* If no spectrum position specified, try to detect it */
    if (spec_pos<1) {
        if ((det_pos = find_brightest_spectrum_1d(inimage,
                        0, NO_SHADOW_SPECTRUM, 10)) == NULL) { 
            e_error("Cannot detect the spectrum - specify its position") ;
            image_del(inimage) ;
            return -1 ;
        }
        spec_pos = (int)det_pos->y[0] ;
        double3_del(det_pos) ;
    }

    /* Verify the validity of spec_pos */
    spec_length = inimage->lx ;
    slit_length = inimage->ly ;
    if (spec_pos <= 1 || spec_pos >= slit_length) {
        e_error("invalid spectrum position: [%d] - aborting",spec_pos) ;
        image_del(inimage) ;
        return -1 ;
    }
    e_comment(1, "Spectrum position:  %d\n", spec_pos) ;

    /* Allocate the ouput arrays */
    array[0] = malloc(spec_length*sizeof(pixelvalue)) ;
    array[1] = malloc(spec_length*sizeof(pixelvalue)) ;

    /* No background subtraction */
    if (sky_width == 0) e_comment(1, "No sky background subtraction") ;

    /* Spectrum position */
    low_side = (int)(spec_pos - (spec_width-1)/2) ;
    up_side = low_side + spec_width ;
    if (low_side < 1) low_side = 1 ;
    if (up_side > slit_length) up_side = slit_length ;
    spec_width = up_side - low_side ;

    /* Positions of the residual sky */
    sky_pos[1] = (int)(spec_pos - sky_dist) ;
    sky_pos[0] = sky_pos[1] - sky_width ;
    sky_pos[2] = (int)(spec_pos + sky_dist) ;
    sky_pos[3] = sky_pos[2] + sky_width ;

    /* Extract the spectrum and get rid of the residual sky */
    for (i=0 ; i<spec_length ; i++) {
        if ((extr_line = image_getvig(inimage,
                        i+1,
                        low_side,
                        i+1,
                        up_side)) == NULL) {
            e_error("error in line extraction - aborting") ;
            free(array[0]) ;
            free(array[1]) ;
            image_del(inimage) ;
            return -1 ;
        }

        if ((sky_pos[0] < 1) && (sky_width !=0)) {
            median_1 = image_getmedian_vig(inimage,
                                                i+1,
                                                sky_pos[2],
                                                i+1,
                                                sky_pos[3]) ;
            sky_estim = median_1 ;
        } else if ((sky_pos[3] > slit_length) && (sky_width !=0)) {
            median_1 = image_getmedian_vig(inimage,
                                                i+1,
                                                sky_pos[0],
                                                i+1,
                                                sky_pos[1]) ;
            sky_estim = median_1 ;
        } else if (sky_width !=0) {
            median_1 = image_getmedian_vig(inimage,
                                                i+1,
                                                sky_pos[0],
                                                i+1,
                                                sky_pos[1]) ;
            median_2 = image_getmedian_vig(inimage,
                                                i+1,
                                                sky_pos[2],
                                                i+1,
                                                sky_pos[3]) ;
            sky_estim=(median_1 + median_2)/2 ;
        } else {
            sky_estim = 0 ;
        }

        array[1][i]=(pixelvalue)image_getsumpix(extr_line) ;
        array[1][i] = (pixelvalue)array[1][i] - spec_width*sky_estim ;
        image_del(extr_line) ;
    }
    image_del(inimage) ;

    /* X coordinate */
    for (i=0 ; i<spec_length ; i++) array[0][i] = (pixelvalue)(i+1) ;

    /* Write the output table */
    sprintf(outtablename, "%s.tfits", name_o) ;
    if (extract_spectrum_write(outtablename,
                            spec_length,
                            (pixelvalue**)array) == -1) {
        e_error("cannot write output FITS table") ;
    }

    /* Display */
    if (nograph == 0) {
        plot_x = malloc(spec_length * sizeof(double));
        plot_y = malloc(spec_length * sizeof(double));

        for (i=0 ; i<spec_length ; i++) {
            plot_x[i] = array[0][i];
            plot_y[i] = array[1][i];
        }
        gnuplot_plot_once("Extracted spectrum",
                          "lines",
                          "pixels",
                          "spectrum",
                          plot_x,
                          plot_y,
                          spec_length);
        free(plot_x) ;
        free(plot_y) ;
    }

    /* Free and return */
    free(array[0]) ;
    free(array[1]) ;
    return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Write the FITS output table
  @param    name        name of the table to write
  @param    nb_lines    nb of lines
  @param    array       pixelvalue array (2 col.)
  @return   int (-1 in error case)  
 */
/*--------------------------------------------------------------------------*/
static int extract_spectrum_write(
        char        *   name,
        int             nb_lines,
        pixelvalue  **  array)
{
    qfits_header    *   fh ;
    qfits_table     *   table ;
    qfits_col       *   col ;
    double          *   array_double[2] ;
    int                 i ;

    /* Write the output qfits_table table (informations) */
    table = qfits_table_new(name, QFITS_BINTABLE, -1, 2, nb_lines) ;
    col = table->col ;
    for (i=0 ; i<table->nc ; i++) {
        qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, " ", " ",
                " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
        col++ ;
    }
    col = table->col ;
    sprintf(col->tlabel, "Pixel") ;
    col++ ;
    sprintf(col->tlabel, "Extracted") ;

    /* Convert pixelvalue array to double array */
    array_double[0] = malloc(nb_lines*sizeof(double)) ;
    array_double[1] = malloc(nb_lines*sizeof(double)) ;
    for (i=0 ; i<nb_lines ; i++) {
        array_double[0][i] = (double)array[0][i] ;
        array_double[1][i] = (double)array[1][i] ;
    }

    /* WRITE THE OUTPUT FILE */
    /* Read the input header */
    if ((fh = qfits_table_prim_header_default()) == NULL) {
        e_error("in writing the output fits file") ;
        qfits_table_close(table) ;
        free(array_double[0]) ;
        free(array_double[1]) ;
        return -1 ;
    }

    /* Write the file on disk */
    if (qfits_save_table_hdrdump((void**)array_double, table, fh) == -1) {
        e_error("cannot write file: %s", name) ;
        qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
        free(array_double[0]) ;
        free(array_double[1]) ;
        return -1 ;
    }
    qfits_table_close(table) ;
    qfits_header_destroy(fh) ;
    free(array_double[0]) ;
    free(array_double[1]) ;

    e_comment(0, "File [%s] produced", name) ;
    return 0 ;
}

