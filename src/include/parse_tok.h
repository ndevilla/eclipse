
/*-------------------------------------------------------------------------*/
/**
  @file     parse_tok.h
  @author   N. Devillard
  @date     Feb 2000
  @version  $Revision: 1.9 $
  @brief    Cut a character string into its components (tokens).

  This module offers two helper functions to tokenize a string into
  sub-components in one function call. It is only meant to be a nicer
  interface than strtok (which it calls anyway).
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: parse_tok.h,v 1.9 2001/10/19 10:49:09 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/19 10:49:09 $
	$Revision: 1.9 $
*/

#ifndef _PARSE_TOK_H_
#define _PARSE_TOK_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Default field separator list */
#define FS_BLANKS	" \t\n"


/*---------------------------------------------------------------------------
						Function ANSI C prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Tokenize a line into smaller strings.
  @param    line        Line to tokenize.
  @param    fs          List of field separators.
  @param    ntok        (returned) number of found tokens.
  @return   Newly allocated list of tokens (argv-like format).

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
char ** tokenize_line(char * line, char * fs, int * ntok);

/*-------------------------------------------------------------------------*/
/**
  @brief    Free a list of tokens allocated by tokenize_line().
  @param    l_tok   List of tokens to free.
  @param    ntok    Number of tokens in the list.
  @return   void

  Frees the list of tokens, i.e. frees each token and then frees the
  main pointer.
 */
/*--------------------------------------------------------------------------*/
void free_tokens(char ** l_tok, int ntok);


#endif
