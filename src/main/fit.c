/*----------------------------------------------------------------------------*/
/**
   @file    fit.c
   @author  Y. Jung
   @date    Feb 2003
   @version	$Revision: 1.3 $
   @brief   Fit a list of points
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: fit.c,v 1.3 2003/04/14 08:57:06 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/14 08:57:06 $
	$Revision: 1.3 $
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

/*-----------------------------------------------------------------------------
   								Functions prototypes
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "fit a list of points" ;

/*-----------------------------------------------------------------------------
  								    Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    double3         *   pts ;
    double          *   res ;
    double3         *   res4plot ;
    gnuplot_ctrl    *   handle ;
    
	char		        inname[FILENAMESZ+1] ;
	int			        c ;
    int                 i ;

    /* Initialize */
	inname[0] = (char)0 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lh",
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
            /* Local options */
            default:
                usage(argv[0]) ;
             break ;
        }
    }

	/* Initialize eclipse environment */
	eclipse_init();

    /* Get arguments    */
    if ((argc - optind) < 1) usage(argv[0]);
	strncpy(inname, argv[optind++], FILENAMESZ) ;

    /* Read the input list of points */
    pts = double3_read(inname) ;
    
    /* Apply the fit */
    res = fit_1d_gauss(pts) ;
    e_comment(0, "Result: a=%g  mu=%g  sigma=%g\n", res[0], res[1], res[2]) ;
    
    /* Generate the gaussian result */
    res4plot = double3_new(pts->n) ; 
    for (i=0 ; i<res4plot->n ; i++) {
        res4plot->x[i] = pts->x[i] ;
        res4plot->y[i] = res[0]*exp(-0.5*((pts->x[i]-res[1])/res[2])*
                ((pts->x[i]-res[1])/res[2])) ;
    }

    /* Display the result */
    handle = gnuplot_init() ;
    gnuplot_setstyle(handle, "points") ;
    gnuplot_set_xlabel(handle, "x") ;
    gnuplot_set_ylabel(handle, "y") ;
    gnuplot_plot_xy(handle,
                        pts->x,
                        pts->y,
                        pts->n,
                        "Function to fit") ;
    printf("press enter to continue\n") ;
    while (getchar() != '\n') {}
    gnuplot_setstyle(handle, "lines") ;
    gnuplot_plot_xy(handle,
                        res4plot->x,
                        res4plot->y,
                        res4plot->n,
                        "Function fitted") ;
    printf("press enter to continue\n") ;
    while (getchar() != '\n') {}
    gnuplot_close(handle) ;

    /* Free  */
    double3_del(pts) ;
    double3_del(res4plot) ;
    free(res) ;
    
	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s <in>\n", pname);
	exit(0) ;
}
