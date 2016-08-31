/*-------------------------------------------------------------------------*/
/**
   @file    cube_arith.h
   @author  Nicolas Devillard
   @date    Aug 02, 1995
   @version $Revision: 1.21 $
   @brief   cube arithmetic routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: cube_arith.h,v 1.21 2002/03/19 14:44:14 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/03/19 14:44:14 $
    $Revision: 1.21 $
*/

#ifndef _CUBE_ARITHMETIC_H_
#define _CUBE_ARITHMETIC_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "comm.h" 
#include "xmemory.h"
#include "cube_defs.h"
#include "cube_handling.h"
#include "image_arith.h"

/*---------------------------------------------------------------------------
  						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    4 arithmetic operations between two cubes.
  @param    cube1       First argument.
  @param    cube2       Second argument.
  @param    operation   Arithmetic operation to perform.
  @return   int 0 if Ok, -1 otherwise.
 
  The first argument is modified to contain the results of the
  operation. Supported operations are '+', '-', '*', '/'.
 
  \begin{verbatim}
  cube_op(&c1, c2, '+') is equivalent to c1+=c2
  \end{verbatim}
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_op(
        cube_t  **  cube1,
        cube_t  *   cube2,
        int         operation) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Arithmetic operations between a cube and a constant.
  @param    cube1       First operand.
  @param    constant    Second operand.
  @param    operation   Operator.
  @return   int 0 if Ok, -1 otherwise.
 
  Performs an arithmetic operation on all pixels on the input cube.
  Possible operators are:
 
  \begin{tabular}{lll}
  Operator          & Symbol    &   Operation \\
  \\
  Addition          & +         & out = in + c \\
  Subtraction       & -         & out = in - c \\
  Multiplication    & *         & out = in * c \\
  Division          & /         & out = in * (1/c) \\
  Logarithm         & l         & out = log(in)/log(c) \\
  Power             & ^         & out = in ^ c \\
  Exponentiation    & e         & out = c ^ in
  \end{tabular}
 
  Notice that the division is already optimized by multiplying all
  pixels by the reciprocal of the given constant. No need to do it
  yourself, i.e.
 
  \begin{verbatim}
  cube_cst_op(cube1, cst, '/');
  is just as efficient as:
  cube_cst_op(cube1, 1.0/cst, '*');
  \end{verbatim}
 
  Every pixel in the input cube is modified.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_cst_op(
        cube_t  *   cube1,
        double      constant,
        int         operation) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Normalize all pixels in a cube.
  @param    cube1   Input cube to normalize.
  @param    mode    Normalization mode.
  @return   int 0 if Ok, -1 if error occurred.
 
  Normalize all planes of a cube. See normalization mode definitions
  in image_arith.h.
 
  The input cube is modified.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_normalize(
        cube_t  *   cube1,
        int         mode) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Scale all images to a given value or to the flux in the
            first plane.
  @param    cube1       Cube to scale.
  @param    to_flux      Flux to scale to.
  @return   int 0 if Ok, -1 otherwise.
 
  All images in the input cube are modified so that their fluxes
  corresponds to the given value. The flux of an image is the sum of
  all its pixel values.
 
  If the input value for 'to_flux' is zero, all plane fluxes will be
  scaled (except the first one) to be equal to the flux in the first
  plane.
 
  The input cube is modified.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_scale_flux(
        cube_t  *   cube1,
        double      to_flux) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Threshold all pixel values to an interval.
  @param    cube1           Cube to threshold.
  @param    lo_cut          Lower threshold value.
  @param    hi_cut          Higher threshold value.
  @param    assign_lo_cut   Value to assign to pixels lower than lo_cut.
  @param    assign_hi_cut   Value to assign to pixels higher than hi_cut.
  @return   int 0 if Ok, -1 otherwise.
 
  Threshold all pixel values in cube frames to a given interval. If
  you do not want to threshold with a lower bound, specify
  MIN_PIX_VALUE for lower threshold. If you do not want to threshold
  with a higher bound, specify MAX_PIX_VALUE for higher threshold.
 
  Values lower than lo_cut will be assigned the value 'assign_lo_cut'.
  Values higher than hi_cut will be assigned the value
  'assign_hi_cut'.
 
  The input cube is modified.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_threshold(
        cube_t      *   cube1,
        pixelvalue      lo_cut,
        pixelvalue      hi_cut,
        pixelvalue      assign_lo_cut,
        pixelvalue      assign_hi_cut) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract one cube from another
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_sub(
        cube_t  *   c1,
        cube_t  *   c2) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Add two cubes.
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_add(
        cube_t  *   c1,
        cube_t  *   c2) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Multiplies two cubes.
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_mul(
        cube_t  *   c1,
        cube_t  *   c2) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Divide a cube by another.
  @param    c1  First operand.
  @param    c2  Second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Both input cubes must have the same dimensions: lx, ly and np.
  This function modifies the first cube to contain the result of the
  operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_div(
        cube_t  *   c1,
        cube_t  *   c2) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Add an image to all planes in a cube.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_add_im(
        cube_t  *   cu,
        image_t *   im) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract an image from all planes in a cube.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_sub_im(
        cube_t  *   cu,
        image_t *   im) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Multiply all planes in a cube by an image.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_mul_im(
        cube_t  *   cu,
        image_t *   im) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Divide all planes in a cube by an image.
  @param    cu      Cube to modify.
  @param    im      Image operand.
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_div_im(
        cube_t  *   cu,
        image_t *   im) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract and Divide all planes in a cube by two images.
  @param    cu      Cube to modify.
  @param    im1     Image to subtract
  @param    im2     Image to divide 
  @return   int 0 if Ok, -1 otherwise.
 
  Every plane in the input cube is modified to contain the result of
  the arithmetic operation.
 */
/*--------------------------------------------------------------------------*/
int cube_subdiv_im(
        cube_t  *   cu, 
        image_t *   im1,
        image_t *   im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Computes a standard deviation image from a cube.
  @param    cube1   Input cube.
  @return   1 newly allocated image.
 
  This function takes the same pixel position in all frames of the
  input cube, computes the standard deviation over time for this
  pixel, and output it into an image (at the same position).
 
  The returned image is thus an image of standard deviations in the
  cube, it does not share the input cube pixel value units.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
image_t * cube_stdev_z(cube_t * cube1) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Transform a cube in its reciprocal.
  @param    c1      Cube to transform.
  @return   int 0 if Ok, -1 otherwise.

  Applies for each pixel p in the cube p=1/p. If p is zero, leave it as
  zero.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_recip(cube_t * c1) ;
/* </python> */


/*-------------------------------------------------------------------------*/
/**
  @brief    Transform a cube in its negation.
  @param    c1      Cube to transform.
  @return   int 0 if Ok, -1 otherwise.

  Applies for each pixel p in the cube p=-p.
 */
/*--------------------------------------------------------------------------*/
/* <python> */
int cube_invert(cube_t * c1) ;
/* </python> */

/*-------------------------------------------------------------------------*/
/**
  @brief    apply dark subtraction and ff division, replace bad pixels
  @param    in  pointer to allocated cube
  @param    ff      flat field
  @param    dark    dark 
  @return   badpix  bad pixel 
 */
/*--------------------------------------------------------------------------*/
int cube_correct_ff_dark_badpix(
    cube_t      *   in,
    image_t     *   ff,
    image_t     *   dark,
    pixelmap    *   badpix);
#endif
