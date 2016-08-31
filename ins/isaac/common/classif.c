/*----------------------------------------------------------------------------*/
/**
   @file	classif.c
   @author	Y. Jung
   @date	July 2000
   @version	$Revision: 1.18 $ 
   @brief	ISAAC common functions for frames classification
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: classif.c,v 1.18 2003/01/13 09:41:17 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/13 09:41:17 $
	$Revision: 1.18 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define INLINESZ		1024

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    compare the grating, the slit and the wl in input files headers 
  @param    file1   first file
  @param    file2   second file
  @return   -1 in error case, 0 for different settings, 1 for equal settings

  Used in sp_flat and arc
 */
/*----------------------------------------------------------------------------*/
int compare_settings(
        char    *   file1,
        char    *   file2)
{
    int             comparison ;
    double          wl1,
                    wl2 ;
    char            grat_name1[FILENAMESZ] ;
    char            grat_name2[FILENAMESZ] ;
    char            opti_id1[FILENAMESZ] ;
    char            opti_id2[FILENAMESZ] ;
    instrument_t    ins ;
    char        *   s ;
        
    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    comparison = 1 ;    
        
    /* Get the slit used */
    if ((s = pfits_get(ins, file1, "optical_id")) == NULL) {
        e_error("cannot get optical id from [%s]", file1) ;
        return -1 ; 
    }               
    strcpy(opti_id1, s) ;
    if ((s = pfits_get(ins, file2, "optical_id")) == NULL) {
        e_error("cannot get optical id from [%s]", file2) ;
        return -1 ;
    }
    strcpy(opti_id2, s) ;

    if (strcmp(opti_id1, opti_id2) != 0) comparison = 0 ;

    /* Get the grating name */
    if (comparison == 1) {
        if ((s = pfits_get(ins, file1, "resolution")) == NULL) { 
            e_error("cannot get resolution from [%s]", file1) ;
            return -1 ;
        }
        strcpy(grat_name1, s) ;
        if ((s = pfits_get(ins, file2, "resolution")) == NULL) { 
            e_error("cannot get resolution from [%s]", file2) ;
            return -1 ;
        }
        strcpy(grat_name2, s) ;

        if (strcmp(grat_name1, grat_name2) != 0) comparison = 0 ;
    }

    /* Compare the central wavelength */
    if (comparison == 1) {
        wl1 = isaac_get_central_wavelength(file1) ;
        if (wl1 == -1.00) {
            e_error("cannot get central wavelength from [%s]", file1) ;
            return -1 ;
        }
        wl2 = isaac_get_central_wavelength(file2) ;
        if (wl2 == -1.00) {
            e_error("cannot get entral wavelength from [%s]", file2) ;
            return -1 ;
        }
		if (fabs(wl1 - wl2) > 1e-4) comparison = 0 ;
    }

    return comparison ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Filter half-cycle frames out of a frame list.
  @param	flist	Framelist object to purge.
  @return	int 0 if Ok, -1 otherwise.
  This function processes a framelist object to remove any half-cycle
  frame. These frames are only expected in long-wavelength mode and they
  can be detected in two ways. Either the frame type in the framelist
  contains the word 'half' (case-insensitive), or the frame name
  corresponds to a FITS file that is referenced as a half-cycle frame.

  Used in zpoint
 */
/*----------------------------------------------------------------------------*/
int isaac_lw_filter_halfcycle(framelist ** flist)
{
	framelist	*	purged ;
	int				i, j ;
	int				nval ;
	int			*	frame_ok ;
	char		*	str ;
    instrument_t    ins ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	
    /* Test inputs */
    if (flist==NULL || (*flist)==NULL) return -1 ;

	frame_ok = malloc((*flist)->n * sizeof(int));
	nval = (*flist)->n ;
	/* Try rejecting frames based on frame type */
	if ((*flist)->type!=NULL) {
		for (i=0 ; i<(*flist)->n ; i++) {
			frame_ok[i] = 1 ;
			if ((*flist)->type[i]!=NULL) {
				if (strstr((*flist)->type[i], "half")!=NULL) {
					frame_ok[i]=0 ;
					nval -- ;
				}
			}
		}
	} else {
		/* Try rejecting frames based on detector ID keyword */
		for (i=0 ; i<(*flist)->n ; i++) {
			frame_ok[i] = 1 ;
			str = pfits_get(ins, (*flist)->name[i], "detector_frame_type");
			if (str!=NULL) {
				if (!strncmp(str, "HALF", 4) ||
                   (!strncmp(str, "HCYCLE", 6))) {
					frame_ok[i]=0 ;
					nval -- ;
				}
			}
		}
	}

	/* If all frames Ok, return now */
	if (nval==(*flist)->n) {
		free(frame_ok);
		return 0 ;
	}

	/* Purge framelist if needed */
	purged = calloc(1, sizeof(framelist));
	purged->filename = strdup((*flist)->filename);
	purged->n = nval ;
	purged->name = calloc(nval, sizeof(char*));
	if ((*flist)->type!=NULL)   purged->type = calloc(nval, sizeof(char*));
	else                        purged->type = NULL ;

	j=0 ;
	for (i=0 ; i<(*flist)->n ; i++) {
		if (frame_ok[i]) {
			purged->name[j] = strdup((*flist)->name[i]);
			if (purged->type!=NULL) {
				if ((*flist)->type[i]!=NULL) {
					purged->type[j] = strdup((*flist)->type[i]);
				}
			}
			j++ ;
		}
	}
	free(frame_ok);
	framelist_del(*flist);
	(*flist) = purged ;
	return 0 ;
}


