/*----------------------------------------------------------------------------*/
/**
   @file    spjclassif.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.3 $
   @brief   Spectroscopic jitter data classification
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjclassif.c,v 1.3 2004/04/27 13:45:54 yjung Exp $
	$Author: yjung $
	$Date: 2004/04/27 13:45:54 $
	$Revision: 1.3 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Private functions
 -----------------------------------------------------------------------------*/

static int spjitter_classif_engine(spjitter_config_t *, double) ;
static int off_comp(double off1, double off2, double thresh) ;
static int double_sort(const void * d1, const void * d2) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the spectroscopic jitter input data classification
  @param    spjc    sjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_classif(spjitter_config_t * spjc)
{
    double      *   sorted ;
    double          offset_min,
                    offset_max,
                    offset_thresh ;
    int             nb_offsets ;
    double          currentoff ;

    int             i, j ;

    /* Find the number of different offsets, the min and the max */
    /* Sort the type_obj offsets */
    sorted = malloc(spjc->nobjframes*sizeof(double)) ;
    j = 0 ;
    for (i=0 ; i<spjc->nframes ; i++) {
        if (spjc->frame[i].type == type_obj) {
            sorted[j] = spjc->frame[i].offset ;
            j++ ;
        }
    }
    qsort(sorted, spjc->nobjframes, sizeof(double), double_sort) ;
    
    /* Count the different offsets */
    nb_offsets = 1 ;
    currentoff = sorted[0] ;
    for (i=1 ; i<spjc->nobjframes ; i++) {
        if (currentoff != sorted[i]) {
            nb_offsets++ ;
            currentoff = sorted[i] ;
        }
    }
    offset_min = sorted[0] ;
    offset_max = sorted[spjc->nobjframes-1] ;
    offset_thresh = (offset_min + offset_max) / 2.0 ;
    free(sorted) ;

    /* Separate the offset total list in offset lists */
    /* Write the parameters in the blackboard */
    if (nb_offsets < 2) {
        e_error("not enough different offsets: [%d]", nb_offsets) ;
        spjc->status_classification = FAILED ;
        return -1 ;
    } else {
        /* Classify the frames according their position in the list */
        if (spjitter_classif_engine(spjc, offset_thresh) == -1) {
            e_error("cannot classify the frames") ;
            spjc->status_classification = FAILED ;
            return -1 ;
        }
        if (spjc->nb_classified_cubes <= 0) {
            e_error("ABBA sequence not recognised") ;
            spjc->status_classification = FAILED ;
            return -1 ;
        } else {
            e_comment(1,"Nb of classified cubes: %d\n", 
                    spjc->nb_classified_cubes);
            spjc->status_classification = OK ;
        }
    }

    /* Return */
    return 0 ;

}

/*----------------------------------------------------------------------------*/
/**
  @brief    match an offsets sequence with a defined pattern    
  @param    spjc        spjitter_config_t object
  @param    threshold   offsets threshold
  @return   int 0 if ok -1 otherwise
  Compares the offsets to a threshold and return a number of
  batches according to the offsets positions. A sequence <<>> gives 2
  batches (<< and >>).  A sequence >>><<<>><< gives 4 batches (>>>,
  <<<, >>, <<). X '>' (resp '<') has to be followed by X (at least)
  '<' (resp. '>').
 */ 
/*----------------------------------------------------------------------------*/
static int spjitter_classif_engine(
        spjitter_config_t   *   spjc,
        double                  threshold)
{
    int     *   obj ;
    double  *   off ;
    int         last_cube ;
    
    int     i,    /* Current frame id */
            j,    /* nb of frames in the first cube */
            k,    /* nb of frames in the second cube */
            l ;
    
    /* Initialize */
    spjc->nb_classified_cubes = 0 ;

    /* Create a look up table to associate the ith obj with the jth frame */
    obj = malloc(spjc->nobjframes * sizeof(int)) ;
    off = malloc(spjc->nobjframes * sizeof(double)) ;
    j = 0 ;
    for (i=0 ; i<spjc->nframes ; i++) {
        if (spjc->frame[i].type == type_obj) {
            obj[j] = i ;
            off[j] = spjc->frame[i].offset ;
            j++ ;
        }
    }
            
    i = 0 ;
    while (i < spjc->nobjframes) {
        j = 0 ;
        /* Count the number of successive '+' or '-' (j) */
        while ((i+j<spjc->nobjframes) && 
                (!off_comp(off[i], off[i+j], threshold))) j++ ;

        if (i+j >= spjc->nobjframes) i = spjc->nobjframes ;
        else {
            k = 0 ;
            /* Check if there are j '-' or '+' (k) */
            while ((i+j+k<spjc->nobjframes)
                    && (!off_comp(off[i+j], off[i+j+k], threshold))
                    && (k<j)) k++ ;
            last_cube = 1 ;
            if (i+j+k < spjc->nobjframes) {
                for (l=i+j+k ; l<spjc->nobjframes ; l++) {
                    if (off_comp(off[i+j], off[l], threshold)) {
                        last_cube = 0 ;
                        break ;
                    }
                }
            }

            if (last_cube == 0) {
                for (l=0 ; l<j ; l++) 
                    spjc->frame[obj[i+l]].cube_id = spjc->nb_classified_cubes+1;
                for (l=0 ; l<k ; l++) 
                    spjc->frame[obj[i+j+l]].cube_id=spjc->nb_classified_cubes+2;
                spjc->nb_classified_cubes += 2 ;
                i += j+k ;
            } else {
                for (l=0 ; l<j ; l++) 
                    spjc->frame[obj[i+l]].cube_id = spjc->nb_classified_cubes+1;
                for (l=0 ; l<spjc->nobjframes - (i+j) ; l++)
                    spjc->frame[obj[i+j+l]].cube_id=spjc->nb_classified_cubes+2;
                spjc->nb_classified_cubes +=2 ;
                i = spjc->nobjframes ;
            }
        }
    }
    free(off) ;
    free(obj) ;

    /* Nb of cubes found should be even */
    if (spjc->nb_classified_cubes % 2) {
        spjc->nb_classified_cubes = 0 ;
        return -1 ;
    }
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    compares two doubles in regard to a given threshold
  @param    off1 first double
  @param    off2 second double
  @param    thresh  threshold
  @return   1 if threshold is between offsets, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int off_comp(double off1, double off2, double thresh)
{
    if (((off1>thresh) && (off2<thresh)) || ((off1<thresh) && (off2>thresh)))
        return 1 ;
    else return 0 ;
}
/*----------------------------------------------------------------------------*/
/**
  @brief    Compare two doubles for qsort
  @param    d1 first double
  @param    d2 second double
  @return   int
 */
/*----------------------------------------------------------------------------*/
static int double_sort(const void * d1, const void * d2)
{
    if (*(double*)d1 > *(double*)d2) return 1 ;
    else if (*(double*)d1 == *(double*)d2) return 0 ;
    else return -1 ;
}





