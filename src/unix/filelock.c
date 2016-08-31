
/*-------------------------------------------------------------------------*/
/**
   @file	filelock.c
   @author	N. Devillard
   @date	Sep 2000
   @version	$Revision: 1.8 $
   @brief	File locking routines (portable).
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: filelock.c,v 1.8 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.8 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/*---------------------------------------------------------------------------
							Private functions
 ---------------------------------------------------------------------------*/

/* Alias for easier reading of the locking functions. */
static int lock_reg(int fd, int cmd, int type)
{
    struct flock    lock;

    lock.l_type 	= type;
    lock.l_start 	= 0;
    lock.l_whence 	= SEEK_SET;
    lock.l_len 		= 0;
    return fcntl(fd, cmd, &lock);
}


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Lock a file and open it.
  @param	filenam		Name of the file to open.
  @param	mode		Open mode, same as fopen().
  @param	timeout		Number of retries (in seconds) before giving up.
  @return	FILE pointer, or NULL if error occurred.

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
FILE * fopen_lock(char * filename, char * mode, int timeout)
{
    int     attempts ;
    FILE *  fp ;
    int     fd ;

    /* Open the file as requested */
    fp = fopen(filename, mode);
    if (fp==NULL) return NULL ;
    fd = fileno(fp);
    attempts=0 ;
    while (attempts<timeout) {
        if (!strcmp(mode, "r")) {
            if (lock_reg(fd, F_SETLK,  F_RDLCK)==0) {
                break ;
            }
        } else {
            if (lock_reg(fd, F_SETLK,  F_WRLCK)==0) {
                break ;
            }
        }
        sleep(1);
        attempts++ ;
    }
    if (attempts>=timeout) {
        fclose(fp);
        return NULL ;
    }
    return fp ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Close a file and release the associated lock.
  @param	f			FILE pointer to be closed.
  @return	int returned from fclose

  This function releases the lock on the provided file pointer and closes
  it. The returned value is the one returned by fclose on the given file.
 */
/*--------------------------------------------------------------------------*/

int fclose_lock(FILE * fp)
{
    lock_reg(fileno(fp), F_SETLK,  F_UNLCK);
    return fclose(fp);
}


/* Demo code */
#ifdef MAIN
int main(int argc, char *argv[])
{
	int		timeout=5 ;
	FILE *	f ;

	if (argc<3) {
		printf("use: %s <file> <mode>\n", argv[0]);
		return 1 ;
	}
	printf("getting lock on %s\n", argv[1]);
	f = fopen_lock(argv[1], argv[2], timeout) ;
	if (f==NULL) {
		printf("cannot get lock\n");
		return -1 ;
	}
	printf("lock acquired Ok\n");
	/* wait */
	sleep(3);
	fclose_lock(f);
	printf("lock released\n");
	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
