/*----------------------------------------------------------------------------*/
/**
   @file    dfits.c
   @author  Nicolas Devillard
   @date    30 Mar 2000
   @version	$Revision: 1.3 $
   @brief   FITS header display
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: dfits.c,v 1.3 2005/07/19 15:38:52 yjung Exp $
	$Author: yjung $
	$Date: 2005/07/19 15:38:52 $
	$Revision: 1.3 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qfits.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define BLOCK_SIZE  2880
#define LGTH    	80
#define MAGIC   	"SIMPLE  ="

/* If compiled against zlib, include support for gzipped files */
#if HAVE_ZLIB
#include "zlib.h"
#define FILE            gzFile
#define fopen           gzopen
#define fclose          gzclose
#define fread(b,s,n,f)  gzread(f,b,n*s)
#endif

/*-----------------------------------------------------------------------------
                                Function prototypes
 -----------------------------------------------------------------------------*/

void usage(char * pname) ;
void parse_cmd_line(int, char **, int *, int *, int *) ;
int dump_fits_filter(FILE *, int) ;
int dump_fits(char *, int) ;
char * rstrip(char *) ;

/*-----------------------------------------------------------------------------
                                Main
 -----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	int		xtnum ;
	int		c_arg ;
	int		filter ;
	int		err ;

	/* No arguments prints out a usage message */
	if (argc<2) usage(argv[0]);

	/* Parse command-line options */
	parse_cmd_line(argc, argv, &xtnum, &filter, &c_arg);

	/* Filter mode: process data received from stdin */
	if (filter) {
#if HAVE_ZLIB
        printf("filter mode does not support gzipped files\n");
        printf("use: gunzip -c file.fits | dfits -\n");
        return 1 ;
#else
		return dump_fits_filter(stdin, xtnum);
#endif
    }

	/* Normal mode: loop on all file names given on command-line */
	err = 0 ;
	while (c_arg < argc) {
		err += dump_fits(argv[c_arg], xtnum);
		c_arg++;
	}
	return err ; /* Returns number of errors during process */
}

void usage(char * pname)
{
	printf(
"\n\n"
"usage: %s [-x xtnum] <list of FITS files>\n"
"usage: %s [-x xtnum] -\n"
"\n"
"The former version expects file names.\n"
"The latter expects data coming in from stdin.\n"
"\n"
"-x xtnum specifies the extension header to print\n"
"-x 0     specifies main header + all extensions\n"
"\n\n",
	pname, pname);

#if HAVE_ZLIB
    printf(
"This program was compiled against zlib %s\n"
"This means you can use it with gzipped FITS files\n"
"as with uncompressed FITS files.\n"
"NB: this does not apply to the '-' option (input from stdin)\n"
"\n\n", ZLIB_VERSION);
#endif
    exit(1) ;
}

void parse_cmd_line(
        int         argc,
        char    **  argv,
        int     *	xtnum,
        int     *	filter,
        int     *	c_arg)
{
	*filter = 0 ;
	*xtnum  = -1 ;
	*c_arg  = argc-1 ;

	/* If '-' is on the command-line, it must be the last argument */
	if (!strcmp(argv[argc-1], "-")) *filter = 1 ;
	/* If -x xtnum is on the command-line, it must be the first two arguments */
	if (!strcmp(argv[1], "-x")) {
		*xtnum = atoi(argv[2]);
		*c_arg = 3 ;
	} else {
		*c_arg = 1 ;
	}
	return ;
}

/* Strip off all blank characters in a string from the right-side. */
char * rstrip(char * s)
{
    int len ;
    if (s==NULL) return s ;
    len = strlen(s);
    if (len<1) return s ;
    len -- ;
    while (s[len]== ' ') {
        s[len]=(char)0 ;
        len --;
        if (len<0) break ;
    }
    return s ;
}

/* Dump the requested header (main or extension) from a filename. */
int dump_fits(char * name, int xtnum)
{
	FILE	*	in ;
	int			err ;

	if ((in=fopen(name, "r"))==NULL) {
		fprintf(stderr, "error: cannot open file [%s]\n", name);
		return 1 ;
	}

	printf("====> file %s (main) <====\n", name) ;
	err = dump_fits_filter(in, xtnum);
	fclose(in);
	return err ;
}

/* Dump the requested header (main or extension) from a FILE * */
int dump_fits_filter(FILE * in, int xtnum)
{
	int		    n_xt ;
	char	    buf[LGTH+1];
	int		    err ;
    int         data_bytes, naxis ;
    char    *   read_val ;
    int         skip_blocks ;
    int         seeked ;

	/* Try getting the first 80 chars */
	memset(buf, 0, LGTH+1);
	if (fread(buf, sizeof(char), LGTH, in)!=LGTH) {
		fprintf(stderr, "error reading input\n");
		return 1;
	}
	/* Check that it is indeed FITS */
	if (strncmp(buf, MAGIC, strlen(MAGIC))) {
		fprintf(stderr, "not a FITS file\n");
		return 1 ;
	}
    naxis = 0 ;
    data_bytes = 1 ;
	if (xtnum<1) {
		/* Output main header */
		printf("%s\n", rstrip(buf));
        data_bytes = 1 ;
        naxis = 0 ;
		while ((err=fread(buf, sizeof(char), LGTH, in))==LGTH) {
			printf("%s\n", rstrip(buf));
            /* Look for BITPIX keyword */
            if (buf[0]=='B' &&
                buf[1]=='I' &&
                buf[2]=='T' &&
                buf[3]=='P' &&
                buf[4]=='I' &&
                buf[5]=='X' &&
                buf[6]==' ') {
                read_val = qfits_getvalue(buf);
                data_bytes *= (int)atoi(read_val) / 8 ;
                if (data_bytes<0) data_bytes *= -1 ;
            } else
            /* Look for NAXIS keyword */
            if (buf[0]=='N' &&
                buf[1]=='A' &&
                buf[2]=='X' &&
                buf[3]=='I' &&
                buf[4]=='S') {

                if (buf[5]==' ') {
                    /* NAXIS keyword */
                    read_val = qfits_getvalue(buf);
                    naxis = (int)atoi(read_val);
                } else {
                    /* NAXIS?? keyword (axis size) */
                    read_val = qfits_getvalue(buf);
                    data_bytes *= (int)atoi(read_val);
                }
            } else
            /* Look for END keyword */
            if (buf[0]=='E' &&
				buf[1]=='N' &&
				buf[2]=='D') {
				break ;
			}
		}
		if (err!=LGTH) return 1 ;
	}
	if (xtnum<0) return 0 ;

	n_xt=0 ;
	while (1) {
        /*
         * Skip the previous data section if pixels were declared
         */
        if (naxis>0) {
            /* Skip as many blocks as there are declared pixels */
            skip_blocks = data_bytes/BLOCK_SIZE ;
            if ((data_bytes % BLOCK_SIZE)!=0) skip_blocks ++ ;
            seeked = fseek(in, skip_blocks*BLOCK_SIZE, SEEK_CUR);
            if (seeked<0) return -1 ;
        }

		/* Look for next XTENSION keyword */
		while ((err=fread(buf, sizeof(char), LGTH, in))==LGTH) {
			if (buf[0]=='X' &&
				buf[1]=='T' &&
				buf[2]=='E' &&
				buf[3]=='N' &&
				buf[4]=='S' &&
				buf[5]=='I' &&
				buf[6]=='O' &&
				buf[7]=='N') break ;
		}
		if (err==0)	break ;
		if (err!=LGTH) return 1 ;

		n_xt++ ;
        
        if (xtnum==0 || xtnum==n_xt) {
			printf("===> xtension %d\n", n_xt) ;
			printf("%s\n", rstrip(buf));
		}

        data_bytes = 1 ;
        naxis = 0 ;
        while ((err=fread(buf, sizeof(char), LGTH, in))==LGTH) {
            if (xtnum==0 || xtnum==n_xt) printf("%s\n", rstrip(buf));

            /* Look for BITPIX keyword */
            if (buf[0]=='B' &&
                buf[1]=='I' &&
                buf[2]=='T' &&
                buf[3]=='P' &&
                buf[4]=='I' &&
                buf[5]=='X' &&
                buf[6]==' ') {
                read_val = qfits_getvalue(buf);
                data_bytes *= (int)atoi(read_val) / 8 ;
                if (data_bytes<0) data_bytes *= -1 ;
            } else
            /* Look for NAXIS keyword */
            if (buf[0]=='N' &&
                buf[1]=='A' &&
                buf[2]=='X' &&
                buf[3]=='I' &&
                buf[4]=='S') {

                if (buf[5]==' ') {
                    /* NAXIS keyword */
                    read_val = qfits_getvalue(buf);
                    naxis = (int)atoi(read_val);
                } else {
                    /* NAXIS?? keyword (axis size) */
                    read_val = qfits_getvalue(buf);
                    data_bytes *= (int)atoi(read_val);
                }
            } else 
            /* Look for END keyword */
            if (buf[0]=='E' &&
                buf[1]=='N' &&
                buf[2]=='D') break ;
        }
        if (n_xt==xtnum) break ;
	}
	return 0 ;
}
