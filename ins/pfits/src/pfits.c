/*----------------------------------------------------------------------------*/
/**
   @file	pfits.c
   @author	N. Devillard
   @date	Mar 2002
   @version	$Revision: 1.15 $
   @brief	Protected FITS keyword read.

   This module handles protected access to FITS headers, i.e. when
   a request in a FITS header is issued, the requested keyword is
   looked for in a table associated to every supported instrument.
   If a match is found, the keyword will be obtained using a dedicated
   function, otherwise a direct FITS header query will be issued.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: pfits.c,v 1.15 2004/02/09 16:02:43 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:02:43 $
	$Revision: 1.15 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "pfits.h"
#include "keyfits.h"
#include "key_isaac.h"
#include "key_naco.h"

#include "qfits.h"

/*-----------------------------------------------------------------------------
   								Defines
 -----------------------------------------------------------------------------*/

#define PFITS_DEBUG   0

#if PFITS_DEBUG
#define pfits_debug( code ) { code }
#else
#define pfits_debug( code )
#endif

#define PFITS_ERRORSZ       256
#define PFITS_STATICBUFS    10

/*-----------------------------------------------------------------------------
                            Private variables
 -----------------------------------------------------------------------------*/

/*
 * List of known instruments and associated key lists
 */
static struct {
    char         *  name ;
    instrument_t    insid ;
    keyfits      *  list ;
} pfits_inslist[] =
{
    {"isaac", {instrument_isaac, insmode_none}, keylist_isaac},
    {"naco",  {instrument_naco,  insmode_none}, keylist_naco},
    {NULL,    {instrument_none,  insmode_none}, NULL}
};

/** This flag makes sure initialization occurs only once */
static int pfits_initialized=0 ;

/** This string contains the latest pfits error */
static char pfits_errorstr[PFITS_ERRORSZ] ;


/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

/*
 * Switch a string to lowercase
 */
static char * pfits_lowercase(char * key)
{
    static char lwc[FITS_LINESZ+1];
    int         i ;

    if (key==NULL)
        return NULL ;
    memset(lwc, 0, FITS_LINESZ+1);
    i=0 ;
    while (key[i]) {
        lwc[i]=(char)tolower(key[i]);
        i++;
    }
    return lwc ;
}

/*
 * Compute a simple hash on a string.
 */
static unsigned pfits_hash(char * key)
{
    int         len ;
    unsigned    hash ;
    int         i ;

    len = (int)strlen(key);
    for (hash=0, i=0 ; i<len ; i++) {
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
    return hash ;
}

/*
 * Initialize all instrument lists of keys by computing the
 * hash value for all stored strings.
 */
static void pfits_init(void)
{
	int i, j ;
    i=0 ;
    pfits_debug(
        fprintf(stderr, "pfits: initializing lists... ");
    );
    /* Compute hash for all stored keys */
	while (pfits_inslist[i].list!=NULL) {
        j=0 ;
        while (pfits_inslist[i].list[j].name!=NULL) {
            pfits_inslist[i].list[j].hash =
                pfits_hash(pfits_inslist[i].list[j].name);
            j++ ;
        }
        i++ ;
	}
    pfits_debug(
        fprintf(stderr, "done\n");
    );
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Get the latest error string from this module.
  @return   1 pointer to a statically allocated string in this module.

  This function returns a pointer to a statically allocated string
  within this module, describing the latest error that occurred.
 */
/*----------------------------------------------------------------------------*/
char * pfits_error(void)
{
    return pfits_errorstr ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get a FITS value from a FITS or PAF file.
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
char * pfits_get(instrument_t ins, char * filename, char * key)
{
    int         i ;
    unsigned    hkey ;
    char    *   lw_key ;
    char    *   val ;
    int         list_rank ;
    keyfits *   list ;

    /* Internal rotating static string buffers */
    static char pfits_getbuf[PFITS_STATICBUFS][FITS_LINESZ+1];
    static int  flip=0 ;
    char    *   val_buf ;

    pfits_debug(
        fprintf(stderr,
                "pfits: pfits_get\n"
                "file: %s\n"
                "key : %s\n",
                filename ? filename : "null",
                key ? key : "null");
    );
    if (filename==NULL || key==NULL) return NULL ;

    /* Initialize all key tables if needed */
	if (pfits_initialized==0) {
		pfits_initialized++ ;
		pfits_init();
	}

    /* Reset error string */
    memset(pfits_errorstr, 0, PFITS_ERRORSZ);

	/* Identify instrument list */
    list=NULL ;
    list_rank=0 ;
    while (pfits_inslist[list_rank].list!=NULL) {
        if (pfits_inslist[list_rank].insid.ins == ins.ins) {
            /* Found matching list */
            list = pfits_inslist[list_rank].list ;
            pfits_debug(
                fprintf(stderr, "pfits: found list [%s]\n",
                        pfits_inslist[list_rank].name);
            );
            break ;
        }
        list_rank++;
    }
    if (list==NULL) {
        /*
         * Instrument cannot be identified, get the key
         * directly from FITS header.
         */
        pfits_debug(
            fprintf(stderr, "pfits: no list found - direct header query\n");
        );
        val = qfits_query_hdr(filename, key);
        if (val==NULL) {
            sprintf(pfits_errorstr,
                    "pfits: cannot find key [%s] in %s",
                    key,
                    filename);
            return NULL ;
        } else {
            val_buf = pfits_getbuf[flip];
            flip++ ;
            if (flip==PFITS_STATICBUFS)
                flip=0 ;
            strcpy(val_buf, qfits_pretty_string(val));
            return val_buf ;
        }
	}

    /*
     * Compute hash for requested key
     * Work on lowercase version of input string.
     */
    lw_key = pfits_lowercase(key);
    hkey = pfits_hash(lw_key);

    /* Locate key in list */
    pfits_debug(
        fprintf(stderr, "pfits: locating key [%s] in list: %s...\n",
                key,
                pfits_inslist[list_rank].name);
    );
    i=0 ;
    while (list[i].name!=NULL) {
        if (list[i].hash == hkey) {
            if (!strcmp(list[i].name, lw_key)) {
                break ;
            }
        }
        i++ ;
    }
    if (list[i].name==NULL) {
        /* No matching key in given list, return direct FITS query */
        pfits_debug(
            fprintf(stderr,
                    "pfits: no key matching [%s] - direct header query\n",
                    key);
        );
        val = qfits_query_hdr(filename, key);
        if (val==NULL) {
            sprintf(pfits_errorstr,
                    "pfits: cannot find key [%s] in %s",
                    key,
                    filename);
            return NULL ;
        } else {
            val = qfits_pretty_string(val);
        }
    } else {
        /* Call getter function */
        pfits_debug(
            fprintf(stderr,
                    "pfits: calling getter for (%s)get(%s)(%s)\n",
                    pfits_inslist[list_rank].name,
                    list[i].name,
                    key);
        );
        val = list[i].get(filename);
        if (val==NULL) {
            sprintf(pfits_errorstr,
                    "pfits: cannot find key (%s)get(%s) in %s",
                    pfits_inslist[list_rank].name,
                    key,
                    filename);
            return NULL ;
        }
    }

    /* Use rotating static string buffer for results */
    val_buf = pfits_getbuf[flip];
    flip++ ;
    if (flip==PFITS_STATICBUFS)
        flip=0 ;
    strcpy(val_buf, val);
    return val_buf ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Identify instrument data type.
  @param    filename        Name of the file to examine.
  @return   1 instrument_t object

  This function examines a given FITS file and recognizes the instrument
  that was used to acquire it.
 */
/*----------------------------------------------------------------------------*/
instrument_t pfits_identify_insstr(char * name)
{
    instrument_t    ins ;

    ins.ins = instrument_none ;
    ins.mode = insmode_none ;

    if (name==NULL)
        return ins ;

    /* ISAAC keyword is 'ISAAC' */
    if (!strncmp(name, "isaac", 5)) {
        ins.ins = instrument_isaac ;
    } else if (!strncmp(name, "sofi", 4)) {
        /* SOFI follows the same rules as ISAAC */
        ins.ins = instrument_isaac ;
    } else if (!strncmp(name, "conica", 6)) {
        /* NAOS+CONICA could be 'CONICA' or 'NAOS+CONICA' or 'NACO' */
        ins.ins = instrument_naco ;
    } else if (!strncmp(name, "naos", 4)) {
        ins.ins = instrument_naco ;
    } else if (!strncmp(name, "naco", 4)) {
        ins.ins = instrument_naco ;
    }
    return ins ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Identify instrument data type.
  @param    filename        Name of the file to examine.
  @return   1 instrument_t object

  This function examines a given FITS file and recognizes the instrument
  that was used to acquire it.
 */
/*----------------------------------------------------------------------------*/
instrument_t pfits_identify_ins(char * filename)
{
    char            *   sval ;
    char            *   lc_name ;
    instrument_t        ins ;
   
    /* First get the ins field */
    /* Test input */
    if (filename == NULL) {
        lc_name = NULL ;
    } else {
        /* Read the INSTRUME keyword frome the input file */
        sval = qfits_query_hdr(filename, "INSTRUME");
        if (sval==NULL) {
            lc_name = NULL ;
        } else {
            lc_name = pfits_lowercase(qfits_pretty_string(sval));
        }
    }
    ins = pfits_identify_insstr(lc_name);
    /* At this point, ins contains the instrument name, not the mode */
    
    /* Get the mode */
    sval = pfits_get(ins, filename, "chopping_status");
    if (sval!=NULL) {
        if (sval[0]=='T')      ins.mode = insmode_chop ;
        else if (sval[0]=='F') ins.mode = insmode_nochop ; 
    }
    return ins ;
}



