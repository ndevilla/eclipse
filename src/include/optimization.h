/*----------------------------------------------------------------------------*/
/**
   @file    optimization.h
   @author  Y. Jung
   @date    Feb 2003
   @version $Revision: 1.2 $
   @brief   Optimization routines
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: optimization.h,v 1.2 2003/02/28 08:36:01 yjung Exp $
    $Author: yjung $
    $Date: 2003/02/28 08:36:01 $
    $Revision: 1.2 $
*/

#ifndef _OPTIMIZATION_H_
#define _OPTIMIZATION_H_

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "xmemory.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define     MAX_NB_ITER     5000

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Minimize a functions with several variables
  @param    x_est   nb_dim+1 initial vectors
  @param    delta_max   quality requested for convergence
  @param    func2min    function to minimize
  @param    pts_list    Anchor points
  @param    neval       nb of function evaluation
  @return   0 if ok, -1 otherwise 
 */
/*----------------------------------------------------------------------------*/
int minimize(
        double  **  x_est,
        int         nb_dim,
        double      delta_max,
        double (*func2min)(double *, double3 *),
        double3 *   pts_list,
        int     *   neval) ;

#endif
