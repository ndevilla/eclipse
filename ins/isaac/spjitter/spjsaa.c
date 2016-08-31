/*----------------------------------------------------------------------------*/
/**
   @file    spjsaa.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.5 $
   @brief   Spectroscopic jitter shift and add utilities
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjsaa.c,v 1.5 2003/11/18 09:37:55 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/18 09:37:55 $
	$Revision: 1.5 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "spjtypes.h"
#include "spjconfig.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define     MAX_SHIFT_ERROR     10

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

static int spjitter_differences_all(spjitter_config_t * spjc) ;
static int spjitter_differences_half(spjitter_config_t * spjc) ;
static double refine_offset(image_t *, image_t *) ;
static int spjitter_combine_differences(spjitter_config_t * spjc) ;
static int spjitter_combine_combined(spjitter_config_t * spjc) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	shift and average each classified cube to one image
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_averaging(spjitter_config_t * spjc)
{
    image_t *   shifted ;
    cube_t  *   cube ;
    image_t *   averaged;
    int     *   selection ;
    double      shift_val ;
    int         ref_id ;
    int         i, j ;

    /* SHIFT the frames if necessary */
    /* For each classified cube */
    for (i=0 ; i<spjc->nb_classified_cubes ; i++) {
        /* Find the reference image for the current cube */
        ref_id = 0 ;
        while (spjc->frame[ref_id].cube_id != i+1) ref_id++ ;
        /* Shift the other frames to align them on the reference one */
        for (j=ref_id+1 ; j<spjc->nframes ; j++) {
            if (spjc->frame[j].cube_id == i+1) {
                shift_val = spjc->frame[j].offset - spjc->frame[ref_id].offset ;
                if (fabs(shift_val) > 1e-3) { 
                    e_comment(1, "shifting frame #%02d", j) ;
                    shifted = image_shift(spjc->frame[j].image, 
                                        0,
                                        shift_val,
                                        (double*)NULL) ;
                    image_del(spjc->frame[j].image) ;
                    spjc->frame[j].image = shifted ;
                    shifted = NULL ;
                }
            }
        }
    }
    
    /* AVERAGE the frames */
    selection = malloc(spjc->nframes * sizeof(int)) ; 
    /* Loop on each cube */
    for (i=0 ; i<spjc->nb_classified_cubes ; i++) {
        /* Fill selection array */
        for (j=0 ; j<spjc->nframes ; j++) { 
            if (spjc->frame[j].cube_id == i+1) selection[j] = 1 ;
            else selection[j] = 0 ;
        }
        /* Get the current cube */
        cube = spjitter_cubeget(spjc, selection) ;
        /* Average the cube */
        averaged = cube_avg_linear(cube) ;
        /* Put the frames back into spjc  */
        spjitter_cubeput(spjc, selection, cube) ;
        cube_del_shallow(cube) ; 
        /* Frames are now averaged */
        for (j=0 ; j<spjc->nframes ; j++) 
            if (spjc->frame[j].cube_id == i+1) {
                spjc->nobjframes -- ;
                spjc->frame[j].type = type_averaged ;
            }
        /* Replace the first cube image by the averaged one */
        ref_id = 0 ;
        while (spjc->frame[ref_id].cube_id != i+1) ref_id++ ;
        image_del(spjc->frame[ref_id].image) ;
        spjc->frame[ref_id].image = averaged ;
        averaged = NULL ;
        spjc->nobjframes ++ ;
        spjc->frame[ref_id].type = type_obj ;
    }

    /* Free and return */
    free(selection) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Check the mode and call the right difference function
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_differences(spjitter_config_t * spjc)
{
    int     ret ;

    /* Check the difference method to be used */
    if (spjc->diff_method == diff_half) {
        ret = spjitter_differences_half(spjc) ;
    } else if (spjc->diff_method == diff_all) {
        ret = spjitter_differences_all(spjc) ;
    } else {
        ret = -1 ;
    }

    return ret ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    compute differences a-b and b-a for each pair ab  
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_differences_all(spjitter_config_t * spjc)
{
    image_t *   tmp_im1,
            *   tmp_im2 ;
    int         a_id, 
                b_id ;
    int         i ;
   
    /* Allocate the main offsets array */
    spjc->main_offset_diff=malloc((spjc->nb_classified_cubes/2)*sizeof(double));

    /* Loop on each pair of cube */
    for (i=0 ; i<spjc->nb_classified_cubes ; i+=2) {
        /* Get index of type_obj frames of cubes i+1 and i+2 */
        a_id = b_id = 0 ;
        while ((spjc->frame[a_id].type != type_obj) || 
               (spjc->frame[a_id].cube_id != i+1)) a_id++ ;
        while ((spjc->frame[b_id].type != type_obj) || 
               (spjc->frame[b_id].cube_id != i+2)) b_id++ ;
        
        /* Compute the offset difference */
        spjc->main_offset_diff[i/2] = spjc->frame[b_id].offset - 
                                    spjc->frame[a_id].offset ;

        /* a <- a-b and b <- b-a */
        tmp_im1 = image_sub(spjc->frame[a_id].image, spjc->frame[b_id].image);
        tmp_im2 = image_sub(spjc->frame[b_id].image, spjc->frame[a_id].image);
        if ((tmp_im1 == NULL) || (tmp_im2 == NULL)) {
            e_error("in differences computation") ;
            if (tmp_im1 != NULL) image_del(tmp_im1) ;
            if (tmp_im2 != NULL) image_del(tmp_im2) ;
            spjc->status_differences = FAILED ;
            return -1 ;
        }
        image_del(spjc->frame[a_id].image) ;
        image_del(spjc->frame[b_id].image) ;
        spjc->frame[a_id].image = tmp_im1 ;
        spjc->frame[b_id].image = tmp_im2 ;
        tmp_im1 = tmp_im2 = NULL ;
    }
    spjc->status_differences = OK ;    
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    compute differences a-b or b-a for each pair ab or ba
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_differences_half(spjitter_config_t * spjc)
{
    int     first_id1 ;
    int     first_id2 ;
    int     c_id1 ;
    int     c_id2 ;
    int     i, j ;
   
    /* Initialize */
    first_id1 = first_id2 = -1 ;

    /* Get first and second cubes ids */
    for (i=0 ; i<spjc->nframes ; i++) {
        if ((spjc->frame[i].type == type_obj)&&(spjc->frame[i].cube_id == 1)) {
            first_id1 = i ;
            break ;
        }
    }
    for (i=0 ; i<spjc->nframes ; i++) {
        if ((spjc->frame[i].type == type_obj)&&(spjc->frame[i].cube_id == 2)) {
            first_id2 = i ;
            break ;
        }
    }
    
    /* Test if the two first cubes have been found  */
    if ((first_id1 < 0) || (first_id2 < 0)) {
        spjc->status_differences = FAILED ;    
        return -1 ;
    }

    /* Allocate memory for main_offset_diff */
    spjc->main_offset_diff=malloc((spjc->nb_classified_cubes/2)*sizeof(double));
    
    /* Loop on each pair of cube */
    for (i=0 ; i<spjc->nb_classified_cubes/2 ; i++) {
        c_id1 = c_id2 = -1 ;
        /* Get ids of the current pair of cubes */
        for (j=0 ; j<spjc->nframes ; j++) {
            if ((spjc->frame[j].type == type_obj) &&
                (spjc->frame[j].cube_id == 2*i+1)) {
                c_id1 = j ;
                break ;
            }
        }
        for (j=0 ; j<spjc->nframes ; j++) {
            if ((spjc->frame[j].type == type_obj) &&
                (spjc->frame[j].cube_id == 2*i+2)) {
                c_id2 = j ;
                break ;
            }
        }
    
        /* Test if the two current cubes have been found  */
        if ((c_id1 < 0) || (c_id2 < 0)) {
            spjc->status_differences = FAILED ;    
            return -1 ;
        }

        /* Test if the current pair is ab or ba (compare to first pair) */
        if ((spjc->frame[first_id2].offset-spjc->frame[first_id1].offset) *
            (spjc->frame[c_id2].offset-spjc->frame[c_id1].offset) > 0) {
            /* ab */
            image_sub_local(spjc->frame[c_id1].image, spjc->frame[c_id2].image);
            image_cst_op_local(spjc->frame[c_id1].image, 2.0, '/') ;
            spjc->frame[c_id2].type = type_subtracted ;
            spjc->nobjframes -- ;
            /* Compute the offset difference */
            spjc->main_offset_diff[i] = spjc->frame[c_id2].offset -
                                        spjc->frame[c_id1].offset ;
        } else {
            /* ba */
            image_sub_local(spjc->frame[c_id2].image, spjc->frame[c_id1].image);
            image_cst_op_local(spjc->frame[c_id2].image, 2.0, '/') ;
            spjc->frame[c_id1].type = type_subtracted ;
            spjc->nobjframes -- ;
            /* Compute the offset difference */
            spjc->main_offset_diff[i] = spjc->frame[c_id1].offset -
                                        spjc->frame[c_id2].offset ;
        }
    }
    spjc->status_differences = OK ;    
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Check the mode and call the right combination function
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_combine(spjitter_config_t * spjc)
{
    int     ret ;

    /* Check the combination method to be used */
    if (spjc->diff_method == diff_half) {
        ret = spjitter_combine_combined(spjc) ;
    } else if (spjc->diff_method == diff_all) {
        ret = spjitter_combine_differences(spjc) ;
        if (ret == 0) ret = spjitter_combine_combined(spjc) ;
    } else {
        ret = -1 ;
    }

    return ret ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Combine (shift and add) each differences pairs together 
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_combine_differences(spjitter_config_t * spjc)
{
    int         a_id,
                b_id ;
    double      new_offset ;
    image_t *   shifted ;
    image_t *   averaged ;
    int         i ;

    /* Combine each pair of cubes together */
    for (i=0 ; i<spjc->nb_classified_cubes/2 ; i++) {
        compute_status("shift and combine...", i,spjc->nb_classified_cubes/2,1);
        /* Get the images id */
        a_id = b_id = 0 ;
        while ((spjc->frame[a_id].type != type_obj) || 
               (spjc->frame[a_id].cube_id != 2*i+1)) a_id++ ;
        while ((spjc->frame[b_id].type != type_obj) || 
               (spjc->frame[b_id].cube_id != 2*i+2)) b_id++ ;
  
        /* Here you may want to refine the OFFSET, because the  */
        /* startrace correction may slightly shift the spectra */
        if (spjc->refine_offsets == 1) {
            new_offset = refine_offset(spjc->frame[a_id].image,
                                        spjc->frame[b_id].image) ;
            if (fabs(new_offset - spjc->main_offset_diff[i]) < MAX_SHIFT_ERROR)
                spjc->main_offset_diff[i] = new_offset ;
        }
        if (spjc->circular_shift == 1) {
            shifted = image_shiftint_circular(spjc->frame[b_id].image,
                                    0, (int)spjc->main_offset_diff[i]) ;
        } else {
            shifted = image_shift(spjc->frame[b_id].image,
                                    0, spjc->main_offset_diff[i],(double*)NULL);
        }
        if (shifted == NULL) {
            e_error("cannot shift the image - aborting") ;
            spjc->status_combination = FAILED ;
            return -1 ;
        }

        /* Combine the images */
        if ((averaged = image_mean(spjc->frame[a_id].image, shifted))==NULL) {
            e_error("image addition failed - aborting") ;
            image_del(shifted) ;
            spjc->status_combination = FAILED ;
            return -1 ;
        }
        image_del(shifted) ;

        /* Put the new combined image in spjc */
        image_del(spjc->frame[a_id].image) ;
        spjc->frame[a_id].image = averaged ;
        averaged = NULL ;
        spjc->frame[b_id].type = type_combined ;
        spjc->nobjframes -- ;
    }

    /* Return */
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Combine (shift and add) all combined images (type_obj frames)
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_combine_combined(spjitter_config_t * spjc)
{
    int         a_id,
                b_id ;
    double      offset ;
    image_t *   shifted ;
    int         lo_rej,
                hi_rej ;
    int     *   selection ;
    cube_t  *   obj_list ;
    int         i ;

    /* Shift the combined images */
    /* Get the reference image id */
    a_id = 0 ;
    while (spjc->frame[a_id].type != type_obj) a_id++ ;
    b_id = a_id ;
    for (i=0 ; i<spjc->nb_classified_cubes/2 ; i++) {
        compute_status("shift combined images",i,spjc->nb_classified_cubes/2,1);
        /* Get the current image id */
        while (spjc->frame[b_id].type != type_obj) b_id++ ;

        /* Compute the offset and shift */
        offset = spjc->frame[b_id].offset - spjc->frame[a_id].offset ;
        shifted=image_shift(spjc->frame[b_id].image, 0, offset, (double*)NULL) ;
        /* Put the result in spjc */
        image_del(spjc->frame[b_id].image) ;
        spjc->frame[b_id].image = shifted ;
        shifted = NULL ;
        b_id ++ ;
    }

    /* Final combination of the combined shifted images */
    /* If nb of planes to combine is < 3 -> linear average */
    if (spjc->nobjframes < 3) spjc->combine_method = combine_linear ;

    e_comment(1, "final combination...");
    /* Get all type_obj frames in a cube */
    selection = spjitter_cubeselect(spjc, type_obj) ;
    obj_list = spjitter_cubeget(spjc, selection) ;
        
    /* Combine */
    if (spjc->combine_method == combine_rejection) {
        lo_rej = (int)(spjc->average_lo_rejection * spjc->nobjframes);
        hi_rej = (int)(spjc->average_hi_rejection * spjc->nobjframes);
        spjc->combined = cube_avg_reject(obj_list, lo_rej, hi_rej);
    } else if (spjc->combine_method == combine_linear) {
        spjc->combined = cube_avg_linear(obj_list) ;
    } else if (spjc->combine_method == combine_median) {
        spjc->combined = cube_avg_median(obj_list) ;
    } else {
        e_warning("final combination method not recognized - use median") ;
        spjc->combined = cube_avg_median(obj_list) ;
    }

    /* Put the images back in spjc */
    spjitter_cubeput(spjc, selection, obj_list) ;
    free(selection);
    cube_del_shallow(obj_list) ;
    
    if (spjc->combined == NULL) {
        e_error("averaging the combined images") ;
        spjc->status_combination = FAILED ;
        return -1 ;
    }

    /* Return */
    spjc->status_combination = OK ;
    return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Refine offsets.
  @param    im1 First image
  @param    im2 Second image
  @return   refined offset
  
  Returned offset has to be verified (compared with an existing estimation)
 */
/*--------------------------------------------------------------------------*/
static double refine_offset(
        image_t *   im1,
        image_t *   im2)
{
    double3 *   position1 ;
    double3 *   position2 ;
    double      new_offset ;

    if ((position1=find_brightest_spectrum_1d(im1, 0.0, NO_SHADOW_SPECTRUM,
                                0)) == NULL) {
        return 0.0 ;
    }
    if ((position2=find_brightest_spectrum_1d(im2, 0.0, NO_SHADOW_SPECTRUM,
                                0)) == NULL) {
        double3_del(position1) ;
        return 0.0 ;
    }

    new_offset = position1->y[0] - position2->y[0] ;

    double3_del(position1) ;
    double3_del(position2) ;

    return new_offset ;
}


