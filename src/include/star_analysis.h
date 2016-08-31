/*----------------------------------------------------------------------------*/
/**
   @file    star_analysis.h
   @author  Y.Jung
   @date    Mar 2003
   @version $Revision: 1.2 $
   @brief   Copied from RTD (pick object)
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: star_analysis.h,v 1.2 2003/03/27 07:24:00 yjung Exp $
    $Author: yjung $
    $Date: 2003/03/27 07:24:00 $
    $Revision: 1.2 $
*/


#ifndef _STAR_ANALYSIS_H_
#define _STAR_ANALYSIS_H_

/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

int iqe(
        float   *   pfm,
        int         mx,
        int         my,
        float   *   parm,
        float   *   sdev) ;



#endif
