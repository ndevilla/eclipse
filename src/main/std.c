/*----------------------------------------------------------------------------*/
/**
   @file    std.c
   @author  Nicolas Devillard
   @date    22 Jan 1999
   @version	$Revision: 1.14 $
   @brief   standard star browser tool
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: std.c,v 1.14 2002/11/20 15:38:36 yjung Exp $
    $Author: yjung $
    $Date: 2002/11/20 15:38:36 $
    $Revision: 1.14 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _ECLIPSE_
#include "eclipse.h"
#else
#include "getopt.h"
#include "e_error.h"
#include "strlib.h"
#endif

#include "irstd.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define OPT_POSITION		1001
#define OPT_NAME			1002
#define OPT_MAGNITUDE		1003
#define OPT_RADIUS			1004
#define OPT_CATALOGS		1005

#define OPT_KEYS			2001

/*-----------------------------------------------------------------------------
                                New types
 -----------------------------------------------------------------------------*/

typedef enum _search_type_ {
	search_unknown,
	search_by_name,
	search_by_position,
	search_around_position,
	search_by_magnitude
} search_type ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static void usage(char * pname) ;
static char prog_desc[] = "standard star search" ;
void irstd_display_result(irstd **, int, int) ;

/*-----------------------------------------------------------------------------
                            Global variables
 -----------------------------------------------------------------------------*/

extern char *optarg ;
extern int   optind ;

char ** catalog_names ;

/*-----------------------------------------------------------------------------
                                 Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int     		c ;
	irstd 		*	one_star ;
	irstd 		**	star_list ;
	int				nstars ;

	char 		*	star_name;
	double			ra, dec, radius ;
	ir_waveband		band ;
	char			mag[3] ;
	double			array_mag[2] ;
	search_type		search ;
	double			d[6] ;
	int				parsed ;
	char			dec_sign[10] ;

	int				key_print ;
	int				catalog_sel ;

    /* Get internal list of catalog names */
    catalog_names = irstd_catalog_names();

	if (argc<2) usage(argv[0]) ;

    /* Initialize */
	star_name 	= NULL ;
	ra 			= -1.0 ;
	dec 		= -1.0 ;
	radius 		= -1.0 ;
	star_list 	= NULL ;
	one_star  	= NULL ;
	nstars 		= 0 ;
	search 		= search_unknown ;
	band 		= WAVEBAND_UNKNOWN;
	key_print 	= 0 ;
	catalog_sel	= 0 ;

	irstd_setactive("none");

    while (1) {
        int     option_index = 0 ;
        static struct option long_options[] =
        {
            {"pos", 	1, 0, OPT_POSITION},
            {"name", 	1, 0, OPT_NAME},
            {"mag", 	1, 0, OPT_MAGNITUDE},
            {"radius", 	1, 0, OPT_RADIUS},
            {"cat", 	1, 0, OPT_CATALOGS},
			{"key",     0, 0, OPT_KEYS}, 

            {0, 0, 0, 0}

        } ;
        c = getopt_long(argc,
                        argv,
                        "c:km:n:p:r:",
                        long_options,
                        &option_index) ;
        if (c==-1) break ;

        switch(c) {
            case OPT_POSITION:
            case 'p':
                search = search_by_position ;
                parsed = sscanf(optarg, "%lg %lg %lg %lg %lg %lg",
                             d, d+1, d+2, d+3, d+4, d+5) ;
                if (parsed == 2) {
                    ra = d[0] ;
                    dec = d[1] ;
                } else if (parsed==6) {
                    ra = 15.0 * (d[0]+d[1]/60.0+d[2]/3600.0) ;
                    dec = d[3] + d[4]/60.0 + d[5]/3600.0 ;
                    sscanf(optarg, "%*g %*g %*g %9s", dec_sign);
                    if (dec_sign[0]=='-') dec = -dec ;
                } else {
                    e_error("-p/--pos expects 2 or 6 args, got %d", parsed) ;
                    return -1 ;
                }
                break ;
			case OPT_RADIUS:
			case 'r':
                search = search_around_position ;
                sscanf(optarg, "%lg", &radius) ;
                break ;
			case OPT_NAME:
			case 'n':
                search = search_by_name ;
                star_name = optarg ;
                break ;
			case OPT_MAGNITUDE:
			case 'm':
                search = search_by_magnitude ;
                sscanf(optarg, "%2s %lg %lg", mag, array_mag, array_mag+1) ;
                strcpy(mag, strlwc(mag)) ;
                if (!strcmp(mag, "h"))          band = WAVEBAND_H ;
                else if (!strcmp(mag, "j"))     band = WAVEBAND_J ;
                else if (!strcmp(mag, "k"))     band = WAVEBAND_K ;
                else if (!strcmp(mag, "ks"))    band = WAVEBAND_KS ;
                else if (!strcmp(mag, "l"))     band = WAVEBAND_L ;
                else if (!strcmp(mag, "lp"))    band = WAVEBAND_Lprime ;
                else if (!strcmp(mag, "m"))     band = WAVEBAND_M ;
                else if (!strcmp(mag, "mp"))    band = WAVEBAND_Mprime ;
                else {
                    e_error("unsupported waveband: [%s]", mag) ;
                    return -1 ;
                }
                if (array_mag[0]>array_mag[1]) {
                    e_error("magnitude min > max: try again") ;
                    return -1 ;
                }
                break ;
			case OPT_KEYS:
			case 'k':
                key_print = 1 ;
                break ;
			case OPT_CATALOGS:
			case 'c':
                if (irstd_setactive(optarg)==-1) return -1 ;
                catalog_sel=1 ;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }
    }

	/* Set catalogs */
	if (catalog_sel==0) {
		irstd_setactive("all");
	}

	if (search == search_unknown) {
		e_error("undefined search: aborting") ;
		return -1 ;
	}
	if ((search == search_around_position) &&
		(ra<0.0)) {
		e_error("radius given but no position was defined");
		return -1 ;
	}

	/* Database browsing takes place here */
	switch (search) {
		case search_by_name:
            star_list = irstd_get_star_by_name(star_name, &nstars) ;
            break ;

		case search_by_position:
            one_star = irstd_get_closest_star(ra, dec) ;
            if (one_star!=NULL) nstars=1 ;
            break ;

		case search_around_position:
            star_list = irstd_get_star_by_position(ra, dec, radius, &nstars);
            break ;

		case search_by_magnitude:
            star_list = irstd_get_star_by_magnitude(band,
                                                    array_mag[0],
                                                    array_mag[1],
                                                    &nstars);
            break ;

		default:
            e_error("unspecified search mode") ;
            break ;
	}

	if (star_list==NULL && one_star==NULL) {
		printf("request returned no star\n") ;
	} else {
		if (star_list==NULL) {
			star_list = malloc(sizeof(irstd*));
			star_list[0] = one_star ;
		}
		irstd_display_result(star_list, nstars, key_print) ;
		free(star_list) ;
	}
    return 0 ;
}


/*-----------------------------------------------------------------------------
                            Function codes
 -----------------------------------------------------------------------------*/

static void usage(char * pname)
{
	int 	i ;

    printf(
	"\n\n"
    "*** %s\n"
    "\n"
    "use: %s [search] [display]\n"
	"\n"
    "Search parameters are:\n"
	"\n", prog_desc, pname);
	printf(
	"\tsearch the closest star to a given point:\n"
	"\t-p or --pos 'HH MM SS DD MM SS' to provide RA & DEC or\n"
	"\t-p or --pos 'DD DD' to provide them in degrees\n"
	"\n");
	printf(
	"\tsearch around a given point:\n"
	"\tspecify a position, and a radius with:\n"
	"\t-r or --radius <value> to specify a radius in degrees\n"
	"\n");
	printf(
	"\tsearch on names with a regular expression:\n"
	"\t-n or --name <expr>\n"
	"\n");
	printf(
	"\tsearch on the magnitude in a band:\n"
	"\t-m or --mag 'band min max'\n"
	"\tsupported bands are H J K Ks L M Lp Mp\n"
	"\n");
	printf(
	"Display options\n"
	"\n"
	"\t-k or --key to get a keyword type output\n"
    "\n");
	printf(
	"Catalogs to be searched (default is all catalogs)\n"
	"\t-c <name1> -c <name2> ... -c <namei>\n"
	"\n"
	);

	printf("Supported catalogs are:\n\n");
    for (i=0 ; catalog_names[i] ; i++) {
		printf("\t%s\n", catalog_names[i]);
	}
	printf("\n\n");
    exit(0) ;
}

#define DISP_ONELINE_FMT \
			"%10s" \
			"\t%02d:%02d:%02d (%.2f)" \
			" %c%02d:%02d:%02d (%.2f)" \
			"\t%s" \
			"\t%d" \
			"\t%g" \
			"\t%g" \
			"\t%g" \
			"\t%g" \
			"\t%g" \
			"\t%g" \
			"\t%g" \
			"\t%g" \
			"\t%s" \
			"\n"

#define DISP_KEY_FMT \
			"NAME        = %s\n" \
			"RA          = %02d:%02d:%02d (%.2f)\n" \
			"DEC         = %c%02d:%02d:%02d (%.2f)\n" \
			"SPTYPE      = %s\n" \
			"TEMPERATURE = %d\n" \
			"MAG_J       = %g\n" \
			"MAG_H       = %g\n" \
			"MAG_K       = %g\n" \
			"MAG_Ks      = %g\n" \
			"MAG_L       = %g\n" \
			"MAG_M       = %g\n" \
			"MAG_Lp      = %g\n" \
			"MAG_Mp      = %g\n" \
			"CATALOG     = %s\n"

void irstd_display_result(
        irstd		**	starlist,
        int				nfound,
        int				key_print)
{
	irstd * found ;
	int		temperature ;
	int		i ;
	int		ra[3] ;
	int		dec[3] ;
	char	dec_sign ;

	if (starlist==NULL || nfound<1) return ;

	for (i=0 ; i<nfound ; i++) {
		found = starlist[i] ;
		temperature = irstd_get_star_temperature((char*)found->sptype) ;
		ra_conv(found->ra, &(ra[0]), &(ra[1]), &(ra[2]));
		dec_conv(found->dec, &dec_sign, &(dec[0]), &(dec[1]), &(dec[2]));
		printf(
		key_print ? DISP_KEY_FMT : DISP_ONELINE_FMT,
		found->name,
		ra[0], ra[1], ra[2], found->ra,
		dec_sign, dec[0], dec[1], dec[2], found->dec,
		found->sptype,
		temperature,
		found->mag_J,
		found->mag_H,
		found->mag_K,
		found->mag_Ks,
		found->mag_L,
		found->mag_M,
		found->mag_Lp,
		found->mag_Mp,
		catalog_names[found->source]);
	}
	return ;
}

