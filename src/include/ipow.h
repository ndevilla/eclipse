/*-------------------------------------------------------------------------*/
/**
   @file    ipow.h
   @author  N. Devillard
   @date    June 1999
   @version $Revision: 1.5 $
   @brief   integer powers

   This function is so generic and used everywhere, it diserves its
   own source file.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: ipow.h,v 1.5 2001/11/07 13:46:16 yjung Exp $
    $Author: yjung $
    $Date: 2001/11/07 13:46:16 $
    $Revision: 1.5 $
*/

#ifndef _IPOW_H_
#define _IPOW_H_

/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Same as pow(x,y) but for integer values of y only (faster).
  @param    x   A double number.
  @param    p   An integer power.
  @return   x to the power p.

  This is much faster than the math function due to the integer. Some
  compilers make this optimization already, some do not.

  p can be positive, negative or null.
 */
/*--------------------------------------------------------------------------*/
double ipow(double x, int p) ;

#endif
