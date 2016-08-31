/*----------------------------------------------------------------------------*/
/**
   @file	lwload.c
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.7 $
   @brief	ISAAC LW cube loading
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: lwload.c,v 1.7 2002/07/15 14:32:14 yjung Exp $
	$Author: yjung $
	$Date: 2002/07/15 14:32:14 $
	$Revision: 1.7 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Load a list of files into a cube.
  @param    flist   Name of framelist to load
  @return   1 pointer to newly allocated cube.

  This function hides the fact that ISAAC LW data may come as list of 
  single-frame or double-frame (NAXIS3=2) files. If the input list designates 
  single-frame files, they are all loaded into a cube. If the list designates 
  double-frame files, each pair of frame is loaded, frame 2 subtracted from 
  frame 1 and the result stored into the returned cube.
  The returned cube must be deallocated using one of the cube deallocators.
 */
/*----------------------------------------------------------------------------*/
cube_t * isaac_loadcube(framelist * flist)
{
    cube_t      *   loaded ;
    cube_t      *   new_cube ;
    int         *   types ;
    char        *   value ;
    instrument_t    ins ;

    int             i, j ;
    
    /* Check input parameter */
    if (flist==NULL) return NULL ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

    /* Load input images in a cube */
    if ((loaded = cube_load_strings(flist->name, flist->n)) == NULL) {
        e_error("cannot load the cube") ;
        return NULL ;
    }
   
    /* Allocate types array */
    types = calloc(flist->n, sizeof(int)) ;
    
    /* Update types array : 0 unknown, 1 for INT, 2 for CUBE1 */
    for (i=0 ; i<flist->n ; i++) {
        if ((value = pfits_get(ins, flist->name[i], 
                        "detector_frame_type")) == NULL) { 
            e_error("cannot read DET FRAME TYPE") ;
            free(types) ;
            cube_del(loaded) ;
            return NULL ;
        } else {
            if (!strcmp(qfits_pretty_string(value), "INT"))        types[i] = 1;
            else if (!strcmp(qfits_pretty_string(value), "CUBE1")) types[i] = 2;
            else {
                e_error("Expected frame types are INT or CUBE1") ;
                free(types) ;
                cube_del(loaded) ;
                return NULL ;
            }
        }
    }

    /* Create the new cube with differences for CUBE1 type */
    new_cube = cube_new(loaded->lx, loaded->ly, flist->n) ;
    j = 0 ;
    for (i=0 ; i<flist->n ; i++) {
        if (types[i] == 1) {
            new_cube->plane[i] = loaded->plane[j] ;
            loaded->plane[j] = NULL ;
            j++ ; 
        } else if (types[i] == 2) {
            image_sub_local(loaded->plane[j], loaded->plane[j+1]) ;
            image_del(loaded->plane[j+1]) ;
            loaded->plane[j+1] = NULL ;
            new_cube->plane[i] = loaded->plane[j] ;
            loaded->plane[j] = NULL ;
            j += 2 ;
        }
    }
    
    /* Free and return */
    free(types) ;
    cube_del_shallow(loaded) ;
    return new_cube ;
}

