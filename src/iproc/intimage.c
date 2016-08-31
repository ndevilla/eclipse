/*-------------------------------------------------------------------------*/
/**
   @file	intimage.c
   @author	Nicolas Devillard
   @date	July 2000
   @version	$Revision: 1.15 $
   @brief	Image object containing integer pixels
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: intimage.c,v 1.15 2001/12/03 10:51:57 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/12/03 10:51:57 $
	$Revision: 1.15 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "config.h"

#include "qfits.h"
#include "intimage.h"
#include "image_handling.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Integer image constructor.
  @param	lx		Size in x.
  @param	ly		Size in y.
  @return	1 newly allocated intimage.

  Allocates the main pointer and the pixel buffer, and returns the newly
  allocated object. The returned image must be deallocated using
  intimage_del().
 */
/*--------------------------------------------------------------------------*/
intimage * intimage_new(
		int	lx, 
		int	ly)
{
	intimage	*	img ;

	img = malloc(sizeof(intimage));
	img->lx = lx ;
	img->ly = ly ;
	img->data = calloc(lx * ly, sizeof(intpix));
	return img ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Integer image destructor.
  @param	img		Integer image to deallocate.
  @return	void

  Deallocates the main pointer and the pixel buffer.
 */
/*--------------------------------------------------------------------------*/
void intimage_del(intimage * img)
{
	if (img==NULL) return ;
	if (img->data!=NULL) {
		free(img->data);
	}
	free(img);
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Integer image loading from a FITS file.
  @param	filename	Name of the file to load.
  @return	1 newly allocated intimage.

  The loading is outsourced to the standard FITS image loader. Only FITS
  integer types are supported.
 */
/*--------------------------------------------------------------------------*/
intimage * intimage_load(char * filename)
{
	image_t	*	in ;
	intimage	*	loaded ;
	intpix		*	po ;
	pixelvalue	*	pi ;
	int				i ;
	char		*	cval ;
	int				ptype ;

	if (filename==NULL) return NULL ;
	/* Check pixel type in input */
	cval = qfits_query_hdr(filename, "BITPIX");
	if (cval==NULL) {
		e_error("getting BITPIX from file [%s]", filename);
		return NULL ;
	}
	ptype = atoi(cval);
	if ((ptype!=8) && (ptype!=16) && (ptype!=32)) {
		e_error("integer image has BITPIX=%d, should be 8, 16 or 32", ptype);
		return NULL ;
	}
	/* Load a normal image */
	in = image_load(filename);
	if (in==NULL) {
		e_error("loading file [%s]: aborting intimage load", filename);
		return NULL ;
	}
	/* Allocate intimage */
	loaded = intimage_new(in->lx, in->ly) ;
	/*
	 * Convert pixels to integer values
	 */
	po = loaded->data ;
	pi = in->data ;
	for (i=0 ; i<(in->lx * in->ly) ; i++) {
		*po++ = (intpix)(*pi++);
	}
	image_del(in);
	return loaded ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Save an intimage to disk as an integer FITS file.
  @param	img			Integer image to save.
  @param	filename	Output file name.
  @return	void

  Save an intimage to disk as an integer FITS file. The number of bits per
  pixel is determined by the resolution of the intpix type.
 */
/*--------------------------------------------------------------------------*/
void intimage_save(
		intimage	*	img, 
		char		* 	filename)
{
	image_t		*	out ;
	int				intpixbpp ;
	int				i ;
	intpix		*	pi ;
	pixelvalue	*	po ;

	intpixbpp = CHAR_BIT * sizeof(intpix) / sizeof(char) ;
	if (intpixbpp!=8 && intpixbpp!=16 && intpixbpp!=32) {
		e_error("fatal: intpix type does not match an integer\n"
				"re-compile this library with a correct type");
		return ;
	}
	if (img==NULL || filename==NULL) return ;
	/* Convert intimage to image_t */
	out = image_new(img->lx, img->ly);
	pi = img->data ;
	po = out->data ;
	for (i=0 ; i<(out->lx * out->ly) ; i++) {
		*po++ = (pixelvalue)(*pi++);
	}
	image_save_fits(out, filename, intpixbpp);
	image_del(out);
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	add 1 in a defined zone
  @param	img		Integer image (modified !)
  @param	xpos	x coordinate of the zone start
  @param	ypos	y coordinate of the zone start
  @param	xsize	x size of the zone
  @param	ysize	y size of the zone
  @return	0 if ok, -1 otherwise
  	
  Input image is modified. The coordinates respect C convention: first 
  pixel is (0,0).

 */
/*--------------------------------------------------------------------------*/
int	intimage_increment_zone(
		intimage	*	img,
		int				xpos,
		int				ypos,
		int				xsize,
		int				ysize)
{
	int		i,
			j ;

	for (j=ypos ; j<ypos+ysize ; j++) {
		for (i=xpos ; i<xpos+xsize ; i++) {
			img->data[i+j*img->lx]++ ;
		}
	}
	return 0 ;
}


/*
 * This code was pulled out the Usenet and freely adapted to eclipse.
 * Credits: Paul Heckbert (posted on comp.graphics 28 Apr 1988)
 * It is highly unreadable and makes use of goto and other fairly
 * bad programming practices, but works fine and fast. No questions
 * asked...
 */
#define FFSTACK_PUSH(Y, XL, XR, DY) \
    if (sp<stack+stacksz && Y+(DY)>=wy1 && Y+(DY)<=wy2) \
    {sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define FFSTACK_POP(Y, XL, XR, DY) \
    {sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

#define FFSTACK_MAXLINES	10

static void intimage_floodfill(
		intimage	*	lab,
		int				x,
		int				y,
		intpix			label)
{
	struct { int y, xl, xr, dy ; } * stack, * sp ;
	int				wx1, wx2, wy1, wy2 ;
	int				l, x1, x2, dy ;
	intpix			ov ;
	int				stacksz ;
			
	stacksz = FFSTACK_MAXLINES * lab->ly ;
	stack = malloc(stacksz * 4 * sizeof(int));
	sp = stack ;
	wx1 = 0;
	wx2 = lab->lx-1 ;
	wy1 = 0 ;
	wy2 = lab->ly-1 ;

    ov = lab->data[x+y*lab->lx] ;
    if (ov==label || x<wx1 || x>wx2 || y<wy1 || y>wy2) return;
    FFSTACK_PUSH(y, x, x, 1);           /* needed in some cases */
    FFSTACK_PUSH(y+1, x, x, -1);        /* seed segment (popped 1st) */

    while (sp>stack) {
		/* pop segment off stack and fill a neighboring scan line */
		FFSTACK_POP(y, x1, x2, dy);
		/*
		 * segment of scan line y-dy for x1<=x<=x2 was previously filled,
		 * now explore adjacent pixels in scan line y
		 */
		for (x=x1; x>=wx1 && lab->data[x+y*lab->lx]==ov; x--)
			lab->data[x+y*lab->lx] = label ;
		if (x>=x1) goto skip;
		l = x+1;
		if (l<x1) FFSTACK_PUSH(y, l, x1-1, -dy);        /* leak on left? */
		x = x1+1;
		do {
			for (; x<=wx2 && lab->data[x+y*lab->lx]==ov; x++)
			lab->data[x+y*lab->lx] = label ;
			FFSTACK_PUSH(y, l, x-1, dy);
			if (x>x2+1) FFSTACK_PUSH(y, x2+1, x-1, -dy);    /* leak on right? */
skip:       for (x++; x<=x2 && lab->data[x+y*lab->lx]!=ov; x++);
			l = x;
		} while (x<=x2);
	}
	free(stack);
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Labelize a pixel map into an intimage
  @param	map			Pixelmap to labelize.
  @param	maxlabel	Returned number of labels found in map.
  @return	1 newly allocated intimage.

  This function labelizes all blobs in a pixelmap. All 4-neighbour
  connected zones set to 1 in the input pixel map will end up in
  the return intimage as zones where all pixels are set to the same
  (unique for this blob in this image) label.

  A non-recursive flood-fill is applied to label the zones. The flood-fill
  is dimensioned by the number of lines in the image, and the maximal
  number of lines possibly covered by a blob.
 */
/*--------------------------------------------------------------------------*/
intimage * intimage_labelize_pixelmap(
		pixelmap	*	map, 
		int 		* 	maxlabel)
{
	intimage	*	lab ;
	int				i, j ;
	int				npix ;
	intpix			label ;
	int				offpos ;

	/* Copy pixelmap into intimage */
	lab = intimage_new(map->lx, map->ly);
	npix = lab->lx * lab->ly ;
	for (i=0 ; i<npix ; i++) {
		if (map->data[i]==PIXELMAP_0)
			/* No object on this pixel */
			lab->data[i] = (intpix)0 ;
		else
			/* -1 means unprocessed pixel */
			lab->data[i] = (intpix)-1 ;
	}
	/* Now work on intimage */
	label=(intpix)1 ;
	for (j=0 ; j<lab->ly ; j++) {
		offpos = j*lab->lx ;
		for (i=0 ; i<lab->lx ; i++) {
			/* Look up if unprocessed pixel */
			if (lab->data[i+offpos]==(intpix)-1) {
				/* Flood fill from this pixel with the assigned label */
				intimage_floodfill(lab, i, j, label);
				label++ ;
			}
		}
	}
	if (maxlabel!=NULL)
		(*maxlabel)=label-1 ;
	return lab ;
}




