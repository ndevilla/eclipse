/*----------------------------------------------------------------------------*/
/**
   @file    ins_id.h
   @author  N. Devillard
   @date    Mar 2002
   @version	$Revision: 1.7 $
   @brief   Instrument ID declaration
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: insid.h,v 1.7 2004/02/09 16:02:43 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:02:43 $
	$Revision: 1.7 $
*/

#ifndef _INS_ID_H_
#define _INS_ID_H_

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/
typedef struct instrument_t {
    enum {
        instrument_none		= 0,
        instrument_auto     = 1,
        /* ISAAC */
        instrument_isaac	= 10,
        /* NAOS/CONICA */
        instrument_naco 	= 20
    } ins ;

    enum {
        insmode_none        = 0,
        /* Non Chopping mode */
        insmode_nochop      = 10,
        /* Chopping mode */
        insmode_chop        = 20
    } mode ;
} instrument_t ;

#endif
