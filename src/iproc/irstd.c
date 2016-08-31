/*-------------------------------------------------------------------------*/
/**
   @file	irstd.c
   @author	N. Devillard
   @date	21 Jan 1999
   @version	$Revision: 1.16 $
   @brief	Infrared Standard Star list handling

   This module contains a default list of infrared standard stars. Since 
   this belongs to code, the list might not be the most up-to-date, and it 
   is recommended to provide data files rather than relying on it.
   Nevertheless, it allows processing to go faster and a default process in 
   most cases, to have such a list hardcoded here.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: irstd.c,v 1.16 2004/02/18 15:50:10 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/18 15:50:10 $
	$Revision: 1.16 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#ifdef _ECLIPSE_
#include "eclipse.h"
#else
#include "e_error.h"
#endif

#include "irstd.h"
#include "irlist.h"
#include "irtemp.h"

/*---------------------------------------------------------------------------
   							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Sets the active catalogs for search
  @param	catalog		Name of catalog to activate.
  @return	int Total number of stars still active in star list.

  Pass a catalog name to activate for further searches with
  irlist_get_star functions. Invalid catalog names trigger an error
  message.

  If catalog name is "none", all catalogs are deactivated.
  If catalog name is "all", all catalogs are activated.
  If catalog name is NULL, the number of active stars in list is
  computed and returned.
 */
/*--------------------------------------------------------------------------*/
int irstd_setactive(char * catalog)
{
	int	i;
	int	found ;
	int	active ;

	/* NULL: Compute number of active stars in list */
	if (catalog==NULL) {
		found=0 ;
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if (irstd_list[i].select)
				found++;
			i++ ;
		}
		return found ;
	}

	/* "none": disable all stars */
	if (!strcmp(catalog, "none")) {
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			irstd_list[i].select=0 ;
			i++;
		}
		return 0 ;
	}

	/* "all": enable all stars */
	if (!strcmp(catalog, "all")) {
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			irstd_list[i].select=1 ;
			i++;
		}
		return i ;
	}

	/* General case: activate only required catalog */
	i=0 ;
	active=0 ;
	found=0 ;
	while (irstd_list[i].name!=NULL) {
		if (!strcmp(catalog, irstd_catalogs[irstd_list[i].source])) {
			found=1 ;
			irstd_list[i].select=1 ;
		}
		if (irstd_list[i].select) {
			active++ ;
		}
		i++ ;
	}
	if (found<1) {
		e_error("invalid catalog name: %s", catalog);
		return -1 ;
	}
	return active ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Return a catalog name
  @param    cat_id  Catalog Id as stored in the star source field
  @return	1 pointer to a static catalog name.

  This function is useful to get the static catalog name supported
  by the current internal database. Do not modify or try to free the
  returned string. Since it is static, this would cause a segfault.
 */
/*--------------------------------------------------------------------------*/
char * irstd_catalog_name(int cat_id)
{
	return (char*)irstd_catalogs[cat_id] ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Return a list of catalog names
  @return	1 pointer to a static list of catalog names.

  This function is useful to get a static list of catalog names supported
  by the current internal database. Do not modify or try to free the
  returned list of strings or the strings themselves. Since they are
  static, this would cause a segfault.
 */
/*--------------------------------------------------------------------------*/
char ** irstd_catalog_names(void)
{
	return (char**)irstd_catalogs;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find a star in the current list by name.
  @param	name	Regular expression for star name matching.
  @param	nstars	Output number of stars found matching the name.
  @return	Newly allocated list of standard stars.

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
		char	*	name, 
		int 	* 	nstars)
{
	char		name_exp[40] ;
	regex_t 	re_name ;
	int			found ;
	int			i;
	irstd **	starlist ;

	sprintf(name_exp, "^%s$", name) ;
	if (regcomp(&re_name, &name[0], REG_EXTENDED|REG_NOSUB)!=0) {
		e_error("cannot compile regexp: [%s]", name) ;
		return NULL ;
	}

	starlist = NULL ;
	found = 0 ;
	i=0 ;
	while (irstd_list[i].name!=NULL) {
		/* matching using a regexp */
		if ((irstd_list[i].select==1) &&
			(regexec(&re_name, irstd_list[i].name, 0, NULL, 0)==0)) {
			found++ ;
		}
		i++ ;
	}
	if (found<1) {
		*nstars = 0 ;
		return NULL ;
	}
	starlist = malloc(found * sizeof(irstd*)) ;
	*nstars = found ;
	found = 0 ;
	i=0 ;
	while (irstd_list[i].name!=NULL) {
		if ((irstd_list[i].select==1) &&
			(regexec(&re_name, irstd_list[i].name, 0, NULL, 0)==0)) {
			starlist[found] = &(irstd_list[i]);
			found++ ;
		}
		i++ ;
	}
	return starlist ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find the closest star to a given position
  @param	ra_d		Right ascension {\em in degrees}.
  @param	dec_d		Declination {\em in degrees}.
  @return	Pointer to a standard star object.

  Finds out the closest star to a given position and returns it as a
  newly allocated star object. Provide RA and DEC in degrees!

  The returned star object is a pointer to the found star in the
  internal irstd_list structure, thus must not be freed.
 */
/*--------------------------------------------------------------------------*/
#define IRSTD_MAXRADIUS		(2.0/60.0)		/* 2 arcminutes in degrees */
#define IRSTD_SQMAXRADIUS	(IRSTD_MAXRADIUS*IRSTD_MAXRADIUS)
irstd * irstd_get_closest_star(
		double	ra_d, 
		double	dec_d)
{
	irstd *	starlist ;
	double	ra, dec ;
	double	cur_dist ;
	double	min_dist ;
	int		min_index ;
	int		i ;

	starlist = NULL ;

	/* Find first valid star to initialize minimum distance */
	i=0 ;
	while (irstd_list[i].select==0 && irstd_list[i].name!=NULL)
		i++ ;
	if (irstd_list[i].name==NULL) {
		return NULL ;
	}
	ra  = irstd_list[i].ra ;
	dec = irstd_list[i].dec ;
	min_dist = (ra_d-ra)*(ra_d-ra)+(dec_d-dec)*(dec_d-dec) ;
	min_index = i ;

	/* Look for minimum distance star */
	i=0 ;
	while (irstd_list[i].name!=NULL) {
		if (irstd_list[i].select) {
			ra  = irstd_list[i].ra ;
			dec = irstd_list[i].dec ;
			cur_dist = (ra_d-ra)*(ra_d-ra)+(dec_d-dec)*(dec_d-dec) ;
			if (cur_dist<min_dist) {
				min_dist = cur_dist ;
				min_index = i ;
			}
		}
		i++ ;
	}

	if (min_dist>IRSTD_SQMAXRADIUS) {
		/* The closest star is more than 2 arcminutes away */
		starlist = NULL ;
	} else {
		starlist = &(irstd_list[min_index]);
	}
	return starlist ;
}

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
irstd * irstd_get_star_magnitude_one_cat(
        double          ra, 
        double          dec, 
        ir_waveband     band, 
        char        *   cat,
        double      *   mag)
{
    irstd       *   refstar ;
    
    /* Test entries */
    if (cat == NULL) return ;
    if (!strcmp(cat, "all")) return NULL ;
    
    /* Search closest star */
    irstd_setactive("none");
    irstd_setactive(cat) ;
    refstar = irstd_get_closest_star(ra, dec) ;

    /* Star not found */
    if (refstar == NULL) return NULL ;
    
    /* Keep the star if magnitude is known */
    switch (band) {
        case WAVEBAND_J:
            if (refstar->mag_J < 98.0) {
                *mag = (double)(refstar->mag_J) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_H:
            if (refstar->mag_H < 98.0) {
                *mag = (double)(refstar->mag_H) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_K:
            if (refstar->mag_K < 98.0) {
                *mag = (double)(refstar->mag_K) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_KS:
            if (refstar->mag_Ks < 98.0) {
                *mag = (double)(refstar->mag_Ks) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_L:
            if (refstar->mag_L < 98.0) {
                *mag = (double)(refstar->mag_L) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_M:
            if (refstar->mag_M < 98.0) {
                *mag = (double)(refstar->mag_M) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_Lprime:
            if (refstar->mag_Lp < 98.0) {
                *mag = (double)(refstar->mag_Lp) ;
                return refstar ;
            } else return NULL ;
        case WAVEBAND_Mprime:
            if (refstar->mag_Mp < 98.0) {
                *mag = (double)(refstar->mag_Mp) ;
                return refstar ;
            } else return NULL ;
        default:
            return NULL ;
    }
    
    /* Free and return */
    return NULL ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find the closest star from the catalog where the mag is known
  @param    ra_d        Right ascension {\em in degrees}.
  @param    dec_d       Declination {\em in degrees}.
  @param    band        Band in which we want the magnitude
  @param    mag         The magnitude
  @return   Pointer to one standard star object
 */
/*----------------------------------------------------------------------------*/
irstd * irstd_get_star_magnitude(
        double          ra, 
        double          dec, 
        ir_waveband     band, 
        double      *   mag)
{
    char        **  catalog_names ;
    irstd       *   refstar ;
    int             nfound ;
    irstd       **  refstars ;
    int             i, j ;
    
    /* Initialise */
    catalog_names = irstd_catalog_names() ;
    nfound = 0 ;
    
    /* Loop on the catalogs and get the closest stars */
    for (i=0 ; catalog_names[i] ; i++) {
        irstd_setactive("none");
        irstd_setactive(catalog_names[i]) ;
        refstar = irstd_get_closest_star(ra, dec) ;
        if (refstar != NULL) nfound ++ ;
    }
    refstars = malloc(nfound*sizeof(irstd*)) ;
    j = 0 ;
    for (i=0 ; catalog_names[i] ; i++) {
        irstd_setactive("none");
        irstd_setactive(catalog_names[i]) ;
        refstar = irstd_get_closest_star(ra, dec) ;
        if (refstar != NULL) {
            refstars[j] = refstar ;
            j++ ;
        }
    }
    
    /* Keep the first one whose magnitude is known */
    refstar = NULL ;
    switch (band) {
        case WAVEBAND_J:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_J < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_J) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_H:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_H < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_H) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_K:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_K < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_K) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_KS:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_Ks < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_Ks) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_L:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_L < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_L) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_M:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_M < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_M) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_Lprime:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_Lp < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_Lp) ;
                    break ;
                }
            }
            break ;
        case WAVEBAND_Mprime:
            for (i=0 ; i<nfound ; i++) {
                if (refstars[i]->mag_Mp < 98.0) {
                    refstar = refstars[i] ;
                    *mag = (double)(refstar->mag_Mp) ;
                    break ;
                }
            }
            break ;
        default:
            break ;
    }
    
    /* Free and return */
    free(refstars) ;
    return refstar ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Find all stars within a given radius around a position.
  @param	ra_d	Right ascension of the center {\em in degrees}.
  @param	dec_d	Declination of the center {\em in degrees}.
  @param	radius	Euclidean radius around the center.
  @param	nfound	Output number of found stars.
  @return	Newly allocated list of stars (or NULL if none found).

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
		double  	ra_d,
    	double  	dec_d,
    	double  	radius,
    	int   	* 	nfound)
{
	irstd **	starlist ;
	double		ra, dec ;
	double		cur_dist ;
	int			i ;
	int			found ;

	starlist = NULL ;
	found = 0 ;
	i=0 ;
	while (irstd_list[i].name!=NULL) {
		if (irstd_list[i].select) {
			ra  = irstd_list[i].ra ;
			dec = irstd_list[i].dec ;
			cur_dist = (ra_d-ra)*(ra_d-ra)+(dec_d-dec)*(dec_d-dec) ;
			if (cur_dist <= (radius * radius)) {
				found++ ;
			}
		}
		i++ ;
	}

	if (found<1) return NULL ;
	starlist = malloc(found * sizeof(irstd*)) ;

	found = 0 ;
	i=0 ;
	while (irstd_list[i].name!=NULL) {
		if (irstd_list[i].select) {
			ra  = irstd_list[i].ra ;
			dec = irstd_list[i].dec ;
			cur_dist = (ra_d-ra)*(ra_d-ra)+(dec_d-dec)*(dec_d-dec) ;
			if (cur_dist <= (radius * radius)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
		}
		i++ ;
	}
	*nfound = found ;
	return starlist ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find all stars within a magnitude range in a waveband.
  @param	band	Wave band to search (see enum definition in irstd.h).
  @param	mag_min	Minimal magnitude.
  @param	mag_max	Maximal magnitude.
  @param	nstars	Output number of found stars.
  @return	Newly allocated list of stars.

  Finds out all stars in a given wave band, which magnitude is
  strictly greater than mag_min and strictly lower than mag_max.

  The returned list of stars must be freed using free().
 */
/*--------------------------------------------------------------------------*/
irstd ** irstd_get_star_by_magnitude(
    ir_waveband		band,
    double			mag_min,
    double			mag_max,
    int			*	nstars)
{
	irstd **	starlist ;
	int			i ;
	int			found ;

	starlist = NULL ;
	found = 0 ;

	/*
	 * Find out how many stars are within this magnitude range
	 */
	switch(band) {
		case WAVEBAND_J:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
				((double)irstd_list[i].mag_J > mag_min) &&
				((double)irstd_list[i].mag_J < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_H:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
				((double)irstd_list[i].mag_H > mag_min) &&
				((double)irstd_list[i].mag_H < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_K:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_K > mag_min) &&
				((double)irstd_list[i].mag_K < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_KS:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_Ks > mag_min) &&
				((double)irstd_list[i].mag_Ks < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_L:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_L > mag_min) &&
				((double)irstd_list[i].mag_L < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_M:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_M > mag_min) &&
				((double)irstd_list[i].mag_M < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_Lprime:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_Lp > mag_min) &&
				((double)irstd_list[i].mag_Lp < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_Mprime:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_Mp > mag_min) &&
				((double)irstd_list[i].mag_Mp < mag_max)) found ++ ;
			i++ ;
		}
		break ;

		case WAVEBAND_UNKNOWN:
		default:
		e_error("unsupported wave band requested") ;
		return NULL ;
	}

	if (found<1) {
		*nstars=0;
		return NULL ;
	}


	/* Now build up the list of matching stars */
	starlist = malloc(found * sizeof(irstd*)) ;
	*nstars = found ;
	found = 0 ;
	switch(band) {
		case WAVEBAND_J:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_J > mag_min) &&
				((double)irstd_list[i].mag_J < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_H:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_H > mag_min) &&
				((double)irstd_list[i].mag_H < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_K:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_K > mag_min) &&
				((double)irstd_list[i].mag_K < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_KS:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_Ks > mag_min) &&
				((double)irstd_list[i].mag_Ks < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_L:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_L > mag_min) &&
				((double)irstd_list[i].mag_L < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_M:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_M > mag_min) &&
				((double)irstd_list[i].mag_M < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_Lprime:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_Lp > mag_min) &&
				((double)irstd_list[i].mag_Lp < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		case WAVEBAND_Mprime:
		i=0 ;
		while (irstd_list[i].name!=NULL) {
			if ((irstd_list[i].select) &&
			    ((double)irstd_list[i].mag_Mp > mag_min) &&
				((double)irstd_list[i].mag_Mp < mag_max)) {
				starlist[found] = &(irstd_list[i]);
				found ++ ;
			}
			i++ ;
		}
		break ;

		default:
		break ;
	}
	return starlist ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find a star temperature from its spectral type.
  @param	sptype	Character string describing the spectral type.
  @return	Star temperature in Kelvins as an int, -1 if cannot be found.

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
int irstd_get_star_temperature(char * sptype)
{
	int		i ;
	int		temperature ;
	char	sptype_ext[20] ;

	temperature = -1 ;

	/* Look for exact match with given spectral type */
	i=0 ;
	while (irstd_temperature_table[i].temperature > 0) {
		if (!strcmp(sptype, irstd_temperature_table[i].type)) {
			temperature = irstd_temperature_table[i].temperature ;
			break ;
		}
		i++ ;
	}
	/* if no match was found, try to match []V for a V type star */
	if (temperature == -1) {
		sprintf(sptype_ext, "%sV", sptype) ;
		i=0 ;
		while (irstd_temperature_table[i].temperature > 0) {
			if (!strcmp(sptype_ext, irstd_temperature_table[i].type)) {
				temperature = irstd_temperature_table[i].temperature ;
				break ;
			}
			i++ ;
		}
	}
	return temperature ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Convert right ascension from degrees to HH:MM:SS
  @param	ra		Right ascension in degrees
  @param	hh		Output HH
  @param	mm		Output MM
  @param	ss		Output SS
  @return 	void

  Convert right ascension from degrees to HH:MM:SS.
 */
/*--------------------------------------------------------------------------*/
void ra_conv(
		double		ra,
		int		*	hh,
		int		*	mm,
		int		*	ss)
{
	ra /= 15.0 ;
	*hh = (int)ra ;
	ra -= (double)(*hh) ;
	ra *= 60.0 ;
	*mm = (int)ra ;
	ra -= (double)(*mm) ;
	ra *= 60.0 ;
	*ss = (int)(ra+0.5) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Convert declination from degrees to DD:MM:SS
  @param	dec			Declination in degrees
  @param	sign_char	Output Sign
  @param	hh			Output HH
  @param	mm			Output MM
  @param	ss			Output SS
  @return 	void

  Convert declination from degrees to Sign:HH:MM:SS. Careful about the
  sign! A value of -0 is usually parsed as +0, but that inverts the
  value for the declination.
 */
/*--------------------------------------------------------------------------*/
void dec_conv(
		double		dec,
		char	*	sign_char,
		int		*	dd,
		int		*	mm,
		int		*	ss)
{
    if (dec<0) {
        *sign_char = '-' ;
        dec = -dec ;
    } else {
        *sign_char = '+' ;
    }
    *dd = (int)dec ;
    dec -= (double)(*dd) ;
    dec *= 60.0 ;
    *mm = (int)dec ;
    dec -= (double)(*mm) ;
    dec *= 60.0 ;
    *ss = (int)(dec+0.5) ;
}

