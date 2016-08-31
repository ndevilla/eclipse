/*-------------------------------------------------------------------------*/
/**
   @file	image_handling.c
   @author	Nicolas Devillard
   @date	Aug 17, 1995
   @version	$Revision: 1.37 $
   @brief	image data structure handling routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: image_handling.c,v 1.37 2002/02/11 12:07:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/02/11 12:07:06 $
	$Revision: 1.37 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

/* Shared memory handling */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "image_handling.h"
#include "image_rtd.h"
#include "qfits.h"

/* Get the definition of CHAR_BIT */
#include "config.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocate an image structure and pixel buffer for an image.
  @param    size_x  Size in x
  @param    size_y  Size in y
  @return   1 newly allocated image.
 
  Allocates both space for the image structure and the pixel buffer. The
  returned pixel buffer is always seen as if it were in memory.
 
  The returned image must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_new(
		int 	size_x, 
		int 	size_y)
{
    image_t    *	image_new ;

    if ((size_x<1) ||
        (size_x>MAX_COLUMN_NUMBER) || 
        (size_y<1) || 
        (size_y>MAX_LINE_NUMBER)) {
        e_error("cannot create image with size [%dx%d]", size_x, size_y) ;
        return NULL ;
    }

    image_new = calloc(1, sizeof(image_t)) ;

    image_new->lx = size_x ;
    image_new->ly = size_y ;
    image_new->data = calloc(size_x * size_y, sizeof(pixelvalue));
    return image_new ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Free memory associated to an image object.
  @param	d	Image to destroy.
  @return	void

  Frees all memory associated to an image.
 */
/*--------------------------------------------------------------------------*/
void image_del(image_t * d)
{
    if (d == NULL) return ;
	if (d->data != NULL) {
		free(d->data) ;
	}
	free(d) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Get the size of an image in bytes.
  @param	im	Image to examine.
  @return	size of the image in memory in bytes, as an int.

  This function computes the size taken in memory by the given image, in
  bytes, and returns it as an int. If a problem occurs during computation
  (e.g. because of inconsistent structure values), this function returns
  -1.
 
  This function is useful to declare the size of a given image to a garbage
  collector, for example. Since it depends on the local implementation
  chosen by the compiler, it is likely that identical image sizes might
  return different values. This is only useful to know the amount of memory
  taken up by one image in the current program.
 */
/*--------------------------------------------------------------------------*/
int image_get_bytesize(image_t * im)
{
	int		bs ;

	if (im==NULL) return -1 ;

	bs = 0 ;
	/* Start with the size of the struct itself */
	bs += sizeof(image_t);
	/* Add up plane size */
	bs += (im->lx * im->ly * sizeof(pixelvalue));

	return bs ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Copy an image.
  @param	src_img		Source image.
  @return	1 newly allocated image.

  Copy an image into a new image object. The pixel buffer is also copied.
  The returned image must be deallocated using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_copy(image_t * src_img)
{
    image_t * dest_img ;

    if (src_img==NULL) return NULL ;
    dest_img = image_new(src_img->lx, src_img->ly) ;
    memcpy(dest_img->data,
           src_img->data,
           src_img->lx * src_img->ly * sizeof(pixelvalue)) ;
    return dest_img ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Build an image from a shared memory segment.
  @param	shmid		Id of the shared memory segment to use.
  @param	offset		Offset from segment start.
  @param	lx			Image size in X.
  @param	ly			Image size in Y.
  @param	bpp			Bytes per pixel a la FITS (8,16,32,-32,-64).
  @return	1 newly allocated image.

  This function expects a shared memory ID and basic image info.
  It will attach itself to the segment and copy the segment contents
  over to a new image structure, which is then returned.

  This is expected to work with programs that allocate images in
  shared memory like RTD (VLT software).
 */
/*--------------------------------------------------------------------------*/
image_t * image_from_shmem(
		int shmid,
		int	offset,
		int lx,
		int ly,
		int bpp)
{
	image_t	*	newim ;
	BYTE	*	psource ;

	/* Test entries */
	if (bpp!=8 && bpp!=16 && bpp!=32 && bpp!=-32 && bpp!=-64) {
		return NULL ;
	}

	/* Retrieve a pointer to the shared memory segment */
	psource = (BYTE*)shmat(shmid, 0, 0);
	if (psource==NULL || psource==(void*)-1) {
		perror("shmat");
		e_error("cannot attach to shared memory segment");
		return NULL ;
	}

	/* Allocate new image */
	newim = malloc(sizeof(image_t));
	newim->lx = lx ;
	newim->ly = ly ;

	/* Convert input pixels to pixelvalue format */
	newim->data = 
        (pixelvalue*)
		qfits_pixin_float(	(byte*)psource+offset,
							lx * ly,
							bpp,
							1.0,
							0.0);
	/* Detach segment */
	shmdt((void*)psource);
	return newim ;
}

