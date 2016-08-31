
/*---------------------------------------------------------------------------
   
   File name 	:	wfi_.c
   Author 		:	N. Devillard
   Created on	:
   Description	:	WFI

 *--------------------------------------------------------------------------*/

/*
	$Id
	$Author
	$Date
	$Revision
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/
 
#include <stdio.h>
#include <string.h>

#include "eclipse.h"
#include "wfip_lib.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define OPT_		1001


static void usage(char *pname) ;
static char prog_desc[] = "... missing program description ..." ;


/*---------------------------------------------------------------------------
									Main	
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int             c ;

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

	/* Processing takes place here */

    if (debug_active())
		xmemory_status() ;
    return 0 ;
} 



/*
 * This function only gives the usage for the program
 */
 
static void
usage(char *pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] <WFI extension>\n", pname) ;
    printf("options are:\n");
    printf("\t-o or --option to ...\n");
    printf("\n\n") ;
    exit(1) ;
} 

