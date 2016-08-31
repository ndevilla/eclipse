
/*-------------------------------------------------------------------------*/
/**
  @file     t_stamp.h
  @author   N. Devillard
  @date     Jan 2001
  @version  $Revision: 1.5 $
  @brief    Time stamp routines.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: t_stamp.h,v 1.5 2001/10/19 08:51:59 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 08:51:59 $
	$Revision: 1.5 $
*/

#ifndef _T_STAMP_H_
#define _T_STAMP_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

/* none */

/*---------------------------------------------------------------------------
  							Function ANSI prototypes
 ---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
    @brief  Creates a time stamp and return a pointer to it.

    This function creates a time stamp in an internal statically allocated
    string buffer, and returns a pointer to it. The time stamp contains the
    user name, host name, and generation date (if known) as in:

    @verbatim
    "ndevilla@pollux Wed Jan 24 10:28:09 CET 2001"
    @endverbatim

    The returned pointer points to a string statically allocated in this
    function, so do not free it or overwrite its contents! The returned
    string has max size @c ASCIILINESZ.
 */
/*---------------------------------------------------------------------------*/
char * create_timestamp(void);

/*----------------------------------------------------------------------------*/
/**
    @brief  Creates a time stamp for a log file and return a pointer to it.

    This function creates a time stamp in an internal statically allocated
    string buffer, and returns a pointer to it. The time stamp contains the
    user name, host name, and process ID as in:

    @verbatim
    "ndevilla@pollux:19100"
    @endverbatim

    The returned pointer points to a string statically allocated in this
    function, so do not free it or overwrite its contents! The returned
    string has max size @c ASCIILINESZ.
 */
/*---------------------------------------------------------------------------*/
char * create_logtimestamp(void);

#endif
