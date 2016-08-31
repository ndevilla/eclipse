
/*---------------------------------------------------------------------------
   
   File name 	:	wfip_lib.h
   Author 		:	N. Devillard
   Created on	:	18 May 2000
   Description	:	WFI library utilities

 *--------------------------------------------------------------------------*/

/*
	$Id: wfip_lib.h,v 1.3 2001/03/23 14:05:21 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/03/23 14:05:21 $
	$Revision: 1.3 $
*/

#ifndef _WFIP_LIB_H_
#define _WFIP_LIB_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "eclipse.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

/* The following parameters are WFI-wide */

/* Prescan/Overscan region */
#define WFI_PRESCAN_X_MIN		(5)
#define WFI_PRESCAN_X_MAX		(48)

#define WFI_OVERSCAN_X_MIN		(2100)
#define WFI_OVERSCAN_X_MAX		(2142)

/* Cropping region */
/* Declared obsolete 22 Nov 2000 */
/*
#define WFI_CROP_X_MIN			(55)
#define WFI_CROP_X_MAX			(2090)
#define WFI_CROP_Y_MIN			(35)
#define WFI_CROP_Y_MAX			(4120)
*/

/* New values installed 22 Nov 2000 */
#define WFI_CROP_X_MIN			(60)
#define WFI_CROP_X_MAX			(2093)
#define WFI_CROP_Y_MIN			(30)
#define WFI_CROP_Y_MAX			(4126)


/* Number of CCD chips on WFI */
#define WFI_NCHIPS				(8)

/* Saturation level for pre-processing */
#define WFI_SATLEVEL			(45000)
/* Maximal acceptable percentage of pixels above saturation level */
#define WFI_SATMAX				(0.05)



/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

int wfi_split(char * name_i, char * name_o, int xtnum);

cube_t * wfi_cube_load(char * filename, int xtnum);


image_t * wfi_load_ext(char * filename, int xtnum);

int wfi_is_extension(char * filename);

image_t * wfi_overscan_correction(
    image_t    *   wfi_frame,
    int         *   prescan_x,
    int         *   overscan_x,
    int         *   rej_int,
    int         *   crop_reg
) ;

int wfi_gradient_check(
    image_t    *   wfi_frame,
    double          max_grad_level
) ;

#endif
