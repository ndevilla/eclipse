/*----------------------------------------------------------------------------*/
/**
   @file    xcorr2d.c
   @author  Nicolas Devillard
   @date    March 2002
   @version	$Revision: 1.2 $
   @brief   Compute offsets between planes of a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: xcorr2d.c,v 1.2 2002/11/21 10:17:09 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/21 10:17:09 $
	$Revision: 1.2 $
 */

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define OPT_REFPLANE		1001

/*-----------------------------------------------------------------------------
   							Function prototype
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static int xcorr2d(char * name_i, int refplane);
static char prog_desc[] = "compute offsets between planes of a cube";

/*-----------------------------------------------------------------------------
   									Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int     refplane ;    
    char *  name_i ;
    int     err ;
	int     c ;

    /* Default reference plane is the first one */
    refplane = 1 ;

    if (argc<2) usage(argv[0]);
    
    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

			{"refplane",	1, 0, OPT_REFPLANE},

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
						"Lhr:",
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
            /* xcorr2d specific options */
			case OPT_REFPLANE:
			case 'r':
                refplane = (int)atoi(optarg);
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
    }

	if ((argc-optind)<1) {
		e_error("missing argument: input file name(s)") ;
		return -1 ;
	}
    name_i = argv[optind] ;

	/* Initialize eclipse environment */
	eclipse_init();

    refplane -- ;
    err = xcorr2d(name_i, refplane);

	if (debug_active()) xmemory_status() ;
	return err ;
}

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ; 
	printf("use : %s [options] <cube>\n", pname);
	printf(
"options are :\n"
"\t-r or --refplane <num>   to specify reference plane (start from 1)\n"
"\n\n") ;
    exit(0) ;
}

static int xcorr2d(
        char    *   name_i, 
        int         refplane)
{
	cube_t	*	in ;
    double3 *   blind_offs ;
    int         i ;

    /* Load input cube */
    in = cube_load(name_i);
    if (in==NULL) {
        e_error("cannot load %s", name_i);
        return -1 ;
    }

    /* Check reference plane is valid */
    if (refplane<0 || refplane>(in->np-1)) {
        e_error("invalid reference plane: %d (input has %d planes)",
                refplane+1, in->np);
        cube_del(in);
        return -1 ;
    }

    /* Apply blind offsets */
    blind_offs = cube_blindoffsets(in, in->plane[refplane]);
    cube_del(in);
    if (blind_offs==NULL) {
        e_error("computing blind offsets");
        return -1 ;
    }

    /* Print out offsets */
    fprintf(stdout, "plane  #:       dx       dy\n");
    for (i=0 ; i<blind_offs->n ; i++) {
        fprintf(stdout, "plane %02d: %8.2f %8.2f\n", i+1, blind_offs->x[i],
        blind_offs->y[i]);
    }
    double3_del(blind_offs);
	return 0 ;
}

