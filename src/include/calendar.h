
/*-------------------------------------------------------------------------*/
/**
   @file    calendar.h
   @author  N. Devillard
   @date    Dec 2000
   @version $Revision: 1.4 $
   @brief   Calendar routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: calendar.h,v 1.4 2001/10/17 10:29:01 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 10:29:01 $
	$Revision: 1.4 $
*/


#ifndef _CALENDAR_H_
#define _CALENDAR_H_


/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the date of yesterday for a given date.
  @param    day     Day
  @param    month   Month
  @param    year    Year
  @return   void

  Computes the date for "yesterday", for the given date. The given
  parameters are modified. Year is expected with 4 digits.
  Example (dates are given as DD/MM/YYYY):

  - yesterday for 01.01.2000 is 31.12.1999
  - yesterday for 01.03.2000 is 29.02.1999

  This function handles leap years (yes, 2000 was a leap year).
 */
/*--------------------------------------------------------------------------*/
void calendar_getprev(int * day, int * month, int * year);

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the date of tomorrow for a given date.
  @param    day     Day
  @param    month   Month
  @param    year    Year
  @return   void

  Computes the date for "tomorrow", for the given date. The given
  parameters are modified. Year is expected with 4 digits.
  Example (dates are given as DD/MM/YYYY):

  - tomorrow for 31.12.1999 is 01.01.2000
  - tomorrow for 28.02.2000 is 29.02.2000

  This function handles leap years (yes, 2000 was a leap year).
 */
/*--------------------------------------------------------------------------*/
void calendar_getnext(int * day, int * month, int * year);


#endif
