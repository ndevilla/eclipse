/*-------------------------------------------------------------------------*/
/**
   @file	key_isaac.c
   @author	Y. Jung
   @date	July 2000
   @version	$Revision: 1.15 $
   @brief	ISAAC functions using FITS header keywords
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: key_isaac.c,v 1.15 2004/02/09 09:19:14 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 09:19:14 $
	$Revision: 1.15 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "qfits.h"
#include "keyfits.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    get airmass start from an ISAAC FITS file
  @param    filename    source FITS file
  @return   Char string containing what was found in header, NULL if error.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_airmass_start(char * filename)
{
    char   *    val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.AIRM.START") ;
    } else {
        val = qfits_query_hdr(filename, "TEL.AIRM.START");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    get airmass end from an ISAAC FITS file
  @param    filename    source FITS file
  @return   Char string containing what was found in header, NULL if error.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_airmass_end(char * filename)
{
    char   *    val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.AIRM.END") ;
    } else {
        val = qfits_query_hdr(filename, "TEL.AIRM.END");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the arcfile   
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_arcfile(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "ARCFILE");
    } else {
        val = qfits_query_hdr(filename, "ARCFILE");
    }
    return qfits_pretty_string(val);
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find out which arm is active in ISAAC
  @param    filename    ISAAC FITS file name.
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_arm(char * filename)
{
    char * val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "OCS.SELECT-ARM");
    } else {
        val = qfits_query_hdr(filename, "OCS.SELECT-ARM");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Find out which arm is active in ISAAC
  @param    filename    ISAAC FITS file name.
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_chip(char * filename)
{
    char * val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.CHIP.NAME");
    } else {
        val = qfits_query_hdr(filename, "DET.CHIP.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get chopping cycle.
  @param    filename    Name of the file to query.
  @return   statically allocated char string, no need to free() it.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_chopping_cycle(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.CHOP.NCYCLES");
    } else {
        val = qfits_query_hdr(filename, "DET.CHOP.NCYCLES");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get chopping frequency.
  @param    filename    Name of the file to query.
  @return   statically allocated char string, no need to free() it.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_chopping_frequency(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.CHOP.FREQ");
    } else {
        val = qfits_query_hdr(filename, "TEL.CHOP.FREQ");
    }
    return qfits_pretty_string(val);
}

/*---------------------------------------------------------------------------*/
/** 
  @brief    Get chopping status
  @param    filename    Input FITS file.
  @return   statically allocated char string, no need to free() it
*/
/*---------------------------------------------------------------------------*/
char * isaac_get_chopping_status(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.CHOP.ST");
    } else {
        val = qfits_query_hdr(filename, "TEL.CHOP.ST");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get chopping throw.
  @param    filename    Name of the file to query.
  @return   statically allocated char string, no need to free() it.
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_chopping_throw(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.CHOP.THROW");
    } else {
        val = qfits_query_hdr(filename, "TEL.CHOP.THROW");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the CUMOFFSETX
            keyword in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_cumoffsetx(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "SEQ.CUMOFFSETX");
    } else {
        val = qfits_query_hdr(filename, "SEQ.CUMOFFSETX");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the CUMOFFSETY
            keyword in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_cumoffsety(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "SEQ.CUMOFFSETY");
    } else {
        val = qfits_query_hdr(filename, "SEQ.CUMOFFSETY");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the current exposure number 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_current_exp_nb(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TPL.EXPNO");
    } else {
        val = qfits_query_hdr(filename, "TPL.EXPNO");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the date   
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_date(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DATE");
    } else {
        val = qfits_query_hdr(filename, "DATE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the date of observation  
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_date_obs(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DATE-OBS");
    } else {
        val = qfits_query_hdr(filename, "DATE-OBS");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the DEC keyword
            in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_dec(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DEC");
    } else {
        val = qfits_query_hdr(filename, "DEC");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/** 
  @brief    get the string describing the frame type
  @param    filename ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*-------------------------------------------------------------------------*/
char * isaac_get_detector_frame_type(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        if ((val = qfits_paf_query(filename, "DET.FRAME.TYPE")) == NULL) {
            /* New keyword (July 2001) */
            val = qfits_paf_query(filename, "DET.FRAM.TYPE") ;
        }
    } else {
        if ((val = qfits_query_hdr(filename, "DET.FRAME.TYPE")) == NULL) {
            /* New keyword (July 2001) */
            val = qfits_query_hdr(filename, "DET.FRAM.TYPE") ;
        }
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/** 
  @brief    Get the read out mode name.
  @param    filename    ISAAC FITS file name
  @return pointer to statically allocated char string, or NULL.
 */
/*-------------------------------------------------------------------------*/
char * isaac_get_detector_readout_mode(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.MODE.NAME");
    } else {
        val = qfits_query_hdr(filename, "DET.MODE.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the DIT keyword
            in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_dit(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.DIT");
    } else {
        val = qfits_query_hdr(filename, "DET.DIT");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the data category as defined by the DataFlow
  @param    filename    source FITS file
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_dpr_catg(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DPR.CATG");
    } else {
        val = qfits_query_hdr(filename, "DPR.CATG");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the data tech as defined by the DataFlow
  @param    filename    source FITS file
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_dpr_tech(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DPR.TECH");
    } else {
        val = qfits_query_hdr(filename, "DPR.TECH");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the data type as defined by the DataFlow
  @param    filename    source FITS file
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_dpr_type(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DPR.TYPE");
    } else {
        val = qfits_query_hdr(filename, "DPR.TYPE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out which wave band is active in long wavelength
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_filter_lw(char * filename)
{
    char        *   val ;

    if ((val = qfits_query_hdr(filename, "INS.FILT3.ID")) == NULL) return NULL ;
    val = qfits_pretty_string(val);
    /* If FILT3 is not open, return its value */
    if (strcmp(val, "open")) return val ;
    /* FILT3 is open, return value from FILT4 */
    if ((val = qfits_query_hdr(filename, "INS.FILT4.ID")) == NULL) return NULL ;
    val = qfits_pretty_string(val);
    if (strcmp(val, "open")) return val ;
    return NULL ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out which wave band is active in short wavelength
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_filter_sw(char * filename)
{
    char        *   val ;

    if ((val = qfits_query_hdr(filename, "INS.FILT1.ID")) == NULL) return NULL ;
    val = qfits_pretty_string(val);
    /* If FILT1 is not open, return its value */
    if (strcmp(val, "open")) return val ;
    /* FILT1 is open, return value from FILT2 */
    if ((val = qfits_query_hdr(filename, "INS.FILT2.ID")) == NULL) return NULL ;
    val = qfits_pretty_string(val);
    if (strcmp(val, "open")) return val ;
    return NULL ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the first dispersion coefficient in HISTORY
            fields
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_hist_disp1(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "HISTORY DISPCOE1");
    } else {
        val = qfits_query_hdr(filename, "HISTORY DISPCOE1");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the second dispersion coefficient in HISTORY
            fields
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_hist_disp2(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "HISTORY DISPCOE2");
    } else {
        val = qfits_query_hdr(filename, "HISTORY DISPCOE2");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the second dispersion coefficient in HISTORY
            fields
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_hist_disp3(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "HISTORY DISPCOE3");
    } else {
        val = qfits_query_hdr(filename, "HISTORY DISPCOE3");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the second dispersion coefficient in HISTORY
            fields
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_hist_disp4(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "HISTORY DISPCOE4");
    } else {
        val = qfits_query_hdr(filename, "HISTORY DISPCOE4");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the ambient humidity level as defined in the
            relevant ambient monitor dictionary
  @param    filename    source FITS file
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_humidity_level(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.AMBI.RHUM");
    } else {
        val = qfits_query_hdr(filename, "TEL.AMBI.RHUM");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the instrument
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_instrument(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INSTRUME");
    } else {
        val = qfits_query_hdr(filename, "INSTRUME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the first lamp name. 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_lamp1_name(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP1.NAME");
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP1.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the first lamp status. 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_lamp1_status(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP1.ST");
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP1.ST");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the second lamp name. 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_lamp2_name(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP2.NAME");
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP2.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the second lamp status. 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_lamp2_status(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP2.ST");
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP2.ST");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the third lamp intensity. 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_lamp3_intensity(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP3.SET");
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP3.SET");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the mjd-obs keyword 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_mjdobs(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "MJD-OBS");
    } else {
        val = qfits_query_hdr(filename, "MJD-OBS");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the working mode   
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_mode(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.MODE");
    } else {
        val = qfits_query_hdr(filename, "INS.MODE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the NDIT keyword
            in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_ndit(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NDIT");
    } else {
        val = qfits_query_hdr(filename, "DET.NDIT");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the NDSAMPLES key
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_ndsamples(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NDSAMPLES");
    } else {
        val = qfits_query_hdr(filename, "DET.NDSAMPLES");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the number of expositions  
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_numbexp(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TPL.NEXP");
    } else {
        val = qfits_query_hdr(filename, "TPL.NEXP");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the objective name 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_objective(char * filename)
{
    char * mode ;
    char * val ;

    mode = isaac_get_mode(filename) ;

    /* The keyword used to store the objective is different for LW and SW */
    if (mode[0] == 'S') {
        if (qfits_is_paf_file(filename)) {
            val = qfits_paf_query(filename, "INS.OPTI2.NAME");
        } else {
            val = qfits_query_hdr(filename, "INS.OPTI2.NAME") ;
        }
    } else if (mode[0] == 'L') {
        if (qfits_is_paf_file(filename)) {
            val = qfits_paf_query(filename, "INS.OPTI3.NAME");
        } else {
            val = qfits_query_hdr(filename, "INS.OPTI3.NAME") ;
        }
    } else {
        return NULL ;
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OBS ID keyword 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_obs_id(char * filename)
{
    char        *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "OBS.ID") ;
    } else {
        val = qfits_query_hdr(filename, "OBS.ID");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OBS TARG NAME keyword 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_obs_targ_name(char * filename)
{
    char        *   val ;

    if (qfits_is_paf_file(filename)) {
        if ((val = qfits_paf_query(filename, "STAR.NAME")) == NULL) {
            val = qfits_paf_query(filename, "QC.STDNAME") ;
        }
    } else {
        val = qfits_query_hdr(filename, "OBS.TARG.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the optical ID 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_optical_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI1.ID");
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI1.ID");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store order. 
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_order(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.GRAT.ORDER");
    } else {
        val = qfits_query_hdr(filename, "INS.GRAT.ORDER");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the origfile   
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_origfile(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "ORIGFILE");
    } else {
        val = qfits_query_hdr(filename, "ORIGFILE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the PIXSCALE 
            keyword in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_pixscale(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.PIXSCALE");
    } else {
        val = qfits_query_hdr(filename, "INS.PIXSCALE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the pro catalog as defined by the DataFlow
  @param    filename    source FITS file
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_pro_catalog(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "QC.LAMP");
    } else {
        val = qfits_query_hdr(filename, "PRO.CATALOG");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the pro catg as defined by the DataFlow
  @param    filename    Name of a FITS or PAF file.
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_pro_catg(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "PRO.CATG");
    } else {
        val = qfits_query_hdr(filename, "PRO.CATG");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the RA keyword
            in an ISAAC header
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_ra(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "RA");
    } else {
        val = qfits_query_hdr(filename, "RA");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the ISAAC
            default keyword used to store the resolution name. Should
            be 'MR' or 'LR'
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_resolution(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.GRAT.NAME");
    } else {
        val = qfits_query_hdr(filename, "INS.GRAT.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the ID of the readout-mode used for a frame.
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_romode_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NCORRS");
    } else {
        val = qfits_query_hdr(filename, "DET.NCORRS");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the name of the readout-mode used for a frame.
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_romode_name(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.MODE.NAME");
    } else {
        val = qfits_query_hdr(filename, "DET.MODE.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the name of the readout-mode used for a frame.
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_romode_name2(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NCORRS.NAME");
    } else {
        val = qfits_query_hdr(filename, "DET.NCORRS.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the read speed
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_rspeed(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.RSPEED");
    } else {
        val = qfits_query_hdr(filename, "DET.RSPEED");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the template id    
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_templateid(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TPL.ID");
    } else {
        val = qfits_query_hdr(filename, "TPL.ID");
    }
    return qfits_pretty_string(val);
}




/*-------------------------------------------------------------------------*/
/**
  @brief    find out which wave band is active
  @param    filename    ISAAC FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * isaac_get_filter(char * filename)
{
    char    *   val1 ;
    char    *   val2 ;

    if (qfits_is_paf_file(filename)) {
        if ((val1 = qfits_paf_query(filename, "INS.FILTER.ID")) == NULL) {
            val1 = qfits_paf_query(filename, "QC.FILTER.OBS") ;
        }
    } else {
        val2 = isaac_get_arm(filename) ;
        if (val2 != NULL) {
            if (toupper(val2[0])=='S') {
                return isaac_get_filter_sw(filename);
            } else if (toupper(val2[0])=='L') {
                return isaac_get_filter_lw(filename);
            }
        } else return isaac_get_filter_sw(filename);
    }
    return qfits_pretty_string(val1) ;
}

