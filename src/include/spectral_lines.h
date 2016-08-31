/*-------------------------------------------------------------------------*/
/**
   @file    spectral_lines.h
   @author  N. Devillard
   @date    November 1999
   @version $Revision: 1.27 $
   @brief   spectrum line handling routines
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: spectral_lines.h,v 1.27 2003/11/20 14:12:44 llundin Exp $
    $Author: llundin $
    $Date: 2003/11/20 14:12:44 $
    $Revision: 1.27 $
*/

#ifndef _SPECTRAL_TABLE_H_
#define _SPECTRAL_TABLE_H_

/*---------------------------------------------------------------------------
   								Includes	
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "xmemory.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define SLITWIDTH_TO_SIGMA 0.25

/* Evaluate 3rd degree (wavelength calibration) polynomial at
   ipix using Horners method
   Caveat: Both arguments are evaluated more than once here */
#define WAVELEN(poly,ipix) (poly[0] + (ipix) * \
                           (poly[1] + (ipix) * \
                           (poly[2] + (ipix) * \
                            poly[3])))

/* Evaluate differentiated 3rd degree (wavelength calibration) polynomial at
   ipix also using Horners method
   Caveat: Both arguments are evaluated more than once here */
#define WAVEDIF(poly,ipix) (poly[1]   + (ipix) * \
                           (poly[2]*2 + (ipix) * \
                            poly[3]*3))

/* Evaluate WAVELEN(poly, ipix + 0.5) - WAVELEN(poly, ipix - 0.5) in terms
   of WAVEDIF(). This is the width of pixel ipix in wavelengths
   Caveat: Both arguments are evaluated more than once here */
#define WAVEDLT(poly,ipix) (0.25*poly[3] + WAVEDIF(poly, ipix))

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	an emission line type

  this structure contain the position (wavel), the relative intensity 
  (intens) ant the type (type) of the emission line. type can be "oh", "xe",
  "ar", it always has to be 2 letters.
 */
/*-------------------------------------------------------------------------*/
typedef struct _EMISSION_LINE_ {
	double		wavel ;
	double		intens ;
	char		type[2] ;
} emission_line ;


/*-------------------------------------------------------------------------*/
/**
  @brief	a spectral table type
  
  The following struct stores in the simplest way a list of spectral
  lines.  nlines is the number of lines in the table wave contains
  their wavelength in Angstroems irel contains their relative
  intensity (no units)
 */
/*-------------------------------------------------------------------------*/
typedef struct _SPECTRAL_TABLE_ {
    int         		nlines ;
    emission_line	*	lines ;  
} spectral_table ;


/*---------------------------------------------------------------------------
						Function ANSI prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Initialize a spectral line table.
  @param    path    Name of the table to initialize.
  @return   1 newly allocated spectral_table object.

  The received path string indicates where the spectral table should be
  loaded from.

  \begin{tabular}{ll}
  "oh"          &   OH table \\
  "Xe"          &   Xenon table \\
  "Ar"          &   Argon table \\
  "Xe+Ar"       &   Xenon+Argon table \\
  /path/file    &   User-specified external table
  \end{tabular}

  If the given table name corresponds to a user-specified table, it is 
  loaded from the disk and dynamically allocated. 
  See spectral_table_parse_list() for the acceptable
  The returned table must be deallocated using spectral_table_destroy()
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_init(char * path) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Create a spectral table
  @param    size    Size of the created table
  @return   the allocated spectral table
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_create(int size) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Deallocate a spectral table.
  @param    spt Spectral table to deallocate.
  @return   void
 */
/*--------------------------------------------------------------------------*/
void spectral_table_destroy(spectral_table * spt) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Sort a spectral table 
  @param    table   spectral table to sort (modified) 
  @return   0 if Ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int spectral_table_sort(spectral_table * table) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a spectral table
  @param    table   spectral table to dump
  @param    out     Opened file pointer
  @return   0 if Ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int spectral_table_dump(
        spectral_table  *   table,
        FILE            *   out) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Merge two spectral tables
  @param    spt1    First spectral table
  @param    spt2    Second spectral table
  @return   a spectral table composed with the two input ones
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_merge(
        spectral_table  *   spt1,
        spectral_table  *   spt2) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Select lines in a spectral table
  @param    ref     reference spectral table
  @param    type    type of lines selected
  @return   a spectral table created with selected lines
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_select(
        spectral_table	*	ref,
        char            *   type) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Read a spectral table from an external file.
  @param    path    Full path name of the file to load.
  @return   1 pointer to newly allocated spectral table.

  This function loads an external spectral table into a spectral_table
  object. The file must be an ASCII file, following these properties:

  \begin{itemize}
  \item Lines starting with a '#' are comments and ignored.
  \item Blank lines are ignored.
  \item Spectral lines are given as two values per line, separated by any
  number of blanks or tabs. The first value gives the wavelength in
  Angstroems, the second gives the relative intensity, which must be
  consistent with all lines in the same table.
  \end{itemize}

  The returned object must be deallocated using spectral_table_destroy().
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_parse_list(char * path) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Build a 1d signal from a spectral table 
  @param    spt         Spectral table to use.
  @param    disprel     4 coeffs of the wavelength calibration polynomial
  @param    order       Order used in the spectral table look-up
  @param    slit_width  Width in pixels of the slit used
  @param    size        Size of the signal to generate.
  @param    found       Number of non-zero samples in signal
  @return   1 pointer to a newly allocated array of 'size' doubles.

  Provide an allocated spectral table and a 3rd deegree wavelength calibration
  polynomial, and the size in pixels of the signal to generate.
  The returned array is a 1d signal (of doubles) containing the spectral
  lines as pixels.

  The spectral lines are smoothed (with a gaussian). Spectral lines just outside
  the range of the wavelength calibration polynomial, i.e. lines just below
  wl_low = WAVELEN(1-0.5) and just above wl_high = WAVELEN(size+0.5) are thus
  also used to generate the spectrum.

  The intensities of the spectrum are transformed with log(1+I).

  Returns NULL in case of error (i.e. no emission lines found,
  or non-positive slit width).
 */
/*--------------------------------------------------------------------------*/
double * spectral_table_build_signal(
        const spectral_table  *   spt,
        const double          *   disprel,
        const int                 order,
        const double              slit_width,
        const int                 size,
        int                   *   found) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Output a list of table lines as a signal to a file.
  @param    table_name    Name of the table to output.
  @param    outfilename   Name of the output file.
  @param    disprel       Coeffs of the wavelength calibration polynomial
  @param    order         Order
  @param    slit_width    Width of the slit used
  @param    size          Number of samples to produce.
  @return   void

  This function builds a 1d signal based on the requested table name and
  wavelength range, and outputs it to an ASCII file. This is useful to dump
  out lists of lines for e.g. debugging purposes, to compare a calibrated
  signal with a list of reference lines.

  Provide NULL or the character string "STDOUT" to output the list to
  stdout. Any other name specifies a file. If another file by the same name
  already exists, it will be overwritten.
 */
/*--------------------------------------------------------------------------*/
void spectral_table_build_spectrum(
        const char    *   table_name,
        const char    *   outfilename,
        const double  *   disprel,
        const int         order,
        const double      slit_width,
        const int         size) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Count the positive-intensity lines in a given wavelength range
  @param    spt         Spectral table
  @param    wave_min    minimum wavelength
  @param    wave_max    maximum wavelength
  @param    order       order
  @return   the number of lines, or -1 on error (invalid spectral table)
 */
/*--------------------------------------------------------------------------*/
int spectral_table_count_lines(
        spectral_table  *   spt,
        double              wave_min,
        double              wave_max,
        int                 order) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Count all lines in a given wavelength range
  @param    spt         Spectral table
  @param    wave_min    minimum wavelength
  @param    wave_max    maximum wavelength
  @param    order       order
  @return   the number of lines, or -1 on error (invalid spectral table)
 */
/*--------------------------------------------------------------------------*/
int spectral_table_count_linez(
        spectral_table  *   spt,
        double              wave_min,
        double              wave_max,
        int                 order) ;

#endif
