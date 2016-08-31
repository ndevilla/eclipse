
/*-------------------------------------------------------------------------*/
/**
  @file     t_iso8601.h
  @author   N. Devillard
  @date     Aug 1999
  @version  $Revision: 1.10 $
  @brief    Get date/time, possibly in ISO8601 format.

  This module contains various utilities to get the current date/time, 
  and possibly format it according to the ISO 8601 format.
*/
/*--------------------------------------------------------------------------*/

/*

	$Id: t_iso8601.h,v 1.10 2001/10/19 08:47:18 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 08:47:18 $
	$Revision: 1.10 $

*/

#ifndef _T_ISO8601_H_
#define _T_ISO8601_H_

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------
						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Returns the current date as a long (CCYYMMDD).
  @return   The current date as a long number.

  Returns the current date as a long value (CCYYMMDD). Since most
  system clocks do not return a century, this function assumes that
  all years 80 and above are in the 20th century, and all years 00 to
  79 are in the 21st century.  For best results, consume before 1 Jan
  2080.

  Example:  19 Oct 2000 is returned as 20001019
 */
/*--------------------------------------------------------------------------*/
long date_now (void);

/*-------------------------------------------------------------------------*/
/**
  @brief    Returns the current time as a long (HHMMSSCC).
  @return   The current time as a long number.

  Returns the current time as a long value (HHMMSSCC). If the system
  clock does not return centiseconds, these are set to zero.

  Example: 15:36:12.84 is returned as 15361284
 */
/*--------------------------------------------------------------------------*/
long time_now(void);

/*--------------------------------------------------------------------------*/
/**
  @brief    Returns the current date as a static string.
  @return   Pointer to statically allocated string.
 
  Build and return a string containing the date of today in ISO8601
  format. The returned pointer points to a statically allocated string
  in the function, so no need to free it.
  */
/*--------------------------------------------------------------------------*/
char * get_date_iso8601(void);

/*-------------------------------------------------------------------------*/
/**
  @brief    Returns the current date and time as a static string.
  @return   Pointer to statically allocated string
 
  Build and return a string containing the date of today and the
  current time in ISO8601 format. The returned pointer points to a
  statically allocated string in the function, so no need to free it.
 */
/*-------------------------------------------------------------------------*/
char * get_datetime_iso8601(void);

#ifdef __cplusplus
}
#endif

#endif
