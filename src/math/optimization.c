/*----------------------------------------------------------------------------*/
/**
   @file	optimization.c
   @author	Y. Jung
   @date	Feb 2003
   @version	$Revision: 1.3 $
   @brief	Optimization methods
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: optimization.c,v 1.3 2003/10/24 11:32:18 yjung Exp $
	$Author: yjung $
	$Date: 2003/10/24 11:32:18 $
	$Revision: 1.3 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include "matrix.h"
#include "doubles.h"
#include "optimization.h"
#include "comm.h"

/*-----------------------------------------------------------------------------
   								Functions prototypes
 -----------------------------------------------------------------------------*/

static double min_test(
        double  **  x,
        double  *   y,
        double  *   x_sum,
        int         nb_dim,
        double (*func2min)(double *, double3 *),
        double3 *   pts_list,
        int         hi1_ind,
        double      factor) ;

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
        int     *   neval)
{
    double  *   y_est ;
    double  *   x_sum ;
    double      sum ;
    int         low_ind , 
                hi1_ind, 
                hi2_ind ;
    int         i, j ;
        
    double      delta, 
                y_test, 
                y_tmp ;

    /* Initialize */
    *neval = 0 ;

    /* Create and initialize y_est */
    y_est = malloc((nb_dim+1) * sizeof(double)) ;
    for (i=0 ; i<nb_dim+1 ; i++) y_est[i] = (*func2min)(x_est[i], pts_list) ;
    
    /* Create and initialize x_sum */
    x_sum = malloc(nb_dim * sizeof(double)) ;
    for (i=0 ; i<nb_dim ; i++) {
        sum = 0.0 ;
        for (j=0 ; j<nb_dim+1 ; j++) sum += x_est[j][i] ;
        x_sum[i] = sum ;
    }
    
    for (;;) {
        /* Find the lowest, highest and second highest values indexes */
        low_ind = 0 ;
        if (y_est[0] > y_est[1]) {
            hi1_ind = 0 ;
            hi2_ind = 1 ;
        } else {
            hi1_ind = 1 ;
            hi2_ind = 0 ;
        }
        for (i=0 ; i<nb_dim+1 ; i++) {
            if (y_est[i] < y_est[low_ind]) low_ind = i ;
            if (y_est[i] > y_est[hi1_ind]) {
                hi2_ind = hi1_ind ;
                hi1_ind = i ;
            } else if ((y_est[i] > y_est[hi2_ind]) && (i != hi1_ind)) hi2_ind=i;
        }

        /* Check the quality and stop if ok */
        delta = 2.0 * fabs(y_est[hi1_ind]-y_est[low_ind]) /
            (fabs(y_est[hi1_ind])+fabs(y_est[low_ind])+ 1.0e-10) ;
        if (delta < delta_max) {
            /* Quality reached */
            free(x_sum) ;
            free(y_est) ;
            e_comment(0, "Ok after %d iterations", *neval) ;
            return 0 ; 
        } else if (*neval >= MAX_NB_ITER) { 
            /* Check if neval still ok */
            free(x_sum) ;
            free(y_est) ;
            e_error("too many iterations") ;
            return -1 ;
        }
       
        /* Update neval */
        *neval += 2 ;

        /* Begin a new iteration */
        y_test = min_test(x_est, y_est, x_sum, nb_dim, func2min, pts_list, 
                hi1_ind, -1.0);
        if (y_test < y_est[low_ind]) {
            /* Better than the best, try additional extrapolation fac 2.0 */
            y_test=min_test(x_est, y_est, x_sum, nb_dim, func2min, pts_list,
                    hi1_ind,2.0);
        } else if (y_test > y_est[hi2_ind]) {
            /* The reflected point is worse than the second highest, look for */
            /* an intermediate lower pt (1d contraction) */
            y_tmp = y_est[hi1_ind] ;
            y_test=min_test(x_est, y_est, x_sum, nb_dim, func2min, pts_list,
                    hi1_ind,0.5);
            if (y_tmp < y_test) {
                /* This bad point does not want to improve */
                /* Contract around the lowest (best) point */
                for (i=0 ; i<nb_dim+1 ; i++) {
                    if (i != low_ind) {
                        for (j=0 ; j<nb_dim ; j++) {
                            x_est[i][j] = x_sum[j] = 
                                0.5 *(x_est[i][j] + x_est[low_ind][j]) ; 
                            y_est[i] = (*func2min)(x_sum, pts_list);
                        }
                    }
                }
                *neval += nb_dim ;
        
                /* Update x_sum */
                for (i=0 ; i<nb_dim ; i++) {
                    sum = 0.0 ;
                    for (j=0 ; j<nb_dim+1 ; j++) sum += x_est[j][i] ;
                    x_sum[i] = sum ;
                }
            }
        } else --(*neval) ;
    }
}
        
static double min_test(
        double  **  x,
        double  *   y,
        double  *   x_sum,
        int         nb_dim,
        double (*func2min)(double *, double3 *),
        double3 *   pts_list,
        int         hi1_ind,
        double      factor)
{
    double      *   x_test ;
    double          y_test ;
    double          factor1,
                    factor2 ;
    int             i ;

    /* Create and initialize x_test */
    x_test = malloc(nb_dim * sizeof(double)) ;
    factor1 = (1.0 - factor) / nb_dim ;
    factor2 = factor1 - factor ;
    for (i=0 ; i<nb_dim ; i++) 
        x_test[i] = x_sum[i]*factor1 - x[hi1_ind][i]*factor2 ;

    /* Apply the function on it */
    y_test = (*func2min)(x_test, pts_list);
    
    if (y_test < y[hi1_ind]) {
        y[hi1_ind] = y_test ;
        for (i=0 ; i<nb_dim ; i++) {
            x_sum[i] += x_test[i]-x[hi1_ind][i] ;
            x[hi1_ind][i] = x_test[i] ;
        }
    }
    free(x_test) ;
    return y_test ;
} 


