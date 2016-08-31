/*-------------------------------------------------------------------------*/
/**
   @file	cube_handling.c
   @author	Nicolas Devillard
   @date	Aug 03, 1995
   @version	$Revision: 1.24 $
   @brief	handling of cubes in working area
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: cube_handling.c,v 1.24 2001/10/26 14:33:45 yjung Exp $
	$Author: yjung $
	$Date: 2001/10/26 14:33:45 $
	$Revision: 1.24 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <string.h>
#include "cube_handling.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Cube constructor.
  @param	lx		Size in x of all planes.
  @param	ly		Size in y of all planes.
  @param	n_im	Number of planes in the cube.
  @return	Newly allocated cube object.

  This constructor takes care of allocating pointers and all
  housekeeping stuff, but does not allocate the image planes
  themselves. You need to hook in image planes by assigning into the
  'plane' field of this structure.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_new(int lx, int ly, int n_im)
{
    cube_t *	n ;
    int         i ;

    if ((lx<=0)||(lx>MAX_COLUMN_NUMBER)||(ly<=0)||(ly>MAX_LINE_NUMBER) ||
        (n_im<=0) || (n_im>MAX_IMAGE_NUMBER)) {
        e_error("error in requested cube size: [%d x %d x %d]", lx, ly, n_im) ;
        return NULL ;
    }

    /* Try to get allocation for a new structure    */
    n = malloc(sizeof(cube_t)) ;
	n->plane = malloc(n_im * sizeof(image_t*)) ;
    for (i=0 ; i<n_im ; i++) {
        n->plane[i] = NULL ; 
	}
    n->lx = lx ;
    n->ly = ly ;
    n->np = n_im ;

    return n;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Get the size of a cube in bytes.
  @param	cu	Cube to examine.
  @return	size of the cube in memory in bytes, as an int.

  This function computes the size taken in memory by the given cube, in
  bytes, and returns it as an int. If a problem occurs during computation
  (e.g. because of inconsistent structure values), this function returns
  -1.

  This function is useful to declare the size of a given cube to a garbage
  collector, for example. Since it depends on the local implementation
  chosen by the compiler, it is likely that identical cube sizes might
  return different values. This is only useful to know the amount of memory
  taken up by one cube in the current program.
 */
/*--------------------------------------------------------------------------*/
int cube_get_bytesize(cube_t * cu)
{
	int		bs ;
	int		i ;

	if (cu==NULL) return -1 ;

	bs = 0 ;
	/* Start with the size of the struct itself */
	bs += sizeof(cube_t);
	/* Add up individual image sizes */
	for (i=0 ; i<cu->np ; i++) {
		bs += image_get_bytesize(cu->plane[i]);
	}
	return bs ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Transform an image into a one-plane cube.
  @param	candidate	Image to promote.
  @return	1 newly allocated cube object.

  The returned cube contains a copy of the image given in input.
  Example code:
  \begin{verbatim}
    image_t	*	im ;
    cube_t		*	cb ;
    im = image_load("test.fits") ;
    cb = cube_from_image(im) ;
    cube_del(cb) ;
    
    ---> still need to image_del(im);
    
    image_del(im) ;
  \end{verbatim}
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_from_image(image_t * candidate)
{
    cube_t * promoted ;

    if (candidate == NULL) return NULL ;
    promoted = cube_new(candidate->lx, candidate->ly, 1) ;
    promoted->plane[0] = image_copy(candidate) ;
    if (promoted->plane[0] == NULL) {
        cube_del(promoted) ;
        return NULL;
    }
    return promoted ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Make a cube from a list of images.
  @param	list		A list of image pointers.
  @param	np			Number of pointers in the list.
  @return	1 newly allocated cube structure holding the image pointers.

  This function makes a cube structure and fills its plane field with the
  provided image pointers. The pointers are just copied, the pixel data are
  not.

  Attention: when calling the cube destructure, cube_del will also
  deallocate all images. If you only want to deallocate the cube structure
  which has been allocated in this function, use cube_del().

  The input images must all share the same size in x and y.

  This function returns NULL in case of failure.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_from_list(image_t ** list, int np)
{
	cube_t * cu ;
	int		  i ;

	/* Check entries */
	if (list==NULL || np<1) return NULL ;

	/* Check image pointers and image sizes */
	for (i=0 ; i<np ; i++) {
		if (list[i]==NULL) {
			e_error("NULL image pointer in list: aborting cube creation");
			return NULL ;
		}
		if ((list[i]->lx != list[0]->lx) ||
			(list[i]->ly != list[0]->ly)) {
			e_error("images have different sizes: aborting cube creation");
			return NULL ;
		}
	}

	/* Create new cube, fill it up with image pointers */
	cu = cube_new(list[0]->lx, list[0]->ly, np);
	memcpy(cu->plane, list, np * sizeof(image_t*));

	return cu ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Copy a cube and all its planes.
  @param	src_cube	Cube to copy.
  @return	1 newly allocated cube objet.
  
  The returned copy contains copies of all frames in the input cube.
 */
/*--------------------------------------------------------------------------*/
cube_t * cube_copy(cube_t *src_cube)
{
    cube_t	*	dest_cube ;
	int			i ;

    if (src_cube==NULL) return NULL ;
    /* allocate data zone, fill up information structure */
    dest_cube  = cube_new(  src_cube->lx,
                            src_cube->ly,
                            src_cube->np) ;
    
    /* Then copy data zones from one cube to the other  */
    for (i=0 ; i<src_cube->np ; i++) {
		dest_cube->plane[i] = image_copy(src_cube->plane[i]) ;
	}
    return dest_cube ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Frees all memory associated to a cube.
  @param	d	Cube to free.
  @return	void

  Frees all memory associated to a cube. Recursively calls
  image_del on all planes of the cube. If some of the planes in
  the cube are referenced by other pointers, data will be lost. If you
  only want to deallocate the top cube structure without deallocating
  the planes, see cube_del. If you want to deallocate all image
  planes and other dynamic data associated to the structure, but keep the
  top-level structure allocated, use cube_del_contents.
 */
/*--------------------------------------------------------------------------*/
void cube_del(cube_t * d)
{
    if (d == NULL) return ;
	cube_del_contents(d);
    free(d) ;
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Free the top structure of a cube object.
  @param	d	Cube to free.
  @return	void

  This version only frees the top-level structure of a cube, without
  trying to free the image planes. This is useful when several
  pointers are referring to image planes contained within this cube.
  Beware, however, that if you do not know the references to all
  planes in the cube when calling this function, you will definitely
  loose all references to the planes.

 */
/*--------------------------------------------------------------------------*/
void cube_del_shallow(cube_t * d)
{ 
    if (d == NULL) return ;

	/* free general plane pointer */
	if (d->plane != NULL)
		free(d->plane) ;
	d->plane = NULL ;

    /* free structure itself    */
    free(d) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Free the contents of a cube structure, keeping the upper level.
  @param	d	Cube to free.
  @return	void

  This version is the logical complement to cube_del_shallow. It frees
  only the internal parts of a cube structure but keeps the upper level
  (i.e. the cube_t struct pointer) allocated. This is useful when eclipse
  is used within scripting languages with a garbage collector. Deallocating
  the contents of a cube makes sure most of the memory is freed whenever
  requested, the top-level can be garbage-collected later on.

  Beware that cube_del is NOT equivalent to two consecutive calls, one
  to cube_del_contents and one to cube_del_shallow, since both
  latter will try to deallocate some common parts.
 */
/*--------------------------------------------------------------------------*/
void cube_del_contents(cube_t * d)
{
	int	i ;

	if (d==NULL) return ;

	/* Free images planes */
	if (d->plane!=NULL) {
		for (i=0 ; i<d->np ; i++) {
			image_del(d->plane[i]);
			d->plane[i]=NULL ;
		}
		free(d->plane);
	}
	d->plane = NULL ;

	/* Stop here: leave top structure allocated */
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the reference to the image 
  @param    cube Cube to get plane from
  @param    number of plane
  @return   refernce to plane
 
  The returned cube contains a copy of the image given in input.
  Example code:
  \begin{verbatim}

    image_t *   im ;
    cube_t  *   cube ;

    im = cube_getplane(cube, 0) ;
  \end{verbatim}
 */
/*--------------------------------------------------------------------------*/
image_t * cube_getplane(cube_t* cube,int plane)
{
    if (cube==NULL) return NULL ;
    if (cube->np == 0 || cube->np < plane){
        e_error("requested plane %d not in cube", plane) ;
        return NULL ;
    }
    return cube->plane[plane];
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the number of planes in a cube
  @param    cube Cube to get plane from
  @return   number of planes (int)
 
  The returned cube contains a copy of the image given in input.
  Example code:
  \begin{verbatim}
    cube_t     *   cube ;
    int         np;
    np = cube_getnp(cube) ;
  \end{verbatim}
 */
/*--------------------------------------------------------------------------*/
int cube_getnp(cube_t *	cube)
{
	return cube ? cube->np : -1 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Reject some planes in a cube according to an array of flags.
  @param	rej		Pointer to cube to examine for plane rejection.
  @param	valid	Array of boolean flags (int).
  @return	int 0 if Ok, -1 otherwise.

  This function expects in input a valid cube and an array of integers
  taken as boolean flags (0=false, anything else is true). The array is
  expected to contain at least as many values as there are planes in the
  cube. Valid planes are kept, invalid planes are deleted (image_del)
  in the cube. The cube structure is modified to reflect these changes.

  If all flags are valid, the cube is untouched. If no plane is valid, the
  cube is simply destroyed and the pointer set to NULL.
 */
/*--------------------------------------------------------------------------*/
int cube_reject_planes(cube_t ** rej, int * valid)
{
	cube_t	*	squeezed ;
	int			nval ;
	int			np ;
	int			i, j ;

	if (rej==NULL || valid==NULL) return -1 ;
	/* Count number of valid planes */
	nval=0 ;
	np  =(*rej)->np ;
	for (i=0 ; i<np ; i++) {
		if (valid[i])
			nval++ ;
	}
	if (nval==np)
		/* All planes are valid: do nothing */
		return 0 ;

	if (nval==0) {
		/* No valid plane: destroy the cube */
		cube_del(*rej);
		*rej = NULL ;
		return 0 ;
	}

	squeezed = cube_new((*rej)->lx, (*rej)->ly, nval);
	j=0 ;
	for (i=0 ; i<np ; i++) {
		if (valid[i]) {
			squeezed->plane[j] = (*rej)->plane[i];
			j++ ;
		} else {
			image_del((*rej)->plane[i]);
		}
	}
	cube_del_shallow(*rej);
	*rej = squeezed ;
	return 0 ;
}
