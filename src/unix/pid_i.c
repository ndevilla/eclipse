
/*-------------------------------------------------------------------------*/
/**
  @file		pid_i.c
  @author	N. Devillard
  @date		Sep 2000
  @version	$Revision: 1.6 $
  @brief	Process information routines.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: pid_i.c,v 1.6 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.6 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include "pid_i.h"


/*---------------------------------------------------------------------------
  							Function codes
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

int pid_exists(long pid)
{
	int	status ;

	if (pid<0)
		return 0;

	status = kill((pid_t)pid, 0);
	if ((status==0) || ((status==-1) && (errno==EPERM))) {
		return 1 ;
	}
	return 0 ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Find the name of the process for a given PID.
  @param	pid	PID of the process to search for.
  @return	pointer to statically allocated string.

  The returned pointer is statically allocated in this function. Do not try
  to free it or modify it. If no process can be found with this name, this
  function returns NULL.

  This function taken from the comp.unix.programmer FAQ.
 */
/*--------------------------------------------------------------------------*/

char * pid_getname(long pid)
{
   static char line[133], command[80], *linep, *token, *cmd;
   FILE *fp;

   if (pid==0) return NULL ;

   sprintf(command, "ps -p %ld 2>/dev/null", pid);
   if ((fp = popen(command, "r"))==NULL) return NULL ;

   /* read the header line */
   if (fgets(line, sizeof line, fp)==NULL) {
      pclose(fp);
      return NULL ;
   }

   /* figure out where the command name is from the column headings.
    * (BSD-ish machines put the COMMAND in the 5th column, while SysV
    * seems to put CMD or COMMAND in the 4th column.)
    */
   for (linep=line; ; linep=NULL) {
      if (NULL == (token = strtok(linep, " \t\n"))) {
         pclose(fp);
         return NULL ;
      }
      if (0 == strcmp("COMMAND", token) || 0 == strcmp("CMD", token)) {
		 /*  found the COMMAND column */
         cmd = token;
         break;
      }
   }

   /* read the ps(1) output line */
   if (NULL == fgets(line, sizeof line, fp)) {
      pclose(fp);
      return NULL ;
   }

   /* grab the "word" underneath the command heading... */
   if ((char *)0 == (token = strtok(cmd, " \t\n"))) {
      pclose(fp);
      return NULL ;
   }

   pclose(fp);
   return token;
}


#ifdef TEST
int main(int argc, char *argv[])
{
	long	pid ;
	char *	pid_name ;

	if (argc<2) {
		printf("use: %s <PID>\n", argv[0]);
		return 1 ;
	}
	pid = (long)atoi(argv[1]);
	if (pid_exists(pid)) {
		printf("process %ld exists\n", pid);
		pid_name = pid_getname(pid);
		if (pid_name==NULL) {
			printf("cannot find name for process\n");
		} else {
			printf("process called [%s]\n", pid_name);
		}
	} else {
		printf("no such process: %ld\n", pid);
	}
	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
