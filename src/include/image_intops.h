/*-------------------------------------------------------------------------*/
/**
   @file    image_intops.h
   @author  Nicolas Devillard
   @date    September 1999
   @version $Revision: 1.18 $
   @brief   image integer operation

   This family of operations are just moving pixels around without modifying 
   the values themselves. Most operations happen "in place", i.e. they modify
   their image argument.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: image_intops.h,v 1.18 2002/07/31 14:34:38 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/07/31 14:34:38 $
    $Revision: 1.18 $
*/

#ifndef _IMAGE_INTOPS_H_
#define _IMAGE_INTOPS_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "local_types.h"
#include "image_handling.h"

/*---------------------------------------------------------------------------
						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Turn an image by integer half turns.
  @param    image_in        Image to turn.
  @param    orientation     Turn direction.
  @return   int 0 if Ok, -1 otherwise.

  This function operates locally on the pixel buffer.

  Orientation can take one of the following values:
  \begin{itemize}
  \item 90 to turn 90 degrees counterclockwise.
  \item 180 to turn 180 degrees.
  \item -90 to turn 90 degrees clockwise.
  \end{itemize}

  Optimized by Claudio Cumani (ESO). Thanks!
 */
/*--------------------------------------------------------------------------*/
int image_turn(
        image_t *   image_in,
        int         orientation) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Perform a symmetry around a diagonal in the image.
  @param    in          Image to symmetrize.
  @param    diagonal    Diagonal around which symmetry is applied.
  @return   int 0 if Ok, -1 otherwise.

  Provide 1 for symmetry around y=x (the diagonal from lower left corner to
  upper right) and -1 for symmetry around y=-x (the diagonal from upper
  left corner to lower right). This function also works with non-square
  images.

  This function operates locally on the pixel buffer.
 */
/*--------------------------------------------------------------------------*/
int image_diagonal_symmetry(
        image_t *   in,
        int         diagonal) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Shift an image by an integer amount.
  @param    inimage     Image to shift.
  @param    x_shift     Shift to apply in x.
  @param    y_shift     Shift to apply in y.
  @return   1 newly allocated image.

  This function shifts an image by an integer amount in x and y directions.
  Circularity: pixels disappearing on an edge come back on the other.

  The returned image must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_shiftint_circular(
        image_t *   inimage,
        int         x_shift,
        int         y_shift) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Fill a rectangular zone in an image with a given value.
  @param    in      Image to modify.
  @param    val     Value to insert.
  @param    mini    Minimum i (inclusive)
  @param    maxi    Maximum i (inclusive)
  @param    minj    Minimum j (inclusive)
  @param    maxj    Maximum j (inclusive)
  @return   int 0 if Ok, -1 otherwise.

  This function modifies an image by setting all pixels within a given
  rectangle to the given pixel value. Beware that all bounds are 
  inclusive.
 */
/*--------------------------------------------------------------------------*/
int image_fillrect(
        image_t *   in,
        pixelvalue      val,
        int             mini,
        int             maxi,
        int             minj,
        int             maxj) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Paste an image into another.
  @param    frame       Image receiving the paste.
  @param    insert      Image to insert.
  @param    xpos        x position of the insert.
  @param    ypos        y position of the insert.
  @return   1 newly allocated image.
 
  Position where to paste refers to coordinates in the frame image. It is
  the position where to paste the first pixel of insert image (first pixel
  being at lower left corner) First pixel is lower left at coordinates
  (1,1).
 
  The returned image is a new object, containing pixels from both input
  frames. It must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_paste(
        image_t     *   frame,
        image_t     *   insert,
        int             xpos,
        int             ypos) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Paste an image vig into another image.
  @param    frame       Image receiving the paste.
  @param    insert      Image vig to insert.
  @param    xpos        x position of the insert.
  @param    ypos        y position of the insert.
  @param    llx_vig     lower left x coordinate of the vig in insert
  @param    lly_vig     lower left y coordinate of the vig in insert
  @param    urx_vig     upper right x coordinate of the vig in insert
  @param    ury_vig     upper right y coordinate of the vig in insert
  @return   1 newly allocated image.
 
  Position where to paste refers to coordinates in the frame image. It is
  the position where to paste the first pixel of insert image (first pixel
  being at lower left corner) First pixel is lower left at coordinates
  (1,1).
    
  The vig to paste is defined by its position in the insert image. The 
  coordinates have the same convention: (1,1) is the first pixel.
  
  The returned image is a new object, containing pixels from both input
  frames. It must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_paste_vig(
        image_t *   frame,
        image_t *   insert,
        int         xpos,
        int         ypos,
        int         llx_vig,
        int         lly_vig,
        int         urx_vig,
        int         ury_vig) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Paste an image vig into another image.
  @param    frame       Image receiving the paste.
  @param    insert      Image vig to insert.
  @param    xpos        x position of the insert.
  @param    ypos        y position of the insert.
  @param    llx_vig     lower left x coordinate of the vig in insert
  @param    lly_vig     lower left y coordinate of the vig in insert
  @param    urx_vig     upper right x coordinate of the vig in insert
  @param    ury_vig     upper right y coordinate of the vig in insert
  @return   int 0 if Ok, -1 otherwise
 
  Position where to paste refers to coordinates in the frame image. It is
  the position where to paste the first pixel of insert image (first pixel
  being at lower left corner) First pixel is lower left at coordinates
  (1,1).
    
  The vig to paste is defined by its position in the insert image. The 
  coordinates have the same convention: (1,1) is the first pixel.

  The 'frame' input image is modified.
 */
/*--------------------------------------------------------------------------*/
int image_paste_vig_local(
        image_t     *   frame,
        image_t     *   insert,
        int             xpos,
        int             ypos,
        int             llx_vig,
        int             lly_vig,
        int             urx_vig,
        int             ury_vig) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Swap 4 quadrants in an image.
  @param    in      Input image.
  @return   int 0 if Ok, -1 otherwise.

  Swap 4 quadrants in an image:
  \begin{verbatim}
  1  2                 4  3
           becomes
  3  4                 2  1
  \end{verbatim}

  The input image is modified.
 */
/*--------------------------------------------------------------------------*/
int image_swapquad(image_t * in) ;



/*-------------------------------------------------------------------------*/
/**
  @brief	Draw a circle in an image.	
  @param	im		Image to draw into
  @param	cx		Circle center x (pixels)
  @param	cy		Circle center y (pixels)
  @param	rad		Circle radius (pixels)
  @param	colour	Colour to assign.
  @return	int 0 if Ok, -1 if error occurred.

  This function draws a circle in an image. The given circle coordinates
  are expected in C notation: x from 0 to lx-1, y from 0 to ly-1.
 */
/*--------------------------------------------------------------------------*/
int image_draw_circle(
	image_t	* im,
	int	      cx,
	int	      cy,
	int	      rad,
	int		  colour
);

#endif
