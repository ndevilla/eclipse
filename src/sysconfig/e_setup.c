/*----------------------------------------------------------------------------*/
/**
   @file    e_setup.c
   @author  N. Devillard
   @date    March 2000
   @version	$Revision: 1.8 $
   @brief   eclipse rc file setup
   This is a script written in C.
   It asks various questions about eclipse configuration setup and writes the 
   results out to the default rc file.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: e_setup.c,v 1.8 2003/02/14 09:33:16 yjung Exp $
	$Author: yjung $
	$Date: 2003/02/14 09:33:16 $
	$Revision: 1.8 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

#if OS_HPUX
#include <sys/param.h>
#include <sys/pstat.h>
#endif

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define FILENAMESZ		1024

#define EVERB_DEF		1
#define EDEB_DEF		0

/*-----------------------------------------------------------------------------
   								Global variables
 -----------------------------------------------------------------------------*/

char	line[FILENAMESZ];
char	e_path[FILENAMESZ];
int		e_verbose ;
int		e_debug ;
char	e_tmpdir[FILENAMESZ];
int		e_logfile_active ;
char	e_logfile[FILENAMESZ];
char	e_rcname[FILENAMESZ];

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/
void chop(char * s, char c)
{
	int	len ;
	if (!s) return ;
	len = strlen(s);
	if (len<1) return ;
	if (s[len-1]==c)
		s[len-1]=(char)0;
}

int is_valid_dir(char * path)
{
	DIR	* adir ;

	if (!path) return 0 ;
	if ((adir=opendir(path))==NULL) {
		return 0 ;
	} else {
		closedir(adir);
	}
	return 1;
}

void e_welcome(void)
{
	printf(
		"\n\n"
		"*** eclipse configuration setup ***\n"
		"\n\n"
		"Answer the following questions to configure eclipse\n"
		"on your machine. These informations will be stored\n"
		"in your home directory in a file called .eclipse-rc\n"
		"\n\n"
		"Proceed? (CTRL-C to interrupt, ENTER to continue)\n"
		);
	fgets(line, FILENAMESZ, stdin);
}

void e_getpath(void)
{
	int			ok ;
	char		bindir[FILENAMESZ];
	char		mandir[FILENAMESZ];

	getcwd(e_path, FILENAMESZ);

	printf(
		"\n\n"
		"---------- E_PATH\n"
		"\n\n"
		);

	ok = 0 ;
	while (!ok) {
		printf("where is the root eclipse directory on your disk?\n");
		printf("[%s]: ", e_path);
		fgets(line, FILENAMESZ, stdin);
		chop(line, '\n');
		chop(line, '/');
		if (line[0]==0) {
			strcpy(line, e_path);
		}
		sprintf(bindir, "%s/bin", line);
		sprintf(mandir, "%s/man", line);
		if (!is_valid_dir(bindir) || !is_valid_dir(mandir)) {
			printf(
				"\n"
				"*** not a valid eclipse home\n"
				"*** does not contain a bin/ or man/ directory\n"
				"\n"
				);
		} else {
			ok++ ;
		}
	}
	strcpy(e_path, line);
	printf("E_PATH set to %s\n", e_path);
	return ;
}

void e_getverbose(void)
{
	printf("\n\n");
	printf("---------- E_VERBOSE\n");
	printf("\n\n");

	e_verbose = EVERB_DEF ;
	printf("Verbose [y]? ");
	fgets(line, FILENAMESZ, stdin);
	chop(line, '\n');
	if (line[0]==0) {
		e_verbose = EVERB_DEF ;
	} else if (line[0]=='y' || line[0]=='Y') {
		e_verbose = 1 ;
	} else if (line[0]=='n' || line[0]=='N') {
		e_verbose = 0 ;
	} else if (isdigit(line[0])) {
		e_verbose = atoi(line);
	}
	if (e_verbose==0) {
		printf("E_VERBOSE deactivated\n");
	} else if (e_verbose==1) {
		printf("E_VERBOSE activated\n");
	} else {
		printf("E_VERBOSE set to %d\n", e_verbose);
	}
	return ;
}
	
void e_getdebug(void)
{
	printf("\n\n");
	printf("---------- E_DEBUG\n");
	printf("\n\n");

	e_debug = EDEB_DEF ;
	printf("Debug [n]? ");
	fgets(line, FILENAMESZ, stdin);
	chop(line, '\n');
	if (line[0]==0) {
		e_debug = EDEB_DEF ;
	} else if (line[0]=='y' || line[0]=='Y') {
		e_debug = 1 ;
	} else if (line[0]=='n' || line[0]=='N') {
		e_debug = 0 ;
	} else if (isdigit(line[0])) {
		e_debug = atoi(line);
	}
	if (e_debug==0) {
		printf("E_DEBUG deactivated\n");
	} else if (e_debug==1) {
		printf("E_DEBUG activated\n");
	} else {
		printf("E_DEBUG set to %d\n", e_debug);
	}
	return ;
}

void e_gettmpdir(void)
{
	int		ok ;

	printf(
		"\n"
		"\n"
		"---------- E_TMPDIR\n"
		"\n"
		"\n"
		"When an eclipse process runs out of memory (RAM and swap)\n"
		"it starts creating its own swap space on a path you specify.\n"
		"You must have read/write access to this path.\n"
		"\n"
		"You can specify '.' (the current directory) as a path.\n"
		"It means that any eclipse command will always use the\n"
		"directory it is working in a temporary swap space.\n"
		"\n"
		);

	ok=0;
	while (!ok) {
		printf("\n");
		printf("Path to swap area: ");
		fgets(line, FILENAMESZ, stdin);
		chop(line, '\n');
		chop(line, '/');
		if (!strcmp(line, ".")) {
			ok++ ;
			strcpy(e_tmpdir, ".");
		} else if (is_valid_dir(line)) {
			if (!access(line, W_OK)) {
				ok++ ;
				strcpy(e_tmpdir, line);
			}
		} else {
			printf("invalid path.\n");
		}
	}
	printf("E_TMPDIR set to %s\n", e_tmpdir);
	return ;
}

void e_getlogfile(void)
{
	FILE	*	tryme ;
	int			ok ;

	e_logfile[0]=0;

	printf(
		"\n\n"
		"---------- E_LOGFILE\n"
		"\n\n"
		);
	printf("Do you want to log all eclipse activities [n]? ");
	fgets(line, FILENAMESZ, stdin);
	chop(line, '\n');
	if (line[0]=='y' || line[0]=='Y') {
		printf(
			"Provide a file name (with fully qualified path)\n"
			"This file will receive in append all activities\n"
			"issued from eclipse commands.\n"
			"\n"
			"\n"
			);
		ok=0;
		while (!ok) {
			printf("file name: ");
			fgets(line, FILENAMESZ, stdin);
			chop(line, '\n');
			if ((tryme=fopen(line, "w"))==NULL) {
				printf("cannot create [%s]\n", line);
			} else {
				fclose(tryme);
				ok=1 ;
			}
		}
		strcpy(e_logfile, line);
	}
	if (e_logfile[0]!=0) {
		printf("E_LOGFILE set to %s\n", e_logfile);
		e_logfile_active = 1 ;
	} else {
		printf("E_LOGFILE not set\n");
		e_logfile_active = 0 ;
	}
	return ;
}

int e_findshell(void)
{
	char	*	shell ;
	int			shell_int ;
	int			ok ;

	shell_int=0 ;

	shell = getenv("SHELL");
	if (shell==NULL) {
		printf("environment variable SHELL is undefined.\n");
		ok=0 ;
		while (!ok) {
			printf("what is your default shell:\n");
			printf("[1] bash, sh, ksh\n");
			printf("[2] csh, tcsh\n");
			printf("Your shell (1 or 2): ");
			fgets(line, FILENAMESZ, stdin);
			chop(line, '\n');
			if (line[0]=='1') {
				shell_int = 1;
				ok++ ;
			} else if (line[0]=='2') {
				shell_int = 2 ;
				ok++;
			} else {
				printf("invalid shell - %s\n", line);
			}
		}
	} else {
		if (!strcmp(shell, "/bin/bash") ||
			!strcmp(shell, "/bin/sh") ||
			!strcmp(shell, "/bin/ksh")) {
			shell_int = 1 ;
		} else if (!strcmp(shell, "/bin/csh") ||
				   !strcmp(shell, "/bin/tcsh")) {
			shell_int = 2 ;
		} else {
			printf("unknown shell: %s\n", shell);
			ok=0;
			while (!ok) {
				printf("what is your default shell: ");
				printf("[1] bash, sh, ksh\n");
				printf("[2] csh, tcsh\n");
				printf("Your shell (1 or 2): ");
				fgets(line, FILENAMESZ, stdin);
				chop(line, '\n');
				if (line[0]=='1') {
					shell_int = 1;
					ok++ ;
				} else if (line[0]=='2') {
					shell_int = 2 ;
					ok++;
				} else {
					printf("invalid shell - %s\n", line);
				}
			}
		}
	}
	return shell_int ;
}

void e_makerc(void)
{
	int			shell_int ;
	FILE	*	rcfile ;
	char	*	homedir ;
	int			ok ;

	/* Find out user shell */
	shell_int = e_findshell() ;
	if (shell_int==0) {
		printf("cannot determine your shell: exiting\n");
		exit(-1);
	}
	
	printf("\n\n");
	printf("---------- generating rc file ");
	switch (shell_int) {
		case 1:
		printf("for bash/sh/ksh\n");
		break ;
		
		case 2:
		printf("for csh/tcsh\n");
		break ;

		default:
		break ;
	}

	homedir = getenv("HOME");
	if (homedir==NULL) {
		printf("environment variable HOME is undefined.\n");
		ok=0 ;
		while (!ok) {
			printf("where is your home directory: ");
			fgets(line, FILENAMESZ, stdin);
			chop(line, '\n');
			if (is_valid_dir(line)) {
				homedir = line ;
				ok++ ;
			} else {
				printf("invalid directory [%s]\n", line);
			}
		}
	}
	sprintf(e_rcname, "%s/.eclipse-rc", homedir);

	rcfile = fopen(e_rcname, "w");

	switch (shell_int) {

		case 1:
		fprintf(rcfile,
		"#\n"
		"# eclipse configuration file\n"
		"#\n"
		"\n\n"
		"# Add eclipse to path\n"
		"E_PATH='%s' ; export E_PATH\n"
		"PATH=\"$PATH:$E_PATH/bin\"\n"
		"\n"
		"# Add verbose/debug\n"
		"E_VERBOSE=%d ; export E_VERBOSE\n"
		"E_DEBUG=%d ; export E_DEBUG\n"
		"# Temporary swap directory\n"
		"E_TMPDIR=%s ; export E_TMPDIR\n",
		e_path,
		e_verbose,
		e_debug,
		e_tmpdir);

		if (e_logfile_active) {
			fprintf(rcfile,
			"# Log file\n"
			"E_LOGFILE=%s ; export E_LOGFILE\n",
			e_logfile);
		}
		fprintf(rcfile,
				"\n"
				"# end of file\n");
		break ;

		case 2:
		fprintf(rcfile,
		"#\n"
		"# eclipse configuration file\n"
		"#\n"
		"\n\n"
		"# Add eclipse to path\n"
		"setenv E_PATH %s\n"
		"set path=($path $E_PATH/bin)\n"
		"\n"
		"# Add eclipse/man to man path\n"
		"if $?MANPATH then\n"
		"setenv MANPATH \"${MANPATH}:$E_PATH/man\"\n"
		"endif\n"
		"\n"
		"# Add verbose/debug\n"
		"setenv E_VERBOSE %d\n"
		"setenv E_DEBUG   %d\n"
		"# Temporary swap directory\n"
		"setenv E_TMPDIR  %s\n",
		e_path,
		e_verbose,
		e_debug,
		e_tmpdir);
		
		if (e_logfile_active) {
			fprintf(rcfile,
			"# Log file\n"
			"setenv E_LOGFILE %s\n"
			"\n"
			"# end of file\n",
			e_logfile);
		} else {
			fprintf(rcfile,
			"# Log file\n"
			"unsetenv E_LOGFILE\n"
			"\n"
			"# end of file\n");
		}
		break ;
	}

	printf(
		"\n"
		"\n"
		"*** Summary\n"
		"\n"
		"eclipse root directory: [%s]\n"
		"verbose : %d\n"
		"debug   : %d\n"
		"tmpdir  : [%s]\n",
		e_path,
		e_verbose,
		e_debug,
		e_tmpdir
		);
	
	if (e_logfile[0]==0) {
		printf("logfile : no\n");
	} else {
		printf("logfile : [%s]\n", e_logfile);
	}
	printf(
		"\n"
		"\n"
		"configuration saved to [%s]\n"
		"Add the following line to your %s or %s file\n"
		"to have these options valid for all sessions:\n"
		"\n"
		"source ~/.eclipse-rc\n"
		"\n"
		"\n",
		e_rcname,
		(shell_int==1) ? ".bashrc" : ".cshrc",
		(shell_int==1) ? ".profile" : ".login"
		);
	return ;
}

int main(int argc, char *argv[])
{
	e_welcome();

	/* Ask for values */
	e_getpath();
	e_getverbose();
	e_getdebug();
	e_gettmpdir();
	e_getlogfile();

	/* Make rc file */
	e_makerc();
	return 0 ;
}
