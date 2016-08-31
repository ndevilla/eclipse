/*----------------------------------------------------------------------------*/
/**
   @file    utils.c
   @author  N. Devillard
   @date    August 1998
   @version	$Revision: 1.16 $
   @brief   ISAAC various utilities
*/
/*----------------------------------------------------------------------------*/

/*
 $Id: utils.c,v 1.16 2003/11/18 09:37:55 yjung Exp $
 $Author: yjung $
 $Date: 2003/11/18 09:37:55 $
 $Revision: 1.16 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "utils.h"
#include "classif.h"
#include "pfits.h"

/*-----------------------------------------------------------------------------
						Function ANSI C code 
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out the slit width
  @param    filename    ISAAC fits file name 
  @return   the slit width in pixels or -1 in error case 
 */
/*----------------------------------------------------------------------------*/
double isaac_get_slitwidth(char * filename)
{
    char        *   sval ;
    double          slit_width ;
    double          pscale ;
    instrument_t    ins ;
    
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

    /* Get the slit name used */
    if ((sval = pfits_get(ins, filename, "optical_id")) == NULL) {
        e_error("cannot get slit used") ;
        return -1 ;
    }
    
    /* Get the slit width in arcseconds */
    if (!strcmp(sval, "slit_1"))                 slit_width = 1 ;    
    else if (!strcmp(sval, "slit_2"))            slit_width = 2 ;
    else if (!strcmp(sval, "slit_0.3_tilted"))   slit_width = 0.3 ;
    else if (!strcmp(sval, "slit_0.8"))          slit_width = 0.8 ;
    else if (!strcmp(sval, "slit_1.5"))          slit_width = 1.5 ;
    else if (!strcmp(sval, "slit_0.6_tilted"))   slit_width = 0.6 ;
    else {
        e_error("unrecognized slit") ;
        return -1 ;
    }
    
    /* Get the pixelscale and convert arsec -> pixels */
    if ((sval = pfits_get(ins, filename, "pixscale")) == NULL) {
        e_error("cannot get pixscale") ;
        return -1 ;
    }
    pscale = (double)atof(sval) ;

    if (pscale <= 0) {
        e_error("Illegal pixscale") ;
        return -1;
    }

    if (debug_active() >= 2)
        e_comment(2, "Slit width = %g arcsec (%-.2f pixels)\n",
                  slit_width, slit_width / pscale);

    slit_width /= pscale;

    /* Return  */
    return slit_width;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find out all header offsets for a frame list.
  @param    filename    Name of the ASCII list to parse.
  @return   1 newly allocated double3 array.

  This function calls iteratively get_cumoffsetx and get_cumoffsety on each
  file name in the input ASCII frame list, and stores the results
  into a newly allocated double3 array. If an error occurs, this function
  returns NULL.
 */
/*----------------------------------------------------------------------------*/
double3 * isaac_get_offsets(char * filename)
{
    double3     *   offs ;
    int             i ;
    framelist   *   flist ;
    char        *   sval ;
    instrument_t    ins ;

    /* Test inputs */
    if (filename==NULL) return NULL;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    
    if ((flist = framelist_load(filename)) == NULL) {
        e_error("reading frame list: %s", filename);
        return NULL ;
    }

    offs = double3_new(flist->n);
    for (i=0 ; i<flist->n ; i++) {
        /* Get X offset */
        if ((sval = pfits_get(ins, flist->name[i], "cumoffsetx")) == NULL) {
            e_error("getting X offset from frame %s", flist->name[i]);
            double3_del(offs);
            return NULL ;
        }
        offs->x[i] = (double)atof(sval);

        /* Get Y offset */
        if ((sval = pfits_get(ins, flist->name[i], "cumoffsety")) == NULL) {
            e_error("getting Y offset from frame %s", flist->name[i]);
            double3_del(offs);
            return NULL ;
        }
        offs->y[i] = (double)atof(sval);

        /* Set z field to zero */
        offs->z[i] = 0.0 ;
    }

    /* Free and return */
    framelist_del(flist);
    return offs ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Read the central wl in header and convert it in Angstroms
  @param    filename    ISAAC FITS file name
  @return   central wavelength in angstroms, -1 in error case
 */
/*----------------------------------------------------------------------------*/
double isaac_get_central_wavelength(char * filename)
{
    char    *   wlen_s ;
    double      wlen ;

    /* Initialize */
    wlen = -1.0 ;
    
    /* So far, only one convention used for central wavelength */
    if (qfits_is_paf_file(filename)) {
        wlen_s = qfits_paf_query(filename, "INS.GRAT.WLEN");
        if (wlen_s != NULL) wlen = (double)atof(wlen_s);
    } else {
        wlen_s = qfits_query_hdr(filename, "INS.GRAT.WLEN");
            /* Factor 10000.0 due to conversion microns->angstroms */
        if (wlen_s != NULL) wlen = 10000.0 * (double)atof(wlen_s);
    }
    return wlen ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    find out for a given ISAAC file, if the Argon lamp was active
  @param    filename    ISAAC FITS file name
  @return   1 if the lamp is active, 0 if not, -1 in error case
  Based on the status of keyword INS.LAMP1.ST
 */
/*----------------------------------------------------------------------------*/
int isaac_is_argon_lamp_active(char * filename)
{
    char    *   status ;
    char    *   sval ;

    /* So far, only one convention used for first lamp status */
    status = qfits_query_hdr(filename, "INS.LAMP1.ST");
    if (status == NULL) return -1 ;
    status = qfits_pretty_string(status);
    if (toupper(status[0])=='T') {
        /* Still has to verify that shutter is open */
        sval = qfits_query_hdr(filename, "INS.CALSHUT.ST") ;
        if (sval == NULL) return 1 ;
        sval = qfits_pretty_string(sval) ;
        if (toupper(sval[0]) == 'T') return 1 ;
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    find out for a given ISAAC file, if the Xenon lamp was active
  @param    filename    ISAAC FITS file name
  @return   1 if the lamp is active, 0 if not, -1 in error case
  Based on the status of keyword INS.LAMP2.ST
 */
/*----------------------------------------------------------------------------*/
int isaac_is_xenon_lamp_active(char * filename)
{
    char    *   status ;
    char    *   sval ;
    /* So far, only one convention used for second lamp status */
    status = qfits_query_hdr(filename, "INS.LAMP2.ST");
    if (status == NULL) return -1 ;
    status = qfits_pretty_string(status);
    if (toupper(status[0])=='T') {
        /* Still has to verify that shutter != 0 */
        sval = qfits_query_hdr(filename, "INS.CALSHUT.ST") ;
        if (sval == NULL) return 1 ;
        sval = qfits_pretty_string(sval) ;
        if (toupper(sval[0]) == 'T') return 1 ;
    }
    return 0 ;
}

