/*----------------------------------------------------------------------------*/
/**
   @file    jini.h
   @author
   @date    March 2002
   @version	$Revision: 1.6 $
   @brief   Jitter ini file handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jini.h,v 1.6 2002/04/19 09:36:03 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 09:36:03 $
	$Revision: 1.6 $
*/

#ifndef _JINI_H_
#define _JINI_H_

#include "jtypes.h"

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a default ini file for the jitter command.
  @param    ininame     Name of ini file to generate.
  @param    name_i      Name of input file
  @param    name_o      Name of output file
  @param    name_c      Name of calibration file
  @param    inst        Name of the instrument to configure for.
  @return   int 0 if Ok, -1 otherwise

  This function generates a default ini file for the jitter command. The
  generated file will have the requested name. If you do not want to
  provide names for the input/output/calib files or for the instrument,
  feed NULL pointers or character strings starting with (char)0.
 */
/*----------------------------------------------------------------------------*/
int jitter_ini_generate(
        char    *   ininame,
        char    *   name_i,
        char    *   name_o,
        char    *   name_c,
        char    *   inst) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the jitter.ini file and fill up the config.
  @param    ininame     Name of the input ini file
  @param    jc          jitter_config_t to fill up.
  @return   int 0 if Ok, anything else if error occurred.

  This function tries to fill in as much as possible inside a
  jitter_config_t struct. It is verbose and will print out detailed
  messages in cases of errors.
 */
/*----------------------------------------------------------------------------*/
int jitter_ini_parse(char * ininame, jitter_config_t * jc) ;

#endif
