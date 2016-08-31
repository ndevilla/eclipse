/*-------------------------------------------------------------------------*/
/**
   @file    shift.h
   @author  Y. Jung
   @date    Jan. 2001
   @version $Revision: 1.11 $
   @brief   Shift related routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: shift.h,v 1.11 2002/03/22 14:57:44 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/03/22 14:57:44 $
    $Revision: 1.11 $
*/

#ifndef _SHIFT_H_
#define _SHIFT_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "resampling.h"
#include "image_intops.h"
#include "doubles.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/* Number of pixels set to 0 by the shift resampling */
#define SHIFT_REJECT_L          2
#define SHIFT_REJECT_R          2
#define SHIFT_REJECT_T          2
#define SHIFT_REJECT_B          2

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift an image by a given (non-integer) 2d offset.
  @param    image_in        Image to shift.
  @param    shift_x         Shift in x.
  @param    shift_y         Shift in y.
  @param    interp_kernel   Interpolation kernel to use.
  @return   1 newly allocated image.

  This function shifts an image by a non-integer offset, using
  interpolation. You can either generate an interpolation kernel once and
  pass it to this function, or let it generate a default kernel. In the
  former case, use generate_interpolation_kernel() to generate an
  appropriate kernel. In the latter case, pass NULL as last argument. A
  default interpolation kernel is then generated then discarded before this
  function returns.

  The returned image is a newly allocated object, it must be deallocated
  using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_shift(
        image_t *   image_in,
        double          shift_x,
        double          shift_y,
        double      *   interp_kernel) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift an image by an integer 2d offset.
  @param    image_in    Image to shift.
  @param    shift_x     Shift in X.
  @param    shift_y     Shift in Y.
  @return   1 newly allocated image.

  Shifts an image by an integer offset. The returned object is a newly
  allocated image, it must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_shift_int(
        image_t     *   image_in,
        int             shift_x,
        int             shift_y) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @return   int 0 if Ok, -1 otherwise.
 
  Runs cube_shift_int_slice over the whole cube.
 */
/*--------------------------------------------------------------------------*/
int cube_shift_int(
        cube_t  *   to_shift,
        double3 *   offsets) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @param    from_p      Index of first plane to shift.
  @param    to_p        Index of last plane to shift.
  @return   int 0 if Ok, -1 otherwise.
 
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! Shifted planes are replacing
  one by one the initial planes.

  The input list of offsets is not allowed to contain invalid measurements,
  i.e. cross-correlation distances lower than 0. The input list must
  contain as many offsets as there are planes in the input cube.
 
  The rule is that if an offset (dx,dy) is given, the image is shifted
  with a (-dx,-dy) shift, to stay consistent with the results returned
  from the cross-correlation functions.
 
  The offsets used for shifting are rounded up from the input offsets,
  to the closest integer value. The shifting does not involve any
  resampling, only integer pixel shifting.
 
  This is much faster than cube_shift() but of course troublesome,
  since it only handles offsets to pixel resolution.
 */
/*--------------------------------------------------------------------------*/
int cube_shift_int_slice(
        cube_t      *   to_shift,
        double3     *   offsets,
        int             from_p,
        int             to_p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @return   a contribution map 
    
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! 
  The final size is defined by the union of all the shifted planes. Each
  input plane is placed in such a huge blank frame.
 
  The planes are first shifted with the decimal part of the offsets, and 
  then, cube_shift_int_expand is used to shift the planes with the E() part 
  of the offsets.
 
 */
/*--------------------------------------------------------------------------*/
intimage * cube_shift_expand(
        cube_t         **   to_shift,
        double3         *   offsets) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @return   a contribution map 
    
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! 
  The final size is defined by the union of all the shifted planes. Each
  input plane is placed in such a huge blank frame.
  
  The rule is that if an offset (dx,dy) is given, the image is shifted
  with a (-E(dx),-E(dy)) shift, to stay consistent with the results returned
  from the cross-correlation functions (not (E(dx), E(dy))).
 
  Here, only the int shift is done. We suppose that the offsets decimal part 
  has already been used to shift the frames. 
  
  The input list of offsets is not allowed to contain invalid measurements,
  i.e. cross-correlation distances lower than 0. The input list must
  contain as many offsets as there are planes in the input cube.
 
 */
/*--------------------------------------------------------------------------*/
intimage * cube_shift_int_expand(
        cube_t         **   to_shift,
        double3         *   offsets) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @param    kernel      Kernel to use for resampling.
  @return   int 0 if Ok, -1 otherwise.
 
  Runs cube_shift_slice over the whole cube.
 */
/*--------------------------------------------------------------------------*/
int cube_shift(
        cube_t  *   to_shift,
        double3 *   offsets,
        char    *   kernel) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift planes in a cube according to a list of offsets.
  @param    to_shift    Cube to shift.
  @param    offsets     List of offset measurements to use for shifts.
  @param    kernel      Kernel to use for resampling.
  @param    from_p      Index of first plane to shift.
  @param    to_p        Index of last plane to shift.
  @return   int 0 if Ok, -1 otherwise.
 
  All planes in the cube are shifted according to the given list of
  offsets. The input cube is modified! Shifted planes are replacing
  one by one the initial planes.
 
  The rule is that if an offset (dx,dy) is given, the image is shifted
  with a (-dx,-dy) shift, to stay consistent with the results returned
  from the cross-correlation functions.
 
  The input list of offsets is not allowed to contain invalid measurements,
  i.e. cross-correlation distances lower than 0. The input list must
  contain as many offsets as there are planes in the input cube.
 
  There is a faster version (shift_cube_int) but only handling offsets
  to pixel resolution, i.e. without resampling.

  The only planes which will be shifted are the ones between index 'from_p'
  and 'to_p' (including 'from_p', not 'to_p'). Nevertheless, the provided
  list of offsets must be consistent with the input cube, i.e. have as many
  offsets as there are planes in the cube.  To shift the whole cube, use
  the macro cube_shift(). If from_p is negative, it is assumed to be 0 (the
  first plane). If to_p is negative, it is assumed to be to_shift->np (the
  last plane). This slicing is meant for multi-threading support.
 */
/*--------------------------------------------------------------------------*/
int cube_shift_slice(
        cube_t  *   to_shift,
        double3 *   offsets,
        char    *   kernel,
        int         from_p,
        int         to_p) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift and add a cube to a single frame.
  @param    in          Cube to process.
  @param    offs        List of offsets between frames.
  @param    kernel      Interpolation kernel to use.
  @param    rejmin      Number of min pixels to reject in stacking
  @param    rejmax      Number of max pixels to reject in stacking
  @param    union_flag  Flag to create a union image.
  @return   1 newly allocated image.

  This function does everything related to the final shift-and-add of
  a stack of frames. It takes in input a cube and a list of offsets
  to apply to the cube to register all frames to a common position,
  applies an interpolation kernel to resample the frames to sub-pixel
  accuracy and accumulates them into an output image, using 3d filtering
  if requested.

  If the union flag is non-zero, the final frame is a union of all input
  frames, i.e. it is always bigger than the input image. If union_flag
  is set to zero, only the intersection of all frames will be built,
  i.e. all pixels in the final image have been seen by all input
  frames.

  The returned frame is a newly allocated object, to be deallocated
  using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * cube_shiftandadd(
        cube_t  * in,
        double3 * offs,
        char    * kernel,
        int       rejmin,
        int       rejmax,
        int       union_flag);

#endif
