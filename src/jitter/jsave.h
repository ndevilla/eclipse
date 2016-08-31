/*----------------------------------------------------------------------------*/
/**
   @file    jsave.h
   @author
   @date    March 2002
   @version	$Revision: 1.5 $
   @brief   Jitter result save
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jsave.h,v 1.5 2002/04/19 11:27:04 yjung Exp $
	$Author: yjung $
	$Date: 2002/04/19 11:27:04 $
	$Revision: 1.5 $
*/

#ifndef _JSAVE_H_
#define _JSAVE_H_


/*-----------------------------------------------------------------------------
                            Functions prototypes 
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Add PRO keywords in a FITS header
  @param    jc          Jitter config object.
  @param    fh          FITS header to update.      
  @param    cat         pro category id 
  @return   void
 */
/*----------------------------------------------------------------------------*/
void jitter_add_pro_keys(
        jitter_config_t *   jc,
        qfits_header    *   fh,
        procat              cat) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Save the FITS combined image
  @param    jc          Jitter config object.
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int jitter_save(jitter_config_t * jc) ;

#endif

