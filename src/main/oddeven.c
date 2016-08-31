/*----------------------------------------------------------------------------*/
/**
   @file    oddeven.c
   @author  Y. Jung
   @date    October 2003
   @version	$Revision: 1.3 $
   @brief   Odd even effect correction
*/  
/*----------------------------------------------------------------------------*/

/*
	$Id: oddeven.c,v 1.3 2004/01/07 13:18:57 yjung Exp $
	$Author: yjung $
	$Date: 2004/01/07 13:18:57 $
	$Revision: 1.3 $
*/

/*------------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define OPT_QUAD            1000
#define OPT_FORCE           1001
#define OPT_OUTPUT          1002

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
static char prog_desc[] = "Odd-even effect correction" ;

/*------------------------------------------------------------------------------
                                    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int             c ;
	int		        by_quad ;
	int		        force ;
    char            name_o[FILENAMESZ] ;
    char            outname[FILENAMESZ] ;
    image_t     *   im_in ;
    image_t     *   im_out ;
    qfits_header*   fh ;
    char        *   sval ;
	
    if (argc<2) usage(argv[0]);

    /* Initialize */
	by_quad = 0 ;
    force = 0 ;
    name_o[0] = 0 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

			{"quad",	0, 0, OPT_QUAD},
			{"output",	1, 0, OPT_OUTPUT},
			{"force",	0, 0, OPT_FORCE},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhqo:f",
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
			case OPT_OUTPUT:
			case 'o':
                strncpy(name_o, optarg, FILENAMESZ) ;
                break ;
			case OPT_FORCE:
			case 'f':
                force = 1 ;
                break ;
			case OPT_QUAD:
			case 'q':
                by_quad = 1 ;
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
		/* Current image */
        printf("# file: %s\n", argv[optind]) ;

        /* Check input header */
        fh = qfits_header_read(argv[optind]) ;
        if (fh == NULL) {
            e_error("cannot read header from %s: aborting", argv[optind]) ;
            return -1 ;
        }
        sval = qfits_header_getstr(fh, "OEFILT") ;
        if (sval!=NULL) {
            e_warning("file %s already processed with OEFILT %s", argv[optind],
                    sval);
            if (!force) {
                e_error("nothing done -- use -f/--force to force filter");
                qfits_header_destroy(fh) ;
                return -1 ;
            }
        }

        /* Output name */
        if (name_o[0] == 0) {
            sprintf(outname, "%s_oec.fits",
                    get_rootname(get_basename(argv[optind]))) ;
        } else {
            sprintf(outname, "%s", name_o) ;
        }

        /* Load input image */
		if ((im_in = image_load(argv[optind])) == NULL) {
			e_error("loading %s: aborting", argv[optind]) ;
            qfits_header_destroy(fh) ;
			return -1 ;
		}

        /* Apply odd-even filter */
        if (by_quad) {
            im_out = image_de_oddeven_byquad(im_in) ;
        } else {
            im_out = image_de_oddeven(im_in) ;
        }
        image_del(im_in) ; 
        if (im_out == NULL) {
            e_error("in odd-even filter: aborting");
            qfits_header_destroy(fh);
            return -1 ;
        }

        /* Build output FITS header */
        e_comment(1, "saving result as [%s]", outname);
        qfits_header_add(fh,
                        "OEFILT",
                        OEFILT_VERSION,
                        "Odd-even filter algorithm version",
                        NULL);
        image_save_fits_hdrdump(im_out, outname, fh, BPP_DEFAULT);
        qfits_header_destroy(fh) ;
        image_del(im_out) ;
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
	"\t-q (--quad)         to operate for each quadrant separately\n"
	"\t-f (--force)        to force the operation ieven of already applied\n"
	"\t-o (--output)       to specify the output name\n");
    printf("\n\n") ;
    exit(1) ;
}

