

/*---------------------------------------------------------------------------
   
   File name 	:	e_config.h
   Author 		:	N. Devillard
   Created on	:	Apr 1999
   Description	:	eclipse machine configuration handling

 *--------------------------------------------------------------------------*/

/*
	$Id: e_config.h,v 1.11 2001/10/17 10:20:30 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 10:20:30 $
	$Revision: 1.11 $
*/

#ifndef _E_CONFIG_H_
#define _E_CONFIG_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>


/*---------------------------------------------------------------------------
							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Sets eclipse config.
  @return   int 0 if Ok, anything else otherwise
 
  The following variables are read from the environment:
 
  - @c E_VERBOSE for the verbose level (see comm.h).
  - @c E_DEBUG for the debug level (see comm.h).
  - @c E_TMPDIR for the tmpdirname parameter (see xmemory.h)
  - @c E_LOGFILE for the logfile parameter (see comm.h)
 
  Notice that @c E_LOGFILE is tested in other places (see comm.h) for
  logfile output.
 */
/*--------------------------------------------------------------------------*/
void eclipse_init(void);

#endif
