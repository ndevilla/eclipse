
/*-------------------------------------------------------------------------*/
/**
   @file    filesys.h
   @author  N. Devillard
   @date    Jul 1998
   @version $Revision: 1.6 $
   @brief   File system info utilities
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: filesys.h,v 1.6 2001/10/18 10:03:24 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2001/10/18 10:03:24 $
    $Revision: 1.6 $
*/

#ifndef _ECLIPSE_FILESYS_H_
#define _ECLIPSE_FILESYS_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/*-------------------------------------------------------------------------*/
/**
  @typedef	disk_info
  @brief	Disk information structure

  This structure concentrates a number of informations about a filesystem,
  like the absolute path, available number of kbytes, etc. It is to be
  understood as a read-only structure built by functions in this module.
 */
/*-------------------------------------------------------------------------*/
typedef struct _DISK_INFO_ {
    char            filesystem[256] ;
    unsigned long   kbytes ;
    unsigned long   used ;
    unsigned long   avail ;
    int             capacity ;
    char            path[256] ;
} disk_info ;



/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out write permissions on a given path.
  @param    path    Path to examine.
  @return   boolean (int)

  Find out if the current user has write permissions on the provided
  path name. Uses the @c access system call to check this out.
 */
/*--------------------------------------------------------------------------*/
int test_write_permission(char * path);

/*-------------------------------------------------------------------------*/
/**
  @brief    Find out how maby kbytes are available on a disk.   
  @param    path    Path to examine.
  @return   Available kbytes, zero if cannot determine.

  This very complete function is compiled differently on each Unix flavour,
  since there is no portable way of determining the amount of free space
  available on a filesystem.
 */
/*--------------------------------------------------------------------------*/
long get_avail_kbytes(char * path);

/*-------------------------------------------------------------------------*/
/**
  @brief    Build a list of available filesystem on the local host. 
  @param    dlist       Output disk list.
  @param    ndisks      Output number of found disks.
  @return   int 0 if Ok, -1 otherwise.

  Build up a list of available filesystems on the local host and
  return it. Both input parameters are actually output parameters.

  The returned disk list must be freed using free().

  Example:

  @code
  disk_info * dlist ;
  int         ndisks ;

  if (get_disk_list(&dlist, &ndisks)!=0) {
      printf("cannot get disk list\n"); 
  } else {
      printf("disk list Ok\n");
  }
  @endcode

  The job is outsourced to the 'df' command.
 */
/*--------------------------------------------------------------------------*/
int get_disk_list(disk_info ** dlist, int * ndisks);

/*-------------------------------------------------------------------------*/
/**
  @brief    Print out a disk list like 'df' would.
  @param    dlist       List of disk info structs.
  @param    ndisks      Number of structs in list.
  @return   void

  Print out a list of disk info structs as 'df' would. This is
  actually overkill, since the list of disks has been obtained using
  'df'... The output list is sorted by decreasing amount of available
  disk space.
 */
/*--------------------------------------------------------------------------*/
void print_disk_list(disk_info * dlist, int ndisks);

/*-------------------------------------------------------------------------*/
/**
  @brief    qsort helper function to sort disk_info structs.
  @param    d1  First disk_info struct.
  @param    d2  Second disk_info struct.
  @return   int 1 if d1->avail < d2->avail, -1 otherwise.

  This function is expected to be passed as an argument to
  @c qsort to help sorting out a list of disk_info structures
  by decreasing available disk space. See print_disk_list for an example.
 */
/*--------------------------------------------------------------------------*/
int sort_disks_by_dec_avail(const void * d1, const void *d2);


#endif
