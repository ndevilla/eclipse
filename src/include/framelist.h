/*-------------------------------------------------------------------------*/
/**
   @file    framelist.h
   @author  N. Devillard
   @date    July 2000
   @version $Revision: 1.15 $
   @brief   Framelist parsing routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: framelist.h,v 1.15 2002/08/20 08:08:40 yjung Exp $
    $Author: yjung $
    $Date $
    $Revision: 1.15 $
*/

#ifndef _FRAMELIST_H_
#define _FRAMELIST_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmemory.h"
#include "charmatrix.h"

/*---------------------------------------------------------------------------
   								New Types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Framelist object

  This object stores information found from an ASCII list file. This kind
  of file is expected to contain a valid file name in first column and
  optionally a file type in second column.
 */
/*-------------------------------------------------------------------------*/
typedef struct _framelist_ {
	int			n ;
	char	*	filename ;
	char	**	name ;
	char	**	type ;
	int		*	label ;	/* User-defined label */
} framelist ;

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Load a list of valid file names from an ASCII list.
  @param    filename        Name of the ASCII list to parse.
  @return   1 newly allocated ascii_list object.

  This function expects in input the name of a valid ASCII list, i.e. an
  ASCII file with the following format:

  \begin{itemize}
  \item First column contains a valid filename.
  \item Second column might contain a file type.
  \end{itemize}

  If a given file name does not correspond to a valid existing file,
  the list is not loaded and NULL is returned.

  The returned object must be deallocated using framelist_del().
 */
/*--------------------------------------------------------------------------*/
framelist * framelist_load(char * filename) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Get the first valid file name in an ASCII list.
  @param    filename    Name of the ASCII list to parse.
  @return   1 pointer to statically allocated string, or NULL.

  This function looks up an ASCII list file to localize the first valid
  FITS file name, and returns a pointer to a statically allocated string
  containing the file name. If an error occurs, it returns NULL.

  This function is actually implemented as a wrapper around framelist_load
  to avoid recoding a second ASCII list parser. So it costs just as much
  to call this function or framelist_load.
 */
/*--------------------------------------------------------------------------*/
char * framelist_firstname(char * filename) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a framelist to an opened file pointer.
  @param    dumped  Framelist to dump.
  @param    out     Opened file pointer.
  @return   void

  This function dumps the information contained in a framelist object to an
  opened file pointer. It is Ok to provide stdout or stderr as file
  pointers.
 */
/*--------------------------------------------------------------------------*/
void framelist_dump(framelist * dumped, FILE * out) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Allocate space to hold a new frame list.
  @param    n   Number of frames to hold in the list.
  @return   Pointer to newly allocated framelist object.

  This constructor will allocate the space for the new framelist object,
  set the number of frames to the required amount, and allocate space to
  hold names and types. Name and type entries are set to NULL.
 */
/*--------------------------------------------------------------------------*/
framelist * framelist_new(int n) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Deallocated a framelist object.
  @param    f   Framelist object to deallocate.
  @return   void

  This function frees all memory associated to a framelist object.
 */
/*--------------------------------------------------------------------------*/
void framelist_del(framelist * f) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Copy contents of a framelist to a new framelist object.
  @param    f   Framelist object to copy.
  @return   1 newly allocated framelist object.

  All contents of a framelist are copied into a newly allocated framelist,
  which is returned to the caller. The returned object must be freed using
  framelist_del().
 */
/*--------------------------------------------------------------------------*/
framelist * framelist_copy(framelist * f) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Select contents of a framelist to a new framelist object.
  @param    f       Framelist object to examine.
  @param    label   Label to use for selection
  @return   1 newly allocated framelist object.
 
  This function selects frames in a framelist which have their label
  set to the same value as 'label'. The returned object must be deallocated
  using framelist_del().
 */
/*--------------------------------------------------------------------------*/
framelist * framelist_select(
        framelist   *   f,
        int             label) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Select only some frames in a list.
  @param    f           Framelist to check.
  @param    token       Token to identify for each frame.
  @param    token_get   Function to use to get token for each file.
  @return   Pointer to a newly allocated framelist.

  This function applies a 'token_get' function to each file in the input
  list, getting back a character token for each file. It compares the
  returned token with the value provided in 'token' and rejects from the
  list all non-matching frames. If no matching frame can be found, this
  function returns NULL.

  The returned framelist must be deallocated using framelist_del().
 */
/*--------------------------------------------------------------------------*/
framelist * framelist_select_tokenget(
        framelist   *   f,
        char        *   token,
        char        *   (*token_get)(char*)) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Purge some frames in a list.
  @param    f           Framelist to check.
  @param    token       Token to identify for each frame.
  @param    token_get   Function to use to get token for each file.
  @return   Pointer to a newly allocated framelist.

  This function applies a 'token_get' function to each file in the input
  list, getting back a character token for each file. It compares the
  returned token with the value provided in 'token' and rejects from the
  list all matching frames. If no matching frame can be found, this
  function returns NULL.

  The returned framelist must be deallocated using framelist_del().
 */
/*----------------------------------------------------------------------------*/
framelist * framelist_purge_tokenget(
        framelist   *   f,
        char        *   token,
        char        *   (*token_get)(char*)) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Separate a list of frames into groups, according to labels.
  @param    lnames      Input framelist.
  @param    compare     Pointer to comparison function to use.
  @return   int -1 in error case, nb of settings otherwise 

  This function takes in input a framelist, and a comparison function
  to sort the frames. It will sort the frames according to the labels
  found by the comparison function.

  The comparison function receives two frame names, and is responsible
  for fetching whatever keyword in each frame header and compare it.
  If keywords match, the comparison function must return 1, otherwise
  0. This function will count the number of possible different values
  found by the comparison function and update nsettings accordingly.

  See an example in ins/isaac/recipes/dark_average.c
 */
/*--------------------------------------------------------------------------*/
int framelist_labelize(
        framelist   *   lnames,
        int             (*compare)(char*,char*)) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Returns 1 if the file is an ASCII list, 0 else.
  @param    filename name of the file to check
  @return   int 0, 1, or -1

  Returns 1 if the file name corresponds to a valid ASCII list.
  Returns 0 else. If the file does not exist, returns -1.
 */
/*--------------------------------------------------------------------------*/


int is_ascii_list(char * filename);


#endif
