

/*-------------------------------------------------------------------------*/
/**
   @file	e_config.c
   @author	N. Devillard
   @date	Apr 1999
   @version	$Revision: 1.21 $
   @brief	eclipse general configuration settings.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: e_config.c,v 1.21 2005/03/02 10:40:29 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/02 10:40:29 $
	$Revision: 1.21 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/


#include "e_config.h" 
#include "xmemory.h"
#include "comm.h"

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

void eclipse_init(void)
{
	char	*	env_var ;
	int			val ;
	int			log ;


	env_var = getenv("E_VERBOSE");
	if (env_var != NULL) {
		val = atoi(env_var);
		set_verbose(val);
	}

	env_var = getenv("E_DEBUG");
	if (env_var != NULL) {
		val = atoi(env_var);
		set_debug(val);
	}

	env_var = getenv("E_LOGFILE");
	if (env_var != NULL) {
		set_logfile(1);
		set_logfilename(env_var);
	}

	if (debug_active()>1) {
		log = logfile_active();
		fprintf(stderr,
				"\n"
				"----- eclipse run-time configuration\n"
				"\n"
				"      verbose  : [%d]\n"
				"      debug    : [%d]\n",
				verbose_active(),
				debug_active());
		if (log)
			fprintf(stderr,
				"      logfile  : [%s]\n",
				get_logfilename());

		fprintf(stderr,
				"\n"
				"------------------------------------\n"
				"\n");
	}
	return ;
}
/* vim: set ts=4 et sw=4 tw=75 */
