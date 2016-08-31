/*-------------------------------------------------------------------------*/
/**
   @file    irstd.h
   @author  N. Devillard
   @date    21 Jan 1999
   @version $Revision: 1.17 $
   @brief   Infrared Standard Star list handling

   This module contains a default list of infrared standard stars. Since 
   this belongs to code, the list might not be the most up-to-date, and it 
   is recommended to provide data files rather than relying on it.
   Nevertheless, it allows processing to go faster and a default process in 
   most cases, to have such a list hardcoded here.
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: irstd.h,v 1.17 2004/02/18 14:54:06 yjung Exp $
    $Author: yjung $
    $Date: 2004/02/18 14:54:06 $
    $Revision: 1.17 $
*/

#ifndef _IRSTD_H_
#define _IRSTD_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/*---------------------------------------------------------------------------
   								New types
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	ir_std object
 */
/*-------------------------------------------------------------------------*/
typedef struct _IR_STD_ {
	int				select ;
    const char    * name ;
	const double	ra ;
	const double	dec ;
    const char    *	sptype ;
    const float  	mag_J ;
    const float  	mag_H ;
    const float  	mag_K ;
    const float  	mag_Ks ;
    const float  	mag_L ;
    const float  	mag_M ;
    const float  	mag_Lp ;
    const float  	mag_Mp ;
    const int       source ;
} irstd ;


/*-------------------------------------------------------------------------*/
/**
  @brief	sptype_temp object
 */
/*-------------------------------------------------------------------------*/
typedef struct _SPTYPE_TEMP_
{
	char	*	type ;
	int			temperature ;
} sptype_temp ;


/*-------------------------------------------------------------------------*/
/**
  @brief	ir_waveband object
 */
/*-------------------------------------------------------------------------*/
typedef enum _WAVEBAND_ {
	WAVEBAND_J,
	WAVEBAND_H,
	WAVEBAND_K,
	WAVEBAND_KS,
	WAVEBAND_L,
	WAVEBAND_M,
	WAVEBAND_Lprime,
	WAVEBAND_Mprime,
	WAVEBAND_UNKNOWN
} ir_waveband ;


/*---------------------------------------------------------------------------
   							Function prototypes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Sets the active catalogs for search
  @param    catalog     Name of catalog to activate.
  @return   int Total number of stars still active in star list.

  Pass a catalog name to activate for further searches with
  irlist_get_star functions. Invalid catalog names trigger an error
  message.

  If catalog name is "none", all catalogs are deactivated.
  If catalog name is "all", all catalogs are activated.
  If catalog name is NULL, the number of active stars in list is
  computed and returned.
 */
/*--------------------------------------------------------------------------*/
int irstd_setactive(char * catalog) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Return a catalog name
  @param    cat_id  Catalog Id as stored in the star source field
  @return   1 pointer to a static catalog name.

  This function is useful to get the static catalog name supported
  by the current internal database. Do not modify or try to free the
  returned string. Since it is static, this would cause a segfault.
 */
/*--------------------------------------------------------------------------*/
char * irstd_catalog_name(int cat_id) ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Return a list of catalog names
  @return   1 pointer to a static list of catalog names.

  This function is useful to get a static list of catalog names supported
  by the current internal database. Do not modify or try to free the
  returned list of strings or the strings themselves. Since they are
  static, this would cause a segfault.
 */
/*--------------------------------------------------------------------------*/
char ** irstd_catalog_names(void) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find a star in the current list by name.
  @param    name    Regular expression for star name matching.
  @param    nstars  Output number of stars found matching the name.
  @return   Newly allocated list of standard stars.

  Provide a regular expression, and all stars which name matches this
  expression will be stored into a newly allocated list returned to
  the caller. See general Unix documentation about regular expressions
  to learn more about matching strings.

  The returned list contains pointers to the internal star list, 
  to avoid copying any data. This means that the returned structure
  does not need to be freed in depth, but only with a simple free().
 */
/*--------------------------------------------------------------------------*/
irstd ** irstd_get_star_by_name(
        char    *   name,
        int     *   nstars) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find the closest star to a given position
  @param    ra_d        Right ascension {\em in degrees}.
  @param    dec_d       Declination {\em in degrees}.
  @return   Pointer to one newly allocated standard star object.

  Finds out the closest star to a given position and returns it as a
  newly allocated star object. Provide RA and DEC in degrees!

  The returned star object is a pointer to the found star in the
  internal irstd_list structure, thus must not be freed.
 */
/*--------------------------------------------------------------------------*/
irstd * irstd_get_closest_star(double ra_d, double dec_d) ;

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the closest star from one catalog if the mag is known
  @param    ra_d        Right ascension {\em in degrees}.
  @param    dec_d       Declination {\em in degrees}.
  @param    band        Band in which we want the magnitude
  @param    cat         The catalog to search in (cannot be "all")
  @param    mag         The magnitude
  @return   Pointer to one standard star object
 */
/*----------------------------------------------------------------------------*/
irstd * irstd_get_star_magnitude_one_cat(double, double, ir_waveband, char *,
        double *) ;

/*----------------------------------------------------------------------------*/
/**
  @brief	Find the closest star from the catalog where the mag is known
  @param    ra_d        Right ascension {\em in degrees}.
  @param    dec_d       Declination {\em in degrees}.
  @param    band        Band in which we want the magnitude
  @param    mag         The magnitude
  @return   Pointer to one standard star object
 */
/*----------------------------------------------------------------------------*/
irstd * irstd_get_star_magnitude(double, double, ir_waveband, double *) ; 

/*-------------------------------------------------------------------------*/
/**
  @brief    Find all stars within a given radius around a position.
  @param    ra_d    Right ascension of the center {\em in degrees}.
  @param    dec_d   Declination of the center {\em in degrees}.
  @param    radius  Euclidean radius around the center.
  @param    nfound  Output number of found stars.
  @return   Newly allocated list of stars (or NULL if none found).

  This functions locates all stars in a given disk. The disk is
  defined by a center which coordinates are given in degrees (RA and
  Dec), and the radius is defined as a euclidean distance, i.e.:

  A star at (r,d) is in the disk of center (r0,d0) and radius R if:

  \[
  (r-r_0)^2+(d-d_0)^2 < R^2
  \]

  The returned list of stars must be freed using free().
 */
/*--------------------------------------------------------------------------*/
irstd ** irstd_get_star_by_position(
    double      ra_d,
    double      dec_d,
    double      radius,
    int     *   nfound) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find all stars within a magnitude range in a waveband.
  @param    band    Wave band to search (see enum definition in irstd.h).
  @param    mag_min Minimal magnitude.
  @param    mag_max Maximal magnitude.
  @param    nstars  Output number of found stars.
  @return   Newly allocated list of stars.

  Finds out all stars in a given wave band, which magnitude is
  strictly greater than mag_min and strictly lower than mag_max.

  The returned list of stars must be freed using free().
 */
/*--------------------------------------------------------------------------*/
irstd ** irstd_get_star_by_magnitude(
    ir_waveband     band,
    double          mag_min,
    double          mag_max,
    int         *   nstars) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find a star temperature from its spectral type.
  @param    sptype  Character string describing the spectral type.
  @return   Star temperature in Kelvins as an int, -1 if cannot be found.

  Spectral type identification algorithm courtesy of Jean-Gabriel
  Cuby. The spectral type pattern is:

  \begin{verbatim}
  %c one character in {O B A F G K M}
  %d(.%d) an integer or half-integer.
  %s a roman number, not supported yet.
  \end{verbatim}

  Not all spectral types are recognized. The list is hardcoded in the
  software itself, more spectral types should be added later on.
 */
/*--------------------------------------------------------------------------*/
int irstd_get_star_temperature(char * sptype) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Convert right ascension from degrees to HH:MM:SS
  @param    ra      Right ascension in degrees
  @param    hh      Output HH
  @param    mm      Output MM
  @param    ss      Output SS
  @return   void

  Convert right ascension from degrees to HH:MM:SS.
 */
/*--------------------------------------------------------------------------*/
void ra_conv(
        double      ra,
        int     *   hh,
        int     *   mm,
        int     *   ss) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Convert declination from degrees to DD:MM:SS
  @param    dec         Declination in degrees
  @param    sign_char   Output Sign
  @param    hh          Output HH
  @param    mm          Output MM
  @param    ss          Output SS
  @return   void

  Convert declination from degrees to Sign:HH:MM:SS. Careful about the
  sign! A value of -0 is usually parsed as +0, but that inverts the
  value for the declination.
 */
/*--------------------------------------------------------------------------*/
void dec_conv(
        double      dec,
        char    *   sign_char,
        int     *   dd,
        int     *   mm,
        int     *   ss) ;

#endif
