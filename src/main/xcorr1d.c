/*----------------------------------------------------------------------------*/
/**
   @file    xcorr1d.c
   @author  Yves Jung
   @date    July 2002
   @version	$Revision: 1.4 $
   @brief   Compute offsets between 1d signals
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: xcorr1d.c,v 1.4 2002/11/21 10:03:47 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/21 10:03:47 $
	$Revision: 1.4 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define XCORR_WIDTH_PIX             50

/*-----------------------------------------------------------------------------
   						    Function prototype
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static int xcorr1d(char * input1, char * input2) ;
static char prog_desc[] = "compute offsets between 1d signals";

/*-----------------------------------------------------------------------------
   									Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char    *   in1,
            *   in2 ;
    int         err ;
	int         c ;

    if (argc<3) usage(argv[0]);
    
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

            default:
                usage(argv[0]) ;
                break ;
        }
    }

	if ((argc-optind)<2) {
		e_error("missing argument: input file name(s)") ;
		return -1 ;
	}
    in1 = argv[optind] ;
    in2 = argv[optind+1] ;
    
	/* Initialize eclipse environment */
	eclipse_init() ;

    err = xcorr1d(in1, in2);

	if (debug_active()) xmemory_status() ;
	return err ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ; 
	printf("use : %s input_1 input_2\n", pname);
    exit(0) ;
}

static int xcorr1d(
        char    *   in1, 
        char    *   in2)
{
    image_t     *   ima1 ;
    image_t     *   ima2 ;
    pixelvalue  *   function1 ;
    pixelvalue  *   function2 ;
    int             size1, 
                    size2 ;
    double          xcorr ;
    double          delta ;
    int             i ;

    /* Verify the input files validity */
    if (is_fits_file(in1)) {
        if (!is_fits_file(in2)) {
            e_error("Input files have to be both FITS or ASCII") ;
            return -1 ;
        }
        /* Load input FITS files */
        ima1 = image_load(in1) ;
        ima2 = image_load(in2) ;
        if ((ima1 == NULL) || (ima2 == NULL)) {
            e_error("cannot load input FITS files") ;
            if (ima1 != NULL) image_del(ima1) ;
            if (ima2 != NULL) image_del(ima2) ;
        }
        /* Check the sizes */
        if ((ima1->lx != ima2->lx) || (ima1->ly != 1) || (ima2->ly != 1)) { 
            e_error("Sizes are not compatible") ;
            image_del(ima1) ;
            image_del(ima2) ;
            return -1 ;
        }
        /* Build function1 and function2 */
        size1 = size2 = ima1->lx ;
        function1 = malloc(size1 * sizeof(pixelvalue)) ; 
        function2 = malloc(size2 * sizeof(pixelvalue)) ; 
        for (i=0 ; i<size1 ; i++) {
            function1[i] = ima1->data[i] ;
            function2[i] = ima2->data[i] ;
        }
        image_del(ima1) ;
        image_del(ima2) ;
    } else { 
        /* if (is_fits_file(in2)) { */
            e_error("Input files have to be both FITS or ASCII") ;
            return -1 ;
        /* } */
        /* Read the ASCII files and build function1 and function2 */
    }
    
    /* Apply the X-correlation for each different scale */
    xcorr = function1d_xcorrelate(function1, size1, function2, size2,
            110, &delta) ;

    /* Free and return */
    free(function1) ;
    free(function2) ;
    return 0 ;
}

