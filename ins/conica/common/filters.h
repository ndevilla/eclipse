#ifndef _CN_FILTERS_H_
#define _CN_FILTERS_H_

/*
 * The following enum contains all valid CONICA filter labels 
 */

typedef enum _conica_filterid_ {
	conica_filter_invalid=0, /* Reserve invalid filter label */
		
	conica_filter_j,
	conica_filter_jc,
	conica_filter_h,
	conica_filter_k,
	conica_filter_ks,
	conica_filter_l,
	conica_filter_lprime,
	conica_filter_mprime,
	conica_filter_sj,
	conica_filter_sh,
	conica_filter_sk,
	conica_filter_nb104,
	conica_filter_nb108,
	conica_filter_nb109,
	conica_filter_nb124,
	conica_filter_nb126,
	conica_filter_nb128,
	conica_filter_nb164,
	conica_filter_nb175,
	conica_filter_nb374,
	conica_filter_ib200,
	conica_filter_ib203,
	conica_filter_ib206,
	conica_filter_ib209,
	conica_filter_ib212,
	conica_filter_nb212,
	conica_filter_ib215,
	conica_filter_nb217,
	conica_filter_ib218,
	conica_filter_ib221,
	conica_filter_ib224,
	conica_filter_ib227,
	conica_filter_ib230,
	conica_filter_ib233,
	conica_filter_ib236,
	conica_filter_ib239,
	conica_filter_ib242,
	conica_filter_ib245,
	conica_filter_ib248,
	conica_filter_nb405,

	/* Reserve end label for loops. */
	conica_filter_end

} conica_filter_id ;



/*-------------------------------------------------------------------------*/
/**
  @brief	Associate a filter label to a string.
  @param	key		A key found in a FITS keyword to represent a filter.
  @return	1 filter label.

  This function expects a string as read from e.g. the INS.FILT1.ID
  keyword in a PAF or FITS header, and converts it to a valid filter
  label. The returned label is invalid (conica_filter_invalid) if the
  string has no known association.
 */
/*--------------------------------------------------------------------------*/
conica_filter_id conica_get_filterid(char * key);



/*-------------------------------------------------------------------------*/
/**
  @brief	Associate a filter name to a filter ID.
  @param	f_id	Valid CONICA filter ID.
  @return	Pointer to static char string.

  This function expects a valid CONICA filter ID and returns the associated
  character string as can be found in the FITS header. The returned string
  is static, do not free or modify it!
  This function returns NULL if no matching ID can be found.
 */
/*--------------------------------------------------------------------------*/

char * conica_get_filtername(conica_filter_id f_id);


/*-------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/

int conica_get_filterdef(
	conica_filter_id f_id,
	double * w_cen,
	double * w_wid
);

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump all known CONICA filter data to an opened file pointer.
  @param    fp      (opened) file pointer to dump to.
  @return   void

  This function dumps all known informations about CONICA filters
  to the passed file pointer. It is Ok to give stdout or stderr as
  file pointers.
 */
/*--------------------------------------------------------------------------*/
void conica_filters_dump(FILE * fp);

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate a broad band filter to a narrow band filter.
  @param    f_id    ID of the filter to associate
  @return   A valid filter ID corresponding to a broadband filter.

  This function performs an association between a narrow band and a
  broad-band filter.
 */
/*--------------------------------------------------------------------------*/
conica_filter_id conica_associate_filter(conica_filter_id f_id) ;

#endif
