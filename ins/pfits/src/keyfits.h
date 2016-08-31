/*----------------------------------------------------------------------------*/
/**
   @file	keyfits.h
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.2 $
   @brief	FITS key structure definition

   This common definition should be loaded by all instrument getters
   modules.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: keyfits.h,v 1.2 2003/01/20 15:11:02 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/20 15:11:02 $
	$Revision: 1.2 $
*/

#ifndef _KEYFITS_H_
#define _KEYFITS_H_

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/
typedef struct keyfits {
	char	 *	name ;
	unsigned	hash ;
    char     *  (*get)(char*);
} keyfits ;

#endif
