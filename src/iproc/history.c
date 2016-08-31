/*-------------------------------------------------------------------------*/
/**
   @file	history.c
   @author	N. Devillard
   @date	June 2001
   @version	$Revision: 1.5 $
   @brief	History handling class, useful for FITS headers
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: history.c,v 1.5 2001/10/26 14:34:52 yjung Exp $
	$Author: yjung $
	$Date: 2001/10/26 14:34:52 $
	$Revision: 1.5 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "history.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define HISTORY_INITSZ		16

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Create a new history object.
  @return	1 newly allocated history object.

  The history object is meant to store historical comments. Anything can be
  stored, but the main purpose is to complete the HISTORY fields of a FITS
  header before dumping it to disk. Lines can only be added to a history
  object, not removed. A history object is a storage structure for an
  unlimited number of char strings of unlimited size, that can be dumped
  to the screen or to a FITS header.

  The returned object must be freed using history_del().
 */
/*--------------------------------------------------------------------------*/
history * history_new(void)
{
	history	*	hs ;

	hs = malloc(sizeof(history));
	hs->n = 0 ;
	hs->size = HISTORY_INITSZ ;
	hs->line = calloc(hs->size, sizeof(char*));
	return hs ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Delete a history object.
  @param	hs	History object to deallocate.
  @return	void

  Deletes all data associated to a history object.
 */
/*--------------------------------------------------------------------------*/
void history_del(history * hs)
{
	int	i ;

	if (hs==NULL) return ;
	if (hs->size<1) return ;
	for (i=0 ; i<hs->n ; i++) {
		if (hs->line[i]!=NULL) {
			free(hs->line[i]);
		}
	}
	free(hs->line);
	free(hs);
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Append a line to a history object.
  @param	hs		History object to update.
  @param	format	printf-like format string
  @return	void

  Appends a character string as a line into a history object.
  Works just like a printf statement.
  Probably unsafe on string lengths, this has to be worked on.
 */
/*--------------------------------------------------------------------------*/
void history_add(
		history	* 	hs, 
		char 	* 	format, ...)
{
	va_list	ap ;
	void  * newptr ;
	char	line[1024] ;
	char	elem[1024] ;

	if (hs==NULL || format==NULL) return ;

	/* Build a single string from a variable # of arguments */
	memset(elem, 0, 1024);
	memset(line, 0, 1024);
	va_start(ap, format);
	vsprintf(elem, format, ap);
	strcat(line, elem);
	va_end(ap);

	/* See if current storage is exhausted */
	if (hs->n == hs->size) {
		/* Double storage space */
		newptr = calloc(2 * hs->size, sizeof(char*));
		memcpy(newptr, hs->line, hs->size * sizeof(char*));
		free(hs->line);
		hs->line = newptr ;
		hs->size *= 2 ;
	}
	/* Add new line */
	hs->line[hs->n] = strdup(line);
	hs->n ++ ;
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Dump a history object to an open FILE pointer.
  @param	hs		History object to dump.
  @param	fp		Open file pointer.
  @return	void

  This function dumps the contents of a history object onto an open
  file pointer. It is Ok to provide stdout or stderr as file pointers.
 */
/*--------------------------------------------------------------------------*/
void history_dump(
		history	*	hs, 
		FILE 	* 	fp)
{
	int	i ;
	if (hs==NULL || fp==NULL) return ;

	printf("--> history dump\n");
	for (i=0 ; i<hs->n ; i++) {
		if (hs->line[i]!=NULL) {
			printf("%s\n", hs->line[i]);
		}
	}
	return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Dump a history object into a FITS header object.
  @param	hs		History object to dump.
  @param	fh		FITS header to modify.
  @return	int 0 if Ok, -1 otherwise.

  This function dumps a history object as a list of HISTORY keys into
  an allocated FITS header.
 */
/*--------------------------------------------------------------------------*/
int history_addfits(
		history			*	hs, 
		qfits_header 	* 	fh)
{
	int	i ;

	if (hs==NULL || fh==NULL) return -1 ;

	for (i=0 ; i<hs->n ; i++) {
		if (hs->line[i]!=NULL) {
			/*
			 * Do not worry about truncation, this is handled in the FITS
			 * routines.
			 */
			qfits_header_add(fh, "HISTORY", hs->line[i], NULL, NULL);
		}
	}
	return 0 ;
}

