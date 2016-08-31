/*-------------------------------------------------------------------------*/
/**
   @file	key_naco.c
   @author	Y. Jung
   @date	Mar 2002
   @version	$Revision: 1.15 $
   @brief	NACO functions using FITS header keywords
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: key_naco.c,v 1.15 2004/01/07 16:37:11 yjung Exp $
	$Author: yjung $
	$Date: 2004/01/07 16:37:11 $
	$Revision: 1.15 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "qfits.h"
#include "keyfits.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    get rotation start from a NACO FITS file
  @param    filename    source FITS file
  @return   Char string containing what was found in header, NULL if error.
 */ 
/*--------------------------------------------------------------------------*/
char * naco_get_absrot_start(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "ADA.ABSROT.START") ;
    } else {
        val = qfits_query_hdr(filename, "ADA.ABSROT.START");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    get airmass start from a NACO FITS file
  @param    filename    source FITS file
  @return   Char string containing what was found in header, NULL if error.
 */ 
/*--------------------------------------------------------------------------*/
char * naco_get_airmass_start(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.AIRM.START") ;
    } else {
        val = qfits_query_hdr(filename, "TEL.AIRM.START");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    get airmass end from a NACO FITS file
  @param    filename    source FITS file
  @return   Char string containing what was found in header, NULL if error.
 */ 
/*--------------------------------------------------------------------------*/
char * naco_get_airmass_end(char * filename)
{
    char    *   val ;

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
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_arcfile(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "ARCFILE") ;
    } else {
        val = qfits_query_hdr(filename, "ARCFILE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the CUMOFFSETX
            keyword in an conica header
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_cumoffsetx(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "SEQ.CUMOFFSETX") ;
    } else {
        val = qfits_query_hdr(filename, "SEQ.CUMOFFSETX");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the CUMOFFSETY
            keyword in an conica header
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_cumoffsety(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "SEQ.CUMOFFSETY") ;
    } else {
        val = qfits_query_hdr(filename, "SEQ.CUMOFFSETY");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**     
  @brief    find out the date of observation  
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */ 
/*--------------------------------------------------------------------------*/
char * naco_get_date_obs(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DATE-OBS") ;
    } else {
        val = qfits_query_hdr(filename, "DATE-OBS");
    }
    return qfits_pretty_string(val);
}
       
/*-------------------------------------------------------------------------*/
/** 
  @brief    find out the DEC keyword in a NACO header
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_dec(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DEC") ;
    } else {
        val = qfits_query_hdr(filename, "DEC");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/** 
  @brief    find out the INS.DICH.POSNAM keyword in a NACO header
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_dich_posname(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.INS.DICH.POSNAM") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.INS.DICH.POSNAM");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the DIT keyword
            in an conica header
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_dit(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.DIT") ;
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
char * naco_get_dpr_catg(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DPR.CATG") ;
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
char * naco_get_dpr_tech(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DPR.TECH") ;
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
char * naco_get_dpr_type(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DPR.TYPE") ;
    } else {
        val = qfits_query_hdr(filename, "DPR.TYPE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the ECMEAN keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_ecmean(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.RTC.DET.DST.ECMEAN") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.RTC.DET.DST.ECMEAN");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the number of the current exposition  
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_expno(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TPL.EXPNO") ;
    } else {
        val = qfits_query_hdr(filename, "TPL.EXPNO");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the EXPTIME keyword
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_exptime(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "EXPTIME") ;
    } else {
        val = qfits_query_hdr(filename, "EXPTIME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the filter used in a NACO FITS frame.
  @param    filename    source FITS file
  @return   statically allocated char string, no need to free() it
 */
/*--------------------------------------------------------------------------*/
char * naco_get_filter(char * filename)
{
    char * val ;
    char * opti_id;

    if (qfits_is_paf_file(filename)) {
        return NULL ;
    }
    /* Get value for OPTI5.ID */
    val = qfits_query_hdr(filename, "INS.OPTI5.ID");
    if (val!=NULL) {
        opti_id = qfits_pretty_string(val);
        if (strcmp(opti_id, "empty")) {
            /* OPTI5.ID is non-empty */
            return opti_id ;
        }
    }

    /* Get value for OPTI6.ID */
    val = qfits_query_hdr(filename, "INS.OPTI6.ID");
    if (val!=NULL) {
        opti_id = qfits_pretty_string(val);
        if (strcmp(opti_id, "empty")) {
            /* OPTI6.ID is non-empty */
            return opti_id ;
        }
    }

    /* Get value for OPTI5.ID */
    val = qfits_query_hdr(filename, "INS.OPTI4.ID");
    if (val!=NULL) {
        opti_id = qfits_pretty_string(val);
        if (strcmp(opti_id, "empty")) {
            /* OPTI4.ID is non-empty */
            return opti_id ;
        }
    }
    return NULL ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the FLUXMEAN keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_fluxmean(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.RTC.DET.DST.FLUXMEAN") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.RTC.DET.DST.FLUXMEAN");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the FOCUS keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_focus(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.INS.FOCU.ABSPOS") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.INS.FOCU.ABSPOS");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the frame type   
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_frame_type(char * filename)
{
	char	*	val ;

    if (qfits_is_paf_file(filename)) {
        if ((val = qfits_paf_query(filename, "DET.FRAME.TYPE")) == NULL) {
		    /* DET.FRAME moved to DET.FRAM in DICB on 28-07-2001 */
            val = qfits_paf_query(filename, "DET.FRAM.TYPE") ;
        }
    } else {
        if ((val = qfits_query_hdr(filename, "DET.FRAME.TYPE")) == NULL) {
		    /* DET.FRAME moved to DET.FRAM in DICB on 28-07-2001 */
            val = qfits_query_hdr(filename, "DET.FRAM.TYPE") ;
        }
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
char * naco_get_humidity_level(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TEL.AMBI.RHUM") ;
    } else {
        val = qfits_query_hdr(filename, "TEL.AMBI.RHUM");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the instrument
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_instrument(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INSTRUME") ;
    } else {
        val = qfits_query_hdr(filename, "INSTRUME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the L0MEAN keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_l0mean(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.RTC.DET.DST.L0MEAN") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.RTC.DET.DST.L0MEAN");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the INS.LAMP2.CURRENT keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_lamp2_cur(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP2.CURRENT") ;
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP2.CURRENT");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the INS.LAMP2.NAME keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_lamp2_name(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP2.NAME") ;
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP2.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the INS.LAMP2.SET keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_lamp2(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP2.SET") ;
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP2.SET");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the INS.LAMP2.TYPE keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_lamp2_type(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.LAMP2.TYPE") ;
    } else {
        val = qfits_query_hdr(filename, "INS.LAMP2.TYPE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the mjd-obs keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_mjdobs(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "MJD-OBS") ;
    } else {
        val = qfits_query_hdr(filename, "MJD-OBS");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to DET.MODE.NAME
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_mode(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.MODE.NAME") ;
    } else {
        val = qfits_query_hdr(filename, "DET.MODE.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the NDIT keyword
            in an conica header
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_ndit(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NDIT") ;
    } else {
        val = qfits_query_hdr(filename, "DET.NDIT");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the DET NDSAMPLES keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_ndsamples(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NDSAMPLES") ;
    } else {
        val = qfits_query_hdr(filename, "DET.NDSAMPLES");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the number of expositions  
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_numbexp(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TPL.NEXP") ;
    } else {
        val = qfits_query_hdr(filename, "TPL.NEXP");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OBS ID keyword 
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_obs_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "OBS.ID") ;
    } else {
        val = qfits_query_hdr(filename, "OBS.ID");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OPTI1 ID    
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti1_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI1.ID") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI1.ID");
    }
    return qfits_pretty_string(val);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OPTI3 ID    
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti3_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI3.ID") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI3.ID");
    }
    return qfits_pretty_string(val);
}


/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OPTI7 ID    
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti7_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI7.ID") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI7.ID");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OPTI4 ID    
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti4_id(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI4.NAME") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI4.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OPTI3.NAME keyword 
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti3_name(char * filename)
{
    char        *   val ;
    
    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI3.NAME") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI3.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OPTI7.NAME keyword 
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti7_name(char * filename)
{
    char        *   val ;
    
    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI7.NAME") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI7.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the OBS ID keyword 
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_opti7_no(char * filename)
{
    char        *   val ;
    
    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.OPTI7.NO") ;
    } else {
        val = qfits_query_hdr(filename, "INS.OPTI7.NO");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the PIXSCALE 
            keyword in an conica header
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_pixscale(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.PIXSCALE") ;
    } else {
        val = qfits_query_hdr(filename, "INS.PIXSCALE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the R0MEAN keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_r0mean(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.RTC.DET.DST.R0MEAN") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.RTC.DET.DST.R0MEAN");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the RA keyword in a NACO header
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_ra(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "RA") ;
    } else {
        val = qfits_query_hdr(filename, "RA");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the INS CON XREFZERO keyword 
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_refzerox(char * filename)
{
    char        *   val ;
    
    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.CON.XREFZERO") ;
    } else {
        val = qfits_query_hdr(filename, "INS.CON.XREFZERO");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the INS CON YREFZERO keyword 
  @param    filename    NACO FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_refzeroy(char * filename)
{
    char        *   val ;
    
    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "INS.CON.YREFZERO") ;
    } else {
        val = qfits_query_hdr(filename, "INS.CON.YREFZERO");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to the DET.NCORRS key.
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_rom(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NCORRS") ;
    } else {
        val = qfits_query_hdr(filename, "DET.NCORRS");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the character string associated to DET.NCORRS.NAME
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
 */
/*--------------------------------------------------------------------------*/
char * naco_get_rom_name(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "DET.NCORRS.NAME") ;
    } else {
        val = qfits_query_hdr(filename, "DET.NCORRS.NAME");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the T0MEAN keyword 
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_t0mean(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.RTC.DET.DST.T0MEAN") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.RTC.DET.DST.T0MEAN");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the template id    
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_templateid(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "TPL.ID") ;
    } else {
        val = qfits_query_hdr(filename, "TPL.ID");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the WFS MODE keyword  
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_wfs_mode(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.OCS.WFS.MODE") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.OCS.WFS.MODE");
    }
    return qfits_pretty_string(val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    find out the WFS TYPE keyword   
  @param    filename    conica FITS file name
  @return   pointer to statically allocated character string
*/
/*--------------------------------------------------------------------------*/
char * naco_get_wfs_type(char * filename)
{
    char    *   val ;

    if (qfits_is_paf_file(filename)) {
        val = qfits_paf_query(filename, "AOS.OCS.WFS.TYPE") ;
    } else {
        val = qfits_query_hdr(filename, "AOS.OCS.WFS.TYPE");
    }
    return qfits_pretty_string(val);
}

