/*----------------------------------------------------------------------------*/
/**
   @file    ipaste.c
   @author  Nicolas Devillard
   @date    July 31, 1996
   @version	$Revision: 1.24 $
   @brief   Paste an image into another
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: ipaste.c,v 1.24 2002/11/20 12:21:17 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 12:21:17 $
	$Revision: 1.24 $
 */

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
  								Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "paste an image into another" ;
 
/*-----------------------------------------------------------------------------
 								Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	image_t		*	frame_receive,
				*	frame_insert ;
	image_t		*	frame_pasted ;
	cube_t		*	cube_out ;
	char		*	framename ;
    char        *   insname ;
    char        *   outname ;
	int				xpos,
					ypos ;
	int				c ;
	history		*	hs ;

    /* Initialize */
	xpos = ypos = 1 ;
    outname = "pasted.fits" ;

    if (argc<2) usage(argv[0]);

	/* Command line parsing */
    while ((c = getopt(argc, argv, "Lx:y:")) != EOF)
        switch(c) {
		    /* Standard option: display license */
			case 'L':
			    eclipse_display_license() ;
			    return 0 ;

    		/* Parameters: xpos and ypos for pasting	*/
			case 'x': xpos = (int)atoi(optarg) ; break ;
			case 'y': ypos = (int)atoi(optarg) ; break ;

            default:
                usage(argv[0]) ;
                break ;
        }

	/* Initialize eclipse environment */
	eclipse_init();

	/* Get arguments */
    if ((argc-optind) < 2) {
        e_error("missing arguments") ;
        return -1 ;
    }

	/* After the options, there must be a frame name and an insert name */
    framename   = argv[optind++] ;
    insname     = argv[optind++] ;
	/* optional: output name	*/
    if ((argc-optind) >= 1) outname = argv[optind];

	/* Load requested images */
	if ((frame_receive = image_load(framename)) == NULL) {
		e_error("in loading frame [%s]: aborting", framename) ;
		return -1 ;
	}
	if ((frame_insert = image_load(insname)) == NULL) {
		e_error("in loading insert [%s]: aborting", insname) ;
		image_del(frame_receive) ;
		return -1 ;
	}

	/* Now insert one image into another	*/
	frame_pasted = image_paste(	frame_receive, frame_insert, xpos, ypos) ;
	image_del(frame_receive) ;
	image_del(frame_insert) ;
	if (frame_pasted == NULL) {
		e_error("during paste: aborting") ;
		return -1 ;
	}		

	/* Promote output image to cube to allow comments insert	*/
	cube_out = cube_from_image(frame_pasted) ;
	image_del(frame_pasted) ;
	if (cube_out == NULL) {
		e_error("cannot promote image to cube: aborting save") ;
		return -1 ;
	}
	/* Add comments to output image	*/
	hs = history_new() ;
	history_add(hs, "--- eclipse ipaste") ;
	history_add(hs, "initial image is:") ; 
	history_add(hs, "%s", framename) ;
	history_add(hs, "pasted image is:") ;
	history_add(hs, "%s", insname) ;
	history_add(hs, "at position [%d, %d]", xpos, ypos) ;
	cube_save_fits_hdrcopy_wh(cube_out, outname, framename, hs) ;
	history_del(hs);

	cube_del(cube_out) ;
	if (debug_active()) xmemory_status() ;
	return 0 ;
}

static void usage(char * pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s [parameters] [options] frame insert [out]\n", pname) ;
	printf(
"parameters are:\n"
"\t[-x LowerLeftXPosInFrame]\n"
"\t[-y LowerLeftYPosInFrame]\n"
"\n\n");
	exit(0) ;
}
