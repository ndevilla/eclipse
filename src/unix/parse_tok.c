

/*-------------------------------------------------------------------------*/
/**
  @file		parse_tok.c
  @author	N. Devillard
  @date		Feb 2000
  @version	$Revision: 1.15 $
  @brief	Cut a character string into its components (tokens).

  This module offers two helper functions to tokenize a string into
  sub-components in one function call. It is only meant to be a nicer
  interface than strtok (which it calls anyway).
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: parse_tok.c,v 1.15 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.15 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/
#include <ctype.h>

#include "parse_tok.h"
#ifdef _ECLIPSE_
#include "xmemory.h"
#endif


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Tokenize a line into smaller strings.
  @param	line		Line to tokenize.
  @param	fs			List of field separators.
  @param	ntok		(returned) number of found tokens.
  @return	Newly allocated list of tokens (argv-like format).

  This function takes in input a line of ASCII characters and a list
  of field separators. It cuts the input line into smaller strings
  (tokens).

  Example:

  @code
  char ** ltok ;
  int     ntok ;
  int     i ;

  strcpy(line, "\tThis is a line to   \t   tokenize\n");
  ltok = tokenize_line(line, FS_BLANKS, &ntok);

  for (i=0 ; i<ntok ; i++) {
      printf("tok[%d] is [%s]\n", i, ltok[i]);
  }

  free_tokens(ltok, ntok);
  @endcode

  When tokenize_line() returns, ntok is 6 and ltok is an array
  of 6 character strings as:

  - ltok[0] is "This"
  - ltok[1] is "is"
  - ltok[2] is "a"
  - ltok[3] is "line"
  - ltok[4] is "to"
  - ltok[5] is "tokenize"
  - ltok[6] is NULL.

  Notice that the returned list must be freed using free_tokens()
  (see below). Individual tokens are allocated using strdup().

  Field separators can be any characters. Since in most cases you will
  want to tokenize with respect to blank characters, you can use the
  FS_BLANKS constant defined in this module (published in the .h file).
 */
/*--------------------------------------------------------------------------*/



char ** tokenize_line(char * line, char * fs, int * ntok)
{
    char *  cline ;
    char ** l_tok ;
    char *  tok ;
    int     nt ;


	if ((line==NULL) || (ntok==NULL) || (fs==NULL)) {
		*ntok = 0 ;
		return NULL ;
	}
    nt = 0 ;
    cline = strdup(line);
    tok = strtok(cline, fs);
    if (tok == NULL) {
        *ntok = 0 ;
		free(cline);
        return NULL ;
    }
    while (tok!=NULL) {
        nt ++ ;
        tok = strtok(NULL, fs);
    }
    free(cline);
    cline = strdup(line);
    l_tok = calloc(nt+1, sizeof(char*));
    *ntok = nt ;
    tok = strtok(cline, fs);
    nt = 0 ;
    while (tok!=NULL) {
        l_tok[nt] = strdup(tok);
        tok = strtok(NULL, fs);
        nt ++ ;
    }
    free(cline);
    return l_tok ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Free a list of tokens allocated by tokenize_line().
  @param	l_tok	List of tokens to free.
  @param	ntok	Number of tokens in the list.
  @return	void

  Frees the list of tokens, i.e. frees each token and then frees the
  main pointer.
 */
/*--------------------------------------------------------------------------*/

void free_tokens(char ** l_tok, int ntok)
{
    int     i ;
    if ((ntok<1) || (l_tok==NULL)) return ;
    for (i=0 ; i<ntok ; i++) {
        if (l_tok[i]!=NULL) free(l_tok[i]);
    }
    free(l_tok);
    return ;
}


/* Simple test code */
#ifdef TEST
int main(int argc, char *argv[])
{
	char	**	ltok ;
	int			ntok ;
	int			i ;

	if (argc<2) {
		printf("use: %s 'long sentence enclosed in quotes'\n", argv[0]);
		return 1 ;
	}

	ltok = tokenize_line(argv[1], FS_BLANKS, &ntok);
	printf("found %d tokens\n", ntok);
	if (ntok>0) {
		for (i=0 ; i<ntok ; i++) {
			printf("tok[%d] is [%s]\n", i, ltok[i]);
		}
	}
	if (ltok!=NULL)
		free_tokens(ltok, ntok);
	return 0;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
