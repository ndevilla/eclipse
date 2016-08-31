
/*-------------------------------------------------------------------------*/
/**
  @file		t_stamp.c
  @author	N. Devillard
  @date		Jan 2001
  @version	$Revision: 1.7 $
  @brief	Time stamp routines.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: t_stamp.c,v 1.7 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.7 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "static_sz.h"
#include "userid.h"

/*---------------------------------------------------------------------------
  							Defines	
 ---------------------------------------------------------------------------*/

/** Max size of a host name, according to SUSv2 */
#define HOSTNAMESZ	256

/** Max size of a date/time string */
#define DATESTRSZ	128

/*---------------------------------------------------------------------------
  							Function codes
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
char * create_timestamp(void)
{
	static
	char	timestamp[ASCIILINESZ];

    char    hostname[HOSTNAMESZ] ;
    char *  username ;
    char    time_str[DATESTRSZ] ;
    time_t  local_t ;
	int		len ;
 
	/* Get host name though libc's gethostname */
    if (gethostname(hostname, HOSTNAMESZ) == -1)
        sprintf(hostname, "localhost") ;
 
	/* Get login name through eclipse's get_login_name */
    username = get_login_name() ;

	/* Get time string */
    if (time(&local_t) == (time_t)-1) {
		time_str[0] = (char)0;
	} else {
        sprintf(time_str, "%s", ctime(&local_t)) ;
		/* Remove trailing \n if any is found */
		len = strlen(time_str);
		if (len>0) {
			len-- ;
			while (time_str[len]=='\n') {
				time_str[len]=0;
				len--;
			}
		}
	}
 
	/* Build up timestamp */
    if (username != NULL) {
		sprintf(timestamp, "%s@%s %s", username, hostname, time_str);
	} else {
		sprintf(timestamp, "?@%s %s", hostname, time_str);
	}

	return timestamp ;
}

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
char * create_logtimestamp(void)
{
	static
	char	timestamp[ASCIILINESZ];
    char    hostname[HOSTNAMESZ] ;
    char *  username ;
 
	/* Get host name though libc's gethostname */
    if (gethostname(hostname, HOSTNAMESZ) == -1)
        sprintf(hostname, "localhost") ;
 
	/* Get login name through eclipse's get_login_name */
    username = get_login_name() ;

	/* Build up timestamp */
    if (username != NULL) {
		sprintf(timestamp, "%s@%s:%ld", username, hostname, (long)getpid());
	} else {
		sprintf(timestamp, "?@%s:%ld", hostname, (long)getpid());
	}

	return timestamp ;
}

#ifdef TEST
/* Link me against userid.o */
int main(int argc, char * argv[])
{
	printf("%s\n%s\n",
			create_timestamp(),
			create_logtimestamp());
	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
