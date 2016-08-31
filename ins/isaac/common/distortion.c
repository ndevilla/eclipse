/*----------------------------------------------------------------------------*/
/**
   @file    distortion.c
   @author  Y. Jung
   @date    Feb 2001
   @version	$Revision: 1.29 $
   @brief   ISAAC distortion utilities 
*/
/*----------------------------------------------------------------------------*/

/*
   $Id: distortion.c,v 1.29 2004/12/01 14:47:32 yjung Exp $
   $Author: yjung $
   $Date: 2004/12/01 14:47:32 $
   $Revision: 1.29 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "isaacp_lib.h"
#include "distortion.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define SQR(x) ((x)*(x))

/*-----------------------------------------------------------------------------
                                Function prototypes
 -----------------------------------------------------------------------------*/

static int isaac_detect_dark_ramp(image_t * in, double * slope) ;
static int isaac_level_dark( double    slope, image_t *in) ;

/*-----------------------------------------------------------------------------
						Function ANSI C code 
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the distortion
  @param    in  input image
  @param	xmin
  @param	ymin	The region of interest
  @param	xmax
  @param	ymax
  @param	auto_dark_sub	Flag to automatically subtract the dark
  @param	arcs	array with arcs positions
  @return   a 2-D polynomial of size 6 double
  
  This function is ISAAC-specific. It attempts to detect a dark ramp and
  subtract it if found. See compute_distortion for a generic version.
 */
/*----------------------------------------------------------------------------*/
poly2d * isaac_compute_distortion(
		image_t    *   in,
		int				xmin,
		int				ymin,
		int				xmax,
		int				ymax,
		int				auto_dark_sub,
		int			*	nb_arcs,
		double		**	arcs)
{
    poly2d      	*	poly_u ;
    int         		err=0;
    int					dark_ramp_present ;
    double      		rampslope ;
    image_t    		* 	loc ;
    int                 arc_sat ;

	/* Initialize */
	poly_u = NULL ;
	dark_ramp_present = 0 ;
	rampslope = 0.0 ;
    arc_sat = ISAAC_ARC_SATURATION ;
	
	/* Local copy */
    loc = image_copy(in) ;

	if (auto_dark_sub == 1) {
    	dark_ramp_present = isaac_detect_dark_ramp(loc, &rampslope);
    	if (dark_ramp_present)
    	    err=isaac_level_dark(rampslope, loc);
    	if (!err)
    	    poly_u=compute_distortion(loc, xmin, ymin, xmax, ymax, arc_sat,
					nb_arcs, arcs) ;
	} else if (auto_dark_sub == 0) {
		poly_u=compute_distortion(loc, xmin, ymin, xmax, ymax, arc_sat, 
                nb_arcs, arcs) ;
	}

	/* Free and return */
	image_del(loc) ;
    return poly_u;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect a dark ramp in an image
  @param    in  input image 
  @param    slope slope 
  @return   a flag signalling whether the ramp has been detected or not
  
  If the the ramp in the input image is found either the
  dark image should be subtracted or (if no dark is available)
  level_dark() may be used. By convention lo has lower y values than
  hi.
 */
/*----------------------------------------------------------------------------*/
#define IS_NB_TESTPOINTS    8
#define IS_MIN_SLOPE        0.01
#define IS_MAX_SLOPE_DIF    0.075
#define IS_MAX_FIT_EDGE_DIF 0.05
#define IS_MIN_RAMP         10.0
#define IS_MAX_MNERR        13.0
#define IS_MAX_MNERR_DIF    8.0
#define IS_MAX_INTER_DIF    20.0
#define IS_SKIPZONE         2.5
static int isaac_detect_dark_ramp(image_t * in, double * slope)
{
    int         	y,
					yhi,
					ylo;
    pixelvalue  *	tmpline ;
	double3		*	testpointlo,
				*	testpointhi ;
    int         	i,
					spacing;
    double      	rampdif,
					fitslope;
    double      *	pol_coefhi,
				*	pol_coeflo ;
    double      	median[IS_NB_TESTPOINTS],
					medianerrlo,
					medianerrhi;

    if (slope==NULL || in==NULL) return 0;

    if (in->ly<IS_SKIPZONE*IS_NB_TESTPOINTS){
        e_error("detect_dark_ramp: image has %d lines, min=%d ",
                in->ly,(int)(IS_SKIPZONE*IS_NB_TESTPOINTS*2));
        return 0;
    }

    *slope=0.0;
    tmpline=malloc(in->lx*sizeof(pixelvalue));
    spacing= in->ly/(IS_SKIPZONE*IS_NB_TESTPOINTS);
    yhi= in->ly/2;
    ylo= yhi-1;
	testpointhi = double3_new(IS_NB_TESTPOINTS);
	testpointlo = double3_new(IS_NB_TESTPOINTS);

    for (i=0; i<IS_NB_TESTPOINTS; i++){
        y = yhi + i * spacing;
        memcpy(tmpline,&(in->data[y*in->lx]), in->lx*sizeof(pixelvalue));
        testpointhi->y[i] = median_pixelvalue(tmpline, in->lx);
        testpointhi->x[i] = y - in->ly/2;
        y = ylo - i * spacing;
        memcpy(tmpline,&(in->data[y*in->lx]), in->lx*sizeof(pixelvalue));
        testpointlo->y[IS_NB_TESTPOINTS-i-1]=median_pixelvalue(tmpline, in->lx);
        testpointlo->x[IS_NB_TESTPOINTS-i-1]=y;
    }
    free(tmpline);

    pol_coefhi = fit_slope_robust(testpointhi);
    pol_coeflo = fit_slope_robust(testpointlo);

    for (i=0; i<IS_NB_TESTPOINTS; i++) {
        median[i]=SQR(testpointhi->y[i]
                - pol_coefhi[0] - pol_coefhi[1] * testpointhi->x[i]);
	}

    medianerrhi = double_median(median,IS_NB_TESTPOINTS);
    for (i=0; i<IS_NB_TESTPOINTS; i++) {
        median[i]=SQR(testpointlo->y[i]
                - pol_coeflo[0] - pol_coeflo[1] * testpointlo->x[i]);
	}
    medianerrlo = double_median(median,IS_NB_TESTPOINTS);
    rampdif = testpointlo->y[IS_NB_TESTPOINTS-1] - testpointhi->y[0];

	double3_del(testpointlo);
	double3_del(testpointhi);

    if (fabs(rampdif)<IS_MIN_RAMP) {
        free(pol_coeflo);
        free(pol_coefhi);
        return 0;
    }
	
    if ((fabs(pol_coefhi[1]) < IS_MIN_SLOPE) ||
        (fabs(pol_coeflo[1]) < IS_MIN_SLOPE) ||
        (pol_coefhi[1]/pol_coeflo[1]<0.5)   ||
        (pol_coefhi[1]/pol_coeflo[1]>2.0)   ||
        (fabs(pol_coefhi[1]-pol_coeflo[1])>IS_MAX_SLOPE_DIF)) {
        free(pol_coeflo);
        free(pol_coefhi);
        return 0;
    }
    if (fabs(pol_coefhi[0]-pol_coeflo[0]) > IS_MAX_INTER_DIF){
        free(pol_coeflo);
        free(pol_coefhi);
        return 0;
    }
    if ((medianerrlo> IS_MAX_MNERR) || (medianerrhi> IS_MAX_MNERR) ||
        (fabs(medianerrlo-medianerrhi) >IS_MAX_MNERR_DIF)){
        free(pol_coeflo);
        free(pol_coefhi);
        return 0;
    }
    /* the ramp is most precisely defined at the center, include ramdif 
       in final estimate with equal weight as the two fit estimates */
    fitslope=(pol_coefhi[1]+pol_coeflo[1])/2.0;
    *slope=rampdif/(in->ly/2.0);
    if ((fabs(*slope-fitslope) > IS_MAX_FIT_EDGE_DIF) ||
        (*slope/fitslope<0.5)   ||
        (*slope/fitslope>2.0)){
        free(pol_coeflo);
        free(pol_coefhi);
        return 0;
    }
    free(pol_coeflo);
    free(pol_coefhi);
    return 1;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Subtract a first order model of the dark current
  @param    slope   slope
  @param    in  Image with two centered ramps in Y direction
  @return   0 if OK -1 if not.
  
  The ramp subtracted image is written on disk
 */
/*----------------------------------------------------------------------------*/
static int isaac_level_dark( double    slope, image_t *in)
{
    int         i,j;
    pixelvalue  val;

    if ((in==NULL)){
        return -1;
    }
    for (j=0;j<in->ly/2;j++){
        val = slope * (j-in->ly/2) ;
        for (i=0;i<in->lx;i++)
            in->data[i+j*in->lx] -= val;
    }
    for (j=in->ly/2;j<in->ly;j++){
        val = slope * (j-in->ly);
        for (i=0;i<in->lx;i++)
            in->data[i+j*in->lx] -= val;
    }
    return 0;
}

