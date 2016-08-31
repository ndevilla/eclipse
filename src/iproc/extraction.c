/*----------------------------------------------------------------------------*/
/**
   @file	extraction.c
   @author	Nicolas Devillard
   @date	Mar 25, 1996
   @version	$Revision: 1.20 $
   @brief	data extraction from a cube
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: extraction.c,v 1.20 2003/03/28 14:17:33 yjung Exp $
	$Author: yjung $
	$Date: 2003/03/28 14:17:33 $
	$Revision: 1.20 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "extraction.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a rectangular zone from a cube into another cube.
  @param    cube_in     Input cube
  @param    loleft_x    Lower left X coordinate
  @param    loleft_y    Lower left Y coordinate
  @param    upright_x   Upper right X coordinate
  @param    upright_y   Upper right Y coordinate
  @return   1 newly allocated cube.
 
  The input coordinates define the extracted region by giving the
  coordinates of the lower left and upper right corners (inclusive).
 
  Coordinates must be provided in the FITS convention: lower left
  corner of the image is at (1,1), x growing from left to right,
  y growing from bottom to top.
 
  The same rectangle is extracted from each plane in the input cube,
  and appended to the output cube.
 
  The returned cube contains pixel copies of the input pixels. It must be
  freed using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_getvig(
    	cube_t *	cube_in,
    	int     	loleft_x,
    	int     	loleft_y,
    	int     	upright_x,
    	int     	upright_y)
{
    cube_t     *	cube_out ;
    int         	i ;
    int         	outlx,
					outly ;

    if (cube_in==NULL) return NULL ;

	if ((loleft_x>upright_x) ||
		(loleft_y>upright_y)) {
		e_error("ill-defined slit for extraction: aborting");
		return NULL ;
	}

    /* Extraction coordinates include rectangular zone  */
    outlx = upright_x - loleft_x + 1 ;
    outly = upright_y - loleft_y + 1 ;

    cube_out = cube_new(outlx, outly, cube_in->np) ;
    /* Loop on all input planes */
    for (i=0 ; i<cube_in->np ; i++) {
        compute_status("extracting subimage", i, cube_in->np, 2) ;
        /* Extract a slit from this plane   */
		cube_out->plane[i] = 
			image_getvig(cube_in->plane[i], 
									loleft_x, loleft_y, 
									upright_x, upright_y) ;
    }
    return cube_out ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a rectangular zone from an image into another image.
  @param    image_in    Input image
  @param    loleft_x    Lower left X coordinate
  @param    loleft_y    Lower left Y coordinate
  @param    upright_x   Upper right X coordinate
  @param    upright_y   Upper right Y coordinate
  @return   1 newly allocated image.
 
  The input coordinates define the extracted region by giving the
  coordinates of the lower left and upper right corners (inclusive).
 
  Coordinates must be provided in the FITS convention: lower left
  corner of the image is at (1,1), x growing from left to right,
  y growing from bottom to top.
 */
/*----------------------------------------------------------------------------*/
image_t * image_getvig(
    	image_t    *	image_in,
    	int         	loleft_x,
    	int         	loleft_y,
    	int         	upright_x,
    	int         	upright_y)
{
    image_t    *	slit_img ;
    int		     	i, j ;
    register
    pixelvalue  *	inpt,
				*	outpt ;
    int         outlx, outly ;

    if (image_in==NULL) return NULL ;

    if ((loleft_x<1) || (loleft_x>image_in->lx) ||
        (loleft_y<1) || (loleft_y>image_in->ly) ||
        (upright_x<1) || (upright_x>image_in->lx) ||
        (upright_y<1) || (upright_y>image_in->ly) ||
        (loleft_x>upright_x) || (loleft_y>upright_y)) {
        e_error("extraction zone is [%d %d] [%d %d]\n"
        		"cannot extract such zone: aborting slit extraction",
                loleft_x, loleft_y, upright_x, upright_y) ;
        return NULL ;
    }

    outlx = upright_x - loleft_x + 1 ;
    outly = upright_y - loleft_y + 1 ; 
    slit_img = image_new(outlx, outly) ;

    for (j=0 ; j<outly ; j++) {
        inpt = image_in->data+loleft_x-1 + (j+loleft_y-1)*image_in->lx ;
        outpt = slit_img->data + j*outlx ;
        for (i=0 ; i<outlx ; i++) {
            *outpt++ = *inpt++ ;
        }
    }
    return slit_img ;
}
                 
/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a row from an image.
  @param    image1      Image to process.
  @param    row_num     Row number.
  @return   1 newly allocated array of pixelvalues.
 
  Extracts a row of pixels from an image. The row number goes from 0 to
  ly-1. The returned array must be freed using free().
 */
/*----------------------------------------------------------------------------*/
pixelvalue * image_getrow(
    	image_t *  image1, 
    	int         row_num)
{
    int           	i ;
    pixelvalue    *	array,
				  *	in,
				  *	out ;

    if (image1==NULL) return NULL ;
    if ((row_num<0) || (row_num>image1->ly-1)) {
        e_error("cannot extract row %d", row_num) ;
        return NULL ;
    }
    array = malloc(image1->lx * sizeof(pixelvalue));
    out = array ;
    in = image1->data + row_num * image1->lx ;
    for (i=0 ; i < image1->lx ; i++){
        *out++ = *in++ ;
    } 
    return array;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a column from an image.
  @param    image       Image to process
  @param    col_num     Column number.
  @return   1 newly allocated array of pixelvalues.
 
  Extracts a column of pixels from an image. The column number goes from
  0 to lx-1. The returned array must be freed using free().
 */
/*----------------------------------------------------------------------------*/
pixelvalue * image_getcol(
    	image_t	*	image1, 
    	int         col_num)
{
    int				i ;
    pixelvalue	*	array,
				*	in,
				*	out ;
    
    if (image1==NULL) return NULL ;
    if ((col_num<0) || (col_num>image1->lx)) {
        e_error("cannot extract column %d", col_num) ;
        return NULL ;
    }
    array = malloc(image1->ly * sizeof(pixelvalue)) ;
    out = array ;
    in = image1->data + col_num ;
    for (i=0 ; i < image1->ly ; i++){
        *out++ = *in ;
        in += image1->lx ;
    }
    return array;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Extract a time line along the z-axis of a cube.
  @param    cube1       Cube to process.
  @param    pos         Position on the detector.
  @return   1 newly allocated image containing one line only.
 
  Extract a line of pixels along the z-axis of a cube. All pixels lying
  on the same detector position are extracted in each plane. A new array
  of pixelvalues is returned, containing as many pixels as planes in the
  input cube. This array is stored into an image containing a single line.
 
  The detector position must be provided as a single number understood
  as i + j*lx, where (i,j) is the position on the detector, in the C
  coordinate convention (i runs from 0 to lx-1, j runs from 0 to ly-1).
 
  The returned image must be freed using image_del().
 */
/*----------------------------------------------------------------------------*/
image_t * cube_get_z(
		cube_t	*	cube1, 
		int 		pos)
{
	image_t	*	time_line ;
	int				p ;
  
	if (cube1==NULL) return NULL ;

	time_line = image_new(cube1->np, 1);
	for (p=0 ; p<cube1->np ; p++){
		time_line->data[p] = cube1->plane[p]->data[pos];
	}
	return time_line;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Create a new cube containing less planes, according to a list.
  @param	cube1		Input cube.
  @param	planes		List of plane indexes to copy.
  @param	np			Number of indexes in the list.
  @return	1 newly allocated cube.

  Create a new cube by extracting from the input cube only the planes
  selected in the list of indexes. Indexes run from 0 to np-1 (incl).

  The returned cube copies the planes of the input cube. It must be freed
  using cube_del().
 */
/*----------------------------------------------------------------------------*/
cube_t * cube_copy_planes(
		cube_t	*	cube1,
		int		*  	planes,
		int			np)
{
    cube_t	*	dest_cube;
    int 		i ;

    if ((cube1==NULL) || (planes==NULL) || (np<1)) return NULL ;
	dest_cube = cube_new(cube1->lx, cube1->ly, np);
	for (i=0 ; i<np ; i++) {
		dest_cube->plane[i] = image_copy(cube1->plane[planes[i]]);
	}
    return dest_cube ;
}

