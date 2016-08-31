/*-------------------------------------------------------------------------*/
/**
   @file	matchpoint.c
   @author	Yves Jung
   @date	July 2001
   @version	$Revision: 1.10 $
   @brief	Points matching
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: matchpoint.c,v 1.10 2002/01/21 10:47:36 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/21 10:47:36 $
	$Revision: 1.10 $
*/

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include "matchpoint.h"
#include "image_handling.h"
#include "resampling.h"
#include "detect.h"
#include "dstats.h"
#include "pi.h"

/*----------------------------------------------------------------------------
                                Define
 ---------------------------------------------------------------------------*/

#define MIN_NB_OF_POINTS    5
#define	NB_SAMPLES			36
#define SQR(x) ((x)*(x))

/*---------------------------------------------------------------------------
                            Prototypes
 ---------------------------------------------------------------------------*/

#define	MATCHPOINT_NBOBJECTS	5

/*---------------------------------------------------------------------------
                            Prototypes
 ---------------------------------------------------------------------------*/

static double * match_computewv(double3	*, int) ;

/*---------------------------------------------------------------------------
                            Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Estimate offsets between two images 
  @param	im1	First image
  @param	im2	Second image
  @param	offsetx	returned x offset
  @param	offsety	returned y offset
  @param	kappa	for detection
  @return 	0 if ok, -1 otherwise	
 */
/*--------------------------------------------------------------------------*/
int offsets_estimates(
		image_t	*	im1,
		image_t	*	im2,
		double	*	offsetx,
		double	*	offsety,
		double		kappa)
{
	image_t		*	sub_im1 ;
	image_t		*	sub_im2 ;
	double3     *   points1 ;
    double3     *   points2 ;
    int         *   array ;
    double      *   offsetsx ;
    double      *   offsetsy ;

    int             i ;

    /* Points detection on sub-sampled first image */
    sub_im1 = image_subsample(im1) ;
    if ((points1 = detected_ks_brightest_stars(sub_im1, 
					MATCHPOINT_NBOBJECTS, 
					kappa)) == NULL) {
        e_error("cannot detect objects") ;
        image_del(sub_im1) ;
        return -1 ;
    }
    image_del(sub_im1) ;

	/* Points detection on sub-sampled first image */
    sub_im2 = image_subsample(im2) ;
    if ((points2 = detected_ks_brightest_stars(sub_im2, 
					MATCHPOINT_NBOBJECTS, 
					kappa)) == NULL) {
        e_error("cannot detect objects") ;
        image_del(sub_im2) ;
		double3_del(points1) ;
        return -1 ;
    }
    image_del(sub_im2) ;

    /* Rescale found positions */
    for (i=0 ; i<points1->n ; i++) {
         points1->x[i] *= 2 ;
         points1->y[i] *= 2 ;
    }
    for (i=0 ; i<points2->n ; i++) {
         points2->x[i] *= 2 ;
         points2->y[i] *= 2 ;
    }

    /* Associate points */
    if ((array = match_pointslist(points1, points2)) == NULL) {
        e_error("cannot match points") ;
        double3_del(points1) ;
        double3_del(points2) ;
        return -1 ;
    }

    /* Compute offsets candidates */
    offsetsx = malloc(points1->n * sizeof(double)) ;
    offsetsy = malloc(points1->n * sizeof(double)) ;

    for (i=0 ; i<points1->n ; i++) {
        offsetsx[i] = points2->x[array[i]] - points1->x[i] ;
        offsetsy[i] = points2->y[array[i]] - points1->y[i] ;
    }
    double3_del(points1) ;
    double3_del(points2) ;
    free(array) ;

    /* Compute offsets */
    *offsetx = double_median(offsetsx, points1->n) ;
    *offsety = double_median(offsetsy, points1->n) ;

    /* Free and return */
    free(offsetsx) ;
    free(offsetsy) ;
    return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Associate points from two lists of points     
  @param    det1	first list of detected points
  @param    det2	second list of detected points
  @return 	look up table
 */
/*--------------------------------------------------------------------------*/
int * match_pointslist(
        double3    *   det1,
        double3    *   det2)
{
    double      **  wv1 ;
    double      **  wv2 ;
	double		**	lkh ;
	double			lkh_var ;
	
	int			*	corres ;	
    int             i, j, k ;

    /* Test input data */
    if ((det1->n < MIN_NB_OF_POINTS) || (det2->n < MIN_NB_OF_POINTS)) {
        e_warning("not enough points for this method") ;
        return NULL ;
    }

    /* Allocate wv1 */
    wv1 = malloc(det1->n * sizeof(double *)) ;

    /* For each point in the first list, compute its world view */
    for (i=0 ; i<det1->n ; i++) wv1[i] = match_computewv(det1, i) ;

    /* Allocate wv2 */
    wv2 = malloc(det2->n * sizeof(double *)) ;

    /* For each point in the second list, compute its world view */
    for (i=0 ; i<det2->n ; i++) wv2[i] = match_computewv(det2, i) ; 

    /* For each point of the first list, compute the likelyhoods */
	lkh = malloc(det1->n * sizeof(double*)) ;
	for (i=0 ; i<det1->n ; i++) lkh[i] = malloc(det2->n * sizeof(double)) ;
	for (i=0 ; i<det1->n ; i++) {
		for (j=0 ; j<det2->n ; j++) {
			lkh_var = 0.0 ;
			for (k=0 ; k<NB_SAMPLES ; k++) lkh_var += SQR(wv1[i][k]-wv2[j][k]) ;
			lkh[i][j] = sqrt(lkh_var) ;
		}
	}
	for (i=0 ; i<det1->n ; i++) free(wv1[i]) ;
	for (i=0 ; i<det2->n ; i++) free(wv2[i]) ;
	free(wv1) ;
	free(wv2) ;

	/* Associate the stars */
    corres = malloc(det1->n * sizeof(int)) ;

    for (i=0 ; i<det1->n ; i++) {
        lkh_var = lkh[i][0] ;
        corres[i] = 0 ;
        for (j=1 ; j<det2->n ; j++) {
           if (lkh[i][j] < lkh_var) {
               lkh_var = lkh[i][j] ;
               corres[i] = j ;
           }
        }
    }

	/* Free and return */
	for (i=0 ; i<det1->n ; i++) free(lkh[i]) ;
	free(lkh) ;
	return corres ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the worlview of a point in a list of points
  @param   	points	List of points 
  @param   	ind		Reference point 
  @return 	the world view	
 */
/*--------------------------------------------------------------------------*/
static double * match_computewv(
		double3	*	points,
		int			ind)
{
	double	*	wv ;
	double		r0 ;	
	double		dist ;
	double		angle ;
	double		X, Y ;
	int			i ;
		
	wv = calloc(NB_SAMPLES, sizeof(double)) ;
	/* Compute the reference distance r0 */
	r0 = 0.0 ;
	for (i=0 ; i<points->n ; i++) {
		dist = sqrt(SQR(points->x[ind]-points->x[i])+
				SQR(points->y[ind]-points->y[i])) ;
		if (dist > r0) r0 = dist ;
	}
	/* Compute wv2 */
	for (i=0 ; i<points->n ; i++) {
		if (i==ind) continue ;
		
		/* Compute current angle */
		X = points->x[i]-points->x[ind] ;
		Y = points->y[i]-points->y[ind] ;
        
		angle = (double)(atan(Y/X)) ;
		if (X<0) angle += PI_NUMB ;
		angle *= 180.0 / PI_NUMB ;
		if (angle<0) angle += 360 ;
        
		/* Compute current distance */
		dist = sqrt(SQR(points->x[ind]-points->x[i])+
				SQR(points->y[ind]-points->y[i])) ;
        
		/* Update wv */
		wv[(int)(angle/(360/NB_SAMPLES))] += dist ;
	}
	return wv ;	
}


