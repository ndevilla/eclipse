/*----------------------------------------------------------------------------*/
/**
   @file	image_intops.c
   @author	Nicolas Devillard
   @date	September 1999
   @version	$Revision: 1.29 $
   @brief	image integer operation

   This family of operations are just moving pixels around without modifying 
   the values themselves. Most operations happen "in place", i.e. they modify
   their image argument.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: image_intops.c,v 1.29 2003/03/19 13:51:45 yjung Exp $
	$Author: yjung $
	$Date: 2003/03/19 13:51:45 $
	$Revision: 1.29 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "image_intops.h"

/*-----------------------------------------------------------------------------
   								Macros
 -----------------------------------------------------------------------------*/

/*
 * This PIX_SWAP requires a local scope variable called temp, of same
 * type as arguments a and b (usually pixelvalue, unless specified otherwise)
 */
#define PIX_SWAP(a,b) { temp=(a);(a)=(b);(b)=temp; }

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Turn an image by integer half turns.
  @param	image_in		Image to turn.
  @param	orientation		Turn direction.
  @return	int 0 if Ok, -1 otherwise.

  This function operates locally on the pixel buffer.

  Orientation can take one of the following values:
  \begin{itemize}
  \item 90 to turn 90 degrees counterclockwise.
  \item 180 to turn 180 degrees.
  \item -90 to turn 90 degrees clockwise.
  \end{itemize}

  Optimized by Claudio Cumani (ESO). Thanks!
 */
/*----------------------------------------------------------------------------*/
int image_turn(
		image_t	*	image_in, 
		int 		orientation)
{
    pixelvalue             * buf ;
    register pixelvalue    * tbuf ;
    register pixelvalue    * tdata ;
    register pixelvalue    * start ;
    int                      bufsize;
    register int             i, j ;
    register int             lx, ly ;


    /* Safety tests */
    if (image_in==NULL) return -1;
    if (image_in->lx<1 || image_in->ly<1) return -1 ;
    if ((orientation !=  -90)  &&
        (orientation !=   90)  &&
        (orientation !=  180)  &&
        (orientation != -180)  &&
        (orientation !=  270)) {
		e_error("angle [%d] does not allow integer rotation", orientation);
		return -1;
	}

    lx = image_in->lx ;
    ly = image_in->ly ;

	/* Copy input image into temporary buffer */
    bufsize=lx*ly*sizeof(pixelvalue);
    buf = malloc(bufsize);
    memcpy (buf, image_in->data, bufsize);
    tbuf=buf;
    tdata=image_in->data;

    switch(orientation) {
        case 90:
		image_in->lx = ly ;
		image_in->ly = lx ;
		start=tdata+ly-1;
		for (j=0 ; j<ly ; j++) {
			tdata=start;
			for (i=0 ; i<lx ; i++) {
				*tdata=*tbuf++;
				tdata+=ly;
			}
			start--;
		}
        break ;

        case 180:
        case -180:
		for (i=0 ; i<lx*ly ; i++) {
			image_in->data[i] = buf[ly*lx-i-1];
		}
        break ;

        case -90:
        case 270:
		image_in->lx = ly ;
		image_in->ly = lx ;
		start=tdata+(lx-1)*ly;
		for (j=0 ; j<ly ; j++) {
			tdata=start;
			for (i=0 ; i<lx ; i++) {
				*tdata=*tbuf++;
				tdata-=ly;
			}
			start++;
		}
        break ;
    }
    free(buf);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Perform a symmetry around a diagonal in the image.
  @param	in			Image to symmetrize.
  @param	diagonal	Diagonal around which symmetry is applied.
  @return	int 0 if Ok, -1 otherwise.

  Provide 1 for symmetry around y=x (the diagonal from lower left corner to
  upper right) and -1 for symmetry around y=-x (the diagonal from upper
  left corner to lower right). This function also works with non-square
  images.

  This function operates locally on the pixel buffer.
 */
/*----------------------------------------------------------------------------*/
int image_diagonal_symmetry(
		image_t	*	in,
		int			diagonal)
{
	register int	i, j ;
	register int	sq, lx, ly ;
	register pixelvalue	temp ;
	pixelvalue  *	buf_out ;

	/* Sanity tests */
	if (in==NULL) return -1 ;
	if (in->lx<1 || in->ly<1) return -1 ;
	if (diagonal!=1 && diagonal!=-1) {
		e_error("wrong diagonal orientation for symmetry: [%d]", diagonal);
		return -1 ;
	}

	/* If the image is square, can swap in place: much faster */
	if (in->lx == in->ly) {
		sq = in->lx ;
		if (diagonal>0) {
			for (j=0 ; j<sq-1 ; j++)
				for (i=j+1 ; i<sq ; i++)
					PIX_SWAP(in->data[i+j*sq], in->data[j+i*sq]);
		} else {
			for (j=0 ; j<sq-1 ; j++)
				for (i=0 ; i<(sq-j-1) ; i++) {
					PIX_SWAP(in->data[i+j*sq],in->data[(sq-1-j)+(sq-1-i)*sq]);
				}
		}
	} else {
	/* Rectangular image needs buf_out */
		lx = in->lx ;
		ly = in->ly ;
		buf_out = malloc(lx * ly * sizeof(pixelvalue)) ;
		if (diagonal>0) {
			for (j=0 ; j<ly ; j++) {
				for (i=0 ; i<lx ; i++) {
					buf_out[j+i*ly] = in->data[i+j*lx] ;
				}
			}
			in->lx = ly ;
			in->ly = lx ;
		} else {
			for (j=0 ; j<ly ; j++) {
				for (i=0 ; i<lx ; i++) {
					buf_out[(ly-1-j)+(lx-1-i)*ly] = in->data[i+j*lx] ;
				}
			}
			in->lx = ly ;
			in->ly = lx ;
		}
		free(in->data);
		in->data = buf_out ;
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Shift an image by an integer amount.
  @param	inimage		Image to shift.
  @param	x_shift		Shift to apply in x.
  @param	y_shift		Shift to apply in y.
  @return	1 newly allocated image.

  This function shifts an image by an integer amount in x and y directions.
  Circularity: pixels disappearing on an edge come back on the other.

  The returned image must be deallocated using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t * image_shiftint_circular(
		image_t	*	inimage,
        int			x_shift,
        int			y_shift)
{
    image_t    *   outimage ;
    int             i, j ;
    int             x_correct, y_correct ;

    /* Bullet proof shift arguments */
    x_shift = x_shift%inimage->lx ;
    y_shift = y_shift%inimage->ly ;

    /* Error handling */
    if (inimage == NULL) return NULL ;
    if (x_shift==0 && y_shift==0) return image_copy(inimage) ;

    outimage = image_new(inimage->lx, inimage->ly) ;

    /* To support the negative shifts cases */
    if (x_shift>=0) {
        x_correct = inimage->lx ;
    } else {
        x_correct = -inimage->lx ;
    }
    if (y_shift>=0) {
        y_correct = inimage->ly ;
    } else {
        y_correct = -inimage->ly ;
    }

    /* Shift the image */
    for (i=0 ; i<(inimage->lx * inimage->ly) ; i++) {
        if (((i%inimage->lx)+x_shift >= inimage->lx)
            || ((i%inimage->lx)+x_shift < 0)) {
            if (((i/inimage->lx)+y_shift >= inimage->ly)
                || ((i/inimage->lx)+y_shift < 0)) {
                j=i+(x_shift-x_correct)+(y_shift-y_correct)*inimage->lx ;
                outimage->data[j] = inimage->data[i] ;
            } else {
                j = i + (x_shift-x_correct) + y_shift*inimage->lx ;
                outimage->data[j] = inimage->data[i] ;
            }
        } else if (((i/inimage->lx)+y_shift >= inimage->ly)
                || ((i/inimage->lx)+y_shift < 0)) {
            j = i + x_shift + (y_shift-y_correct) * inimage->lx ;
            outimage->data[j] = inimage->data[i] ;
        } else {
            j = i + x_shift + y_shift * inimage->lx ;
            outimage->data[j] = inimage->data[i] ;
        }
    }
    return outimage ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Fill a rectangular zone in an image with a given value.
  @param	in		Image to modify.
  @param	val		Value to insert.
  @param	mini	Minimum i (inclusive)
  @param	maxi	Maximum i (inclusive)
  @param	minj	Minimum j (inclusive)
  @param	maxj	Maximum j (inclusive)
  @return	int 0 if Ok, -1 otherwise.

  This function modifies an image by setting all pixels within a given
  rectangle to the given pixel value. Beware that all bounds are inclusive.
 */
/*----------------------------------------------------------------------------*/
int image_fillrect(
		image_t	*	in,
		pixelvalue 		val,
		int				mini,
		int				maxi,
		int				minj,
		int				maxj)
{
    int i, j, jl;
 
	if (in == NULL) return -1 ;
	if ((mini<0) || (mini>in->lx) || (maxi>=in->lx) ||
		(minj<0) || (minj>in->ly) || (maxj>=in->ly) ) {
		e_error("incorrect bounds for rectangle fill\n"
				"got i in [%d-%d] and j in [%d-%d]",
				mini,maxi,minj,maxj);
		return -1 ;
	}
    for (j=minj; j<=maxj; j++){
        jl = j * in->lx;
        for (i=mini; i<=maxi; i++){
            in->data[i+jl]= val ;
        }
    }
    return 0;
} 

/*----------------------------------------------------------------------------*/
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
/*----------------------------------------------------------------------------*/
image_t * image_paste(
    	image_t		*	frame,
    	image_t		*	insert,
    	int		     	xpos,
    	int		     	ypos)
{
    int         	startx, endx ;
    int         	starty, endy ;
    int         	i, j ;
    int         	posPaste, posInsert ;
    image_t    *   pasted ;
 
    /* Error handling: test entries */
    if ((frame == NULL) || (insert == NULL)) return NULL ;
 
    /*
     * Position controls:
     * test inconsistent values
     * clip the inserted image if out of frame
     */
    if ((xpos<1) || (xpos>frame->lx)) {
        e_error("wrong x coord for insert: %ld", xpos) ;
        return NULL ;
    }
    if ((ypos<1) || (ypos>frame->ly)) {
        e_error("wrong y coord for insert: %ld", ypos) ;
        return NULL ;
    }
    /* Lower left corner should be Ok, keep values  */
    startx = xpos ;
    starty = ypos ;
 
    /* Upper right corner may be clipped */
    endx = insert->lx + xpos - 1 ;
    if (endx > frame->lx) {
        e_warning("clipping for upper left corner in x") ;
        endx = frame->lx ;
    }
    endy = insert->ly + ypos - 1 ;
    if (endy > frame->ly) {
        e_warning("clipping for upper left corner in y") ;
        endy = frame->ly ;
    }
 
    /*
     * Now paste one image into the other
     * ATTENTION! i and j describe the arrays from 0 to n-1,
     * whereas image coordinates describe arrays from 1 to n.
     * shift all positions to go into same coordinate system.
     */
    startx-- ; starty-- ; endx-- ; endy-- ;
 
    /* First, copy frame into output    */
    pasted = image_copy(frame) ;
 
    /* Now insert the other image   */
    for (j=starty ; j<=endy ; j++) {
        for (i=startx ; i<=endx; i++) {
            /* Compute position in each image   */
            posInsert = (i-startx) + (j-starty)*insert->lx ;
            posPaste = i + j*pasted->lx ;
            pasted->data[posPaste] = insert->data[posInsert] ;
        } /* end loop on x  */
    } /* end loop on y  */
    return pasted ;
} 

/*----------------------------------------------------------------------------*/
/**
  @brief    Paste an image vig into another image.
  @param    frame       Image receiving the paste.
  @param    insert      Image vig to insert.
  @param    xpos        x position of the insert.
  @param    ypos        y position of the insert.
  @param	llx_vig		lower left x coordinate of the vig in insert
  @param	lly_vig		lower left y coordinate of the vig in insert
  @param	urx_vig		upper right x coordinate of the vig in insert
  @param	ury_vig		upper right y coordinate of the vig in insert
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
/*----------------------------------------------------------------------------*/
image_t * image_paste_vig(
		image_t	*	frame,
		image_t	*	insert,
		int	    	xpos,
		int	     	ypos,
		int			llx_vig,
		int			lly_vig,
		int			urx_vig,
		int			ury_vig)
{
    image_t    *   pasted ;
	int				err ;
 
	pasted = image_copy(frame);
	err =
	image_paste_vig_local(	pasted,
							insert,
							xpos,
							ypos,
							llx_vig,
							lly_vig,
							urx_vig,
							ury_vig);
	if (err!=0) {
		image_del(pasted);
		pasted = NULL ;
	}
    return pasted ;
} 

/*----------------------------------------------------------------------------*/
/**
  @brief    Paste an image vig into another image.
  @param    frame       Image receiving the paste.
  @param    insert      Image vig to insert.
  @param    xpos        x position of the insert.
  @param    ypos        y position of the insert.
  @param	llx_vig		lower left x coordinate of the vig in insert
  @param	lly_vig		lower left y coordinate of the vig in insert
  @param	urx_vig		upper right x coordinate of the vig in insert
  @param	ury_vig		upper right y coordinate of the vig in insert
  @return   int 0 if Ok, -1 otherwise
 
  Position where to paste refers to coordinates in the frame image. It is
  the position where to paste the first pixel of insert image (first pixel
  being at lower left corner) First pixel is lower left at coordinates
  (1,1).
	
  The vig to paste is defined by its position in the insert image. The 
  coordinates have the same convention: (1,1) is the first pixel.

  The 'frame' input image is modified.
 */
/*----------------------------------------------------------------------------*/
int image_paste_vig_local(
		image_t		*	frame,
	    image_t    	*	insert,
	    int		     	xpos,
	    int		     	ypos,
		int				llx_vig,
		int				lly_vig,
		int				urx_vig,
		int				ury_vig)
{
    int         	startx, endx ;
    int         	starty, endy ;
    int             llx, lly, urx, ury ;
    int         	i, j ;
    int         	posPaste, posInsert ;

    /* Error handling: test entries */
    if ((frame == NULL) || (insert == NULL)) return -1 ;
    if ((llx_vig > insert->lx) || (lly_vig > insert->ly) || (urx_vig < 1) 
            || (ury_vig < 1)) return -1 ;
    
    /* Check if the vig is completely inserted in the insert image */
    if (llx_vig < 1) llx = 1 ;
    else llx = llx_vig ;
    if (lly_vig < 1) lly = 1 ;
    else lly = lly_vig ;
    if (urx_vig > insert->lx) urx = insert->lx ;
    else urx = urx_vig ;
    if (ury_vig > insert->ly) ury = insert->ly ;
    else ury = ury_vig ;
    
    /*
     * Position controls:
     * test inconsistent values
     * clip the inserted image if out of frame
     */
    if ((xpos<1) || (xpos>frame->lx)) {
        e_error("wrong x coord for insert: %ld", xpos) ;
        return -1 ;
    }
    if ((ypos<1) || (ypos>frame->ly)) {
        e_error("wrong y coord for insert: %ld", ypos) ;
        return -1 ;
    }
    /* Lower left corner should be Ok, keep values  */
    startx = xpos ;
    starty = ypos ;
 
    /* Upper right corner may be clipped */
    endx = urx - llx + xpos ;
    if (endx > frame->lx) {
        e_warning("clipping for upper left corner in x") ;
        endx = frame->lx ;
    }
    endy = ury - lly + ypos ;
    if (endy > frame->ly) {
        e_warning("clipping for upper left corner in y") ;
        endy = frame->ly ;
    }
 
    /*
     * Now paste the vig into the other image
     * ATTENTION! i and j describe the arrays from 0 to n-1,
     * whereas image coordinates describe arrays from 1 to n.
     * shift all positions to go into same coordinate system.
     */
    startx-- ; starty-- ; endx-- ; endy-- ;

    /* Now insert the other image   */
    for (j=starty ; j<=endy ; j++) {
        for (i=startx ; i<=endx; i++) {
            /* Compute position in each image   */
            posInsert = (i-startx+llx-1) + (j-starty+lly-1)*insert->lx ;
            posPaste = i + j*frame->lx ;
            frame->data[posPaste] = insert->data[posInsert] ;
        } /* end loop on x  */
    } /* end loop on y  */
    
    return 0 ;
} 
#undef PIX_SWAP

/*----------------------------------------------------------------------------*/
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
/*----------------------------------------------------------------------------*/
int image_swapquad(image_t * in)
{
    int         i, j ;
    pixelvalue  temp ;
    int         offs1, offs2 ;
    int         n ;

    if (in==NULL) return -1 ;
    if (in->lx != in->ly) {
        e_error("image dimensions are [%d x %d]\n"
        		"image must be square to be swapped : aborting swap",
				in->lx, in->ly) ;
        return -1 ;
    }
    if (in->lx % 2) {   
        e_error("image width is %d\n"
				"should have an even pixel size to be swapped: aborting",
				in->lx) ;
        return -1 ;
    }

    /* 
     * Exchange quadrants :     1 2 becomes     3 4
     *                          3 4             2 1
     *
     * The algorithm is :
     * For every pixel in quadrant 1:
     * swap (pixel, pixel + (n/2)*(n+1))
     * For every pixel in quadrant 2:
     * swap (pixel, pixel + (n/2)*(n-1)
     *
     */

	n = in->lx ;
    offs1 = (n/2)*(n+1) ;
    offs2 = (n/2)*(n-1) ;

    /* Loop on quadrant 1 and 2 only (swapping then covers the whole image) */
    for (j=0 ; j<(n/2) ; j++) {
        /* quadrant 1 & 4 */
        for (i=0 ; i<(n/2) ; i++) {
            temp = in->data[i+j*n+offs1] ;
            in->data[i+j*n+offs1] = in->data[i+j*n] ;
            in->data[i+j*n] = temp ;
        }
        /* quadrant 2 & 3 */
        for (i=n/2 ; i<n ; i++) {
            temp = in->data[i+j*n+offs2] ;
            in->data[i+j*n+offs2] = in->data[i+j*n] ;
            in->data[i+j*n] = temp ;
        }
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
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
/*----------------------------------------------------------------------------*/
int image_draw_circle(
	image_t	* im,
	int	      cx,
	int	      cy,
	int	      rad,
	int		  colour
)
{
    int d, x, y ;

    /* Bulletproof */
    if (im==NULL)
        return -1;
    if (cx<1 || cx>im->lx || cy<1 || cy>im->ly || rad<0)
        return -1 ;

    /* Draw circle in the image */
    d = 3 - 2 * rad ;
    x = 0 ;
    y = rad ;

    while (x<y) {
        im->data[(cx+x)+(cy+y)*im->lx] = colour ;
        im->data[(cx+x)+(cy-y)*im->lx] = colour ;
        im->data[(cx-x)+(cy+y)*im->lx] = colour ;
        im->data[(cx-x)+(cy-y)*im->lx] = colour ;
        im->data[(cx+y)+(cy+x)*im->lx] = colour ;
        im->data[(cx+y)+(cy-x)*im->lx] = colour ;
        im->data[(cx-y)+(cy+x)*im->lx] = colour ;
        im->data[(cx-y)+(cy-x)*im->lx] = colour ;

        if (d<0) {
            d = d + (4*x) + 6;
        } else {
            d = d + 4 * (x-y) + 10;
            y--;
        }
        x++;
    }
    return 0 ;
}
