
/*---------------------------------------------------------------------------
	
    File name 	:	ado_refits.c
	Author 		:	Nicolas Devillard
	Created on	:	October 13, 1995
	Description	:	Reformats and Adonis FITS file to standard FITS

 ---------------------------------------------------------------------------*/

/*
 $Id: ado_refits.c,v 1.3 2001/07/26 09:30:01 ndevilla Exp $
 $Author: ndevilla $
 $Date: 2001/07/26 09:30:01 $
 $Revision: 1.3 $
 */

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eclipse.h"
#include "ado_utils.h"


static void usage(char *pname) ;
static char prog_desc[] = "reformat an Adonis FITS file to standard FITS" ;
 
/*---------------------------------------------------------------------------
  								Main code
 ---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	char		inname[FILENAMESZ];
	int			c ;
	int			status ;
	int			i ;
	int			force_flag=0 ;

    /* Command line parsing by getopt() */
    while ((c = getopt(argc, argv, "fL")) != EOF)
        switch(c)
        {
		/* Standard option: display license (not documented in usage)	*/
			case 'L':
			eclipse_display_license() ;
			return 0 ;

			case 'f':
			e_comment(0, "Force flag set: will reformat all files");
			force_flag=1;
			break;

            default:
            usage(argv[0]) ;
            break ;
        }

    /* Get arguments    */
    if ((argc-optind) < 1) {
        e_error("missing arguments") ;
        return(-1) ;
    }


	/* Initialize eclipse environment */
	eclipse_init();

	for (i=optind ; i<argc ; i++) {
		strcpy(inname, argv[i]) ;
		status = adonis_reformat_fits(inname, force_flag) ;
		if (status) {
			e_error("reformat error in file %s", inname) ;
		}
	}

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
	printf("use : %s [options] <FITS files...>\n", pname) ;
	printf("options are:\n");
	printf("\t-f to force reformatting of all files\n");
	printf("\n\n") ;
	exit(0) ;
}

