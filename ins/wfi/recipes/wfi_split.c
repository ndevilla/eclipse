
/*---------------------------------------------------------------------------
   
   File name 	:	wfi_split.c
   Author 		:	N. Devillard
   Created on	:	18 May 2000
   Description	:	WFI frame splitter

 ---------------------------------------------------------------------------*/

/*
	$Id: wfi_split.c,v 1.3 2001/01/24 13:47:26 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/01/24 13:47:26 $
	$Revision: 1.3 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eclipse.h"
#include "wfip_lib.h"


#define OPT_XTNUM		1001

static void usage(char *pname) ;
static char prog_desc[] = "WFI frame splitter" ;


/*---------------------------------------------------------------------------
									Main	
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int             c ;
	char			name_i[FILENAMESZ] ;
	char			name_o[FILENAMESZ] ;
	int				xtnum ;

	xtnum = 0 ;

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},
 
            {"xtnum", 	1, 0, OPT_XTNUM},

            {0, 0, 0, 0}
 
        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhx:",
                        long_options,
                        &option_index) ;
        if (c==-1)
            break ;
 
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

		/* Local option: xtnum */
			case OPT_XTNUM:
			case 'x':
			xtnum = atoi(optarg);
			break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }
 
    /* Initialize eclipse environment */
    eclipse_init() ;

    /*
     * If no argument, display help message
     */
    if ((argc-optind)<1) usage(argv[0]);

	/* Get input and output names */

	strcpy(name_i, argv[optind++]);
	if ((argc-optind)!=0) {
		strcpy(name_o, get_rootname(argv[optind++]));
	} else {
		strcpy(name_o, get_rootname(name_i));
	}

	wfi_split(name_i, name_o, xtnum);

    if (debug_active())
		xmemory_status() ;
    return(0) ;
} 



/*
 * This function only gives the usage for the program
 */
 
static void
usage(char *pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] <WFI frame> [output base name]\n", pname) ;
	printf("\toptions are:\n");
	printf("\t-x or --xtnum <#>  select extension to extract (0=all)\n");
    printf("\n\n") ;
    exit(1) ;
} 

