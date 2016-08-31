/*----------------------------------------------------------------------------*/
/**
   @file    encircl.c
   @author  Christian Drouet d'Aubigny
   @date    21 Oct 1996
   @version	$Revision: 1.25 $
   @brief   compute the rad. corresponding to a percentage of encircled energy
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: encircl.c,v 1.25 2002/11/19 15:07:47 yjung Exp $
	$Author: yjung $
	$Date: 2002/11/19 15:07:47 $
	$Revision: 1.25 $
 */

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "eclipse.h"

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

static double get_radius_on_image(image_t *, int, int, int, double, double,int);
static void usage(char *pname) ;
static char prog_desc[] = "radius for given percentage encircled energy" ;

/*-----------------------------------------------------------------------------
  							    Main 
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	cube_t		*	cube_in ;
	char			inname[FILENAMESZ+1] ;
	int				c ;
  
	/* Parameters to fwhm computation   */
	int				np ;
	char	*		op_scal ;
	double			platescale ;
	int				x_expect,
					y_expect,
					half_size ;
	double			total_radius ;
	int				percent ;
	double			radius ;
	char			format[200] ;
	int				tot,
  					dec ;

	/* Initialize variables */
	platescale = -1 ;
	total_radius = 1.4 ;
	half_size = 20 ;
  
	if (argc<2) usage(argv[0]) ;
	  
	/* Command line parsing by getopt() */
	while ((c = getopt(argc, argv, "Lh:p:r:")) != EOF)
        switch(c) { 
		    /* Standard option: display license (not documented in usage)   */
		    case 'L':
		        eclipse_display_license() ;
		        return 0 ;
		    /* Special options : plate scale */
		    case 'p':
		        platescale = (double) atof (optarg) ;
		        break ; 
		    /* Special option : 100 % radius size (arc-sec) */ 
            case 'r':
                total_radius = (double) atof (optarg) ;
                break ;
            /* Special option: modify half size defaulted to 20 pix */
            case 'h':
                half_size = (int) atoi (optarg) ;
                break ;
            default:
                usage(argv[0]) ;
                break ;
        }

	/* Initialize eclipse environment */
	eclipse_init();
 
	/* Get arguments    */
	if ((argc-optind)!=4) {
		e_error("invalid number of arguments") ;
		return -1 ;
	}
  
	strncpy(inname, argv[optind++], FILENAMESZ) ;
	x_expect = (int) atoi (argv[optind++]) ;
	y_expect = (int) atoi (argv[optind++]) ;
	percent  = (int) atoi (argv[optind]);

	/* Load the cube and check for errors */
	if (platescale < 0){
		op_scal = qfits_query_hdr(inname, "OP_SCAL");
		if (op_scal==NULL) {
			e_error("cannot find OP_SCAL in header");
			return -1 ;
		}
	}
  
	if ((cube_in = cube_load(inname)) == NULL) {
		e_error("in loading cube [%s]: aborting", inname) ;
		return -1 ;
	}
  
	for (np=0 ; np<cube_in->np ; np++) { 
		/* working on user coordinate format 1,2,3...n and then calling */
		/*  routines in 0,1,2...n-1 eclipse format */    
		if ((radius = get_radius_on_image(cube_in->plane[np],
									 x_expect,
									 y_expect,
									 half_size,
									 platescale,
									 total_radius,
									 percent)) == -1) {
			e_error("cannot compute radius");
			return -1;
		}
		if (platescale < 1) {
			dec = 2 - (int) (log10(platescale)) ;
			tot = 2 + dec ;
		} else if (log10(platescale) < 1) {
   			dec = 1  ;
   		} else {
   			dec = 0  ;
   		}
   		tot = 2 + (int) (log10(platescale)) ;

		sprintf(format, "radius for %d percent is: %%%d.%df arc-sec\n", 
                percent, tot,dec) ;
		printf(format, radius) ;
	}
	cube_del(cube_in) ;
  
	if (debug_active()) xmemory_status() ;
	return 0 ;
}

/*-----------------------------------------------------------------------------
   							Function code
 -----------------------------------------------------------------------------*/

static void usage(char *pname)
{
	hello_world(pname, prog_desc) ;
	printf(
"use : %s [options] incube x_expect y_expect percent \n"
"options are :\n"
"\t[-h halfsize] sets confidence window half size\n"
"\t[-r radius] to override the 1.4 arc-sec hundred percent energy radius \n"
"\t[-p plate_scale] in arc-sec per pixel (overrides FITS header)\n"
"\n\n", pname) ;
    exit(0) ;
}

 
/*----------------------------------------------------------------------------*/
/**
  @brief	computes the rad. of the encircled energy
  @param    image_in    input image
  @param    x_expect    X coordinate of the expected peak
  @param    y_expect    Y coordinate of the expected peak
  @param    half_size   half size of zone containing peak
  @param    plate_scale plate scale in arc second per pixel
  @param    totalradius 100% Energy radius in arc seconds
  @param    percent     percentage of energy for which the radius is wanted
  @return   radius, -1 in error case
 */
/*----------------------------------------------------------------------------*/
static double get_radius_on_image(
        image_t    *   image_in,
        int             x_expect,
        int             y_expect,
        int             half_size,
        double          plate_scale,
        double          totalradius,
        int             percent)
{
    double          radius ;
    double          energy ;
    double          energy_percent ;
    int             radius_pixel ;
    double      *   energy_array ;
    image_t     *   sub_image ;
    int             x_min, 
                    x_max, 
                    y_min, 
                    y_max ;
    image_stats *   sub_ima_stats ;
    int             i ;
    int             x1, 
                    x2 ;
    double          y1, 
                    y2 ;
    int			    value ;
    int             x, 
                    y ;
 
    /* Initialize */
    value = 0 ;
 
    /* Test entries */
    if (image_in == NULL) return -1.0 ;
 
    /* Check that the peak position estimate is in the frame */
    if ((x_expect < 1) || (x_expect > image_in->lx) ||
            (y_expect < 1) || (y_expect > image_in->ly)) {
        e_error("peak estimate out of frame: [%d %d]", x_expect, y_expect) ;
        return -1.0 ;
    }
  
    /* Define the expectation window */
    x_min = x_expect - half_size ;
    y_min = y_expect - half_size ;
    x_max = x_expect + half_size ;
    y_max = y_expect + half_size ;
    if(x_min < 1) x_min = 1 ;
    if(y_min < 1) y_min = 1 ;
    if(x_max > image_in->lx) x_max = image_in->lx ;
    if(y_max > image_in->ly) y_max = image_in->ly ;
  
    /* Extracts subframe */
    sub_image = image_getvig(image_in,x_min,y_min,x_max,y_max) ;
  
    /* Do some stats on that subimage */
    sub_ima_stats = image_getstats(sub_image) ;
    image_del(sub_image);
  
    /* get the position of the peak and check for positivity */
    if (sub_ima_stats->min_pix < 0) {
        e_error("some pixels have neg. value (%g)", sub_ima_stats->min_pix) ;
        return -1.0;
    }
  
    x = sub_ima_stats->max_x + x_min ;
    y = sub_ima_stats->max_y + y_min ;
    free(sub_ima_stats) ;
  
    /* computes the 100 % radius in pixels */
    if (plate_scale < 1e-7) {
        e_error("plate scale too small: %g arcsec/pix", plate_scale);
        return -1.0;
    }
    radius_pixel = (int)(totalradius/plate_scale)  ;
    if (radius_pixel == 0) {
        e_error("total radius in pixel is 0...");
        return -1.0;
    }
    /* Extract a new re-centered re scaled sub image */
    x_min = x - radius_pixel ;
    y_min = y - radius_pixel ;
    x_max = x + radius_pixel ;
    y_max = y + radius_pixel ;
  
    if ((sub_image = image_getvig(image_in,x_min,y_min,x_max,y_max)) == NULL) {
        e_error("100%% energy radius window out of image\n");
        return -1.0;
    }
  
    sub_ima_stats = image_getstats(sub_image);
    x = sub_ima_stats->max_x ;
    y = sub_ima_stats->max_y ;
    free(sub_ima_stats);
  
    /* computes the 100 % energy  */
    energy = image_get_radenergy(sub_image, x, y, radius_pixel);
  
    /* computes the value of x % of the total encircled energy */
    energy_percent = energy * percent / 100 ;
  
    /* reserve a double array to hold the encircled  energy values */
    energy_array = malloc(radius_pixel * sizeof(double));

    /* fills up the array with encicled energies */
    for(i=0 ; i < radius_pixel ; i++){
        energy_array[i] = image_get_radenergy(sub_image, x, y, i+1) ;
    }
    image_del(sub_image) ;
  
    /* finds where the encicled energy equals energy_percent */
    if (energy_array[0] > energy_percent) {
        e_warning("extrapolating. Can be really inaccurate") ;
        if (imstat_x_for_y_between_2_points(1, energy_array[0], 2, 
                    energy_array[1], energy_percent, &radius) != 0) {
            e_error("in extrapolation: aborting") ;
            return -1.0 ;
        }
        if (radius <= 0) {
            e_error("extrapolation returned neg. value, percentage too small") ;
            return -1.0 ;
        }
        return radius * plate_scale ;
    } else {
        x1 = 1 ;
        x2 = 2 ;
        y1 = energy_array[0] ;
        y2 = energy_array[1] ;
        for(i=3 ; i<radius_pixel ; i++) {
            if (!value) {
                if (y2 > energy_percent) {
                    value = 1 ;
                } else {
                    x1++ ;
                    x2++ ;
                    y1 = y2 ;
                    y2 = energy_array[i];
                }
            }
        }
    }
    free(energy_array);
  
    /* Given two points around energy_percent get subpixel precision */
    if (imstat_x_for_y_between_2_points(x1, y1, x2, y2, energy_percent,
                &radius) != 0) {
        e_error("in interpolation: aborting") ;
        return -1.0 ;
    }
    return radius * plate_scale ;
}




