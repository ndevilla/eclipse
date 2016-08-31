
/*-------------------------------------------------------------------------*/
/**
   @file	filesys.c
   @author	N. Devillard
   @date	Jul 1998
   @version	$Revision: 1.14 $
   @brief	File system info utilities
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: filesys.c,v 1.14 2002/01/15 10:05:06 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/01/15 10:05:06 $
    $Revision: 1.14 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "filesys.h"

#ifdef _ECLIPSE_
#include "static_sz.h"
#else
#define FILENAMESZ		1024
#define ASCIILINESZ		1024
#endif

/*
 * OS-dependent include files
 */

#if OS_LINUX
#include <sys/vfs.h>
#endif

#if OS_AIX
#include <sys/statfs.h>
#endif

#if OS_HPUX
#include <sys/vfs.h>
#endif

#if OS_SOLARIS
#include <sys/types.h>
#include <sys/statvfs.h>
#endif

#if OS_DEC
#include <sys/mount.h>
#endif

#if OS_UNKNOWN
/* No include file needed for this method */
static long get_avail_kbytes_generic(char * path);
#endif

#if (OS_LINUX | OS_AIX | OS_HPUX | OS_DEC)
static long get_avail_kbytes_bsd(char * path);
#endif

#if OS_SOLARIS
static long get_avail_kbytes_solaris(char * path);
#endif


/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/** Unix df command on all systems but HP-UX */
#define DF_CMD "df -k"
 
/** HPUX df command call */
#if OS_HPUX
#undef DF_CMD
#define DF_CMD "bdf"
#endif


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Find out write permissions on a given path.
  @param	path	Path to examine.
  @return	boolean (int)

  Find out if the current user has write permissions on the provided
  path name. Uses the @c access system call to check this out.
 */
/*--------------------------------------------------------------------------*/
int test_write_permission(char * path)
{
	return !access(path, W_OK);
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Find out how maby kbytes are available on a disk.	
  @param	path	Path to examine.
  @return	Available kbytes, zero if cannot determine.

  This very complete function is compiled differently on each Unix flavour,
  since there is no portable way of determining the amount of free space
  available on a filesystem.
 */
/*--------------------------------------------------------------------------*/

long get_avail_kbytes(char * path)
{
#if (OS_AIX | OS_LINUX | OS_HPUX | OS_DEC | OS_IRIX)
	return get_avail_kbytes_bsd(path);
#endif
#if OS_SOLARIS
	return get_avail_kbytes_solaris(path);
#endif
#if OS_UNKNOWN
	return get_avail_kbytes_generic(path);
#endif
	return -1 ;
}


#if (OS_AIX | OS_LINUX | OS_HPUX | OS_DEC | OS_IRIX)
static long get_avail_kbytes_bsd(char * path)
{
	struct statfs	fs ;
	long	size ;

	statfs(path, &fs);
	size = fs.f_bavail * (fs.f_bsize / 1024) ;
	return size ;
}
#endif


#if OS_SOLARIS
static long get_avail_kbytes_solaris(char * path)
{
	struct statvfs	fs ;
	long	size ;

	statvfs(path, &fs);
	size = (long)fs.f_bavail * ((long)fs.f_bsize / 1024) ;
	return size ;
}
#endif


#if OS_UNKNOWN
static long get_avail_kbytes_generic(char * path)
{
    FILE            *   df_prog ;
    char                line[ASCIILINESZ] ;
    char                line2[ASCIILINESZ] ;
    char                cmd[ASCIILINESZ] ;
    char                filesystem[FILENAMESZ] ;
    unsigned long       kbytes ;
    unsigned long       used ;
    unsigned long       avail ;
    int                 capacity ;
    char                df_path[ASCIILINESZ] ;
    int                 nval ;

    sprintf(cmd, "%s %s", DF_CMD, path) ;
    df_prog = popen(cmd, "r") ;
    if (df_prog == NULL) {
        fprintf(stderr, "errors launching %s: aborting\n", cmd) ;
        return 0 ;
    }

    fgets(line, ASCIILINESZ, df_prog) ;
    fgets(line, ASCIILINESZ, df_prog) ;
    fgets(line2, ASCIILINESZ, df_prog) ;
    if (pclose(df_prog)==-1) perror("pclose");

    if ((int)strlen(line)<1) {
        fprintf(stderr,"reading output from %s: aborting", cmd) ;
        return 0 ;
    }
    nval = sscanf(  line, "%s %ld %ld %ld %d%% %s",
                    filesystem,
                    &kbytes,
                    &used,
                    &avail,
                    &capacity,
                    df_path) ;
    if (nval!=6) {
        nval = sscanf(  line2, "%ld %ld %ld %d%% %s",
                        &kbytes,
                        &used,
                        &avail,
                        &capacity,
                        df_path) ;
        if (nval!=5) avail=0;
    }
    return avail;
}
#endif



/*-------------------------------------------------------------------------*/
/**
  @brief	Build a list of available filesystem on the local host.	
  @param	dlist		Output disk list.
  @param	ndisks		Output number of found disks.
  @return	int 0 if Ok, -1 otherwise.

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


int get_disk_list(disk_info ** dlist, int * ndisks)
{
    FILE * df_prog ;
    char   line[ASCIILINESZ] ;
    int    i ;
    char   filesystem[FILENAMESZ] ;
    long   kbytes ;
    long   used ;
    long   avail ;
    int    capacity ;
    char   path[FILENAMESZ] ;

    /*
     * Count how many file systems are visible
     */

    df_prog = popen(DF_CMD, "r") ;
    fgets(line, ASCIILINESZ, df_prog) ;
    i=0 ;
    while (fgets(line, ASCIILINESZ, df_prog)!=NULL) i++ ;
    pclose(df_prog) ;

    *ndisks = i ;
    *dlist = malloc(i*sizeof(disk_info)) ;

    df_prog = popen(DF_CMD, "r") ;
    fgets(line, ASCIILINESZ, df_prog) ;
    for (i=0 ; i<(*ndisks) ; i++) {
        fgets(line, ASCIILINESZ, df_prog) ;
        sscanf(line, "%s %ld %ld %ld %d%% %s",
                filesystem,
                &kbytes,
                &used,
                &avail,
                &capacity,
                path) ;
        strcpy((*dlist)[i].filesystem, filesystem) ;
        (*dlist)[i].kbytes = kbytes ;
        (*dlist)[i].used = used ;
        (*dlist)[i].avail = avail ;
        (*dlist)[i].capacity = capacity ;
        strcpy((*dlist)[i].path, path) ;
    }
    pclose(df_prog) ;
    return 0 ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Print out a disk list like 'df' would.
  @param	dlist		List of disk info structs.
  @param	ndisks		Number of structs in list.
  @return	void

  Print out a list of disk info structs as 'df' would. This is
  actually overkill, since the list of disks has been obtained using
  'df'... The output list is sorted by decreasing amount of available
  disk space.
 */
/*--------------------------------------------------------------------------*/
void print_disk_list(disk_info * dlist, int ndisks)
{
    int     i ;

    qsort(dlist, ndisks, sizeof(disk_info),
            sort_disks_by_dec_avail) ;
    printf("found %d filesystems\n", ndisks) ;
    printf("\n") ;
    printf("\n") ;

    printf("name\tsize\tused\tavail\tcapacity\tmount point\n") ;
    printf("\n") ;
    for (i=0 ; i<ndisks ; i++) {
        printf("%s\t%ld\t%ld\t%ld\t%d%%\t%s\n",
                dlist[i].filesystem,
                dlist[i].kbytes,
                dlist[i].used,
                dlist[i].avail,
                dlist[i].capacity,
                dlist[i].path) ;
    }
}


/*-------------------------------------------------------------------------*/
/**
  @brief	qsort helper function to sort disk_info structs.
  @param	d1	First disk_info struct.
  @param	d2	Second disk_info struct.
  @return	int 1 if d1->avail < d2->avail, -1 otherwise.

  This function is expected to be passed as an argument to
  @c qsort to help sorting out a list of disk_info structures
  by decreasing available disk space. See print_disk_list for an example.
 */
/*--------------------------------------------------------------------------*/

int sort_disks_by_dec_avail(const void * d1, const void *d2)
{
    if (((disk_info*)d1)->avail < ((disk_info*)d2)->avail) return 1 ;
    else return -1 ;
}


/* vim: set ts=4 et sw=4 tw=75 */
