/*----------------------------------------------------------------------------*/
/**
   @file	do_catg.c
   @author	Y. Jung
   @date	Apr 2002
   @version	$Revision: 1.5 $
   @brief	DO_CATG keywords
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: do_catg.c,v 1.5 2004/02/09 16:02:43 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:02:43 $
	$Revision: 1.5 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "insid.h"
#include "do_catg.h"
#include "do_catg_isaac.h"
#include "do_catg_naco.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Get docat associated to a string.
  @param    ins     Instrument ID
  @param    value   Product category value as found in DO.CATG (2nd column)
  @return   A category label

  This function expects a string as read from e.g. the second column of a 
  reduction block, and converts it to a valid category label.
  The returned label is docat_invalid (0) if the string has no known 
  association.
 */
/*----------------------------------------------------------------------------*/
docat pfits_getdocat_label(
        instrument_t        ins, 
        char            *   value) 
{
    docat_list_t    *   docatg_list ;
    docat               found ;
    int                 i ;
    
    /* Get the list for the requested instrument */
    switch (ins.ins) {
        case instrument_isaac: docatg_list = docat_list_isaac ; break ;
        case instrument_naco:  docatg_list = docat_list_naco ;  break ;
        default: return docat_invalid ;
    }

    /* Initialize */
    i = 0 ;
    found = docat_invalid ;

    /* Search the label by comparing the string values */
    while (docatg_list[i].label != docat_end) {
        if (!strcasecmp(docatg_list[i].value, value)) {
            found = docatg_list[i].label ;
            break ;
        }
        i++ ;
    }
    return found ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get string associated to a label.
  @param    ins     Instrument ID
  @param    label   DO CATG label
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string.
  The returned string is statically allocated so do not free it or modify it.
 */
/*----------------------------------------------------------------------------*/
char * pfits_getdocat_value(
        instrument_t    ins, 
        docat           label) 
{
    docat_list_t    *   docatg_list ;
    char            *   value ;
    int                 i ;

    /* Get the list for the requested instrument */
    switch (ins.ins) {
        case instrument_isaac: docatg_list = docat_list_isaac ; break ;
        case instrument_naco:  docatg_list = docat_list_naco ;  break ;
        default: return NULL ;
    }

    /* Initialize */
    i = 0 ;
    value = NULL ;

    /* Search the value by comparing the labels */
    while (docatg_list[i].label != docat_end) {
        if (docatg_list[i].label == label) {
            value = docatg_list[i].value ;
            break ;
        }
        i++ ;
    }
    return value ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get description string associated to a label 
  @param    ins     Instrument ID
  @param    label   CO CATG label 
  @return   1 pointer to statically allocated string.

  This function expects a valid category label, and returns an
  associated character string that describes the associated product.
  The returned string is statically allocated so do not free it or modify it.
 */
/*----------------------------------------------------------------------------*/
char * pfits_getdocat_descr(
        instrument_t    ins, 
        docat           label) 
{
    docat_list_t    *   docatg_list ;
    char            *   descr ;
    int                 i ;

    /* Get the list for the requested instrument */
    switch (ins.ins) {
        case instrument_isaac: docatg_list = docat_list_isaac ; break ;
        case instrument_naco:  docatg_list = docat_list_naco ;  break ;
        default: return NULL ;
    }

    /* Initialize */
    i = 0 ;
    descr = NULL ;

    /* Search the descr by comparing the labels */
    while (docatg_list[i].label != docat_end) {
        if (docatg_list[i].label == label) {
            descr = docatg_list[i].descr ;
            break ;
        }
        i++ ;
    }
    return descr ;
}

