/*----------------------------------------------------------------------------*/
/**
   @file	pfits.h
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.9 $
   @brief	Protected FITS keyword read/write.

   This module handles protected access to FITS headers, i.e. when
   a request in a FITS header is issued, the requested keyword is
   looked for in a table associated to every supported instrument.
   If a match is found, the keyword will be obtained using a dedicated
   function, otherwise a direct FITS header query will be issued.

   This module also handles write requests to a FITS/PAF header
   related to known pipeline products.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: pfits.h,v 1.9 2004/02/09 16:03:48 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:03:48 $
	$Revision: 1.9 $
*/

#ifndef _PFITS_H_
#define _PFITS_H_

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/
typedef struct instrument_t {
    enum {
        instrument_none     = 0,
        instrument_auto     = 1,
        /* ISAAC */
        instrument_isaac    = 10,
        /* NAOS/CONICA */
        instrument_naco     = 20
    } ins ;

    enum {
        insmode_none        = 0,
        /* Non Chopping mode */
        insmode_nochop      = 10,
        /* Chopping mode */
        insmode_chop        = 20
    } mode ;
} instrument_t ;

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the latest error string from this module.
  @return   1 pointer to a statically allocated string in this module.

  This function returns a pointer to a statically allocated string
  within this module, describing the latest error that occurred.
 */
/*----------------------------------------------------------------------------*/
char * pfits_error(void);

/*----------------------------------------------------------------------------*/
/**
  @brief    Get a FITS value from a FITS or PAF file.
  @param    ins         Instrument that produced the file.
  @param    filename    Name of the file to look into.
  @param    key         Name of the key to look for.
  @param    status      Returned int signalling errors and status.
  @return   1 pointer to a statically allocated string, or NULL.

  This function implemented the "protected FITS" concept, i.e. the
  ability to request a value in a file's FITS header with support for
  key retrieval algorithms or key history.

  The returned string is statically allocated inside this module.
  Do not modify or free it!

  This module features an internal rotating list of static strings
  to return the results of this function, so it is definitely safe
  to call it several times in the same context without having
  results overwrite each other. Example:

  @code
  // Declare instrument object
  instrument_t ins ;

  // Fill instrument object for ISAAC
  ins.ins = instrument_isaac ;
  ins.mode = insmode_none ;

  printf("NAXIS1 = %s\n"
         "NAXIS2 = %s\n",
         pfits_get(ins, "a.fits", "naxis1", NULL),
         pfits_get(ins, "a.fits", "naxis2", NULL));
  @endcode

 */
/*----------------------------------------------------------------------------*/
char * pfits_get(instrument_t ins, char * filename, char * key);

/*----------------------------------------------------------------------------*/
/**
  @brief    Identify instrument data type.
  @param    filename        Name of the file to examine.
  @return   1 instrument_t object

  This function examines a given FITS file and recognizes the instrument
  that was used to acquire it.
 */
/*----------------------------------------------------------------------------*/
instrument_t pfits_identify_insstr(char * name);

/*----------------------------------------------------------------------------*/
/**
  @brief    Identify instrument data type.
  @param    filename        Name of the file to examine.
  @return   1 instrument_t object

  This function examines a given FITS file and recognizes the instrument
  that was used to acquire it.
 */
/*----------------------------------------------------------------------------*/
instrument_t pfits_identify_ins(char * filename);

#endif
