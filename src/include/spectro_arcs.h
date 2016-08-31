/*-------------------------------------------------------------------------*/
/**
   @file    spectro_arcs.h
   @author  T.Rogon
   @date    October 1999
   @version $Revision: 1.15 $
   @brief   spectroscopy routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: spectro_arcs.h,v 1.15 2003/05/15 07:44:30 yjung Exp $
    $Author: yjung $
    $Date: 2003/05/15 07:44:30 $
    $Revision: 1.15 $
*/

#ifndef _SPECTRO_ARCS_H_
#define _SPECTRO_ARCS_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "eclipse.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define ARC_MEDIAN_SIZE				64
#define ARC_THRESHFACT				(1.0/3.0)
#define ARC_WINDOWSIZE				32
#define ARC_MINNBARCS				4
#define ARC_MINGOODPIX				100
#define ARC_NBSAMPLES				20
#define ARC_MINARCLENFACT			2.0
#define ARC_MAXARCWIDTH				33
#define ARC_GRID_REF_1DMAX			1000
#define ARC_GRID_REF_GRAV_CENT		1001

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/**
  @brief    Computes the inverse dist polynomium of an image containing arcs
  @param    in  grey level image containing arcs i.e deformed parallel lines 
  @param    xmin
  @param    ymin        the region to consider
  @param    xmax
  @param    ymax
  @param    arc_sat Arc saturation value
  @param    nb_arcs Nb of arcs used for calibration
  @param    arcs    Arcs positions
  @return a 2-D polynomial poly2d

  Computes the inverse distortion polynomium c, x, y, xy, x^2, y^2 in order 
  of appearance.
  C-style convention for coordinates is used, i.e.  top,left=0,0.
  The input image is assumed dark subtracted.
  Arcs are assumed vertical
 */
/*---------------------------------------------------------------------------*/
poly2d * compute_distortion(
        image_t     *   in,
        int                 xmin,
        int                 ymin,
        int                 xmax,
        int                 ymax,
        int                 arc_sat,
        int             *   nb_arcs,
        double          **  arcs) ;


/*---------------------------------------------------------------------------*/
/**
  @brief    This is the low level function called by compute_distortion
  @param    in          the arc image
  @param    xmin
  @param    ymin        the region to consider
  @param    xmax
  @param    ymax
  @param    arc_sat Arc saturation value
  @param    nb_arcs Nb of arcs used for calibration
  @param    arcs    Arcs positions
  @return   a 2-D polynomial of size 6 double
    
  Input image is assumed dark subtracted
  Arcs are assumed vertical
*/
/*---------------------------------------------------------------------------*/
double * dist_engine(
        image_t         *   org,
        int                 xmin,
        int                 ymin,
        int                 xmax,
        int                 ymax,
        int                 arc_sat,
        int             *   nb_arcs,
        double          **  arcs) ;

#endif
