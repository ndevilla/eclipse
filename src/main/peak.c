/*----------------------------------------------------------------------------*/
/**
   @file    peak.c
   @author  N. Devillard
   @date    June 2001
   @version	$Revision: 1.39 $
   @brief   Astronomical object detector and stat computation
*/  
/*----------------------------------------------------------------------------*/

/*
	$Id: peak.c,v 1.39 2003/08/07 09:29:58 yjung Exp $
	$Author: yjung $
	$Date: 2003/08/07 09:29:58 $
	$Revision: 1.39 $
*/

/*------------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define OPT_METHOD		            1000
#define OPT_KAPPA		            1010
#define OPT_SMEAR		            1011
#define OPT_SQHSZ		            1020

#define OPT_FINEPOS		            1030
#define OPT_FWHM		            1031
#define OPT_PHOT	                1032

#define OPT_RTD 		            1040

#define DETECT_UNSET				0
#define DETECT_WITH_KAPPASIGMA		1
#define DETECT_WITH_SQUARES			2

/*------------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*------------------------------------------------------------------------------
                                Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "object detection and stat computation" ;

/*------------------------------------------------------------------------------
                                    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int             c ;
	int		        ret ;
	int		        detect_method ;
	int		        fpos_flag ;
	double	        fpos_star, fpos_int, fpos_ext ;
	int		        pos_only ;
	int		        sq_hx, sq_hy ;
	double	        kappa ;
	int		        smear_flag ;
	int		        fwhm_flag ;
	int		        phot_flag ;
	double	        phot_star, phot_int, phot_ext ;
	int		        hx, hy ;
	int		        rtd_active ;
	detected	*	det ;
	double3		*	pts ;
	cube_t		*	c_in ;
	int				i, p ;
	int				err ;
	
    if (argc<2) usage(argv[0]);

    /* Initialize */
	detect_method = DETECT_WITH_KAPPASIGMA ;
	hx 			= -1 ;
	hy			= -1 ;
	smear_flag	= 0 ;
	fwhm_flag	= 0 ;
	fpos_flag 	= 0 ;
	fpos_star	= DETECTED_FPOS_STAR ;
	fpos_int	= DETECTED_FPOS_INT ;
	fpos_ext	= DETECTED_FPOS_EXT ;
	pos_only	= 0 ;
	sq_hx		= DETECTED_SQHX ;
	sq_hy		= DETECTED_SQHY ;
	kappa		= DETECTED_KAPPA ;
	phot_flag	= 0 ;
	phot_star	= DETECTED_PHOT_STAR ;
	phot_int	= DETECTED_PHOT_INT ;
	phot_ext	= DETECTED_PHOT_EXT ;
	rtd_active 	= 0 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

			{"method",	1, 0, OPT_METHOD},
			{"kappa",	1, 0, OPT_KAPPA},
			{"smear",	0, 0, OPT_SMEAR},
			{"sqhsize",	1, 0, OPT_SQHSZ},

			{"fpos",	1, 0, OPT_FINEPOS},
			{"fwhm",	0, 0, OPT_FWHM},
			{"phot",	1, 0, OPT_PHOT},

			{"rtd",		0, 0, OPT_RTD},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "df:Fhk:m:P:sS:L",
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
			case OPT_METHOD:
			case 'm':
                if (!strcmp(optarg, "clip")) {
                    detect_method = DETECT_WITH_KAPPASIGMA ;
                } else if (!strcmp(optarg, "squares")) {
                    detect_method = DETECT_WITH_SQUARES ;
                } else {
                    e_error("invalid method name: [%s]", optarg);
                    return -1 ;
                }
                break ;

			case OPT_KAPPA:
			case 'k':
                ret = sscanf(optarg, "%lg", &kappa);
                if (ret!=1) {
                    e_error("-k/--kappa expects 1 argument, received %d", ret);
                    return -1 ;
                }
                break ;
			case OPT_SMEAR:
			case 's':
                smear_flag=1 ;
                break ;
			case OPT_SQHSZ:
			case 'S':
                ret = sscanf(optarg, "%d %d", &hx, &hy);
                if (ret!=2) {
                    e_error("-S/--sqhsize expects 2 arguments, not %d", ret);
                    return -1 ;
                }
                break ;
			case OPT_FINEPOS:
			case 'f':
                fpos_flag=1 ;
                ret=sscanf(optarg,"%lg %lg %lg",&fpos_star,&fpos_int,&fpos_ext);
                if (ret!=3) {
                    e_error("-f/--fpos expects 3 arguments, received %d", ret);
                    return -1 ;
                }
                break ;
			case OPT_FWHM:
			case 'F':
                fwhm_flag=1 ;
                break ;
			case OPT_PHOT:
			case 'P':
                phot_flag=1;
                ret=sscanf(optarg,"%lg %lg %lg",&phot_star,&phot_int,&phot_ext);
                if (ret!=3) {
                    e_error("-P/--phot expects 3 arguments, received %d", ret);
                    return -1 ;
                }
                break ;
			case OPT_RTD:
            case 'd':
                rtd_active=1 ;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
    }

    /* Initialize eclipse environment */
    eclipse_init() ;

    if ((argc-optind) < 1) {
        e_error("missing arguments: input file name") ;
        return -1 ;
    }
	while ((argc-optind)>0) {
		/* Load input cube */
		if ((c_in = cube_load(argv[optind])) == NULL) {
			e_error("loading %s: aborting", argv[optind]);
			return -1 ;
		}

		printf("# file: %s\n", argv[optind]);
		for (p=0 ; p<c_in->np ; p++) {
			if (rtd_active) {
				e_comment(0, "displaying image...");
				rtd_image_put(c_in->plane[p]);
			}
			/* Detect positions and fill up detected object */
            det = NULL ;
			switch(detect_method) {
				case DETECT_WITH_KAPPASIGMA:
                    det = detected_ks_engine(c_in->plane[p], kappa, smear_flag);
                    break ;
				case DETECT_WITH_SQUARES:
                    det = detected_sq_engine(c_in->plane[p], hx, hy) ;
                    break ;
                default:
                    e_error("no method specified for detection");
                    break ;
			}
			if (det == NULL) {
				e_error("in detection on plane %d: aborting", p+1);
				cube_del(c_in);
				return -1 ;
			}
			if (det->nbobj<1) {
				e_comment(0, "no object found in plane %d of [%s]", p+1,
							argv[optind]);
			} else {
				/* Do fine positioning if requested */
				if (fpos_flag) {
					err+= detected_compute_finepos(	det,
													c_in->plane[p],
													fpos_star,
													fpos_int,
													fpos_ext);
				}
				/* Do FWHM computation if requested */
				if (fwhm_flag) err+= detected_compute_fwhm(det, c_in->plane[p]);
				/* Do Photometry computation if requested */
				if (phot_flag) {
					err+= detected_compute_phot(det,
												c_in->plane[p],
												phot_star,
												phot_int,
												phot_ext);
				}
				/* Display results if requested */
				if (rtd_active) {
					pts = double3_new(det->nbobj);
					for (i=0 ; i<pts->n ; i++) {
						pts->x[i] = det->x[i] ;
						pts->y[i] = det->y[i] ;
						pts->z[i] = 0 ;
					}
					e_comment(0, "displaying found objects...");
					rtd_point_plot(pts);
					double3_del(pts);
				}
			}
			detected_dump(det, stdout);
			detected_del(det);
		}
		cube_del(c_in);
		optind++ ;
	}
	return 0 ;
}

static void usage(char * pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] in\n", pname) ;
	printf(
	"options are:\n"
	"\t-s (--smear)              Smear image before detection\n"
	"\t-f (--fpos) 'r1 r2 r3'    Set radiuses for fine positioning\n"
	"\t-F (--fwhm)               Print out FWHM for all objects\n"
	"\t-P (--phot) 'r1 r2 r3'    Compute photometry for all objects\n"
	"\t-d (--rtd)                Display image and results on RTD\n");
	printf(
	"\n"
	"\t-m (--method) clip        Use kappa-sigma clipping\n"
	"\t-k (--kappa) value        Set value for kappa-sigma clipping\n"
	"\n"
	"\t-m (--method) squares     Use squares method (experimental)\n"
	"\t-S (--sqhsize) size       Set square size\n"
	);
    printf("\n\n") ;
    exit(1) ;
}

