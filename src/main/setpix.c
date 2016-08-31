/*----------------------------------------------------------------------------*/
/**
   @file    setpix.c
   @author  Nicolas Devillard
   @date    March 12th, 1997
   @version	$Revision: 1.15 $
   @brief   Pixel setting for pixel maps, images, or cubes
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: setpix.c,v 1.15 2002/11/20 14:40:31 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/20 14:40:31 $
	$Revision: 1.15 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
                                Fnctions prototypes
 -----------------------------------------------------------------------------*/

static void usage(char *pname) ;
static char prog_desc[] = "pixel editor" ;
void setpixels(char * in, char * out, char * regname);

/*-----------------------------------------------------------------------------
                            	    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char    *   name_i ;
    char        name_o[FILENAMESZ+1] ;
    char        point_file[FILENAMESZ+1] ;            
    int         c ;

    /* Initialize */
    point_file[0] = (char)0 ;

    while ((c = getopt(argc, argv, "Lf:")) != EOF)
        switch(c) {
		    /* Standard option: display license */
	    	case 'L':
                eclipse_display_license() ;
                return 0 ;
	  
            /* Name of file containing points */
            case 'f':
                strncpy(point_file, optarg, FILENAMESZ) ;
                break ;

            default:
                usage(argv[0]) ;
                break ;
        }

	/* Initialize eclipse environment */
	eclipse_init();

	/* Get arguments */
    if ((argc-optind) < 1) usage(argv[0]);

    /* After the options, there must be at least an input name  */
    name_i = argv[optind++];
	if ((argc-optind)<1) sprintf(name_o, "%s_set.fits", get_rootname(name_i));
	else strncpy(name_o, argv[optind], FILENAMESZ);

	/* Default name for file containing points is pts.txt   */
	if (point_file[0] == (char)0) strncpy(point_file, "pts.txt", FILENAMESZ) ;

	setpixels(name_i, name_o, point_file);
    if (debug_active()) xmemory_status() ;
    return 0 ;
}

static void usage(char * pname)
{
    hello_world(pname, prog_desc) ;
    printf("use : %s [options] in\n", pname) ;
    printf(
"options are :\n"
"\t[-f file.txt] file.txt contains pixel coordinates and values\n"
"\n\n") ;
    exit(0) ;
}

void setpixels(
        char    *   in, 
        char    *   out, 
        char    *   regname)
{
	int			*	px ;
	int			*	py ;
	pixelvalue	*	pv ;
	int				x, y ;
	double			dval ;
	int				np ;
	int				i, p ;
	FILE		*	regs ;
	char			line[ASCIILINESZ];
	int				nval ;
	cube_t		*	setcube ;

	/* Load list of pixels to modify */
	if ((regs = fopen(regname, "r"))==NULL) {
		e_error("cannot open file [%s]", regname);
		return ;
	}
	/* Count number of pixels in the list */
	np = 0 ;
	while (fgets(line, ASCIILINESZ, regs)!=NULL) {
		if (line[0]!='#') {
			nval = sscanf(line, "%d %d %lg", &x, &y, &dval);
			if (nval==3) np++ ;
		}
	}
	rewind(regs);
	/* Read and store pixel values and positions */
	px = malloc(np * sizeof(int)) ;
	py = malloc(np * sizeof(int)) ;
	pv = malloc(np * sizeof(pixelvalue));
	i=0 ;
	while (fgets(line, ASCIILINESZ, regs)!=NULL) {
		if (line[0]!='#') {
			nval = sscanf(line, "%d %d %lg", &x, &y, &dval);
			if (nval==3) {
				px[i] = x-1; /* change from FITS to C coordinates */
				py[i] = y-1; /* change from FITS to C coordinates */
				pv[i] = (pixelvalue)dval ;
				i++ ;
			}
		}
	}
	fclose(regs);
	
	/* Load input file */
	setcube = cube_load(in);
	if (setcube==NULL) {
		e_error("cannot load [%s]", in);
		free(px);
		free(py);
		free(pv);
		return ;
	}

	for (p=0 ; p<setcube->np ; p++) {
		for (i=0 ; i<np ; i++) {
			setcube->plane[p]->data[px[i]+py[i]*setcube->lx] = pv[i];
		}
	}
	free(px);
	free(py);
	free(pv);
	cube_save_fits_hdrcopy(setcube, out, in);
	cube_del(setcube);
	return ;
}
