/*-------------------------------------------------------------------------*/
/**
   @file    image_arith.h
   @author  Nicolas Devillard
   @date    Aug 11, 1995
   @version $Revision: 1.23 $
   @brief   basic arithmetic functions over images
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: image_arith.h,v 1.23 2003/10/30 12:44:01 llundin Exp $
    $Author: llundin $
    $Date: 2003/10/30 12:44:01 $
    $Revision: 1.23 $
*/

#ifndef _IMAGE_ARITHMETIC_H_
#define _IMAGE_ARITHMETIC_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "xmemory.h"
#include "cube_defs.h"

#include "image_handling.h"
#include "intimage.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/* Image normalization types    */
#define NORM_SCALE      1
#define NORM_MEAN       2
#define NORM_FLUX       3
#define NORM_AFLUX      4

/*---------------------------------------------------------------------------
  						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Add two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_add(
        image_t *   image1,
        image_t *   image2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_sub(
        image_t *image1,
        image_t *image2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Multiply two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_mul(
        image_t *image1,
        image_t *image2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Divide two images.
  @param    image1  first operand
  @param    image2  second operand
  @return   1 newly allocated image.
 
  Creates a new image, being the result of the operation, and returns it to
  the caller. The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_div(
        image_t *image1,
        image_t *image2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Add two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_add_local(
        image_t * im1,
        image_t * im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_sub_local(
        image_t * im1,
        image_t * im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Multiply two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_mul_local(
        image_t * im1,
        image_t * im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Divide two images, store the result in the first image.
  @param    im1     first operand.
  @param    im2     second operand.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_div_local(
        image_t * im1,
        image_t * im2) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract and divide an image, store the result in the first im. 
  @param    im1     first image
  @param    im2     image subtracted
  @param    im3     image divided
  @return   int 0 if Ok, -1 otherwise.
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_subdiv_local(
        image_t * im1,
        image_t * im2,
        image_t * im3) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Divide an image by an intimage, store the result in the first 
            image.
  @param    im1     first operand.
  @param    im2     second operand (int image).
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_div_intimage_local(
        image_t     * im1,
        intimage    * im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract im2 from im1, and multiply the result by a constant.
  @param    im1     first operand.
  @param    im2     second operand.
  @param    fact    multiplicative factor.
  @return   int 0 if Ok, -1 otherwise.
 
  The first input image is modified to contain the results of the
  operation.
 */
/*--------------------------------------------------------------------------*/
int image_submul_local(
        image_t     *   im1,
        image_t     *   im2,
        pixelvalue      fact) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Add im2 (1D signal) to each row or column of im1.
  @param    im1     image to modify
  @param    im2     1D signal to add
  @return   int 0 if ok -1 otherwise.
    
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_add_1d_local(
        image_t *   im1,
        image_t *   im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract im2 (1D signal) from each row or column of im1.
  @param    im1     image to modify
  @param    im2     1D signal to subtract
  @return   int 0 if ok -1 otherwise.
    
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_sub_1d_local(
        image_t *   im1,
        image_t *   im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Multiply im2 (1D signal) to each row or column of im1.
  @param    im1     image to modify
  @param    im2     1D sgnal to multiply 
  @return   int 0 if ok -1 otherwise.
    
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_mul_1d_local(
        image_t *   im1,
        image_t *   im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Divide each row or column of im1 by im2 (1D signal).
  @param    im1     image to modify
  @param    im2     1D signal 
  @return   int 0 if ok -1 otherwise.
    
  im2->lx and ly are tested to see the orientation.
 */
/*--------------------------------------------------------------------------*/
int image_div_1d_local(
        image_t *   im1,
        image_t *   im2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Arithmetic between an image and a constant.
  @param    image_in    Image operand.
  @param    constant    Constant operand.
  @param    operation   Operation to perform.
  @return   1 newly allocated image.
 
  Performs an operation between an image and a constant, returns a newly
  allocated image containing the result. Possible operations are:
 
  \begin{itemize}
  \item '+' Addition
  \item '-' Subtraction
  \item '*' Multiplication
  \item '/' Division
  \item 'l' Logarithm
  \item '^' Power
  \item 'e' Exponentiation
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
image_t * image_cst_op(
        image_t     *   image_in,
        double          constant,
        int             operation) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Arithmetic between an image and a constant.
  @param    image_in    Image operand.
  @param    constant    Constant operand.
  @param    operation   Operation to perform.
  @return   int 0 if Ok, -1 otherwise.
 
  Performs an operation between an image and a constant, stores the
  results in the input image buffer. Possible operations are:
 
  \begin{itemize}
  \item '+' Addition
  \item '-' Subtraction
  \item '*' Multiplication
  \item '/' Division
  \item 'l' Logarithm
  \item '^' Power
  \item 'e' Exponentiation
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
int image_cst_op_local(
        image_t *   image_in,
        double      constant,
        int         operation) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Normalize pixels in an image.
  @param    image_in    Image operand.
  @param    mode        Normalization mode.
  @return   1 newly allocated image.
 
  Normalizes an image according to a given criterion, stores the results
  in a newly allocated image and returns it. The returned image must be
  freed using image_del().
 
  Possible normalizations are:
  \begin{itemize}
  \item NORM_SCALE sets the pixel interval to [0,1].
  \item NORM_MEAN sets the mean value to 1.
  \item NORM_FLUX sets the flux to 1.
  \item NORM_AFLUX sets the absolute flux to 1.
  \end{itemize}
 */
/*--------------------------------------------------------------------------*/
image_t * image_normalize(
        image_t *   image_in,
        int         mode) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Threshold an image to a given interval.
  @param    image_in        Image to threshold.
  @param    lo_cut          Lower bound.
  @param    hi_cut          Higher bound.
  @param    assign_lo_cut   Value to assign to pixels below low bound.
  @param    assign_hi_cut   Value to assign to pixels above high bound.
  @return   1 newly allocated image.
 
  Pixels outside of the provided interval are assigned the given
  values. To specify no threshold on lower bound, set lo_cut to
  MIN_PIX_VALUE. To specify no threshold on higher bound, set
  hi_cut to MAX_PIX_VALUE.
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_threshold(
        image_t     *   image_in,
        pixelvalue      lo_cut,
        pixelvalue      hi_cut,
        pixelvalue      assign_lo_cut,
        pixelvalue      assign_hi_cut) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Take the absolute value of an image.
  @param    image_in    Image operand.
  @return   1 newly allocated image.
 
  For each pixel, out = abs(in). The returned image must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_abs(image_t * image_in) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Build the average of two images.
  @param    image_1     First image operand.
  @param    image_2     Second image operand.
  @return   1 newly allocated image.
 
  Builds the average of two images and returns a newly allocated image,
  to be freed using image_del(). The average is arithmetic, i.e.
  outpix=(pix1+pix2)/2
 */
/*--------------------------------------------------------------------------*/
image_t * image_mean(
        image_t *   image_1,
        image_t *   image_2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract min value from all pixels.
  @param    image_in    Image to modify.
  @return   int 0 if Ok, -1 otherwise.
 
  Find the minimum value of an image, and subtract it from all pixels.
  This ends up making all pixels in the image non-negative.
 
  The input image is modified.
 */
/*--------------------------------------------------------------------------*/
int image_submin(image_t * image_in) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Build an integer-only image from an image.
  @param    image_in
  @return   1 newly allocated image.
 
  For each pixel in input, the output pixel is computed with the floor()
  function, i.e. all output pixels are integer-valued. floor() finds the
  closest least integer.
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_floor(image_t * image_in) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the reciprocal of an image, i.e. 1/image.
  @param    image_in    Image operand.
  @return   1 newly allocated image.
 
  Compute the reciprocal image, out = 1/in. Notice that zero-valued pixels
  in input are also zero-valued in output.
 
  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_recip(image_t * image_in) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Inverts all pixels in an image, i.e. image = -image.
  @param    in  Image to modify.
  @return   int 0 if Ok, -1 otherwise.
 
  Operates in place: all pixels are inverted, i.e. image=-image.
 */
/*--------------------------------------------------------------------------*/
int image_invert(image_t * in) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Subtract the median of each row from all pixels in the row.
  @param    in  Input image.
  @return   int 0 if Ok, -1 otherwise.
 
  For each row in the input image, compute the median value of all
  pixels in the row, and remove it from all pixels in the row. This
  effectively brings all row to have a zero median value and slightly
  modifies the flux of the input image. This algorithm is used
  to remove row saturation effects.
 
  The input image is modified.
 */
/*--------------------------------------------------------------------------*/
int image_sub_rowmedian(image_t * in) ;


/*---------------------------------------------------------------------------*/
/**
    @brief  Subtract a lowpas
    @param  in              input image (modified)
    @param  orientation     0 horizontal median, 1 for vertical median
    @param  window_size     the window size for the lowpasss
    @return 0 if OK, -1 if not
    
    Subtracts a low pass filtered 1-d median from image.
    The image is modified destructively 
*/
/*---------------------------------------------------------------------------*/
int image_sub_lowpass(
        image_t *   in,
        int         orientation,
        int         window_size) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Collapse a vig along its rows or columns.
  @param    inimage     Image to collapse.
  @param    llx         lower left x coord.
  @param    lly         lower left y coord
  @param    urx         upper right x coord
  @param    ury         upper right y coord
  @param    direction   Collapsing direction.
  @return   collapsed image 
    
  llx, lly, urx, ury are vig coordinates in FITS convention 
 */
/*--------------------------------------------------------------------------*/
image_t * image_collapse_vig(
        image_t *   in,
        int         llx,
        int         lly,
        int         urx,
        int         ury,
        int         direction) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Collapse an image along its rows or columns.
  @param    inimage     Image to collapse.
  @param    direction   Collapsing direction.
  @return   1 newly allocated image having 1 row or 1 column.
 
  Collapsing an image means building up a 1d signal by adding up all pixels
  on the same row or column.
 
  \begin{verbatim}
  Collapse along y:
 
  p7  p8  p9     Input image is a 3x3 image containing 9 pixels.
  p4  p5  p6     The output is an image containing one row with
  p1  p2  p3     3 pixels A, B, C, where:
  ----------
 
  A   B   C      A = p1+p4+p7
                 B = p2+p5+p8
                 C = p3+p6+p9
  \end{verbatim}
 
  Provide the collapsing direction as an int. Give 0 to collapse along y
  (sum of rows) and get an image with a single row in output, or give 1
  to collapse along x (sym of columns) to get an image with a single
  column in output.
 
 */
/*--------------------------------------------------------------------------*/
image_t * image_collapse(
        image_t *   inimage,
        int         direction) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Collapse an image along its rows and columns, with filtering.
  @param    in          Input image.
  @param    direction   Collapsing direction.
  @param    discard_lo  Low rejection parameter.
  @param    discard_hi  High rejection parameter.
  @return   1 newly allocated image having 1 row or 1 column.
  @see      collapse_image
 
  Collapsing is done as for collapse_image(). The difference is that pixels
  are not just summed along rows or columns but instead all sorted, the
  highest and lowest pixel values are removed, and the remaining values
  are linearly averaged to produce the output value.
 
 */
/*--------------------------------------------------------------------------*/
image_t * image_collapse_median(
        image_t     *   in,
        int             direction,
        int             discard_lo,
        int             discard_hi) ;

#endif
