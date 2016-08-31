
/*----------------------------------------------------------------------------
   
   File name    :   utils.h
   Author       :	N. Devillard
   Created on   :	July 2000	
   Description  :	CONICA various utilities 

 ---------------------------------------------------------------------------*/

/*

 $Id: utils.h,v 1.3 2002/07/31 14:05:27 ndevilla Exp $
 $Author: ndevilla $
 $Date: 2002/07/31 14:05:27 $
 $Revision: 1.3 $

 */

#ifndef _CONICA_UTILS_H_
#define _CONICA_UTILS_H_

/*----------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include "eclipse.h"

/*----------------------------------------------------------------------------
                                Defines
 ---------------------------------------------------------------------------*/

#define AIRMASS_START       0
#define AIRMASS_END         1

/*----------------------------------------------------------------------------
						Function ANSI C prototypes 
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Apply dark subtraction and ff division, replace bad pixels
  @param    in  pointer to allocated cube
  @param    ff_name flat field name
  @param    dark_name   dark name
  @return   badpix_name bad pixel name

  The input cube is always modified, to ensure that the returned value is
  read/write.
 */
/*--------------------------------------------------------------------------*/

void conica_ff_dark_badpix_handling(
    cube_t  **  in,
    char    *   ff_name,
    char    *   dark_name,
    char    *   badpix_name
) ;

#endif
