
/*-------------------------------------------------------------------------*/
/**
  @file     pid_i.h
  @author   N. Devillard
  @date     Sep 2000
  @version  $Revision: 1.6 $
  @brief    Process information routines.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: pid_i.h,v 1.6 2001/10/19 10:49:09 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 10:49:09 $
	$Revision: 1.6 $
*/

#ifndef _PID_I_H_
#define _PID_I_H_

/*---------------------------------------------------------------------------
						Function ANSI prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out if a given pid corresponds to a living process.
  @param    pid     Process ID as a long int.
  @return   int 0 or 1.
 
  Returns 0 if the given pid does not correspond to any living process, 1
  if it does.
 */
/*--------------------------------------------------------------------------*/
int pid_exists(long pid);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find the name of the process for a given PID.
  @param    pid PID of the process to search for.
  @return   pointer to statically allocated string.

  The returned pointer is statically allocated in this function. Do not try
  to free it or modify it. If no process can be found with this name, this
  function returns NULL.

  This function taken from the comp.unix.programmer FAQ.
 */
/*--------------------------------------------------------------------------*/
char * pid_getname(long pid);

#endif
