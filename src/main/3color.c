/*----------------------------------------------------------------------------*/
/**
   @file    3color.c
   @author  Nicolas Devillard
   @date    Dec 13, 1997
   @version	$Revision: 1.16 $
   @brief   3 color image combination
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: 3color.c,v 1.16 2003/01/20 09:30:33 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/20 09:30:33 $
	$Revision: 1.16 $
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

#define OPT_COEFFICIENTS        1000

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "3 color combination" ;
 
/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char        	    inn[3][FILENAMESZ+1],
                	    outname[FILENAMESZ+1] ;
	image_t		    *	im[3] ;
	FILE		    *	outppm ;
	int				    pos_in,
					    pos_out ;
	pixelvalue		    minpix[3],
					    maxpix[3] ;
	double			    A[3],
					    B[3] ;
    unsigned char   *   buf ;
    double              coef[3] ;
    int                 ncoeffs ;
    int                 c ;
	int				    i, 
                        j ;

    /* Initialize */
    coef[0] = coef[1] = coef[2] = 1.0 ;
    
    if (argc<5) usage(argv[0]) ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"coeffs",  1, 0, OPT_COEFFICIENTS},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhc:",
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

            /* Local option: collapse direction */
            case OPT_COEFFICIENTS:
            case 'c':
                ncoeffs = sscanf(optarg, "%lg %lg %lg", coef, coef+1, coef+2) ;
                if (ncoeffs != 3) {
                    e_error("-c/--coeffs expects 3 values, got %d", ncoeffs) ;
                    return -1 ;
                }
                break;
            default:
                usage(argv[0]) ;
                break ;
        }
    }
   
	/* Initialize eclipse environment */
	eclipse_init();

    /* Get arguments    */
    if ((argc-optind) < 4) {
        e_error("missing arguments.\n") ;
        return -1 ;
    }
    
	strncpy(inn[0], argv[optind++], FILENAMESZ) ;
	strncpy(inn[1], argv[optind++], FILENAMESZ) ;
	strncpy(inn[2], argv[optind++], FILENAMESZ) ;
	strncpy(outname, argv[optind++], FILENAMESZ) ;

    /* Test coefficients */
    if ((coef[0]<0) || (coef[0]>1) || (coef[1]<0) || (coef[1]>1) ||
        (coef[2]<0) || (coef[2]>1)) {
        e_error("Coefficients should be between 0 and 1");
        return -1;
    }
    
	/* load all images */
	for (i=0 ; i<3 ; i++) {
		im[i] = image_load(inn[i]) ;
		if (im[i]==NULL) {
			e_error("cannot load image [%s]", inn[i]);
			return -1 ;
		}
	}

	if ((im[0]->lx != im[1]->lx) ||
		(im[0]->lx != im[2]->lx) ||
		(im[0]->ly != im[1]->ly) ||
		(im[0]->ly != im[2]->ly)) {
		e_error("input images have incompatible sizes\n"
				"red   is [%d %d]\n"
				"green is [%d %d]\n"
				"blue  is [%d %d]",
				im[0]->lx, im[0]->ly,
				im[1]->lx, im[1]->ly,
				im[2]->lx, im[2]->ly) ;
		image_del(im[0]) ;
		image_del(im[1]) ;
		image_del(im[2]) ;
		return -1 ;
	}

	printf("*** channels\n") ;
	printf("red               : %s (coef : %g)\n", inn[0], coef[0]) ;
	printf("green             : %s (coef : %g)\n", inn[1], coef[1]) ;
	printf("blue              : %s (coef : %g)\n", inn[2], coef[2]) ;
	printf("\n") ;
	printf("24 bit PPM output : %s\n", outname) ;

	/* Find minimum and maximum pixel values for the 3 images */
	maxpix[0] = minpix[0] = im[0]->data[0] ;
	maxpix[1] = minpix[1] = im[1]->data[0] ;
	maxpix[2] = minpix[2] = im[2]->data[0] ;

	for (i=0 ; i<(im[0]->lx * im[0]->ly) ; i++) {
		for (j=0 ; j<3 ; j++) {
			if (im[j]->data[i] < minpix[j])
				minpix[j] = im[j]->data[i] ;
			if (im[j]->data[i] > maxpix[j])
				maxpix[j] = im[j]->data[i] ;
		}
	}

	for (i=0 ; i<3 ; i++) {
		A[i] = coef[i] * 255.0 / (maxpix[i] - minpix[i]) ;
		B[i] = - A[i] * minpix[i] ;
	}

	
	outppm = fopen(outname, "w") ;
	if (outppm == NULL) {
		e_error("cannot open file [%s] for output: aborting", outname);
		image_del(im[0]) ;
		image_del(im[1]) ;
		image_del(im[2]) ;
	}

	/* Output an image in PPM P6 format */

	/* header information */
	fprintf(outppm, "P6\n") ;
	fprintf(outppm, "%d %d\n", im[0]->lx, im[0]->ly) ;
	fprintf(outppm, "255\n") ;

	/* allocate output buffer */
	buf = (unsigned char *)malloc(3 * im[0]->lx * im[0]->ly) ;

	/* pixel values */
	for (j=im[0]->ly-1 ; j>=0 ; j--) {
		for (i=0 ; i<im[0]->lx ; i++) {
			pos_in = i+j*im[0]->lx ;
			pos_out = i+(im[0]->ly-1-j)*im[0]->lx ;

			buf[3*pos_out]=
				(unsigned char)(0.5+A[0]*(double)im[0]->data[pos_in]+B[0]); 
			buf[3*pos_out+1]=
				(unsigned char)(0.5+A[1]*(double)im[1]->data[pos_in]+B[1]); 
			buf[3*pos_out+2]=
				(unsigned char)(0.5+A[2]*(double)im[2]->data[pos_in]+B[2]); 
		}
	}

	fwrite(buf, 3 * im[0]->lx * im[0]->ly, 1, outppm) ;
	free(buf) ;
	fclose(outppm) ;

	image_del(im[0]) ;
	image_del(im[1]) ;
	image_del(im[2]) ;

	if (debug_active()) xmemory_status();

    return 0 ;
}

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ;
	printf("\nuse: %s [options] <red> <green> <blue> <out>\n", pname) ;
    printf("\n") ;
    printf("Options:\n") ;
    printf("\t-c or --coeffs 'coeff1 coeff2 coeff3'\n") ;
    printf("\t\tto specify coefficients (between 0 and 1)\n");
    printf("\t\tto be applied on input images. Default is 1.0.\n") ;
    exit(0) ;
}
