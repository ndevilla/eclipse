/*----------------------------------------------------------------------------*/
/**
   @file	products.c
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.7 $
   @brief	Product key/name handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: products.c,v 1.7 2004/02/09 16:02:43 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:02:43 $
	$Revision: 1.7 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "insid.h"
#include "products.h"
#include "products_isaac.h"
#include "products_naco.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Get procat associated to a string.
  @param	ins		Instrument ID
  @param	key		Product category key as found in PRO.CATG
  @return	A category label.

  This function expects a string as read from e.g. the PRO.CATG keyword in
  a PAF or FITS header, and converts it to a valid category label.
  The returned label is procat_invalid (0) if the string has
  no known association.
 */
/*----------------------------------------------------------------------------*/
procat pfits_getprocat(instrument_t ins, char * key)
{
	prodlist_t	*	prodlist ;
    procat 			found ;
    int 			i ;

    switch (ins.ins) {
        case instrument_isaac:
        prodlist = prodlist_isaac ;
        break ;

        case instrument_naco:
        prodlist = prodlist_naco ;
        break ;

        default:
        return procat_invalid;
    }

    i=0 ;
    found = procat_invalid ;
    while (prodlist[i].cat != procat_end) {
        if (!strcasecmp(prodlist[i].key, key)) {
            found = prodlist[i].cat ;
            break ;
        }
        i++ ;
    }
    return found ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get string associated to a procat.
  @param    ins     Instrument ID
  @param    cat     Pro category (label)
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string to be placed e.g. in the PRO.CATG
  keyword of an output PAF or FITS file. The returned string
  is statically allocated so do not free it or modify it.
 */
/*----------------------------------------------------------------------------*/
char * pfits_getprokey(instrument_t ins, procat cat)
{
    prodlist_t  *   prodlist ;
    int             i ;
    char        *   key ;

    switch (ins.ins) {
        case instrument_isaac:
        prodlist = prodlist_isaac ;
        break ;

        case instrument_naco:
        prodlist = prodlist_naco ;
        break ;

        default:
        return NULL ;
    }

    i=0 ;
    key = NULL ;
    while (prodlist[i].cat != procat_end) {
        if (prodlist[i].cat == cat) {
            key = prodlist[i].key ;
            break ;
        }
        i++ ;
    }
    return key ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get description string associated to a procat.
  @param    ins     Instrument ID
  @param    cat     Pro category (label)
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string that describes the associated product.
  The returned string is statically allocated so do not free it or modify
  it.
 */
/*----------------------------------------------------------------------------*/
char * pfits_getprodesc(instrument_t ins, procat cat)
{
    prodlist_t  *   prodlist ;
    int             i ;
    char        *   desc ;

    switch (ins.ins) {
        case instrument_isaac:
        prodlist = prodlist_isaac ;
        break ;

        case instrument_naco:
        prodlist = prodlist_naco ;
        break ;

        default:
        return NULL ;
    }

    i=0 ;
    desc = NULL ;
    while (prodlist[i].cat != procat_end) {
        if (prodlist[i].cat == cat) {
            desc = prodlist[i].desc ;
            break ;
        }
        i++ ;
    }
    return desc ;
}

