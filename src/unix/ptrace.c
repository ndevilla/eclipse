
/*-------------------------------------------------------------------------*/
/**
  @file		ptrace.c
  @author	N. Devillard
  @date		May 2001
  @version	$Revision: 2.7 $
  @brief	Add tracing capability to any program compiled with gcc.

  This module is only compiled when using gcc and tracing has been
  activated. It allows the compiled program to output messages whenever
  a function is entered or exited.

  To activate this feature, your version of gcc must support
  the -finstrument-functions flag.

  The printed messages yield function addresses, not human-readable
  names. To link both, you need to get a list of symbols from the
  program. There are many (unportable) ways of doing that, see the
  'etrace' project on freshmeat for more information about how to dig
  the information.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: ptrace.c,v 2.7 2002/02/18 08:33:26 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/02/18 08:33:26 $
	$Revision: 2.7 $
*/

#if (__GNUC__>2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 95))

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define PTRACE_PIPENAME	"TRACE"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/** Final trace close */
static void
__attribute__((__no_instrument_function__))
gnu_ptrace_close(void)
{
	FILE	*	trace ;
	if ((trace=fopen(PTRACE_PIPENAME, "a"))==NULL) return ;
	fprintf(trace, "EXIT %ld\n", (long)getpid());
	fclose(trace);
	return ;
}

/** Trace initialization */
static int
__attribute__((__no_instrument_function__))
gnu_ptrace_init(void)
{
	struct stat sta ;

	/* See if a trace file exists */
	if (stat(PTRACE_PIPENAME, &sta)!=0) {
		/* No trace file: do not trace at all */
		return 0 ;
	}
	/* Tracing requested: a trace file was found */
	atexit(gnu_ptrace_close);
	return 1 ;
}

/** Function called by every function event */
void
__attribute__((__no_instrument_function__))
gnu_ptrace(char * what, void * p)
{
	static int first=1 ;
	static int active=1 ;
	FILE * trace ;

	if (first) {
		active = gnu_ptrace_init();
		first=0 ;
	}
	if (active==0)
		return ;

	if ((trace=fopen(PTRACE_PIPENAME, "a"))==NULL) {
		return ;
	}
	fprintf(trace, "%s %p\n", what, p);
	fclose(trace);
	return ;
}

/** According to gcc documentation: called upon function entry */
void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
	gnu_ptrace("enter", this_fn);
}

/** According to gcc documentation: called upon function exit */
void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
	gnu_ptrace("exit", this_fn);
}

#endif
/* vim: set ts=4 et sw=4 tw=75 */
