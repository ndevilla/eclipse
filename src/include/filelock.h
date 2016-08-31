
/*-------------------------------------------------------------------------*/
/**
   @file    filelock.h
   @author  N. Devillard
   @date    Sep 2000
   @version $Revision: 1.5 $
   @brief   File locking routines (portable).
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: filelock.h,v 1.5 2001/10/17 10:26:45 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/17 10:26:45 $
	$Revision: 1.5 $
*/

#ifndef _FILELOCK_H_
#define _FILELOCK_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>


/*---------------------------------------------------------------------------
						Function ANSI prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Lock a file and open it.
  @param    filenam     Name of the file to open.
  @param    mode        Open mode, same as fopen().
  @param    timeout     Number of retries (in seconds) before giving up.
  @return   FILE pointer, or NULL if error occurred.

  This function works just like fopen(), except that it takes an extra
  argument indicating how many times you want to retry to lock (once every
  second) before giving up. If the file is already locked by another
  process, this function will block, persisting in trying to acquire the
  lock until either it times out or the lock is released.
  If any error occurs, the returned file pointer is NULL.

  You must mandatorily close a locked file using fclose_lock().

  Notice that several processes may open the same file in read mode
  and locking it simultaneously. Once at least one process has a read
  lock, no other process can get a write lock to this file.
 */
/*--------------------------------------------------------------------------*/
FILE * fopen_lock(char * filename, char * mode, int timeout);

/*-------------------------------------------------------------------------*/
/**
  @brief    Close a file and release the associated lock.
  @param    f           FILE pointer to be closed.
  @return   int returned from fclose

  This function releases the lock on the provided file pointer and closes
  it. The returned value is the one returned by fclose on the given file.
 */
/*--------------------------------------------------------------------------*/
int fclose_lock(FILE * fp);



#endif
