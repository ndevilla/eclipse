/*----------------------------------------------------------------------------*/
/**
   @file    distortion.c
   @author  Y. Jung
   @date    March 2003
   @version	$Revision: 1.4 $
   @brief   Distortion estimation
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: distortion.c,v 1.4 2003/05/15 07:44:30 yjung Exp $
	$Author: yjung $
	$Date: 2003/05/15 07:44:30 $
	$Revision: 1.4 $
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

#define OPT_ORIENTATION		1001
#define DIST_ARC_SATURATION 100000

/*-----------------------------------------------------------------------------
   								Function prototypes
 -----------------------------------------------------------------------------*/

static char prog_desc[] = "Distortion estimation routine" ;
static void usage(char * pname) ;

static int distortion_estimate(char *, char *, int) ;

/*-----------------------------------------------------------------------------
  								    Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	char		    name_i[FILENAMESZ+1],
				    name_o[FILENAMESZ+1] ;
    int             orient ;
	int			    c ;

	if (argc<2) usage(argv[0]) ;

    /* Initialize */
	name_i[0] = name_o[0] = 0 ;
    orient = 1 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"orientation", 0, 0, OPT_ORIENTATION},

            {"in",      1, 0, OPT_INPUT},
            {"out",     1, 0, OPT_OUTPUT},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhi:o:O:",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

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

		    /* Local options: input and output file names */
			case OPT_INPUT:
			case 'i':
			    strncpy(name_i, optarg, FILENAMESZ) ;
    			break ;
			case OPT_OUTPUT:
			case 'o':
	    		strncpy(name_o, get_rootname(optarg), FILENAMESZ) ;
		    	break ;
    		/* Processing options : begin and end planes	*/
			case OPT_ORIENTATION:
			case 'O':
                orient = (int)atoi(optarg) ;
		    	break ;
            default:
                usage(argv[0]) ;
              break ;
        }
	}

	/* Initialize eclipse environment */
	eclipse_init();

	if (name_i[0] == 0) {
		e_error("no input file name provided, use the -i option");
		return -1 ;
	}

	if (name_o[0] == 0) {
        e_error("no output file name provided, use the -o option") ;
        return -1 ;
	}
	
	e_comment(1, "input     : %s", name_i);
	e_comment(1, "output    : %s", name_o);

	if (distortion_estimate(name_i, name_o, orient) == -1) {
        e_error("failed in distortion estimation") ;
        return -1 ;
    }

	if (debug_active()) xmemory_status() ;
    return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ; 
    printf("use : %s [options] <in>\n", pname);
    printf("options are\n\n") ;
    printf(
        "\t--orientation or -O ori\n"
        "\t\t to specify the line orientaion. ori should be 1 for\n"
        "\t\t vertical lines and 0 for horizontal ones\n"
        "\n");
	exit(0) ;
}

static int distortion_estimate(
        char			*	name_i,
    	char			*	name_o,
        int                 orient)
{
    cube_t          *   cu ;
    image_t         *   lines ;
    double          *   dist_pol ;
    double          **  coeffs ;
    qfits_table     *   table ;
    qfits_col       *   col ;
    qfits_header    *   fh ;
    int                 arc_sat ;
    int                 i ;
    
    /* Initialize */
    arc_sat = DIST_ARC_SATURATION ;
    
    /* Read the inputs */
    if ((cu=cube_load_framelist(name_i)) == NULL) {
        e_error("cannot load input images") ;
        return -1 ;
    }

    /* Average the cube if more than one plane */
    if (cu->np > 1) {
        lines = cube_avg_linear(cu) ;
    } else if (cu->np == 1) {
        lines = image_copy(cu->plane[0]) ;
    } else {
        e_error("no plane in cube - abort") ;
        cube_del(cu) ;
        return -1 ;
    }
    cube_del(cu) ;
    
    /* Create the coeffs array */
    coeffs = malloc(3*sizeof(double*)) ;
    coeffs[0] = malloc(6*sizeof(double)) ;
    coeffs[1] = malloc(6*sizeof(double)) ;
    coeffs[2] = malloc(6*sizeof(double)) ;
        
    /* Check the orientation */
    switch (orient) {
        case 0:
            /* Take the symetry to have vertical lines */
            image_diagonal_symmetry(lines, 1) ;
            /* Compute the distortion */
            dist_pol = dist_engine(lines, 0, 0, lines->lx-1, lines->ly-1, 
                    arc_sat, NULL, NULL) ;
            if (dist_pol == NULL) {
                e_error("cannot compute distortion") ;
                for (i=0 ; i<3 ; i++) free(coeffs[i]) ; 
                free(coeffs) ;
                image_del(lines) ;
                return -1 ;
            }
            /* Copy the polynomial in coeffs - invert x and y !!! */
            coeffs[0][0] = 0 ;
            coeffs[0][1] = 1 ;
            coeffs[0][2] = 0 ;
            coeffs[0][3] = 1 ;
            coeffs[0][4] = 2 ;
            coeffs[0][5] = 0 ;
            coeffs[1][0] = 0 ;
            coeffs[1][1] = 0 ;
            coeffs[1][2] = 1 ;
            coeffs[1][3] = 1 ;
            coeffs[1][4] = 0 ;
            coeffs[1][5] = 2 ;
            coeffs[2][0] = dist_pol[0] ;
            coeffs[2][1] = dist_pol[2] ;
            coeffs[2][2] = dist_pol[1] ;
            coeffs[2][3] = dist_pol[3] ;
            coeffs[2][4] = dist_pol[5] ;
            coeffs[2][5] = dist_pol[4] ;
            free(dist_pol) ;
            break ;

        case 1:
            /* Compute the distortion */
            dist_pol = dist_engine(lines, 0, 0, lines->lx-1, lines->ly-1, 
                    arc_sat, NULL, NULL) ;
            if (dist_pol == NULL) {
                e_error("cannot compute distortion") ;
                for (i=0 ; i<3 ; i++) free(coeffs[i]) ; 
                free(coeffs) ;
                image_del(lines) ;
                return -1 ;
            }
            /* Copy the polynomial in coeffs */
            coeffs[0][0] = 0 ;
            coeffs[0][1] = 1 ;
            coeffs[0][2] = 0 ;
            coeffs[0][3] = 1 ;
            coeffs[0][4] = 2 ;
            coeffs[0][5] = 0 ;
            coeffs[1][0] = 0 ;
            coeffs[1][1] = 0 ;
            coeffs[1][2] = 1 ;
            coeffs[1][3] = 1 ;
            coeffs[1][4] = 0 ;
            coeffs[1][5] = 2 ;
            coeffs[2][0] = dist_pol[0] ;
            coeffs[2][1] = dist_pol[1] ;
            coeffs[2][2] = dist_pol[2] ;
            coeffs[2][3] = dist_pol[3] ;
            coeffs[2][4] = dist_pol[4] ;
            coeffs[2][5] = dist_pol[5] ;
            free(dist_pol) ;
            break ;
        
        default:
            e_error("Bad orientation specified") ;
            e_error("Use 0 for horizontal lines and 1 for vertical lines") ;
            for (i=0 ; i<3 ; i++) free(coeffs[i]) ; 
            free(coeffs) ;
            image_del(lines) ;
            return -1 ;
    }
    image_del(lines) ;

    /* Write out the polynomial in a TFITS file */
    table = qfits_table_new(name_o, QFITS_BINTABLE, -1, 3, 6) ;
    col = table->col ;
    for (i=0 ; i<table->nc ; i++) {
        qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, " ", " ",
                " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
        col++ ;
    }
    /* Update the columns labels */
    col = table->col ;
    sprintf(col->tlabel, "Degree_of_x") ;
    col++ ;
    sprintf(col->tlabel, "Degree_of_y") ;
    col++ ;
    sprintf(col->tlabel, "poly2d_coef") ;
    
    /* WRITE THE OUTPUT FITS TABLE */
    /* Create input header */
    fh = qfits_table_prim_header_default() ;
    
    /* Write the file on disk */
    if ((qfits_save_table_hdrdump((void**)coeffs, table, fh)) == -1) {
        e_error("cannot save table") ;
        qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
        return -1 ;
    }
    
    /* Free and return */
    for (i=0 ; i<3 ; i++) free(coeffs[i]) ; 
    free(coeffs) ;
    qfits_table_close(table) ;
    qfits_header_destroy(fh) ;
	return 0 ;
}

