/*-------------------------------------------------------------------------*/
/**
   @file    cube2image.h
   @author  Nicolas Devillard
   @date    Sept 14, 1995
   @version $Revision: 1.19 $
   @brief   cube averaging to a single plane
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: cube2image.h,v 1.19 2002/07/31 14:34:38 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/07/31 14:34:38 $
    $Revision: 1.19 $
*/

#ifndef _CUBE2IMAGE_H_
#define _CUBE2IMAGE_H_


/*---------------------------------------------------------------------------
  								Includes
 ---------------------------------------------------------------------------*/

#include "qfits.h"

#include "comm.h"
#include "xmemory.h"
#include "filename.h"
#include "cube_handling.h"
#include "image_arith.h"
#include "extraction.h"
#include "pixel_handling.h"

/*---------------------------------------------------------------------------
  								New Types
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	cut_method object

  This type gives the method to use for the cut. The different methods are:

  \begin{itemize}
  \item cut_whole -- averaging is taking into account all planes of
  the input cube.
  \item cut_cycle -- averaging is applied once every cycle. A cycle is
  a consecutive series of cycle_step frames. For example, an input
  cube containing 12 frames being cut with a cycle_step of 4 will
  produce a cube with 3 frames, each frame being the average of the
  corresponding 4 consecutive frames.
  \item cut_running -- the average is applied on an interval of
  frames around the current one. This will yield as many frames as
  there are in the input cube. See cube_avgrun_linear for a
  detailed explanation of what a running average is.
  \end{itemize}
 */
/*-------------------------------------------------------------------------*/
typedef enum _CUT_METHOD_ {
    cut_whole,
    cut_cycle,
    cut_running
} cut_method ;


/*-------------------------------------------------------------------------*/
/**
  @brief	average_method object

  The average method is one of the following:

  \begin{itemize}
  \item avg_linear: the output is a linear average of the input pixels.
  \item avg_median: the output is the median of the input pixels.
  \item avg_sum: the output is the sum of all input pixels.
  \item avg_filtered: the input pixels are sorted, the highest and
  lowest ones are rejected, and the rest is linearly averaged to yield
  the output pixel.
  \end{itemize}
 */
/*-------------------------------------------------------------------------*/
typedef enum _AVG_METHOD_ {
    avg_linear,
    avg_median,
    avg_sum,
    avg_filtered
} average_method ;


/*---------------------------------------------------------------------------
 						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Average a cube to another cube or an image.
  @param    name_in         Name of the input file.
  @param    name_out        Name of the output file.
  @param    amethod         Cut method definition (see cube2image.h).
  @param    amethod         Average method definition (see cube2image.h).
  @param    cycle_step      Cycle step (see below).
  @param    run_hw          Running half-width (see below).
  @param    lo_rej          Number of low rejects.
  @param    hi_rej          Number of high rejects.
  @return   int 0 if Ok, -1 otherwise.

  This engine is actually calling cube_average() underneath after having
  loaded the input cube, and will save its result as indicated. See
  cube_average() for a complete doc about possible averaging methods and
  associated parameters.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int average_engine(
        char        *   name_in,
        char        *   name_out,
        cut_method      cmethod,
        average_method  amethod,
        int             cycle_step,
        int             run_hw,
        int             lo_rej,
        int             hi_rej) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Average a cube
  @param    cube_in     Input cube to average
  @param    cmethod     Cut method
  @param    amethod     Average method
  @param    cycle_step  Cycle step for cut="cycle"
  @param    run_hw      Running half-width for cut="running"
  @param    lo_rej      Low rejection for method="filtered"
  @param    hi_rej      High rejection for method="filtered"
  @return   1 newly allocated cube

  Averages are separated in two dimensions: cut method and average
  method.

  Cutting tells which planes in the cube are to be considered. See 
  cube2image.h for details.

  Averaging over the third dimension can be done in a number of ways.
  Basically, the idea is that the output pixel is determined from a
  time line of input pixels lying at the same position on the
  detector. The way the output pixel is computed from the list of
  input pixels completely specifies the kind of average which is
  performed. See cube2image.h for details.

  The additional parameters run_hw, cycle_step, lo_rej and hi_rej are
  specific to each kind of averaging.

  Notice that all combinations are not yet implemented. Check out the
  source code to see if your specific average need is there.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_average(
        cube_t      *   cube_in,
        cut_method      cmethod,
        average_method  amethod,
        int             cycle_step,
        int             run_hw,
        int             lo_rej,
        int             hi_rej) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Linear average over a whole cube to a single image.
  @param    incube  Cube to average.
  @return   Newly allocated image object.

  Probably the simplest and most intuitive average type: stack a whole
  cube to a single image.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_avg_linear(cube_t * incube) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Median a cube with rejection.
  @param    incube      Cube to average.
  @param    lo_rej      Number of min pixels to reject.
  @param    hi_rej      Number of max pixels to reject.
  @return   Newly allocated image object.

  This median averaging applies to the whole cube. Every time line is
  extracted, sorted out, then the lowest and highest values are
  rejected, and the median of the rest is found to yield the output
  pixel.

  Rejection levels are given as a number of pixels to reject on each side.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_avg_medreject(
        cube_t  *   incube,
        int         lo_rej,
        int         hi_rej) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Median-average pixel values on a time line, with rejection.
  @param    in_cube     Input cube to use for average computation.
  @param    pos         Detector position to use for averaging.
  @param    lo_rej      Number of min pixels to reject.
  @param    hi_rej      Number of max pixels to reject.
  @return   1 pixelvalue.

  This function takes a cube in input and a detector position. The
  detector position is a single number, expected to be of the form
  i+j*lx, where (i,j) is the position on the detector and lx the image
  width.
 */
/*--------------------------------------------------------------------------*/
pixelvalue cube_z_medreject(
        cube_t  *   in_cube,
        int         pos,
        int         lo_rej,
        int         hi_rej) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Average a cube with rejection.
  @param    incube      Cube to average.
  @param    lo_rej      Number of min pixels to reject.
  @param    hi_rej      Number of max pixels to reject.
  @return   Newly allocated image object.

  This averaging applies to the whole cube. Every time line is
  extracted, sorted out, then the lowest and highest values are
  rejected, and the rest is linearly averaged to yield the output
  pixel.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_avg_reject(
        cube_t  *   incube,
        int         lo_rej,
        int         hi_rej) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Average pixel values on a time line, with rejection.
  @param    in_cube     Input cube to use for average computation.
  @param    pos         Detector position to use for averaging.
  @param    lo_rej      Number of min pixels to reject.
  @param    hi_rej      Number of max pixels to reject.
  @return   1 pixelvalue.

  This function takes a cube in input and a detector position. The
  detector position is a single number, expected to be of the form
  i+j*lx, where (i,j) is the position on the detector and lx the image
  width.
 */
/*--------------------------------------------------------------------------*/
pixelvalue cube_z_reject(
        cube_t  *   in_cube,
        int         pos,
        int         lo_rej,
        int         hi_rej) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Sum a cube to a single image.
  @param    incube  Cube to sum.
  @return   Newly allocated image object.

  The output image is a sum of all planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_avg_sum(cube_t * incube) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Average a cube to a median image.
  @param    to_average  Cube to average.
  @return   Newly allocated image object.

  The returned image is a median image of the input cube. See the
  convention for median used in the case of an even number of
  elements, in math/median.c
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_avg_median(cube_t * to_average) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Cycle average a cube linearly.
  @param    incube  Cube to average.
  @param    cycle   Cycle length.
  @return   Newly allocated cube.
 
  The cube is averaged by cycles: sub-cubes of 'cycle' planes are
  extracted from the input cube and each sub-cube is linearly
  averaged.
 
  Example: the input cube has 120 planes, the cycle is 30, the output
  cube will have 120/3=4 planes, each plane being an average of 30
  consecutive planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_avgcyc_linear(
        cube_t  *   incube,
        int         cycle) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Cycle average a cube with sums.
  @param    incube  Cube to average
  @param    cycle   Cycle length.
  @return   Newly allocated cube.
 
  The cube is averaged by cycles: sub-cubes of 'cycle' planes are
  extracted from the input cube and each sub-cube is summed to a
  single plane.
 
  Example: the input cube has 120 planes, the cycle is 30, the output
  cube will have 120/3=4 planes, each plane being a sum of 30
  consecutive planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_avgcyc_sum(
        cube_t  *   incube,
        int         cycle) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Cycle median average a cube.
  @param    incube  Cube to average
  @param    cycle   Cycle length.
  @return   Newly allocated cube.
 
  The cube is averaged by cycles: sub-cubes of 'cycle' planes are
  extracted from the input cube and each sub-cube is median averaged
  to a single plane.
 
  Example: the input cube has 120 planes, the cycle is 30, the output
  cube will have 120/3=4 planes, each plane being a median average of
  30 consecutive planes in the input cube.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_avgcyc_median(
        cube_t  *   incube,
        int         cycle) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the median value of a pixel position along time.
  @param    in_cube     Cube from which the line is extracted.
  @param    pos         Position on the detector as i+j*lx.
  @return   Median pixelvalue along this time line.
 
  Provide a cube and a detector position as one number: i+j*lx, where
  (i,j) specifies the position on the detector with the C convention
  (i and j start at zero). The returned value is the median of the
  pixels on this detector position along time.
 
 */
/*--------------------------------------------------------------------------*/
pixelvalue cube_z_median(
        cube_t      *   in_cube,
        int             pos) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Running linear average of a cube.
  @param    incube      Input cube.
  @param    half_cycle  Half cycle definition.
  @return   1 newly allocated cube.

  A running average is computing plane averages along the cube.
  The returned cube has as many planes as the input cube. Each output
  plane is an average of the planes around [-halfcycle, +halfcycle]
  around the current plane position. For the beginning and end planes,
  only existing plane positions are taken into account.

  Example: a running average of a cube containing 7 planes, with a
  halfcycle of 2, would be done as follows:
  \begin{verbatim}

    output plane:       average of input planes:
    1                   1 2 3
    2                   1 2 3 4
    3                   1 2 3 4 5
    4                     2 3 4 5 6
    5                       3 4 5 6 7
    6                         4 5 6 7
    7                           5 6 7
  \end{verbatim}

  As can be seen, the number of planes truly used to compute one given
  output plane is between halfcycle+1 (on the edges) and 2*halfcycle+1
  (away from the edges). In this example, only planes 3, 4, and 5 will
  be computed using 2*halfcycle+1 frames.

  This function is not optimized for speed to cover the greatest
  possible number of cases. Since it needs to browse the input cube
  back and forth, it is CPU and memory intensive.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_avgrun_linear(
        cube_t  *   incube,
        int         half_cycle) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Running sum of a cube.
  @param    incube      Cube to sum.
  @param    half_cycle  Half cycle definition.
  @return   1 newly allocated cube.

  See above function cube_avgrun_linear for a description of
  a running filter. This variant of average is only summing planes,
  not performing a linear average.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_avgrun_sum (
        cube_t  *   incube,
        int         half_cycle) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Running median average of a cube.
  @param    incube      Cube to median average.
  @param    half_cycle  Half cycle definition.
  @return   1 newly allocated cube.

  See above function cube_avgrun_linear for a description of
  a running filter. This variant of average uses a median average for
  each batch of planes.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
cube_t * cube_avgrun_median(
        cube_t  *   incube,
        int         half_cycle) ;
/* </python> */


#endif
