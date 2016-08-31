/*----------------------------------------------------------------------------*/
/**
   @file    flipx.c
   @author  N. Devillard
   @date    July 2001
   @version	$Revision: 1.5 $
   @brief   flip x axis in a FITS image
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: flipx.c,v 1.5 2002/12/04 13:28:25 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/04 13:28:25 $
	$Revision: 1.5 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "qfits.h"

/*-----------------------------------------------------------------------------
							Private functions
 -----------------------------------------------------------------------------*/

/*
 * Swap pixels between position p1 and p2, regardless of the pixel
 * type and endian-ness of the local host.
 */
static void swap_pix(char * buf, int p1, int p2, int psize)
{
	int	 i ;
	char c ;

	for (i=0 ; i<psize ; i++) {
		c = buf[p1+i] ;
		buf[p1+i] = buf[p2+i];
		buf[p2+i] = c ;
	}
}

/*
 * Main processing function. It expects one only file name
 * and will flip pixels on the input frame.
 */
static int fits_flip(char * pname, char * filename)
{
	char		*	sval ;
	int				dstart;
	int				lx, ly ;
	int				bpp ;
	int				i, j ;
	char		*	buf ;
	char		*	fbuf ;
	int				psize;
	struct stat		fileinfo ;
	int				fd ;

	printf("%s: processing %s\n", pname, filename);

	if (stat(filename, &fileinfo)!=0) {
		return -1 ;
	}
	if (fileinfo.st_size<1) {
		printf("cannot stat file\n");
		return -1 ;
	}

	/* Retrieve image attributes */
	if (is_fits_file(filename)!=1) {
		printf("not a FITS file\n");
		return -1 ;
	}

	sval = qfits_query_hdr(filename, "NAXIS1");
	if (sval==NULL) {
		printf("cannot read NAXIS1\n");
		return -1 ;
	}
	lx = atoi(sval);
	sval = qfits_query_hdr(filename, "NAXIS2");
	if (sval==NULL) {
		printf("cannot read NAXIS2\n");
		return -1 ;
	}
	ly = atoi(sval);
	sval = qfits_query_hdr(filename, "BITPIX");
	if (sval==NULL) {
		printf("cannot read BITPIX\n");
		return -1 ;
	}
	bpp = atoi(sval);

	psize = bpp/8 ;
	if (psize<0) psize=-psize ;

	/* Retrieve start of first data section */
	if (qfits_get_hdrinfo(filename, 0, &dstart, NULL)!=0) {
		printf("reading header information\n");
		return -1 ;
	}

	/* Map the input file in read/write mode (input file is modified) */
	if ((fd=open(filename, O_RDWR))==-1) {
		perror("open");
		printf("reading file\n");
		return -1 ;
	}
	fbuf = (char*)mmap(0,
					   fileinfo.st_size,
					   PROT_READ | PROT_WRITE,
					   MAP_SHARED,
					   fd,
					   0);
	if (fbuf==(char*)-1) {
		perror("mmap");
		printf("mapping file\n");
		return -1 ;
	}
	buf = fbuf + dstart ;

	/* Double loop */
	for (j=0 ; j<ly ; j++) {
		for (i=0 ; i<lx/2 ; i++) {
			/* Swap bytes */
			swap_pix(buf, i*psize, (lx-i-1)*psize, psize);
		}
		buf += lx * psize ;
	}
	if (munmap(fbuf, fileinfo.st_size)!=0) {
		printf("unmapping file\n");
		return -1 ;
	}
	return 0 ;
}

/*-----------------------------------------------------------------------------
								Main
 -----------------------------------------------------------------------------*/
int main(int argc, char * argv[])
{
	int i ;
	int err ;

	if (argc<2) {
		printf("use: %s <list of FITS files...>\n", argv[0]);
		return 1 ;
	}
	err=0 ;
	for (i=1 ; i<argc ; i++) {
		err += fits_flip(argv[0], argv[i]) ;
	}
	if (err>0) {
		fprintf(stderr, "%s: %d error(s) occurred\n", argv[0], err);
		return -1 ;
	}
	return 0 ;
}
