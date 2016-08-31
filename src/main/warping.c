/*----------------------------------------------------------------------------*/
/**
   @file    warping.c
   @author  Nicolas Devillard
   @date    10 Jun 1999
   @version	$Revision: 1.24 $
   @brief   applies an analytical global deformation to an image
   see "Digital Image Warping" from G. Wolberg
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: warping.c,v 1.24 2002/11/21 09:48:54 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/21 09:48:54 $
	$Revision: 1.24 $
*/

/*-----------------------------------------------------------------------------
 								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
 								Define
 -----------------------------------------------------------------------------*/

#define OPT_TRANSLATE		1010
#define OPT_ROTATE			1020
#define OPT_SCALE			1030
#define OPT_LINEARTRANS		1040
#define	OPT_CORRECTARC		1050
#define OPT_CORRECTSTTR		1060

#define OPT_POLY_U			2010
#define OPT_POLY_V			2020

#define OPT_KERNEL_SET		3000
#define OPT_KERNEL_WRITE	3010

#define OPT_SUBSAMPLE		3020

/*-----------------------------------------------------------------------------
 								Functions prototype
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "resample a frame according to a geometrical transf.";

/*-----------------------------------------------------------------------------
 								    Main    
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	image_t	*	in ;
	image_t	*	warped ;
    char    *   name_i ;
    char        name_o[FILENAMESZ];
	int			c ;
	int			nargs ;

	/* Following are for integer/linear transforms */
	double		Tu, Tv ;		/* translation */
	double		theta ;			/* rotation angle */
	int			itheta ;		/* integer rotation angle */
	double		sf ;			/* scale factor */
	double		lineartrans[6];	/* general transform */

	/* Following are for polynomial transforms */
	poly2d	*	poly_u;			/* polynomial definition */
	poly2d	*	poly_v;			/* polynomial definition */
	
	/* Following are for the interpolation kernel settings */
	char		kernel_name[32] ;
	int			kernel_write ;

	/* Various boolean flags */
	int			tr_polynomial ;
	int			tr_linear ;
	int			tr_integer ;
	int			tr_translate ;
	int			tr_rotate ;
	int			tr_scale ;
	int			tr_subsample ;

	if (argc<2) usage(argv[0]) ;

    /* Initialize */
	kernel_write = 0 ;
	strncpy(kernel_name, "default", 31);
	tr_polynomial = 0 ;
	tr_linear     = 0 ;
	tr_integer    = 0 ;
	tr_translate  = 0 ;
	tr_rotate     = 0 ;
	tr_scale      = 0 ;
	tr_subsample  = 0 ;

	Tu    = 0.00 ;
	Tv    = 0.00 ;
	theta = 0.00 ;
	sf    = 1.00 ;

	poly_u = NULL ;
	poly_v = NULL ;
	
    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 	0, 0, OPT_LICENSE},
            {"help",    	0, 0, OPT_HELP},
            {"version", 	0, 0, OPT_VERSION},

            {"translate",	1, 0, OPT_TRANSLATE},
            {"rotate",		1, 0, OPT_ROTATE},
			{"scale",		1, 0, OPT_SCALE},
            {"transform",	1, 0, OPT_LINEARTRANS},

            {"polyu",		1, 0, OPT_POLY_U},
            {"polyv",		1, 0, OPT_POLY_V},

			{"arcfile",		1, 0, OPT_CORRECTARC},
			{"sttrfile", 	1, 0, OPT_CORRECTSTTR},
			
			{"kernel",		1, 0, OPT_KERNEL_SET},
            {"write",		0, 0, OPT_KERNEL_WRITE},
            {"sub",			0, 0, OPT_SUBSAMPLE},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
						"A:LS:T:d:hk:r:s:t:u:v:w",
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
            /* For integer and linear */
			case OPT_TRANSLATE:
			case 't':
                nargs = sscanf(optarg, "%lg %lg", &Tu, &Tv);
                if (nargs!=2) {
                    e_error("-t/--translate expects 2 args, got %d", nargs);
                    return -1 ;
                }
                tr_linear = 1 ;
                tr_translate = 1 ;
                break ;
			case OPT_ROTATE:
			case 'r':
                nargs = sscanf(optarg, "%lg", &theta);
                if (nargs!=1) {
                    e_error("-r/--rotate expects 1 arg, got %d", nargs);
                    return -1 ;
                }
                tr_linear = 1 ;
                tr_rotate = 1 ;
                break ;
			case OPT_SCALE:
			case 's':
                nargs = sscanf(optarg, "%lg", &sf);
                if (nargs!=1) {
                    e_error("-s/--scale expects 1 arg, got %d", nargs);
                    return -1 ;
                }
                tr_linear = 1 ;
                tr_scale  = 1 ;
                break ;
			case OPT_LINEARTRANS:
			case 'T':
                nargs = sscanf(optarg, "%lg %lg %lg %lg %lg %lg",
                        lineartrans,   lineartrans+1, lineartrans+2,
                        lineartrans+3, lineartrans+4, lineartrans+5);
                if (nargs!=6) {
                    e_error("-T/--transform expects 6 values, got %d", nargs);
                    return -1 ;
                }
                tr_linear = 1 ;
                break ;
			case OPT_SUBSAMPLE:
                tr_subsample=1 ;
                break ;
            /* For polynomial */
			case OPT_CORRECTARC:
			case 'A':
                poly_u = read_poly2d_from_table(optarg) ;
                if (poly_u == NULL) {
                    e_error("cannot read 2D poly from arc table") ;
                    return -1 ;
                }
                if (poly_v==NULL) {
                    poly_v=poly2d_build_from_string("0 1 1.0") ;
                }
                tr_polynomial = 1 ;
                break ;
			case OPT_CORRECTSTTR:
			case 'S':
                poly_v = read_poly2d_from_table(optarg) ;
                if (poly_v == NULL) {
                    e_error("cannot read 2D poly from startrace table") ;
                    return -1 ;
                }
                if (poly_u==NULL) {
                    poly_u = poly2d_build_from_string("1 0 1.0") ;
                }
                tr_polynomial = 1 ;
                break ;
			case OPT_POLY_U:
			case 'u':
                poly_u = poly2d_build_from_string(optarg);
                if (poly_u==NULL) {
                    e_error("building polynomial Pu(x,y) from cmd-line");
                    return -1 ;
                }
                if (poly_v==NULL) {
                    poly_v=poly2d_build_from_string("0 1 1.0") ;
                }
                tr_polynomial = 1 ;
                break ;
			case OPT_POLY_V:
			case 'v':
                poly_v = poly2d_build_from_string(optarg);
                if (poly_v==NULL) {
                    e_error("building polynomial Pv(x,y) from cmd-line");
                    return -1 ;
                }
                if (poly_u==NULL) {
                    poly_u = poly2d_build_from_string("1 0 1.0") ;
                }
                tr_polynomial = 1 ;
                break ;
            /* Kernel set/write */
			case OPT_KERNEL_SET:
			case 'k':
                strncpy(kernel_name, optarg, 31);
                break ;
			case OPT_KERNEL_WRITE:
			case 'w':
                kernel_write = 1 ;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
    }

	/* Initialize eclipse environment */
	eclipse_init();

	/* Special case of kernel write: only produce a kernel to disp on stdout. */
	if (kernel_write) {
		if (poly_u!=NULL) poly2d_free(poly_u);
		if (poly_v!=NULL) poly2d_free(poly_v);
		show_interpolation_kernel(kernel_name);
		return 0 ;
	}

	/* Get arguments */
    if ((argc-optind) < 1) {
        e_error("missing arguments: input frame name") ;
		if (poly_u!=NULL) poly2d_free(poly_u);
		if (poly_v!=NULL) poly2d_free(poly_v);
        return -1 ;
    }
    /* After the options, there must be at least an input name  */
    name_i = argv[optind++] ;
	/* See if an output name was provided, otherwise build one */
	if ((argc-optind) > 0) {
		strncpy(name_o, argv[optind], FILENAMESZ);
	} else {
		sprintf(name_o, "%s_warp.fits", get_rootname(get_basename(name_i)));
	}

	/* Detect incompatible modes */
	if (tr_polynomial && tr_linear) {
		e_error("cannot do simultaneously polynomial and linear");
		if (poly_u!=NULL) poly2d_free(poly_u);
		if (poly_v!=NULL) poly2d_free(poly_v);
		return -1 ;
	}

	/* Load input image  */
	if ((in = image_load(name_i)) == NULL) {
		e_error("cannot load image [%s]: aborting", name_i);
		if (poly_u!=NULL) poly2d_free(poly_u);
		if (poly_v!=NULL) poly2d_free(poly_v);
		return -1 ;
	}

	if (tr_subsample) warped = image_subsample(in);
	else if (tr_linear) {
        /* Handle simple linear/integer transformations */
		if (tr_translate && tr_rotate && tr_scale) {
			/* Pure translation */
			if (((double)(int)Tu == Tu) && ((double)(int)Tv == Tv)) {
				/* Integer translation */
				warped = image_shift_int(in, (int)Tu, (int)Tv);
			} else {
				/* Float translation */
				warped = image_shift(in, Tu, Tv, NULL);
			}
		} else if (tr_rotate && !tr_translate && !tr_scale) {
			/* Pure rotation */
			itheta = (int)theta ;
			if ((itheta==0) || (itheta==90) || (itheta==-90) || (itheta==180)) {
				/* Integer rotation: turn image by a quarter or a half */
				warped = image_copy(in);
				if (image_turn(warped, itheta)!=0) {
					e_error("in integer rotation");
					image_del(warped);
					warped = NULL ;
				}
			} else {
				/* Float rotation */
				/* convert theta to radians */
				theta *= M_PI / 180.0 ;
				
				lineartrans[0] =  cos(theta) ;
				lineartrans[1] = -sin(theta) ;
				lineartrans[2] =
					(-in->lx*cos(theta)+in->ly*sin(theta)+in->lx)*0.5 ;

				lineartrans[3] =  sin(theta) ;
				lineartrans[4] =  cos(theta) ;
				lineartrans[5] =
					(-in->lx*sin(theta)-in->ly*cos(theta)+in->ly)*0.5 ;

				warped = image_warp_linear(in, lineartrans, kernel_name);
			}
		} else if (tr_scale && !tr_translate && !tr_rotate) {
			/* Pure scaling */
			lineartrans[0] = sf ;
			lineartrans[1] = 0.00 ;
			lineartrans[2] = 0.00 ;
			lineartrans[3] = 0.00 ;
			lineartrans[4] = sf ;
			lineartrans[5] = 0.00 ;

			warped = image_warp_linear(in, lineartrans, kernel_name);
		} else {
			/* Composite or pure linear transformation */
			if (!tr_rotate && !tr_translate && !tr_scale) {
				/* Pure linear transformation with user-prov linear transf */
				warped = image_warp_linear(in, lineartrans, kernel_name);
			} else {
				/*
				   Composite linear transformation, build up the terms
				   according to the requested transfos
				   The terms are built from the product of 3 matrices:
				   Translation: [T]
				   1  0  0
				   0  1  0
				   Tu Tv 1
				   Rotation:	[R]
				   ct st 0
				  -st ct 0
				   0  0  1
				   Scale:		[S]
				   Su 0  0
				   0  Sv 0
				   0  0  1
				   with Su = Sv = scale factor, the case of different
				   scale factors in x and y is not supported.

				   The product [T][R][S] gives:

				 	Su.ct			 Sv.st				0
				   -Su.st			 Sv.ct				0
				    Su(Tu.ct-Tv.st)  Sv(Tu.st+Tv.ct)	1
				 */

				theta *= M_PI / 180.0 ;

				lineartrans[0] =  sf * cos(theta) ;
				lineartrans[1] = -sf * sin(theta) ;
				lineartrans[2] =  sf * ((Tu*cos(theta))-(Tv*sin(theta)));

				lineartrans[3] =  sf * sin(theta) ;
				lineartrans[4] =  sf * cos(theta) ;
				lineartrans[5] =  sf * ((Tu*sin(theta))+(Tv*cos(theta))) ;

				warped = image_warp_linear(in, lineartrans, kernel_name);
			}
		}
	} else {
		/* Polynomial transformation */
		warped = image_warp_generic(in, kernel_name, poly_u, poly_v);
	}

	if (poly_u!=NULL) poly2d_free(poly_u);
	if (poly_v!=NULL) poly2d_free(poly_v);
	image_del(in) ;
	if (warped==NULL) {
		e_error("during warping: no image produced");
		return -1 ;
	}

	e_comment(0, "saving [%s]", name_o);
	image_save_fits_hdrcopy(warped, name_o, name_i, BPP_DEFAULT);
	image_del(warped);

	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s [parameters] in [out]\n", pname) ;
	printf(
		"\n"
		"---------- transformations\n"
		"\n");
	printf(
		"-> linear\n"
		"\t-t / --translate 'tx ty'\n"
		"\t-r / --rotate 'angle'\n"
		"\t-s / --scale 'factor'\n"
		"\t-T / --transform 'p1 p2 p3 p4 p5 p6' (linear only)\n"
		"\t   where u = p1.x + p2.y + p3\n"
		"\t         v = p3.x + p4.y + p5\n");
	printf(
		"\t   direct transform:\n"
		"\t   (x,y) in original image\n"
		"\t   (u,v) in warped image\n"
		"\n");
	printf(
		"\t--sub subsample by a factor 2\n"
		"\n");
	printf(
		"-> polynomial\n"
		"\t-u / --polyu 'du dv c0 ... du dv cn'\n"
		"\t-v / --polyv 'du dv c0 ... du dv cn'\n"
		"\t   reverse transform:\n"
		"\t   where x = Px(u,v)\n"
		"\t         y = Py(u,v)\n");
	printf(
		"\t-S / --sttrfile file.tfits\n"
		"\twhere file.tfits contains the startrace deformation\n"
		"\t-A / --arcfile file.tfits\n"
		"\twhere file.tfits contains the arc deformation\n"
		"\n");
	printf(
		"---------- kernel\n"
		"\n"
		"\t-k / --kernel name\n"
		"\t-w / --write\n"
		"\n\n");
	exit(0) ;
}


