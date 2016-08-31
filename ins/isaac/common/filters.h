
#ifndef _FILTERS_H_
#define _FILTERS_H_
/*
 * The following enum contains all valid ISAAC filter labels
 */

typedef enum _ISAAC_FILTER_ID_ {
isaac_filter_invalid = 0,    /* Reserve an invalid product label */

/* Broad-band filters */
isaac_filter_z,
isaac_filter_sz,
isaac_filter_js,
isaac_filter_j,
isaac_filter_jblock,
isaac_filter_sh,
isaac_filter_h,
isaac_filter_ks,
isaac_filter_sk,
isaac_filter_k,
isaac_filter_sl,
isaac_filter_l,
isaac_filter_mnb,
isaac_filter_m,

/* Narrow-band filters */
isaac_filter_nb106,
isaac_filter_nb108,
isaac_filter_nb119,
isaac_filter_nb121,
isaac_filter_nb126,
isaac_filter_nb128,
isaac_filter_nb164,
isaac_filter_nb171,
isaac_filter_nb207,
isaac_filter_nb209,
isaac_filter_nb213,
isaac_filter_nb217,
isaac_filter_nb219,
isaac_filter_nb225,
isaac_filter_nb229,
isaac_filter_nb234,
isaac_filter_nb321,
isaac_filter_nb328,
isaac_filter_nb380,
isaac_filter_nb407,

isaac_filter_end             /* Reserve for end of loops */
} isaac_filter_id ;

/* Declare search functions */

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate a filter label to a string.
  @param    key     A string found in a FITS keyword to represent a filter.
  @return   A filter label

  This function expects a string as read from e.g. the INS.FILT1.ID
  keyword in a PAF or FITS header, and converts it to a valid filter label.
  The returned label is isaac_filter_invalid (0) if the string has
  no known association.
 */
/*--------------------------------------------------------------------------*/
isaac_filter_id isaac_get_filterid(char * key);

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate a filter name to a filter ID.
  @param    f_id    Valid ISAAC filter ID.
  @return   Pointer to static char string.

  This function expects a valid ISAAC filter ID and returns the associated
  character string as can be found in a FITS header. The returned string
  is static, do not free or modify it!

  This function returns NULL if no matching ID can be found.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_filtername(isaac_filter_id f_id);

/*-------------------------------------------------------------------------*/
/**
  @brief    Associate a broad band filter to a narrow band filter.
  @param    f_id    ID of the filter to associate
  @return   A valid filter ID corresponding to a broadband filter.

  This function performs an association between a narrow band and a
  broad-band filter.
 */
/*--------------------------------------------------------------------------*/
isaac_filter_id isaac_associate_filter(isaac_filter_id f_id);

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
char * isaac_get_broadband_filter(char * filter) ;

#endif
