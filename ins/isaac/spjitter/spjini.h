/*----------------------------------------------------------------------------*/
/**
   @file    spjini.h
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.2 $
   @brief   Spectroscopic jitter ini file handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjini.h,v 1.2 2002/12/20 14:56:00 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/20 14:56:00 $
	$Revision: 1.2 $
*/

#ifndef _SPJINI_H_
#define _SPJINI_H_

#include "spjtypes.h"

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a default ini file for the spjitter command.
  @param    ininame     Name of ini file to generate.
  @param    name_i      Name of input file
  @param    name_o      Name of output file
  @param    name_c      Name of calibration file
  @param    algo        Name of the algorithm to use.
  @return   int 0 if Ok, -1 otherwise

  This function generates a default ini file for the spjitter command. The
  generated file will have the requested name. If you do not want to provide 
  names for the input/output/calib files or for the instrument, feed NULL
  pointers or character strings starting with (char)0.
 */
/*----------------------------------------------------------------------------*/
int spjitter_ini_generate(
        char    *   ininame,
        char    *   name_i,
        char    *   name_o,
        char    *   name_c,
        char    *   algo) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the spjitter.ini file and fill up the config.
  @param    ininame     Name of the input ini file
  @param    spjc        spjitter_config_t to fill up.
  @return   int 0 if Ok, anything else if error occurred.
 */
/*----------------------------------------------------------------------------*/
int spjitter_ini_parse(char * ininame, spjitter_config_t * spjc) ;

#endif
