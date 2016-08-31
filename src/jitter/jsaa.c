/*----------------------------------------------------------------------------*/
/**
   @file    jsaa.c
   @author
   @date    March 2002
   @version	$Revision: 1.19 $
   @brief   Jitter shift-and-add
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jsaa.c,v 1.19 2003/02/14 09:18:26 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/14 09:18:26 $
	$Revision: 1.19 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"

#include "jtypes.h"
#include "jconfig.h"

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

static int jitter_saa_blind(jitter_config_t * jc);
static int jitter_saa_xcorr(jitter_config_t * jc);
static int jitter_saa_findxcorrp(jitter_config_t * jc);
static int jitter_saa_stack(jitter_config_t * jc);

/*-----------------------------------------------------------------------------
                            Functions code
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Apply shit-and-add to input set of frames.
  @param    jc  Current jitter config.
  @return   int 0 if Ok, -1 if error occured.

  This part includes:
  - offsets search (either from header, or with a provided file or blindly)
  - X-correlation:
    - xcorr object detection
    - xcorrelation to refine offsets
  - Shifting and adding frames
 */
/*----------------------------------------------------------------------------*/
int jitter_saa(jitter_config_t * jc)
{
    double      ref_offset_x,
                ref_offset_y ;
    int         i ;

    /* Apply 50 Hz correction if requested */
    if (jc->preproc_active && jc->preproc_fiftyhertz) { 
        e_comment(1, "Remove 50 hertz frome object images") ; 
        for (i=0 ; i<jc->nframes ; i++) {
            if (jc->frame[i].type == type_obj) {
                image_remove_fiftyhertz(jc->frame[i].image) ;
            }
        }
    }
   
    /* Test if shift-an-add is requested */
    if (jc->saa_active!=1) {
        e_comment(1, "skipped");
        jc->status_saa = ALGO_SKIPPED ;
        return 0 ;
    }
    
    /* Initialize */
    ref_offset_x = ref_offset_y = 0.0 ;

    if (jc->saa_offsource == offsource_blind) {
        /* Apply blind offset search */
        e_comment(1, "applying blind offset search");
        if (jitter_saa_blind(jc)!=0) {
            e_error("applying blind offset search");
            jc->status_saa = ALGO_FAILED ;
            return -1 ;
        }
    }

    /* Subtract the first offsets from all others Only for type_obj */
    /* Find reference offsets (first type_obj frame) */
    for (i=0 ; i<jc->nframes ; i++) { 
        if (jc->frame[i].type == type_obj) {
            ref_offset_x = jc->frame[i].off_x ;
            ref_offset_y = jc->frame[i].off_y ;
            break ;
        }
    }
    /* Subtract */
    for (i=0 ; i<jc->nframes ; i++) {
        if (jc->frame[i].type == type_obj) {
            jc->frame[i].off_x -= ref_offset_x ;
            jc->frame[i].off_y -= ref_offset_y ;
        }
    }

    if (jc->saa_xcorractive==1) {
        /* Refine input offsets by x-correlation */
        e_comment(1, "applying x-correlation");
        if (jitter_saa_xcorr(jc)!=0) {
            e_error("applying cross-correlation");
            jc->status_saa = ALGO_FAILED ;
            return -1 ;
        }
    } else {
        /* Use the estimates for shifting. */
        for (i=0 ; i<jc->nframes ; i++) {
            jc->frame[i].off_cor_x = jc->frame[i].off_x ;
            jc->frame[i].off_cor_y = jc->frame[i].off_y ;
            jc->frame[i].off_dist  = 0 ;
            jc->frame[i].off_err_x = 0 ;
            jc->frame[i].off_err_y = 0 ;
        }
    }
    e_comment(1, "stacking frames to single image");
    if (jitter_saa_stack(jc)!=0) {
        e_error("stacking frames");
        jc->status_saa = ALGO_FAILED ;
        return -1 ;
    }

    jc->status_saa = ALGO_OK ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Blind offset search
  @param    jc  Current jitter config
  @return   int 0 if Ok, -1 otherwise.
  This function is applying a blind offset search to identify a first,
  rough estimate of the offsets between all frames.
 */
/*----------------------------------------------------------------------------*/
static int jitter_saa_blind(jitter_config_t * jc)
{
    int         i, j ;
    cube_t  *   obj ;
    int     *   sel ;
    double3 *   offs ;

    sel = jitter_cubeselect(jc, type_obj);
    obj = jitter_cubeget(jc, sel);
    free(sel);
    offs = cube_blindoffsets(obj, obj->plane[0]);
    cube_del_shallow(obj);
    if (offs==NULL) {
        e_error("blind offsets failed");
        return -1 ;
    }

    /* Put offsets back into config */
    e_comment(1, "plane  #:       dx       dy         dist");
    j=0 ;
    for (i=0 ; i<jc->nframes ; i++) {
        if (jc->frame[i].type == type_obj) {
            jc->frame[i].off_x = offs->x[j] ;
            jc->frame[i].off_y = offs->y[j] ;
            e_comment(1, "plane %02d: %8.2f %8.2f", j+1,offs->x[j], offs->y[j]);
            j++ ;
        }
    }
    double3_del(offs);
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	x-correlation for offset refining.
  @param    jc  Current jitter config.
  @return   int 0 if Ok, -1 otherwise

  This function applies a cross-correlation criterion to compare
  all frames with the first one, and updates the off_cor_x, off_cor_y
  and off_dist fields for each object frame in the blackboard.
 */
/*----------------------------------------------------------------------------*/
static int jitter_saa_xcorr(jitter_config_t * jc)
{
    cube_t  *   xcorr_cube ;
    int     *   selected ;
    double3 *   offs ;
    double3 *   estimates ;
    double3 *   xcorrp ;
    int         i, j ;
    int         ncorrect ;
    double      err_x, err_y ;

    /* Find x-correlation places */
    if (jitter_saa_findxcorrp(jc)!=0) return -1 ;

    /* Select all object planes in input */
    selected = jitter_cubeselect(jc, type_obj);
    /* Extract all object planes to a cube */
    xcorr_cube = jitter_cubeget(jc, selected);
    /* No need for selection array anymore */
    free(selected);
    if (xcorr_cube==NULL) {
        e_error("extracting object frames from input");
        return -1 ;
    }

    /* Copy estimates from config to local double3 object */
    estimates = double3_new(xcorr_cube->np);
    i=0 ;
    for (j=0 ; j<jc->nframes ; j++) {
        if (jc->frame[j].type == type_obj) {
            estimates->x[i] = jc->frame[j].off_x ;
            estimates->y[i] = jc->frame[j].off_y ;
            i++ ;
        }
    }

    /* Copy xcorrelation places from config to local double3 object */
    xcorrp = double3_new(jc->saa_xcorrp_n);
    for (i=0 ; i<xcorrp->n ; i++) {
        xcorrp->x[i] = jc->saa_xcorrp_x[i] ;
        xcorrp->y[i] = jc->saa_xcorrp_y[i] ;
    }

    /* Apply cross-correlation criterion on all objet frames */
    offs = xcorr_with_objs(xcorr_cube,
                           xcorr_cube->plane[0],
                           estimates,
                           xcorrp,
                           jc->saa_xcorrsx,
                           jc->saa_xcorrsy,
                           jc->saa_xcorrhx,
                           jc->saa_xcorrhy);

    cube_del_shallow(xcorr_cube);
    double3_del(xcorrp);

    if (offs==NULL) {
        e_error("during cross-correlation");
        double3_del(estimates);
        return -1 ;
    }

    /* Examine returned offsets, remove meaningless values */
    ncorrect = 0 ;
    e_comment(1, "plane  #:       dx       dy         dist");
    for (i=0 ; i<offs->n ; i++) {
        err_x = fabs(offs->x[i] - estimates->x[i]);
        err_x = fabs(err_x - (double)jc->saa_xcorrsx) ;
        err_y = fabs(offs->y[i] - estimates->y[i]);
        err_y = fabs(err_y - (double)jc->saa_xcorrsy) ;
        if ((err_x<0.1) || (err_y<0.1) || (offs->z[i]<0)) {
            e_warning("xcorrelation failed for frame #%02d", i+1);
            offs->z[i] = -1 ;
        } else {
            e_comment(1, "plane %02d: %8.2f %8.2f %12.2f",
                      i+1, offs->x[i], offs->y[i], offs->z[i]);
            ncorrect++ ;
        }
    }
    double3_del(estimates);

    if (ncorrect<1) {
        e_error("no frame correctly correlated");
        double3_del(offs);
        return -1 ;
    }
    if (ncorrect<(offs->n/2)) {
        e_warning("less than half of the input frames correlate correctly");
    }

    /* Copy found offsets back into config */
    i=0 ;
    for (j=0 ; j<jc->nframes ; j++) {
        if (jc->frame[j].type == type_obj) {

            /* Register correlated offsets */
            jc->frame[j].off_cor_x = offs->x[i];
            jc->frame[j].off_cor_y = offs->y[i];
            /* Register correlation distance */
            jc->frame[j].off_dist  = offs->z[i];

            /* Switch frame type to rejected if needed */
            if (jc->frame[j].off_dist < 0) {
                jc->frame[j].type = type_rej ;

                /* Delete image pointer: not needed anymore */
                image_del(jc->frame[j].image);
                jc->frame[j].image = NULL ;
            }

            /* Register offset errors */
            jc->frame[j].off_err_x = jc->frame[j].off_x-jc->frame[j].off_cor_x;
            jc->frame[j].off_err_y = jc->frame[j].off_y-jc->frame[j].off_cor_y;

            i++ ;
        }
    }
    double3_del(offs);

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Find places for x-correlation
  @param    jc  Current jitter config.
  @return   int 0 if Ok, -1 otherwise.
  This function locates places for x-correlation. It does nothing if these 
  places have already been loaded.
 */
/*----------------------------------------------------------------------------*/
static int jitter_saa_findxcorrp(jitter_config_t * jc)
{
    image_t *   detect_image ;
    image_t *   first_image ;
    image_t *   second_image ;
    double3 *   peaks ;
    int         i, j ;

    if (jc->saa_objsource != objsource_auto) return 0 ;

    /* Apply object detection on either the first (raw) frame, or the */
    /* difference between the first two (raw) object frames. */
    switch (jc->saa_detectim) {

        /* Load first object (raw) frame */
        case detectim_first:
            i=0 ;
            while ((jc->frame[i].type != type_obj) && (i<jc->nframes)) i++ ;
            if (i>=jc->nframes) {
                e_error("cannot find any object frame in input");
                return -1 ;
            }
            /* Select the first oject image image */
            detect_image = image_copy(jc->frame[i].image) ;
            break ;

        /* Load first pair of (raw) object frames */
        case detectim_diff:
            i=0 ;
            while ((jc->frame[i].type != type_obj) && (i<jc->nframes)) i++ ;
            if (i>=jc->nframes) {
                e_error("cannot find any object frame in input");
                return -1 ;
            }
            j=i+1 ;
            while ((jc->frame[j].type != type_obj) && (j<jc->nframes)) j++ ;
            if (j>=jc->nframes) {
                e_error("cannot find two object frames in input");
                return -1 ;
            }
            first_image  = jc->frame[i].image ;
            second_image = jc->frame[j].image ;
            detect_image = image_sub(first_image, second_image);
            break ;
            
        default:
            e_error("bad image detection method (%d)", (int)jc->saa_detectim) ;
            detect_image = NULL ;
            break ;
    }

    if (detect_image==NULL) {
        e_error("cannot get detection image");
        return -1 ;
    }

    /* Detect suitable objects for x-correlation */
    peaks = get_xcorrelation_points(detect_image,
                                    jc->saa_xcorrhx + jc->saa_xcorrsx,
                                    jc->saa_xcorrhy + jc->saa_xcorrsy,
                                    jc->saa_detectk,
                                    jc->saa_detectminp,
                                    jc->saa_detectmaxp);
    image_del(detect_image) ;
    if (peaks == NULL) {
        e_error("cannot find enough valid points for xcorrelation");
        return -1 ;
    }

    /* Update config */
    jc->saa_xcorrp_n = peaks->n ;
    jc->saa_xcorrp_x = malloc(peaks->n * sizeof(double));
    jc->saa_xcorrp_y = malloc(peaks->n * sizeof(double));

    for (i=0 ; i<peaks->n ; i++) {
        jc->saa_xcorrp_x[i] = peaks->x[i];
        jc->saa_xcorrp_y[i] = peaks->y[i];
    }
    double3_del(peaks);

    /* Display Xcorrelation points */
    for (i=0 ; i<jc->saa_xcorrp_n ; i++) {
        e_comment(3, "Object %d: x = %g, y = %g ;\n", 
                i+1,
                jc->saa_xcorrp_x[i] + (double)jc->zone.left,
                jc->saa_xcorrp_y[i] + (double)jc->zone.bottom);
    }
    
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Stack all frames to a single frame.
  @param    jc  Current jitter config.
  @return   int 0 if Ok, -1 otherwise.

  This function applies the found offsets and 3d filtering parameters
  to stack all input frames to a single one. The result is placed
  inside the jitter config (field 'final').
 */
/*----------------------------------------------------------------------------*/
static int jitter_saa_stack(jitter_config_t * jc)
{
    cube_t  *   stack ;
    int     *   selected ;
    double3 *   offs ;
    int         i, j ;

    /* Extract cube of all correct planes */
    selected = jitter_cubeselect(jc, type_obj);
    stack = jitter_cubeget(jc, selected);
    if (stack==NULL) {
        e_error("cannot find any valid object frame");
        return -1 ;
    }
    free(selected);

    /* Copy valid offsets to offs */
    offs = double3_new(stack->np);
    i=0 ;
    for (j=0 ; j<jc->nframes ; j++) {
        if (jc->frame[j].type == type_obj) {
            offs->x[i] = - jc->frame[j].off_cor_x ;
            offs->y[i] = - jc->frame[j].off_cor_y ;
            i++ ;
        }
    }

    /* Launch stacking */
    jc->final = cube_shiftandadd(stack,
                                 offs,
                                 NULL,
                                 jc->saa_3drejmin,
                                 jc->saa_3drejmax,
                                 jc->saa_union);
    double3_del(offs);
    cube_del_shallow(stack);

    if (jc->final==NULL) {
        e_error("stacking failed");
        return -1 ;
    }

    /* Frame data are not needed anymore: free all images */
    for (i=0 ; i<jc->nframes ; i++) {
        if (jc->frame[i].image!=NULL) {
            image_del(jc->frame[i].image);
            jc->frame[i].image = NULL ;
        }
    }
    return 0 ;
}

