/*----------------------------------------------------------------------------*/
/**
   @file    filters.c
   @author  N. Devillard
   @date    July 2001
   @version	$Revision: 1.10 $
   @brief   CONICA observation filter handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: filters.c,v 1.10 2003/07/14 08:47:08 yjung Exp $
	$Author: yjung $
	$Date: 2003/07/14 08:47:08 $
	$Revision: 1.10 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filters.h"
#include "filters_sta.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Associate a filter label to a string.
  @param    key		A string found in a FITS keyword to represent a filter.
  @return   A filter label

  This function expects a string as read from e.g. the INS.FILT1.ID
  keyword in a PAF or FITS header, and converts it to a valid filter label.
  The returned label is conica_filter_invalid (0) if the string has
  no known association.
 */
/*----------------------------------------------------------------------------*/
conica_filter_id conica_get_filterid(char * key)
{
    conica_filter_id    found ;
    int i ;

	if (key==NULL)
		return conica_filter_invalid ;

    i=0 ;
    found = conica_filter_invalid ;
    while (conica_filter_list[i].filtid != conica_filter_end) {
        if (!strcmp(conica_filter_list[i].key, key)) {
            found = conica_filter_list[i].filtid ;
            break ;
        }
        i++ ;
    }
    return found ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Associate a filter name to a filter ID.
  @param	f_id	Valid CONICA filter ID.
  @return   Pointer to static char string.

  This function expects a valid CONICA filter ID and returns the associated
  character string as can be found in a FITS header. The returned string
  is static, do not free or modify it!

  This function returns NULL if no matching ID can be found.
 */
/*----------------------------------------------------------------------------*/
char * conica_get_filtername(conica_filter_id f_id)
{
	char * name ;
    int    i ;

    i=0 ;
	name = NULL ;
    while (conica_filter_list[i].filtid != conica_filter_end) {
        if (conica_filter_list[i].filtid == f_id) {
            name = conica_filter_list[i].key ;
            break ;
        }
        i++ ;
    }
    return name ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get filter central wavelength and bandwidth
  @param	f_id	Valid CONICA filter ID
  @param	w_cen	Central wavelength (returned)
  @param	w_wid	Bandwidth (returned)
  @return	int 0 if Ok, -1 otherwise.

  This function expects a valid CONICA filter ID and returns
  into the passed doubles the value for central wavelength and
  bandwidth for this filter. If no matching filter can be found,
  both values are set to zero and the function returns a non-zero
  integer.
 */
/*----------------------------------------------------------------------------*/
int conica_get_filterdef(
        conica_filter_id f_id,
        double * w_cen,
        double * w_wid)
{
	int i ;

	if (w_cen==NULL || w_wid==NULL) return -1 ;
	i=0 ; 
	*w_cen = 0 ;
	*w_wid = 0 ;
	while (conica_filter_list[i].filtid!=conica_filter_end) {
		if (conica_filter_list[i].filtid==f_id) {
			*w_cen = conica_filter_list[i].central ;	
			*w_wid = conica_filter_list[i].width ;	
			return 0 ;
		}
		i++ ;
	}
	return -1 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Dump all known CONICA filter data to an opened file pointer.
  @param	fp		(opened) file pointer to dump to.
  @return	void

  This function dumps all known informations about CONICA filters
  to the passed file pointer. It is Ok to give stdout or stderr as
  file pointers.
 */
/*----------------------------------------------------------------------------*/
void conica_filters_dump(FILE * fp)
{
	int i ;

	if (fp==NULL) return ;
	i=0 ;
	printf("\n"
		   "List of valid CONICA filter IDs:\n"
		   "\n");
	printf("Filter ID       central (um) width (um)\n");
	printf("---------------------------------------\n");
	while (conica_filter_list[i].filtid!=conica_filter_end) {
		printf("%-12s    %5.3f        %5.3f\n",
				conica_filter_list[i].key,
				conica_filter_list[i].central,
				conica_filter_list[i].width);
		i++;
	}
	printf("\n");
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Associate a broad band filter to a narrow band filter.
  @param    f_id    ID of the filter to associate
  @return   A valid filter ID corresponding to a broadband filter.

  This function performs an association between a narrow band and a
  broad-band filter.
 */
/*----------------------------------------------------------------------------*/
conica_filter_id conica_associate_filter(conica_filter_id f_id)
{
    conica_filter_id     assoc_id ;

    switch (f_id) {
		case conica_filter_j:
		case conica_filter_jc:
		case conica_filter_h:
		case conica_filter_k:
		case conica_filter_ks:
		case conica_filter_l:
		case conica_filter_lprime:
		case conica_filter_mprime:
		case conica_filter_sj:
		case conica_filter_sh:
		case conica_filter_sk:
            assoc_id = f_id ;
            break ;

		case conica_filter_nb104:
		case conica_filter_nb108:
		case conica_filter_nb109:
		case conica_filter_nb124:
		case conica_filter_nb126:
		case conica_filter_nb128:
            assoc_id = conica_filter_j ;
            break ;

		case conica_filter_nb164:
		case conica_filter_nb175:
            assoc_id = conica_filter_h ;
            break ;

		case conica_filter_nb374:
            assoc_id = conica_filter_lprime ;
            break ;

		case conica_filter_ib200:
		case conica_filter_ib203:
		case conica_filter_ib206:
		case conica_filter_ib209:
		case conica_filter_ib212:
		case conica_filter_nb212:
		case conica_filter_ib215:
		case conica_filter_nb217:
		case conica_filter_ib218:
		case conica_filter_ib221:
		case conica_filter_ib224:
		case conica_filter_ib227:
		case conica_filter_ib230:
		case conica_filter_ib233:
		case conica_filter_ib236:
		case conica_filter_ib239:
		case conica_filter_ib242:
		case conica_filter_ib245:
		case conica_filter_ib248:
            assoc_id = conica_filter_k ;
            break ;

		case conica_filter_nb405:
            assoc_id = conica_filter_mprime ;
            break ;

        default:
            assoc_id = f_id ;
            break ;
    }
    return assoc_id ;
}

