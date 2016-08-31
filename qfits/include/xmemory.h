/*----------------------------------------------------------------------------*/
/**
  @file     xmemory.h
  @author   Nicolas Devillard
  @date     Oct 2000
  @version  $Revision: 1.24 $
  @brief    POSIX-compatible extended memory handling.

  xmemory is a small and efficient module offering memory extension
  capabitilies to ANSI C programs running on POSIX-compliant systems. It
  offers several useful features such as memory leak detection, protection for
  free on NULL or unallocated pointers, and virtually unlimited memory space.
  xmemory requires the @c mmap() system call to be implemented in the local C
  library to function. This module has been tested on a number of current Unix
  flavours and is reported to work fine.
  The current limitation is the limited number of pointers it can handle at
  the same time.
  See the documentation attached to this module for more information.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: xmemory.h,v 1.24 2005/05/18 14:38:01 yjung Exp $
	$Author: yjung $
	$Date: 2005/05/18 14:38:01 $
	$Revision: 1.24 $
*/

#ifndef XMEMORY_H
#define XMEMORY_H

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

/* To know if the current module has been linked against xmemory.c or not */
#define _XMEMORY_	1

/*-----------------------------------------------------------------------------
   								Macros
 -----------------------------------------------------------------------------*/

/* Protect strdup redefinition on systems which #define it */
#ifdef strdup
#undef strdup
#endif

#define malloc(s)       xmemory_malloc(s,       __FILE__,__LINE__)
#define calloc(n,s)     xmemory_calloc(n,s,     __FILE__,__LINE__)
#define realloc(p,s)    xmemory_realloc(p,s,    __FILE__,__LINE__)
#define free(p)         xmemory_free(p,         __FILE__,__LINE__)
#define strdup(s)       xmemory_strdup(s,       __FILE__,__LINE__)
#define falloc(f,o,s)   xmemory_falloc(f,o,s,   __FILE__,__LINE__)
#define fdealloc(f,o,s) xmemory_fdealloc(f,o,s, __FILE__,__LINE__)
#define xmemory_status() xmemory_status_(__FILE__,__LINE__)

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

void * 	xmemory_malloc(size_t, const char *, int) ;
void * 	xmemory_calloc(size_t, size_t, const char *, int) ;
void * 	xmemory_realloc(void *, size_t, const char *, int) ;
void   	xmemory_free(void *, const char *, int) ;
char * 	xmemory_strdup(const char *, const char *, int) ;
char *	xmemory_falloc(char *, size_t, size_t *, const char *, int) ;
void    xmemory_fdealloc(void *, size_t, size_t, const char *, int) ;

void xmemory_status_(const char * filename, int lineno) ;

#endif
