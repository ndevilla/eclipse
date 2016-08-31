/*----------------------------------------------------------------------------*/
/**
  @file		static_sz.h
  @author	N. Devillard
  @date		Dec 1999
  @version	$Revision: 1.5 $
  @brief	Definitions for various fixed sizes.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: static_sz.h,v 1.5 2003/11/24 09:44:53 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/24 09:44:53 $
	$Revision: 1.5 $
*/

#ifndef STATIC_SZ_H
#define STATIC_SZ_H

#ifdef __cplusplus
extern "C" {
#endif

/* <dox> */
/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

/** Maximum supported file name size */
#define FILENAMESZ				512

/** Maximum number of characters per line in an ASCII file */
#define ASCIILINESZ				1024

/* </dox> */
#ifdef __cplusplus
}
#endif

#endif
/* vim: set ts=4 et sw=4 tw=75 */
