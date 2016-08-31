
/*-------------------------------------------------------------------------*/
/**
   @file    comm.h
   @author  N. Devillard
   @date    Sep 1999
   @version $Revision: 1.14 $
   @brief   Output message handling.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: comm.h,v 1.14 2001/10/17 08:36:54 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 08:36:54 $
	$Revision: 1.14 $
*/

#ifndef _COMM_H_
#define _COMM_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>


/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Max size of an output message */
#define OUTPUTMSGSZ				256

/** Defined for backwards compatibility only */
#define ComputeStatus       compute_status
/** Defined for backwards compatibility only */
#define HelloWorld			hello_world

/*----------------------------------------------------------------------------
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
void set_verbose(int flag);

/*-------------------------------------------------------------------------*/
/**
  @brief    Get verbose level
  @return   Current verbose level

  Gets the current verbose level.
 */
/*--------------------------------------------------------------------------*/
int verbose_active(void);

/*-------------------------------------------------------------------------*/
/**
  @brief    Set debug level
  @param    flag New debug level
  @return   void

  Sets the debug level to a given value. Any integer is accepted, 0
  is considered as deactivated.
 */
/*--------------------------------------------------------------------------*/
void set_debug(int flag);

/*-------------------------------------------------------------------------*/
/**
  @brief    Get debug level
  @return   Current debug level

  Gets the current debug level.
 */
/*--------------------------------------------------------------------------*/
int debug_active(void);

/*-------------------------------------------------------------------------*/
/**
  @brief     Set ANSI term support
  @param    flag ANSI term support flag.
  @return   void

  Sets the ANSI term support level to a given value. Any integer is
  accepted, 0 is considered as deactivated.
 */
/*--------------------------------------------------------------------------*/
void set_ansiterm(int flag);

/*-------------------------------------------------------------------------*/
/**
  @brief    Get ANSI term support level
  @return   Current ANSI term support level

  Gets the current ANSI term support level.
 */
/*--------------------------------------------------------------------------*/
int ansiterm_active(void);

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
void set_logfile(int flag);

/*-------------------------------------------------------------------------*/
/**
  @brief    Get logfile activity status.
  @return   Logfile activity status as an integer.

  If the returned integer is non-zero, the logfile feature is
  currently activated, otherwise it is turned off.
 */
/*--------------------------------------------------------------------------*/
int logfile_active(void);

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
void set_logfilename(char * name);

/*-------------------------------------------------------------------------*/
/**
  @brief    Gets the current log file name
  @return   Pointer to statically allocated string (in this module).

  Find out what the current log file name is. The returned string is
  statically allocated, so no need to free it. Do not try to modify
  the contents of the returned string, use set_logfilename for that.
 */
/*--------------------------------------------------------------------------*/
char * get_logfilename(void);

/*-------------------------------------------------------------------------*/
/**
  @brief    Prints out program name and a short help.
  @param    pname   Program name
  @param    shelp   Short help (one-liner)
  @return   void
 
  To be called in every main() as an ID of the current command.
 */
/*--------------------------------------------------------------------------*/
void hello_world(char * pname, char * shelp);

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
void e_warning(char *fmt, ...) ;

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
void e_error(char *fmt, ...);

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
void e_comment(int level, char * fmt, ...);

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
void compute_status(char *Msg, int done, int total, int level);

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
void e_logfile_start(void);

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
void e_logfile_stop(void);

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
void e_logfile(char * type, char * msg);


#endif
