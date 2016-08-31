
/*---------------------------------------------------------------------------
   
   File name 	:	fitsppm.c
   Author 		:	Nicolas Devillard
   Created on	:	April 2nd, 1996
   Description	:	FITS to PPM converter

 ---------------------------------------------------------------------------*/

/*
	$Id: fitsppm.c,v 1.18 2002/07/31 13:53:18 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/07/31 13:53:18 $
	$Revision: 1.18 $
 */

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eclipse.h"

#define OPT_LUT		1001

/*---------------------------------------------------------------------------
                         Functions ANSI C declarations
 ---------------------------------------------------------------------------*/

static void usage(char *pname) ;

static BYTE * convert_local_to_ppm(image_t * img, BYTE * lut);
static BYTE * get_LUT(char * filename);

static char prog_desc[] = "FITS to PPM conversion" ;

/*---------------------------------------------------------------------------
  								main()	
 ---------------------------------------------------------------------------*/

int 
main(int argc, char *argv[])
{
	FILE		*	ppmfile ;
	BYTE		*	buffer ;
	image_t		*	image_in ;
	BYTE		*	lut ;
	int				c ;
	char 		*	fitsname ;
    char			ppmname[FILENAMESZ+1],
					lutname[FILENAMESZ+1] ;

	lutname[0] = (char)0 ;	
	ppmname[0] = (char)0 ;

    if (argc<2)
        usage(argv[0]);

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"license", 0, 0, OPT_LICENSE},
            {"help",    0, 0, OPT_HELP},
            {"version", 0, 0, OPT_VERSION},

            {"lut",     1, 0, OPT_LUT},

            {0, 0, 0, 0}
        } ;
        c = getopt_long(argc,
                        argv,
                        "Lhl:",
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

        /* Local options */
			case OPT_LUT:
			case 'l':
			strncpy(lutname, optarg, FILENAMESZ) ;
			break ;

            default:
            usage(argv[0]) ;
            break ;
        }
    }


	/* Initialize eclipse environment */
	eclipse_init();

	/* Get arguments */
	if ((argc-optind)<1) {
		e_error("missing argument: input file name") ;
		return -1 ;
	}

    fitsname = argv[optind] ;
    optind ++ ;

	if ((argc-optind) >0) {
		strncpy(ppmname, argv[optind], FILENAMESZ) ;
	} else {
		sprintf(ppmname, "%s.ppm", get_rootname(fitsname)) ;
	}
	if ((image_in = image_load(fitsname)) == NULL) {
		e_error("error in loading file [%s]: aborting conversion", fitsname) ;
		return -1 ;
	}
	/* Get lut */
	if ((lut = get_LUT(lutname)) == NULL) {
		e_error("cannot create get lookup table [%s]: aborting", lutname) ;
		return -1 ;
	}
	/* Convert in FITS buffer to out PPM buffer	*/
	buffer = convert_local_to_ppm(image_in, lut) ;
	free(lut) ;

	/* Output data to file */
	if (!strcmp(ppmname, "STDOUT")) {
		ppmfile = stdout ;
	} else {
		ppmfile = fopen(ppmname, "w") ;
	}
	if (ppmfile==NULL) {
		e_error("cannot create output: aborting") ;
		return -1 ;
	}

	fprintf(ppmfile, "P6 %d %d 255\n", image_in->lx, image_in->ly) ;
	fwrite(buffer, 1, 3 * image_in->lx * image_in->ly, ppmfile) ;
	image_del(image_in) ;
	free(buffer) ;
	if (ppmfile!=stdout)
		fclose(ppmfile) ;

	if (debug_active())
		xmemory_status();

	return 0 ;
}


/*
 * This function only gives the usage for the program
 */

static void                                                                    
usage(char *pname)
{
	hello_world(pname, prog_desc) ;
	printf("use : %s [options] <image.fits> [image.ppm]\n", pname) ;
	printf(
"options are:\n"
"\t[-l <lutfile>] or [--lut <lutfile>] to request a LUT\n"
"Specify 'STDOUT' as output file name to output to stdout\n"
"\n\n");
	exit(0) ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Saves an image in memory to PPM format.
  @param    In      Image to save.
  @param    lut     Colour lookup table to use.
  @return   Pointer to newly allocated byte array.

  This function performs a conversion of an image in memory to a byte
  buffer that is suitable for dumping into e.g. a PPM file. The pixel
  conversion is performed to bring pixels into 256 colours, so images
  with a large dynamic scale will loose their pixel depth resolution.

  The chosen 256 colours are provided by the user as a colour lookup
  table. This table is a set of 256 x 3 bytes (256 times RGB).
 */
/*--------------------------------------------------------------------------*/

static BYTE * convert_local_to_ppm(image_t * img, BYTE * lut)
{
    register BYTE       *out, *ret ;
    register pixelvalue *cur ;
    register int        i,j ;
    pixelvalue          min_pix, max_pix ;
    double              scale_factor ;
    int                 gray ;

    /* Error handling: test entries */
    if (img==NULL || lut==NULL) return NULL ;

    /*
     * Rescaling is necessary : we need first minimum and maximum values
     * For speed reasons, it is made directly here instead of using
     * dedicated functions in ImageStats.c
     */

    min_pix = img->data[0] ;
    max_pix = img->data[0] ;
    cur = img->data ;

    for (i=1 ; i<(img->lx * img->ly) ; i++) {
        if (*cur > max_pix)
            max_pix = *cur ;
        else if (*cur < min_pix)
            min_pix = *cur ;
        cur++ ;
    }

    if (max_pix == min_pix)
        scale_factor = 0.0 ;
    else
        scale_factor = 255.0 / (double)(max_pix - min_pix) ;

    /* Allocate output buffer */
    ret = malloc(3 * img->lx * img->ly) ;


    /* 
     * Input is in pixelvalue format, output is rescaled to 3 bytes.
     * these 3 bytes are actually mapped by an 8-bit LUT, which is
     * overkill in terms of disk space but a generic format.
     */
    cur = img->data ;
    out = ret ;
    for (j=img->ly ; j ; j--) {
        cur = img->data + (j-1) * img->lx ;
        for (i=0 ; i<img->lx ; i++) {
            gray = 3*(int)(scale_factor*((double)(*cur++)-(double)min_pix)+0.5);
            *out++ = lut[gray++] ;
            *out++ = lut[gray++] ;
            *out++ = lut[gray] ;
        }
    }
    return ret ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Gets a colour lookup table from a file, or provide one.
  @param    filename    Name of the file to read the table from.
  @return   Newly allocated array of bytes

  This function reads an ASCII file describing a colour lookup table.
  The colour entries are given by sets of 3 numbers on each line of
  the ASCII file. There must be 256 entries.

  The returned buffer has the exact size 256x3 (256 RGB triplets). It
  must be freed using free().

  If any error occurs during the lookup table file read, a default
  lookup table is provided. The triplets are all such as R=G=B, thus a
  standard graylevel lookup table.
 */
/*--------------------------------------------------------------------------*/

static BYTE * get_LUT(char * filename)
{
    FILE    *lutfile ;
    BYTE    *outlut ;
    int     i ;
    double  r, g, b ;

    outlut = malloc(256 * 3 * sizeof(BYTE)) ;

    if ((lutfile = fopen(filename, "r")) != NULL) {
        /* successfully opened file */
        for (i=0 ; i<256 ; i++) {
            fscanf(lutfile, "%lg %lg %lg", &r, &g, &b) ;
            outlut[3*i] = (BYTE)(255.0 * r + 0.5) ;
            outlut[3*i+1] = (BYTE)(255.0 * g + 0.5) ;
            outlut[3*i+2] = (BYTE)(255.0 * b + 0.5) ;
        }
        fclose(lutfile) ;
    } else {
        /* file was not found or opened : provide default LUT (gray) */
        for (i=0 ; i<256 ; i++)
            outlut[3*i] = outlut[3*i+1] = outlut[3*i+2] = (BYTE)i ;
    }

    return outlut ;
}


