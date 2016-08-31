
/*-------------------------------------------------------------------------*/
/**
   @file	manpage.c
   @author	N. Devillard
   @date	Jul 2001
   @version	$Revision: 2.3 $
   @brief	Man page pretty-printing on console.

   This module is useful to print out manual pages stored into
   static character strings onto the console.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: manpage.c,v 2.3 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 2.3 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "manpage.h"
#include "parse_tok.h"
#include "static_sz.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define ASCIIRULER \
"------------------------------------------------------------------------\n"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*/
/**
  @brief	Pretty-print an RCS tag
  @param	rcsval		Value to be pretty-printed
  @return	1 pointer to statically allocated string, NULL if error occurs.

  This function takes a character string containing an RCS tag, makes
  it pretty inside a statically allocated character string and returns
  a pointer to the string. The returned value is weak, since it changes
  with each function call. Do not try to free or modify the contents
  of the returned pointer.
 */
/*--------------------------------------------------------------------------*/

char * rcs_value(char * rcsval)
{
    static char rcs_pretty[ASCIILINESZ];
    char    **  tok ;
    int         ntok ;
    int         i ;
    
    tok = tokenize_line(rcsval, FS_BLANKS, &ntok);
    if (tok==NULL)
        return rcsval;
    
    if (ntok<=2) {
        free_tokens(tok, ntok);
        return rcsval;
    }   
    
    memset(rcs_pretty, 0, ASCIILINESZ);
    for (i=1 ; i<(ntok-1) ; i++) {
        strcat(rcs_pretty, tok[i]);
        if (i<(ntok-2)) {
            strcat(rcs_pretty, " ");
        }   
    }
    free_tokens(tok, ntok);
    return rcs_pretty ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Filter a string for HTML output
  @param	fp	File pointer to dump to.
  @param	s	String to dump
  @return	void

  The following function takes a string and dumps it to the passed
  (opened) file pointer, converting characters like '<' and '>' to
  their equivalent valid HTML format.
 */
/*--------------------------------------------------------------------------*/
static void manpage_htmlfilter(FILE * fp, char * s)
{
    char    line[ASCIILINESZ];
    int     i ;
    int     s_index ;
    int     len ;

    len = (int)strlen(s);
    /* Search and replace */
    i=0 ;
    s_index=0 ;
    while (s_index<len) {
        switch (s[s_index]) {
            case '<':
            line[i++]='&';
            line[i++]='l';
            line[i++]='t';
            line[i++]=';';
            break ;

            case '>':
            line[i++]='&';
            line[i++]='g';
            line[i++]='t';
            line[i++]=';';
            break ;

            case 0:
            break ;

            case '\n':
            fprintf(fp, "%s\n", line) ;
            memset(line, 0, ASCIILINESZ);
            i=0 ;
            break ;

            default:
            line[i++]=s[s_index];
            break ;
        }
        s_index++;
    }
    return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Print out a man page string to an opened file pointer.
  @param	title		Man page title.
  @param	manpage		String containing the man page to dump.
  @param	version		(Optional) version string to dump.
  @param	lastmod		(Optional) 'last modified' string to dump.
  @param	fp			Opened file pointer to dump to.
  @param	format		Output file format (see below).
  @return	void

  Dump a given man page string to an opened file pointer.
  Supported formats are: standard Unix man page in pure ASCII form
  (no troff formatting commands), or HTML. The 'format' parameter
  is a character string containing either "man" or "html".

  version and lastmod are optional parameters to give more information
  about the version number and last modification date.
 */
/*--------------------------------------------------------------------------*/

void manpage_dump(
	char * title,
	char * manpage,
	char * version,
	char * lastmod,
	FILE * fp,
	char * format
)
{
	if (title==NULL || manpage==NULL || fp==NULL || format==NULL) return ;
	if (!strcmp(format, "html")) {
		/* Header */
		fprintf(fp,
				"<html>\n"
				"<title>%s man page</title>\n"
				"<body>\n"
				"<pre>\n",
				title);
		/* Title */
		fprintf(fp,
				"<hr size=\"4\">\n"
				"<h2>Man page for %s", title);
		if (version) {
			fprintf(fp, " - %s", rcs_value(version));
		}
		fprintf(fp, "</h2>\n");
		if (lastmod) {
			fprintf(fp, "Last updated %s\n", rcs_value(lastmod));
		}
		fprintf(fp, "<hr size=\"4\">\n");
		/* Page body */
		manpage_htmlfilter(fp, manpage);
		/* Footer */
		fprintf(fp,
				"<hr size=\"4\">\n"
				"</pre>\n"
				"</body>\n"
				"</html>\n");
	} else {
		/* Man-like format in ASCII */
		fprintf(fp, ASCIIRULER);
		fprintf(fp, " Man page for %s", title);
		if (version) {
			fprintf(fp, " - %s", rcs_value(version));
		}
		fprintf(fp, "\n");
		if (lastmod) {
			fprintf(fp, " Last updated %s\n", rcs_value(lastmod));
		}
		fprintf(fp, ASCIIRULER);
		fprintf(fp, "%s", manpage);
		fprintf(fp, ASCIIRULER);
	}
	return ;
}

/* vim: set ts=4 et sw=4 tw=75 */
