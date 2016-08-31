
/*-------------------------------------------------------------------------*/
/**
   @file	charmatrix.c
   @author	N. Devillard
   @date	Jul 2000
   @version	$Revision: 1.10 $
   @brief	Read tables in ASCII files to a 2d char matrix.

   This modules handles tables stored in ASCII files, which have the
   following format: data are separated by blanks (spaces or tabs)
   and lines by <CR>. The object used for storage is a 2d character
   matrix. This object is low-level enough to accomodate any kind
   of higher-level data structure.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: charmatrix.c,v 1.10 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.10 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "charmatrix.h"

#ifdef _ECLIPSE_
#include "static_sz.h"
#include "comm.h"
#include "parse_tok.h"
#include "xmemory.h"
#else
#define FILENAMESZ		512
#define ASCIILINESZ		1024
#include "e_error.h"
#include "unmem.h"
#include "parse_tok.h"
#endif


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Allocate a new charmatrix object.
  @param	lx		Max number of columns in the matrix.
  @param	ly		Max number of lines in the matrix.
  @return	1 newly allocated charmatrix object.

  This is the constructor for the charmatrix object. It requires a maximal
  number of lines and columns for creation. Once such an object is created,
  its size is fixed and should not be changed in any way.

  The returned object must be deallocated using charmatrix_del().
 */
/*--------------------------------------------------------------------------*/

charmatrix * charmatrix_new(int lx, int ly)
{
	charmatrix	*	m ;

	m = malloc(sizeof(charmatrix));
	m->lx = lx ;
	m->ly = ly ;
	m->c  = calloc(lx * ly, sizeof(char*));
	return m ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Deallocate a charmatrix object.
  @param	m	Charmatrix to free.
  @return	void

  Deallocates all memory associated to a charmatrix. This includes the word
  pointers and the structure pointers.
 */
/*--------------------------------------------------------------------------*/
void charmatrix_del(charmatrix * m)
{
	int		i, j ;
	char *	s ;

	if (m==NULL) return ;
	if (m->c == NULL) {
		free(m);
		return ;
	}
	for (j=0 ; j<m->ly ; j++) {
		for (i=0 ; i<m->lx ; i++) {
			s = charmatrix_elem(m,i,j);
			if (s!=NULL) {
				free(s);
				s = NULL ;
			}
		}
	}
	free(m->c);
	free(m) ;
	return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Copy a charmatrix into a new object.
  @param	c	Charmatrix to copy.
  @return	1 newly allocated charmatrix object.

  Allocates a new charmatrix object and initializes its contents with a
  copy of the provided charmatrix. The returned object must be deallocated
  using charmatrix_del().
 */
/*--------------------------------------------------------------------------*/

charmatrix * charmatrix_copy(charmatrix * c)
{
	charmatrix	*	cpy ;
	int				i, j ;
	char		*	s ;

	if (c==NULL) return NULL ;

	/* Allocate new charmatrix */
	cpy = charmatrix_new(c->lx, c->ly);
	/* Populate it with strings from input matrix */
	for (j=0 ; j<cpy->ly ; j++) {
		for (i=0 ; i<cpy->lx ; i++) {
			s = charmatrix_elem(c,i,j);
			if (s!=NULL) {
				charmatrix_elem(cpy, i, j) = strdup(s);
			}
		}
	}
	return cpy ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Dump a charmatrix object to an opened file pointer.
  @param	m		Charmatrix to dump.
  @param	out		Output file.
  @return	void

  The following function dumps a charmatrix object to an opened file
  pointer. It is Ok to dump to standard streams (stdout or stderr).

  For debugging purposes mostly.
 */
/*--------------------------------------------------------------------------*/

void charmatrix_dump(charmatrix * m, FILE * out)
{
	int		i, j ;
	char *	s ;

	for (j=0 ; j<m->ly ; j++) {
		for (i=0 ; i<m->lx ; i++) {
			s = charmatrix_elem(m,i,j) ;
			if (s!=NULL) {
				fprintf(out, "[%s]\t", s);
			}
			/* else { fprintf(out, "[]\t"); } */
		}
		fprintf(out, "\n");
	}
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Read a charmatrix from an ASCII file.
  @param	filename	Name of the ASCII file to parse.
  @return	1 newly allocated charmatrix object.

  This function reads a charmatrix from an ASCII file. The number of
  columns is determined by the column in the file that has the maximal
  number of tokens. The number of lines is determined by the number of
  lines containing valid tokens.

  The input ASCII file has the following syntax:

  - Blank lines are ignored
  - Lines starting with a hash are ignored (comments)
  - Tokens are separated by any number of whitespaces (tabs, blanks)
  - Whitespace is not allowed within one token
 */
/*--------------------------------------------------------------------------*/

charmatrix * charmatrix_read(char * filename)
{
	charmatrix	*	m ;
	char		**	tokens ;
	int				ntok ;
	int				i, j ;
	int				lx, ly ;
	char			line[ASCIILINESZ];
	FILE		*	in ;

	if ((in=fopen(filename, "r"))==NULL) {
		e_error("cannot open file [%s]", filename);
		return NULL ;
	}

	/* Count number of lines and columns */
	lx=0 ;
	ly=0 ;
	while (fgets(line, ASCIILINESZ, in)!=NULL) {
		if (line[0]!='#') {
			tokens = tokenize_line(line, FS_BLANKS, &ntok) ;
			if (tokens!=NULL) {
				/* One more valid line */
				ly ++ ;
				/* Count max number of columns */
				if (lx<ntok) {
					lx = ntok ;
				}
				free_tokens(tokens, ntok);
			}
		}
	}
	rewind(in);

	if (lx<1 || ly<1) {
		e_error("no data in file [%s]", filename);
		fclose(in);
		return NULL ;
	}

	m = charmatrix_new(lx, ly);
	/* Get values */
	i=j=0 ;
	while (fgets(line, ASCIILINESZ, in)!=NULL) {
		if (line[0]!='#') {
			tokens = tokenize_line(line, FS_BLANKS, &ntok) ;
			if (tokens!=NULL) {
				for (i=0 ; i<lx ; i++) {
					if (i<ntok) {
						charmatrix_elem(m,i,j) = strdup(tokens[i]);
					} else {
						charmatrix_elem(m,i,j) = NULL ;
					}
				}
				free_tokens(tokens, ntok);
				j++ ;
			}
		}
	}
	fclose(in);
	return m ;
}


/* Test program to check this module only */
#ifdef MAINCHARMATRIX
int main(int argc, char * argv[])
{
	charmatrix	*	c ;

	if (argc<2) {
		printf("usage: %s <ASCII file>\n", argv[0]);
		return 1 ;
	}
	c = charmatrix_read(argv[1]);
	printf("%d cols %d lines\n", c->lx, c->ly);
	charmatrix_dump(c, stdout);
	charmatrix_del(c);
	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
