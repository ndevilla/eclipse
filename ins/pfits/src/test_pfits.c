
/*-------------------------------------------------------------------------*/
/**
   @file	test_pfits.c
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.1 $
   @brief	Protected FITS keyword read.

   This module handles protected access to FITS headers, i.e. when
   a request in a FITS header is issued, the requested keyword is
   looked for in a table associated to every supported instrument.
   If a match is found, the keyword will be obtained using a dedicated
   function, otherwise a direct FITS header query will be issued.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: test_pfits.c,v 1.1 2002/03/14 16:00:05 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/03/14 16:00:05 $
	$Revision: 1.1 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pfits.h"



int main(int argc, char * argv[])
{
	char * filename ;
	char * ins ;
	char * key ;
	char * val ;

	if (argc<2) {
		printf("use: %s <ins> <file> <key>\n", argv[0]);
		return 1 ;
	}
	ins = argv[1];
	filename = argv[2] ;
	key = argv[3] ;

	val = pfits_get(ins, filename, key);
	printf("val=[%s]\n", val);
	return 0 ;
}

