/*----------------------------------------------------------------------------*/
/**
   @file    prokeys.c
   @author  Y. Jung
   @date    July 2000
   @version	$Revision: 1.9 $
   @brief   ISAAC common fonctions to write (in) produced files
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: prokeys.c,v 1.9 2002/12/10 16:42:16 yjung Exp $
	$Author: yjung $
	$Date: 2002/12/10 16:42:16 $
	$Revision: 1.9 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "isaacp_lib.h"
#include "prokeys.h"

/*-----------------------------------------------------------------------------
      							Define
 -----------------------------------------------------------------------------*/

#define FITSLINESZ                      80

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Create a first version of an output image header
  @param    fh		input fits header
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int isaac_header_for_image(qfits_header * fh)
{
	if (fh==NULL) return -1 ;
	return 0 ;
}	

/*----------------------------------------------------------------------------*/
/**
  @brief    Create a first version of an output table header
  @param    fh		input fits header
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int isaac_header_for_table(qfits_header * fh)
{
	char            *   date ;
	char                str_val[FITSLINESZ] ;

	if (fh==NULL) return -1 ;
	
   /* Modify the header to fit a table */

    /* Update BITPIX */
    qfits_header_mod(fh, "BITPIX", "8", "8-bits unsigned integers") ;
    /* Update NAXIS */
    qfits_header_mod(fh, "NAXIS", "0", "Empty Prime data matrix") ;
    /* Write EXTEND */
    qfits_header_add_after(fh, "NAXIS", "EXTEND", "T",
                            "FITS extension may be present", NULL) ;
    /* Write BLOCKED */
    qfits_header_add_after(fh, "EXTEND", "BLOCKED", "T",
                            "FITS file may be blocked", NULL) ;
    /* Update ORIGIN */
    qfits_header_mod(fh, "ORIGIN", "ESO-ECLIPSE", "Written by ECLIPSE") ;

    /* Add DATAMD5 */
    qfits_header_del(fh, "DATAMD5");
    qfits_header_add_after(fh,"ORIGIN","DATAMD5","'0'","MD5 checksum",NULL);
    /* Update DATE */
    date = get_datetime_iso8601() ;
    sprintf(str_val, "'%s'", date) ;
    qfits_header_mod(fh, "DATE", str_val, "[UTC] Date of writting") ;

    /* Some specific image kewords have to be commented to keep this  */
    /* TFITS file readable by MIDAS */
    qfits_header_add_after(fh, "NAXIS1", "COMMENT",
            qfits_header_getline(fh, "NAXIS1"), NULL, NULL) ;
    qfits_header_del(fh, "NAXIS1") ;
    qfits_header_add_after(fh, "NAXIS2", "COMMENT",
            qfits_header_getline(fh, "NAXIS2"), NULL, NULL) ;
    qfits_header_del(fh, "NAXIS2") ;
    qfits_header_add_after(fh, "CRVAL1", "COMMENT",
            qfits_header_getline(fh, "CRVAL1"), NULL, NULL) ;
    qfits_header_del(fh, "CRVAL1") ;
    qfits_header_add_after(fh, "CRVAL2", "COMMENT",
            qfits_header_getline(fh, "CRVAL2"), NULL, NULL) ;
    qfits_header_del(fh, "CRVAL2") ;
    qfits_header_add_after(fh, "CRPIX1", "COMMENT",
            qfits_header_getline(fh, "CRPIX1"), NULL, NULL) ;
    qfits_header_del(fh, "CRPIX1") ;
    qfits_header_add_after(fh, "CRPIX2", "COMMENT",
            qfits_header_getline(fh, "CRPIX2"), NULL, NULL) ;
    qfits_header_del(fh, "CRPIX2") ;
    qfits_header_add_after(fh, "CDELT1", "COMMENT",
            qfits_header_getline(fh, "CDELT1"), NULL, NULL) ;
    qfits_header_del(fh, "CDELT1") ;
    qfits_header_add_after(fh, "CDELT2", "COMMENT",
            qfits_header_getline(fh, "CDELT2"), NULL, NULL) ;
    qfits_header_del(fh, "CDELT2") ;
    qfits_header_add_after(fh, "CTYPE1", "COMMENT",
            qfits_header_getline(fh, "CTYPE1"), NULL, NULL) ;
    qfits_header_del(fh, "CTYPE1") ;
    qfits_header_add_after(fh, "CTYPE2", "COMMENT",
            qfits_header_getline(fh, "CTYPE2"), NULL, NULL) ;
    qfits_header_del(fh, "CTYPE2") ;
    qfits_header_add_after(fh, "CROTA1", "COMMENT",
            qfits_header_getline(fh, "CROTA1"), NULL, NULL) ;
    qfits_header_del(fh, "CROTA1") ;
    qfits_header_add_after(fh, "CROTA2", "COMMENT",
            qfits_header_getline(fh, "CROTA2"), NULL, NULL) ;
    qfits_header_del(fh, "CROTA2") ;
    qfits_header_add_after(fh, "PC001001", "COMMENT",
            qfits_header_getline(fh, "PC001001"), NULL, NULL) ;
    qfits_header_del(fh, "PC001001") ;
    qfits_header_add_after(fh, "PC001002", "COMMENT",
            qfits_header_getline(fh, "PC001002"), NULL, NULL) ;
    qfits_header_del(fh, "PC001002") ;
    qfits_header_add_after(fh, "PC002001", "COMMENT",
            qfits_header_getline(fh, "PC002001"), NULL, NULL) ;
    qfits_header_del(fh, "PC002001") ;
    qfits_header_add_after(fh, "PC002002", "COMMENT",
            qfits_header_getline(fh, "PC002002"), NULL, NULL) ;
    qfits_header_del(fh, "PC002002") ;

	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Add HISTORY entries in the FITS header 
  @param    fh		Fits header
  @param    filenames   list of files to write
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int isaac_add_files_history(qfits_header * fh, framelist * filenames)
{
	char    value[73] ;
	int		i ;
	
	if (fh==NULL || filenames==NULL) return -1 ;

	strncpy(value, get_dirname(filenames->name[0]), 72) ;
	value[72] = (char)0 ;
	qfits_header_add(fh, "HISTORY", value, "files path", NULL) ;

	for (i=0 ; i<filenames->n ; i++) {
		strncpy(value, get_basename(filenames->name[i]), 72) ;
		value[72] = (char)0 ;
		qfits_header_add(fh, "HISTORY", value, NULL, NULL) ;
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Fill up a given FITS header with PRO keywords.
  @param    fh              fits header in which values are written
  @param    filename        produced file name
  @param    pro_type        product type
  @param    pro_redlevel    product reduction level
  @param    pro_catg	    product category key
  @param    pro_status      product status
  @param    pro_rec_id      
  @param    pro_datancom    number of reduced files
  @param	rawfiles		list of input raw files
  @param	calibfiles		list of calibration files
  @return   0 if ok, -1 otherwise
    DFS only. See the DICB dictionaries to have details on the keywords
 */
/*----------------------------------------------------------------------------*/
int isaac_pro_fits(
        qfits_header    *   fh,
        char            *   pipefile,
        char            *   pro_type,
        char            *   pro_redlevel,
        procat       	   	pro_catg,
        char            *   pro_status,
        char            *   pro_rec_id,
        int                 pro_datancom,
        framelist		*	rawfiles,
        framelist		*	calibfiles)
{
    char                cval[80];
    char                cval2[80];
	int			        nb ;
    instrument_t        ins ;

	char	        *	arcfile ;
	int			        i ;
	
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    
	/* Find the new nb in RECnb */
	nb = 1 ;
	
	if (fh==NULL) return -1 ;
    
	/* Parameter Name:    PIPEFILE */
	if (pipefile!=NULL) {
        qfits_header_add(fh, "PIPEFILE", get_basename(pipefile),
                        "pipeline filename", NULL);
    }

    /* Parameter Name:    PRO TYPE */
	/* Value: "TEMPORARY", "PREPROCESSED", "REDUCED" or "QCPARAM". */
    if (pro_type!=NULL) {
        qfits_header_add(fh,
						 "HIERARCH ESO PRO TYPE",
						 pro_type,
						 "product type",
						 NULL);
    }

    /* Parameter Name:    PRO REDLEVEL */
	/* Value: "DETECTOR", "GEOMETRIC" or "PHOTOMETRIC" */
    if (pro_redlevel!=NULL) {
        qfits_header_add(fh, "HIERARCH ESO PRO REDLEVEL",
                        pro_redlevel, "reduction level", NULL);
    }

	/* Parameter Name:    PRO STATUS */
	/* Valid fields are "OK" or "FAILED". */
    if (pro_status!=NULL) {
        qfits_header_add(fh, "HIERARCH ESO PRO STATUS",
                        pro_status, "frame pipeline status", NULL);
    }

   /* Parameter Name:    PRO DATE */
    qfits_header_add(fh, "HIERARCH ESO PRO DATE",
                    get_date_iso8601(), "pipeline execution date", NULL);

	/* Parameter Name:    PRO DATANCOM */
    if (pro_datancom>0) {
        sprintf(cval, "%d", pro_datancom);
        qfits_header_add(fh, "HIERARCH ESO PRO DATANCOM",
                        cval, "# of combined frames", NULL);
    }

	/* Parameter Name:    PRO CATG */
    qfits_header_add(fh, "HIERARCH ESO PRO CATG",
            pfits_getprokey(ins, pro_catg), "product category", NULL);

	/* Parameter Name:    PRO RECi ID */
    if (pro_rec_id!=NULL) {
		sprintf(cval, "HIERARCH ESO PRO REC%d ID", nb) ;
        qfits_header_add(fh, cval, pro_rec_id, "recipe ID", NULL);
    }

    /* Parameter Name:    PRO RECi DRS ID */
    sprintf(cval2, "eclipse-%s", get_eclipse_version());
	sprintf(cval, "HIERARCH ESO PRO REC%d DRS ID", nb) ;
    qfits_header_add(fh, cval, cval2, "data reduction system ID", NULL);


	/* Raw files */
	if (rawfiles != NULL) {
		for (i=0 ; i<rawfiles->n ; i++) {
			sprintf(cval, "HIERARCH ESO PRO REC%d RAW%d NAME", nb, i+1) ;
			if ((arcfile = pfits_get(ins, rawfiles->name[i], 
                            "arcfile")) != NULL) {
				qfits_header_add(fh, cval, arcfile, NULL, NULL) ;
			}
			if (rawfiles->type != NULL) {
				if (rawfiles->type[i] != NULL) {
					sprintf(cval, "HIERARCH ESO PRO REC%d RAW%d CATG", nb, i+1);
					qfits_header_add(fh, cval, rawfiles->type[i], NULL, NULL) ;
				}
			}
		}
	}
	
	/* Calibration files */
	if (calibfiles != NULL) {
		for (i=0 ; i<calibfiles->n ; i++) {
			sprintf(cval, "HIERARCH ESO PRO REC%d RAW%d NAME", nb, i+1) ;
			if ((arcfile = pfits_get(ins, calibfiles->name[i], 
                            "arcfile")) != NULL) {
				qfits_header_add(fh, cval, arcfile, NULL, NULL) ;
			}
			if (calibfiles->type != NULL) {
				if (calibfiles->type[i] != NULL) {
					sprintf(cval, "HIERARCH ESO PRO REC%d RAW%d CATG", nb, i+1);
					qfits_header_add(fh, cval, calibfiles->type[i], NULL, NULL);
				}
			}
		}
	}
	
    return 0 ;
}

