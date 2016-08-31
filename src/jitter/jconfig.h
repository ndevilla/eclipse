/*----------------------------------------------------------------------------*/
/**
   @file    jconfig.h
   @author
   @date    March 2002
   @version	$Revision: 1.8 $
   @brief   Jitter calibration handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jconfig.h,v 1.8 2002/04/19 09:11:59 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 09:11:59 $
	$Revision: 1.8 $
*/

#ifndef _JCONFIG_H_
#define _JCONFIG_H_

#include "jtypes.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter config constructor
  @return   1 newly allocated jitter config.

  Simple constructor, uses calloc() so all fields are set to zero.
 */
/*----------------------------------------------------------------------------*/
jitter_config_t * jitter_config_new(void) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter config destructor.
  @param    jc  Current jitter config.
  @return   void

  Safe destructor, will delete all possibly allocated fields if non-NULL
  and free the top structure.
 */
/*----------------------------------------------------------------------------*/
void jitter_config_del(jitter_config_t * jc) ;

/*-----------------------------------------------------------------------------
  Convert enums to strings
 -----------------------------------------------------------------------------*/
char * jconv_ftype(jframe_type t) ;
char * jconv_skymethod(jsky_method m) ;
char * jconv_algo(jalgo_status t) ;
char * jconv_ins(instrument_t i) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Jitter config dump
  @param    jc      Current jitter config.
  @param    out     Opened file pointer for output.
  @return   void

  This function dumps the status of the current jitter config
  to the provided file pointer. It is Ok to pass stdout or stderr
  as file pointers.
 */
/*----------------------------------------------------------------------------*/
void jitter_config_dump(jitter_config_t * jc, FILE * out) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Select planes in the config and build a cube from them.
  @param    jc      Current jitter config.
  @param    t       Frame type to select.
  @return   1 newly allocated integer array.

  This function examines the passed jitter config and extracts
  all planes which type corresponds to the passed type. It builds
  an integer array of size jc->nframes in which selected frames
  are assigned 1 and non-selected are assigned 0.

  This function returns NULL if no plane can be selected.
 */
/*----------------------------------------------------------------------------*/
int * jitter_cubeselect(jitter_config_t * jc, jframe_type t) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Get planes from a config and build a cube.
  @param    jc      Current jitter config.
  @param    sel     Selection array, see jitter_cubeselect()
  @return   1 newly allocated cube.

  This function selects planes in a jitter config and builds a new
  cube containing only the selected planes, or NULL if no plane is
  selected.

  The returned cube should be deallocated using cube_del_shallow().

  If the passed selection list is NULL, all planes are selected.
 */
/*----------------------------------------------------------------------------*/
cube_t * jitter_cubeget(jitter_config_t * jc, int * sel) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Put planes back into jitter config.
  @param    jc      Current jitter config.
  @param    sel     Selection array.
  @param    c       Cube to copy to jitter config.
  @return   void

  This function copies plane pointers back into a jitter config,
  according to a selection array. If sel is NULL, all plane
  pointers are copied to the jitter config.
 */
/*----------------------------------------------------------------------------*/
void jitter_cubeput(jitter_config_t * jc, int * sel, cube_t * c) ;

#endif
