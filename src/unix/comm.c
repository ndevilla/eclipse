

/*-------------------------------------------------------------------------*/
/**
   @file	comm.c
   @author	N. Devillard
   @date	Sep 1999
   @version	$Revision: 1.29 $
   @brief	Output message handling.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: comm.c,v 1.29 2004/08/26 15:50:29 yjung Exp $
	$Author: yjung $
	$Date: 2004/08/26 15:50:29 $
	$Revision: 1.29 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "comm.h"
#include "version.h"
#include "userid.h"
#include "ansiterm.h"
#include "t_stamp.h"

#include <time.h>
#include <string.h>

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Max size of the log file name */
#define LOGNAMESZ			512
/** Max size of the host name */
#define HOSTNAMESZ			512
/** Max size of the time string */
#define TIMESTRSZ			512


/** Tracing feature for gcc > 2.95 */
#if (__GNUC__>2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 95))
#define __NOTRACE__ __attribute__((__no_instrument_function__))
#else
#define __NOTRACE__
#endif


/*---------------------------------------------------------------------------
							Private to this module
 ---------------------------------------------------------------------------*/

static struct {

    /* Verbose status */
    int verbose ;
    /* Debug status */
    int debug ;
    /* ANSI term support */
    int ansiterm ;

    /* Logfile status */
    int logfile ;
    /* Logfile name */
    char logfilename[LOGNAMESZ];
    /* Default logfile stamp */
    char logfilestamp[512];

} comm_config = { 0, 0, -1, 0, "", "" };


/*---------------------------------------------------------------------------
   							Private functions
 ---------------------------------------------------------------------------*/

/* Chop a string: private function */
static void chop_r(char * s)
{
	if (s) {
		if (strlen(s)>1) {
			while (s[strlen(s)-1] == (char)'\n') s[strlen(s)-1]=(char)0 ;
		}
	}
}

/* Initialize ANSI term support status */
static void ansiterm_init(void)
{
	char	*	term ;

	term = getenv("TERM");
	if (term==NULL) {
		comm_config.ansiterm = 0 ;
	} else if (!strncmp(term, "vt100", 5) ||
			   !strncmp(term, "xterm", 5)) {
		comm_config.ansiterm = 1 ;
	}
	return ;
}


/*---------------------------------------------------------------------------
  						Public toggle functions	
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Set verbose level
  @param    flag New verbose level
  @return   void

  Sets the verbose level to a given value. Any integer is accepted, 0
  is considered as deactivated.
 */
/*--------------------------------------------------------------------------*/

void set_verbose(int flag)
{ comm_config.verbose = flag ; }


/*-------------------------------------------------------------------------*/
/**
  @brief    Get verbose level
  @return   Current verbose level

  Gets the current verbose level.
 */
/*--------------------------------------------------------------------------*/

int verbose_active(void)
{ return comm_config.verbose ; }

/*-------------------------------------------------------------------------*/
/**
  @brief    Set debug level
  @param    flag New debug level
  @return   void

  Sets the debug level to a given value. Any integer is accepted, 0
  is considered as deactivated.
 */
/*--------------------------------------------------------------------------*/

void set_debug(int flag)
{ comm_config.debug = flag ; }

/*-------------------------------------------------------------------------*/
/**
  @brief    Get debug level
  @return   Current debug level

  Gets the current debug level.
 */
/*--------------------------------------------------------------------------*/

int debug_active(void)
{ return comm_config.debug ; }

/*-------------------------------------------------------------------------*/
/**
  @brief     Set ANSI term support
  @param    flag ANSI term support flag.
  @return   void

  Sets the ANSI term support level to a given value. Any integer is
  accepted, 0 is considered as deactivated.
 */
/*--------------------------------------------------------------------------*/

void set_ansiterm(int flag)
{ comm_config.ansiterm = flag ; }


/*-------------------------------------------------------------------------*/
/**
  @brief    Get ANSI term support level
  @return   Current ANSI term support level

  Gets the current ANSI term support level.
 */
/*--------------------------------------------------------------------------*/

int ansiterm_active(void)
{ return comm_config.ansiterm ; }

/*-------------------------------------------------------------------------*/
/**
  @brief    Set log file activity
  @param    flag    Log file activation flag
  @return   void

  Calling this function with a non-zero integer will activate log file
  activity. Calling it with a null parameter (zero) will deactivate
  logging.
 */
/*--------------------------------------------------------------------------*/

void set_logfile(int flag)
{ comm_config.logfile = flag ; }

/*-------------------------------------------------------------------------*/
/**
  @brief    Get logfile activity status.
  @return   Logfile activity status as an integer.

  If the returned integer is non-zero, the logfile feature is
  currently activated, otherwise it is turned off.
 */
/*--------------------------------------------------------------------------*/

int logfile_active(void)
{ return comm_config.logfile ; }

/*-------------------------------------------------------------------------*/
/**
  @brief    Sets the current log file name
  @param    name    Name of the log file
  @return   void

  Provide a name with full path, pointing to a valid file (i.e. a file
  that can be created or appended to). By default, this function is
  called to set the value to the one found in the @c E_LOGFILE
  environment variable.
 */
/*--------------------------------------------------------------------------*/

void set_logfilename(char * name)
{
	if (name==NULL) {
        comm_config.logfilename[0]=(char)0 ;
        return ;
    }
	strcpy(comm_config.logfilename, name) ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Gets the current log file name
  @return   Pointer to statically allocated string (in this module).

  Find out what the current log file name is. The returned string is
  statically allocated, so no need to free it. Do not try to modify
  the contents of the returned string, use set_logfilename for that.
 */
/*--------------------------------------------------------------------------*/

char * get_logfilename(void)
{ return comm_config.logfilename ; }



/*-------------------------------------------------------------------------*/
/**
  @brief    Prints out program name and a short help.
  @param    pname   Program name
  @param    shelp   Short help (one-liner)
  @return   void
 
  To be called in every main() as an ID of the current command.
 */
/*--------------------------------------------------------------------------*/ 

void hello_world(char * pname, char * shelp)
{
	fprintf(stderr, "\
\n\
\n\
********** %s\n\
********** part of eclipse library. (c) ESO 1996-2002\n\
\n\
purpose: %s\n\
\n",
	pname, shelp) ;
}

	

/*-------------------------------------------------------------------------*/
/**
  @brief    Prints a warning to stderr, possibly logs it to a log file.
  @param    fmt     Format a la printf.
  @return   void
 
  Feed in a formatted string and possibly some variables, exactly as
  you would do with printf. This function will print out your message
  as a warning on stderr.
 
  If the environment variable @c E_LOGFILE is defined, the warning is
  also logged into the log file.
 */
/*--------------------------------------------------------------------------*/


void e_warning(char *fmt, ...) 
{
	char   *msg ;
	va_list	ap ;
	char    logmsg[1024] ;

	if (comm_config.ansiterm<0) ansiterm_init();
	
	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_bold);
	}
	fprintf(stderr, "*** ") ;
	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_reset);
	}

	msg = strdup(fmt) ;
	chop_r(msg) ;

	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_underl);
	}
	
    va_start(ap, fmt);
    vfprintf(stderr, msg, ap) ;
    va_end(ap);
    va_start(ap, fmt);
    vsprintf(logmsg, msg, ap) ;
    va_end(ap) ;
	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_reset);
		fprintf(stderr, ansiterm_bold);
	}
	
	fprintf(stderr, " ***\n") ;
	fflush(stderr) ;
	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_reset);
	}
	free(msg) ;

	if (comm_config.logfile) {
		e_logfile("warning", logmsg);
	}
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Prints an error to stderr, possibly logs it to a log file.
  @param    fmt     Format a la printf.
  @return   void
 
  Feed in a formatted string and possibly some variables, exactly as
  you would do with printf. This function will print out your message
  as an error message on stderr.
 
  If the environment variable @c E_LOGFILE is defined, the message is
  also logged as an error into the log file.
 */
/*--------------------------------------------------------------------------*/


void e_error(char *fmt, ...)
{
	char  * msg ;
	va_list	ap ;
	char	logmsg[1024] ;

	if (comm_config.ansiterm<0) ansiterm_init();
	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_bold);
	}
	fprintf(stderr, "error: ") ;
	msg = strdup(fmt) ;
	chop_r(msg) ;

    va_start(ap, fmt);
    vfprintf(stderr, msg, ap) ;
    va_end(ap);
    va_start(ap, fmt);
    vsprintf(logmsg, msg, ap) ;
    va_end(ap) ;
	
	free(msg) ;
	fprintf(stderr, "\n") ;
	fflush(stderr) ;

	if (comm_config.ansiterm) {
		fprintf(stderr, ansiterm_reset);
	}
	if (comm_config.logfile) {
		e_logfile("error", logmsg);
	}
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Prints out a comment on stderr.
  @param    level   Comment indentation level.
  @param    fmt     Format a la printf.
  @return   void
 
  Call this function whenever you want to send a comment to stderr for
  user information. The level indicates how many tabulations are
  inserted in front of your comment. Feed in 0 if you do not want to
  indent your comment. The format string (and further arguments) are
  exactly the same as for printf.
 
  If the environment variable @c E_LOGFILE is defined, the message is
  also logged as an info field into the log file.
 */
/*--------------------------------------------------------------------------*/

void e_comment(int level, char * fmt, ...)
{
	int 	i ;
	char  * msg ;
	va_list	ap ;
	char	logmsg[1024] ;

	if (comm_config.verbose) {
		if (level != 0)
			for (i=0 ; i<level ; i++)
				fprintf(stderr, "\t") ;

		msg = strdup(fmt) ;
		chop_r(msg) ;
        va_start(ap, fmt);
        vfprintf(stderr, msg, ap) ;
        va_end(ap);
        va_start(ap, fmt);
        vsprintf(logmsg, msg, ap) ;
        va_end(ap) ;

		free(msg) ;
		fprintf(stderr,"\n") ;
		fflush(stderr) ;
		if (comm_config.logfile) {
			e_logfile("info", logmsg);
		}
	}
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Prints out an advance status on stderr.
  @param    Msg     Message to print out.
  @param    done    How much of the work is done yet.
  @param    total   How much has to be done in total.
  @param    level   Indentation level.
  @return   void
 
  This function is useful to print out an advance status for long
  computational functions. It is definitely not recommended to put a
  call to this function in the deepest processing loop you have! It
  will make a call to printf whenever it is called, slowing down your
  process.
 
  If @e done is strictly lower than @e total, a message is
  printed as follows:
 
  - The current line on terminal is erased.
  - N tabulations are printed as requested by @e level (indentation level).
  - The message @e Msg is printed.
  - The number of @e done items is printed.
  - The number of @e total items is printed.
 
  Example:
 
  @code
  int i ;
  for (i=0 ; i<100 ; i++) {
      compute_status("computing", i, 100, 0);
  }
  @endcode
 
  would produce:
  @verbatim
  computing: 1 out of 100
  @endverbatim
 
  Whenever @e done is equal or greather than @e total, a
  carriage return character is printed. At that time, if
  @c E_LOGFILE is correctly defined, a message is appended
  into the logfile to indicate that the operation has correctly
  taken place.
 */
/*--------------------------------------------------------------------------*/

void __NOTRACE__ compute_status(char *Msg, int done, int total, int level)
{
	int 	i ;
	char	logmsg[1024];

	if (comm_config.verbose) {
		done++ ;
		fprintf(stderr, "\r");
		if (level) {
			for (i=0 ; i<level ; i++) {
				fprintf(stderr, "\t");
			}
		}
		fprintf(stderr, "%s: %d out of %d ", Msg, done, total);
		fflush(stderr) ;

		if (done >= total)
			fprintf(stderr, "\n") ;
	}
	if (done==total && comm_config.verbose) {
		if (comm_config.logfile) {
			sprintf(logmsg, "%s: completed", Msg);
			e_logfile("info", logmsg);
		}
	}
	return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Appends header information to the E_LOGFILE.
  @return   void
 
  If the @c E_LOGFILE variable is correctly defined, the corresponding
  log file is open in append mode. A header is printed out giving the
  hostname, PID of the current process, and date/time information. The
  log file is then closed.
 
  Notice that the log file is not locked, i.e. it is possible for
  several eclipse commands to write into it at the same time,
  mixing up the information.
 */
/*--------------------------------------------------------------------------*/
void e_logfile_start(void)
{
	FILE	*	log ;
	char		time_str[TIMESTRSZ];
	time_t		local_t ;

	strcpy(comm_config.logfilestamp, create_logtimestamp());

	/* Get date & time */
	if (time(&local_t) == (time_t)-1) {
		sprintf(time_str, "unknown time");
	} else {
		sprintf(time_str, "%s", ctime(&local_t));
		chop_r(time_str);
	}
	if ((log = fopen(comm_config.logfilename, "a"))==NULL) {
		fprintf(stderr, "error: cannot open log file: disabling logging\n");
		/* Disable logging for the rest of the process */
		comm_config.logfile = 0 ;
	} else {
		fprintf(log, "[%s] %8s %s\n",
				comm_config.logfilestamp, "start", time_str);
		fclose(log);
	}

	return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Appends tail information to the E_LOGFILE.
  @return   void
 
  If the @c E_LOGFILE variable is correctly defined, the corresponding
  log file is opened in append mode. A footer is printed out, giving
  the date and time of closure. The log file is then closed.
 
  Notice that the log file is not locked, i.e. it is possible for
  several eclipse commands to write into it at the same time,
  garbaging the information.
 */
/*--------------------------------------------------------------------------*/

void e_logfile_stop(void)
{
	FILE	*	log;
    char    	time_str[TIMESTRSZ] ;
    time_t  	local_t ;

    if (time(&local_t) == (time_t)-1) {
        sprintf(time_str, "unknown date and time") ;
    } else {
        sprintf(time_str, "%s", ctime(&local_t)) ;
		chop_r(time_str);
    }
    if ((log = fopen(comm_config.logfilename, "a"))!=NULL) {
		fprintf(log, "[%s] %8s %s\n\n",
				comm_config.logfilestamp, "stop", time_str);
		fclose(log);
    }
	return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief    Appends a message to the E_LOGFILE.
  @param    type    Message type.
  @param    msg     Message to append.
  @return   void
 
  If the @c E_LOGFILE variable is correctly defined, the corresponding
  log file is opened in append mode. The message type is printed into
  square brackets, then the message itself. The log file is then
  closed.
 
  This routine is not supposed to be called directly, but only through
  e_warning, e_error, e_comment or compute_status.
 */
/*--------------------------------------------------------------------------*/

void e_logfile(char * type, char * msg)
{
	static int	logfile_started = 0 ;
	FILE	*	log ;

	if (logfile_started==0) {
		e_logfile_start();
		atexit(e_logfile_stop);
		logfile_started = 1 ;
	}
	if (msg==NULL || msg[0]==0) return ;
	if ((log = fopen(comm_config.logfilename, "a")) != NULL)  {
		fprintf(log, "[%s] %8s %s\n", comm_config.logfilestamp, type, msg);
		fclose(log);
	}
	return ;
}

/* vim: set ts=4 et sw=4 tw=75 */
