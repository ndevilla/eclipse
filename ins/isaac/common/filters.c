/*---------------------------------------------------------------------------
   
   File name 	:	filters.c
   Author 		:	N. Devillard
   Created on	:	March 2001
   Description	:	ISAAC observation filter handling.

 *--------------------------------------------------------------------------*/

/*
	$Id: filters.c,v 1.4 2002/07/31 14:18:44 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/07/31 14:18:44 $
	$Revision: 1.4 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h"
#include "filters.h"
#include "filters_sta.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate a filter label to a string.
  @param    key		A string found in a FITS keyword to represent a filter.
  @return   A filter label

  This function expects a string as read from e.g. the INS.FILT1.ID
  keyword in a PAF or FITS header, and converts it to a valid filter label.
  The returned label is isaac_filter_invalid (0) if the string has
  no known association.
 */
/*--------------------------------------------------------------------------*/
isaac_filter_id isaac_get_filterid(char * key)
{
    isaac_filter_id    found ;
    int i ;

	if (key==NULL)
		return isaac_filter_invalid ;

    i=0 ;
    found = isaac_filter_invalid ;
    while (isaac_filter_list[i].filtid != isaac_filter_end) {
        if (!strcmp(isaac_filter_list[i].key, key)) {
            found = isaac_filter_list[i].filtid ;
            break ;
        }
        i++ ;
    }
    return found ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate a filter name to a filter ID.
  @param	f_id	Valid ISAAC filter ID.
  @return   Pointer to static char string.

  This function expects a valid ISAAC filter ID and returns the associated
  character string as can be found in a FITS header. The returned string
  is static, do not free or modify it!

  This function returns NULL if no matching ID can be found.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_filtername(isaac_filter_id f_id)
{
	char * name ;
    int    i ;

    i=0 ;
	name = NULL ;
    while (isaac_filter_list[i].filtid != isaac_filter_end) {
        if (isaac_filter_list[i].filtid == f_id) {
            name = isaac_filter_list[i].key ;
            break ;
        }
        i++ ;
    }
    return name ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Associate a broad band filter to a narrow band filter.
  @param	f_id	ID of the filter to associate
  @return	A valid filter ID corresponding to a broadband filter.

  This function performs an association between a narrow band and a
  broad-band filter.
 */
/*--------------------------------------------------------------------------*/
isaac_filter_id isaac_associate_filter(isaac_filter_id f_id)
{
	isaac_filter_id		assoc_id ;

	switch (f_id) {
		case isaac_filter_nb106:
		case isaac_filter_nb108:
		case isaac_filter_nb119:
		case isaac_filter_nb121:
		case isaac_filter_nb126:
		case isaac_filter_nb128:
		assoc_id = isaac_filter_j;
		break ;

		case isaac_filter_nb164:
		case isaac_filter_nb171:
		assoc_id = isaac_filter_h;
		break ;

		case isaac_filter_nb207:
		case isaac_filter_nb209:
		case isaac_filter_nb213:
		case isaac_filter_nb217:
		case isaac_filter_nb219:
		case isaac_filter_nb225:
		case isaac_filter_nb229:
		case isaac_filter_nb234:
		assoc_id = isaac_filter_ks;
		break ;

		case isaac_filter_nb321:
		case isaac_filter_nb328:
		case isaac_filter_nb380:
		case isaac_filter_nb407:
		assoc_id = isaac_filter_l;
		break ;

		default:
		assoc_id = f_id ;
		break ;
	}
	return assoc_id ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    return an associated broad band filter  
  @param    filter    ISAAC filter name as found in the header.
  @return   pointer to statically allocated character string
 
  This function associates a broadband filter to an ISAAC filter, to
  allow e.g. zero point computations with the right star magnitude.

  The current list of associations is implemented:
  \begin{tabular}{ll}
  ISAAC filter  &   Broad band \\
  \\
  NB_1.06       &   J \\
  NB_1.08       &   J \\
  NB_1.19       &   J \\
  NB_1.21       &   J \\
  NB_1.26       &   J \\
  NB_1.28       &   J \\
  Z     &   J \\
  SZ    &   J \\
  Js    &   J \\
  J     &   J \\
  NB_1.64       &   H \\
  NB_1.71       &   H \\
  SH    &   H \\
  H     &   H \\
  SK    &   K \\
  K     &   K \\
  Ks    &   Ks \\
  NB_2.07       &   Ks \\
  NB_2.09       &   Ks \\
  NB_2.13       &   Ks \\
  NB_2.17       &   Ks \\
  NB_2.19       &   Ks \\
  NB_2.25       &   Ks \\
  NB_2.29       &   Ks \\
  NB_2.34       &   Ks \\
  NB_3.21       &   L \\
  NB_3.28       &   L \\
  NB_3.80       &   L \\
  NB_4.07       &   L \\
  SL    &   L \\
  L     &   L \\
  M_NB  &   M \\
  M     &   M
  \end{tabular}
 */
/*--------------------------------------------------------------------------*/
#define streq(s1,s2)    (!strcmp((s1),(s2)))
char * isaac_get_broadband_filter(char * filter)
{
    static char ret_filter[40] ;

    strcpy(ret_filter, filter);
    if (
        /* Narrow band filters in J */
        streq(filter, "NB_1.06") ||
        streq(filter, "NB_1.08") ||
        streq(filter, "NB_1.19") ||
        streq(filter, "NB_1.21") ||
        streq(filter, "NB_1.26") ||
        streq(filter, "NB_1.28") ||
        /* Broad band filters associated to J */
        streq(filter, "Z") ||
        streq(filter, "SZ") ||
        streq(filter, "Js") ||
        streq(filter, "J")
       ) {
        strcpy(ret_filter, "J");
    } else if (
        /* Narrow band filters in H */
        streq(filter, "NB_1.64") ||
        streq(filter, "NB_1.71") ||
        /* Broad band filters associated to H */
        streq(filter, "SH") ||
        streq(filter, "H")
        ) {
        strcpy(ret_filter, "H");
    } else if (
        /* Narrow band filters associated to Ks */
        streq(filter, "NB_2.07") ||
        streq(filter, "NB_2.09") ||
        streq(filter, "NB_2.13") ||
        streq(filter, "NB_2.17") ||
        streq(filter, "NB_2.19") ||
        streq(filter, "NB_2.25") ||
        streq(filter, "NB_2.29") ||
        streq(filter, "NB_2.34") ||
        /* Broad band filters associated to Ks */
        streq(filter, "Ks")
        ) {
        strcpy(ret_filter, "Ks");
    } else if (
        /* No narrow band associated to K, only broadband */
        streq(filter, "SK") ||
        streq(filter, "K")
        ) {
        strcpy(ret_filter, "K");
    } else if (
        /* Narrow band filters associated to L */
        streq(filter, "NB_3.21") ||
        streq(filter, "NB_3.28") ||
        streq(filter, "NB_3.80") ||
        streq(filter, "NB_4.07") ||
        /* Broad band filters associated to L */
        streq(filter, "SL") ||
        streq(filter, "L")
        ) {
        strcpy(ret_filter, "L");
    } else if (
        /* No narrow band associated to M, only broadband */
        streq(filter, "M_NB") ||
        streq(filter, "M")
        ) {
        strcpy(ret_filter, "M");
    } else {
        e_error("unknown ISAAC filter: [%s]", filter);
        return NULL ;
    }
    return &(ret_filter[0]);
}
#undef streq

