/*----------------------------------------------------------------------------*/
/**
   @file    spjconfig.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.3 $
   @brief   Spectroscopic jitter calibration handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjconfig.h,v 1.3 2003/01/09 12:40:41 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/09 12:40:41 $
	$Revision: 1.3 $
*/

#ifndef _SPJCONFIG_H_
#define _SPJCONFIG_H_

/*-----------------------------------------------------------------------------
  						        Include
 -----------------------------------------------------------------------------*/

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Spectroscopic jitter config constructor
  @return   1 newly allocated spjitter config.

  Simple constructor, uses calloc() so all fields are set to zero.
 */
/*----------------------------------------------------------------------------*/
spjitter_config_t * spjitter_config_new(void) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Spectroscopic jitter config destructor.
  @param    spjc  Current spjitter config.
  @return   void

  Safe destructor, will delete all possibly allocated fields if non-NULL
  and free the top structure.
 */
/*----------------------------------------------------------------------------*/
void spjitter_config_del(spjitter_config_t * spjc) ;

/*-----------------------------------------------------------------------------
  Convert enums to strings
 -----------------------------------------------------------------------------*/
char * spjconv_ftype(spjframe_type t) ;
char * spjconv_offsource(spjoff_source o) ;
char * spjconv_diffmeth(spjdiff_meth m) ;
char * spjconv_combmeth(spjcomb_meth m) ;
char * spjconv_algo(spjalgo_status t) ;
char * spjconv_ins(instrument_t i) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Spectroscopic jitter config dump
  @param    spjc    Current spjitter config.
  @param    out     Opened file pointer for output.
  @return   void
  This function dumps the status of the current spjitter config to the provided
  file pointer. It is Ok to pass stdout or stderr as file pointers.
 */
/*----------------------------------------------------------------------------*/
void spjitter_config_dump(spjitter_config_t * spjc, FILE * out) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Select planes in the config and build a cube from them.
  @param    spjc    Current spjitter config.
  @param    t       Frame type to select.
  @return   1 newly allocated integer array.

  This function examines the passed spjitter config and extracts
  all planes which type corresponds to the passed type. It builds
  an integer array of size spjc->nframes in which selected frames
  are assigned 1 and non-selected are assigned 0.

  This function returns NULL if no plane can be selected.
 */
/*----------------------------------------------------------------------------*/
int * spjitter_cubeselect(spjitter_config_t * spjc, spjframe_type t) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Get planes from a config and build a cube.
  @param    spjc    Current spjitter config.
  @param    sel     Selection array, see spjitter_cubeselect()
  @return   1 newly allocated cube.

  This function selects planes in a spjitter config and builds a new
  cube containing only the selected planes, or NULL if no plane is selected.

  The returned cube should be deallocated using cube_del_shallow().

  If the passed selection list is NULL, all planes are selected.
 */
/*----------------------------------------------------------------------------*/
cube_t * spjitter_cubeget(spjitter_config_t * spjc, int * sel) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Put planes back into spjitter config.
  @param    spjc    Current spjitter config.
  @param    sel     Selection array.
  @param    c       Cube to copy to spjitter config.
  @return   void

  This function copies plane pointers back into a spitter config, according to 
  a sel array. If sel is NULL, all plane pointers are copied to the spjitter.
 */
/*----------------------------------------------------------------------------*/
void spjitter_cubeput(spjitter_config_t * spjc, int * sel, cube_t * c) ;

#endif
