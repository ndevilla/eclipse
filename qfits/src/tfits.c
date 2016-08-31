/*----------------------------------------------------------------------------*/
/**
   @file	tfits.c
   @author	Y. Jung
   @date	July 1999
   @version	$Revision: 1.60 $
   @brief
   FITS table handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: tfits.c,v 1.60 2005/07/20 11:54:42 yjung Exp $
	$Author: yjung $
	$Date: 2005/07/20 11:54:42 $
	$Revision: 1.60 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "tfits.h"

#include "ieeefp-compat.h"
#include "fits_std.h"
#include "byteswap.h"
#include "simple.h"
#include "t_iso8601.h"
#include "config.h"
#include "fits_rw.h"
#include "fits_md5.h"
#include "xmemory.h"
#include "qerror.h"

/*-----------------------------------------------------------------------------
   								Define
 -----------------------------------------------------------------------------*/

#define ELEMENT_MAX_DISPLAY_SIZE    50

/*-----------------------------------------------------------------------------
   							Function prototypes
 -----------------------------------------------------------------------------*/

static char * qfits_bintable_field_to_string(qfits_table *, int, int, int) ;
static char * qfits_asciitable_field_to_string(qfits_table *, int, int, int) ;
static char * qfits_build_format(qfits_col *) ;
static int qfits_table_append_bin_xtension(FILE *, qfits_table *, void **) ;
static int qfits_table_append_ascii_xtension(FILE *, qfits_table *, void **) ;
static int qfits_table_append_data(FILE *, qfits_table *, void **) ;
static int qfits_table_get_field_size(int, qfits_col *) ;
static int qfits_table_interpret_type(char *, int *, int*, tfits_type *, int) ;
static char * qfits_strstrip(char *);
static double qfits_str2dec(char *, int) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Identify a file as containing a FITS table in extension.
  @param	filename	Name of the FITS file to examine.
  @param	xtnum		Extension number to check (starting from 1).
  @return	int 1 if the extension contains a table, 0 else.
  Examines the requested extension and identifies the presence of a FITS table.
 */
/*----------------------------------------------------------------------------*/
int qfits_is_table(char * filename, int xtnum)
{
	char	*	value ;
	int			ttype ;
	
	ttype = QFITS_INVALIDTABLE ;
	value = qfits_query_ext(filename, "XTENSION", xtnum);
	if (value==NULL) return ttype ;
	
	value = qfits_pretty_string(value);
	if (!strcmp(value, "TABLE")) {
		ttype = QFITS_ASCIITABLE;
	} else if (!strcmp(value, "BINTABLE")) {
		ttype = QFITS_BINTABLE;
    }
	return ttype;
}	

/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a default primary header to store tables 	
  @return	the header object	
 */
/*----------------------------------------------------------------------------*/
qfits_header * qfits_table_prim_header_default(void)
{
	qfits_header	*	fh ;

	fh = qfits_header_new() ;

	qfits_header_append(fh, "SIMPLE", "T", "Standard FITS file", NULL) ;
	qfits_header_append(fh, "BITPIX", "8", "ASCII or bytes array", NULL) ;
	qfits_header_append(fh, "NAXIS", "0", "Minimal header", NULL) ;
	qfits_header_append(fh, "EXTEND", "T", "There may be FITS ext", NULL);
	qfits_header_append(fh, "BLOCKED", "T", "The file may be blocked", NULL) ;
	qfits_header_append(fh, "END", NULL, NULL, NULL) ;

	return fh ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a default extension header to store tables
  @return   the header object
 */
/*----------------------------------------------------------------------------*/
qfits_header * qfits_table_ext_header_default(qfits_table * t) 
{
    qfits_header    *   fh ;
	qfits_col       *   curr_col ;
    char                str_val[FITS_LINESZ] ;
    char                str_val2[FITS_LINESZ] ;
    char            *   date ;
    int                 tab_width ;
    int                 col_pos ;
    int                 i ;

    /* Compute the table width   */
    if ((tab_width = qfits_compute_table_width(t)) == -1) {
        qfits_error("cannot get the table width") ;
        return NULL ;
    }

	/* Create fits header */
	if ((fh=qfits_header_new()) == NULL) {
		qfits_error("cannot create new fits header") ;
		return NULL ;
	}

    /* Check the kind of table */
    if (t->tab_t == QFITS_BINTABLE) {
       
        /* Write extension header */
        qfits_header_append(fh, "XTENSION", "BINTABLE", 
                "FITS Binary Table Extension", NULL) ;
        qfits_header_append(fh, "BITPIX", "8", "8-bits character format", NULL);
        qfits_header_append(fh, "NAXIS", "2","Tables are 2-D char. array",NULL);
        sprintf(str_val, "%d", tab_width) ;
        qfits_header_append(fh, "NAXIS1", str_val, "Bytes in row", NULL) ;
        sprintf(str_val, "%d", (int)(t->nr)) ;		
        qfits_header_append(fh, "NAXIS2", str_val, "No. of rows in table",NULL);
        qfits_header_append(fh, "PCOUNT", "0", "Parameter count always 0",NULL);
        qfits_header_append(fh, "GCOUNT", "1", "Group count always 1", NULL);
        sprintf(str_val, "%d", (int)(t->nc)) ;
        qfits_header_append(fh, "TFIELDS", str_val, "No. of col in table",NULL);
        /* Columns descriptors */
        curr_col = t->col ;
        for (i=0 ; i<t->nc ; i++) {
            sprintf(str_val, "TFORM%d", i+1) ;
            sprintf(str_val2, "'%s'", qfits_build_format(curr_col)) ;
            qfits_header_append(fh, str_val, str_val2, "Format of field", NULL);
                    
            sprintf(str_val, "TTYPE%d", i+1) ;
            sprintf(str_val2, "%s", curr_col->tlabel) ;
            qfits_header_append(fh, str_val, str_val2, "Field label", NULL) ;

            sprintf(str_val, "TUNIT%d", i+1) ;
            sprintf(str_val2, "%s", curr_col->tunit) ;
            qfits_header_append(fh, str_val, str_val2, "Physical unit of field",
                    NULL) ;	
            if (curr_col->zero_present) {
                sprintf(str_val, "TZERO%d", i+1) ;
                sprintf(str_val2, "%f", curr_col->zero) ;
                qfits_header_append(fh, str_val, str_val2, 
                        "NULL value is defined", NULL) ;
            }
            if (curr_col->scale_present) {
                sprintf(str_val, "TSCAL%d", i+1) ;
                sprintf(str_val2, "%f", curr_col->scale) ;
                qfits_header_append(fh, str_val, str_val2, "Scaling applied", 
                        NULL);
            }
            curr_col++ ;
        }
        qfits_header_append(fh,"ORIGIN","ESO-QFITS", "Written by QFITS", NULL);

        date = qfits_get_datetime_iso8601() ;
        sprintf(str_val, "'%s'", date) ;
        qfits_header_append(fh, "DATE", str_val, "[UTC] Date of writing", NULL);
        qfits_header_append(fh, "END", NULL, NULL, NULL);
    
    } else if (t->tab_t == QFITS_ASCIITABLE) {
    
        /* Write extension header */
        qfits_header_append(fh, "XTENSION", "TABLE",
                        "FITS ASCII Table Extension", NULL) ;
        qfits_header_append(fh, "BITPIX", "8", "8-bits character format", NULL);
        qfits_header_append(fh, "NAXIS", "2", "ASCII table has 2 axes", NULL) ;
               
        /* Fill the header  */
        sprintf(str_val, "%d", tab_width) ;
        qfits_header_append(fh, "NAXIS1", str_val, "Characters in a row", NULL);
        sprintf(str_val, "%d", (int)(t->nr)) ;		
        qfits_header_append(fh, "NAXIS2", str_val, "No. of rows in table",NULL);
        qfits_header_append(fh, "PCOUNT", "0", "No group parameters", NULL) ;	
        qfits_header_append(fh, "GCOUNT", "1", "Only one group", NULL);
        sprintf(str_val, "%d", (int)(t->nc)) ;
        qfits_header_append(fh, "TFIELDS", str_val, "No. of col in table",NULL);
        qfits_header_append(fh, "ORIGIN","ESO-QFITS","Written by QFITS",NULL);
        date = qfits_get_datetime_iso8601() ;
        sprintf(str_val, "'%s'", date) ;
        qfits_header_append(fh, "DATE", str_val, "[UTC] Date of writing", NULL);

        /* Columns descriptors */
        curr_col = t->col ;
        col_pos = 1 ;
        for (i=0 ; i<t->nc ; i++) {
            sprintf(str_val, "TTYPE%d", i+1) ;
            sprintf(str_val2, "%s", curr_col->tlabel) ;
            qfits_header_append(fh, str_val, str_val2, "Field label", NULL) ;
            
            sprintf(str_val, "TFORM%d", i+1) ;
            sprintf(str_val2, "'%s'", qfits_build_format(curr_col)) ;
            qfits_header_append(fh, str_val, str_val2, "Format of field", NULL);
                    
            sprintf(str_val, "TBCOL%d", i+1) ;
            sprintf(str_val2, "%d", col_pos) ;
            qfits_header_append(fh, str_val, str_val2,"Start column of field",
                    NULL);
            col_pos += curr_col->atom_nb ;
            
            sprintf(str_val, "TUNIT%d", i+1) ;
            sprintf(str_val2, "%s", curr_col->tunit) ;
            qfits_header_append(fh, str_val, str_val2, "Physical unit of field",
                    NULL) ;	
            if (curr_col->zero_present) {
                sprintf(str_val, "TZERO%d", i+1) ;
                sprintf(str_val2, "%f", curr_col->zero) ;
                qfits_header_append(fh, str_val, str_val2, 
                        "NULL value is defined", NULL) ;
            }
            if (curr_col->scale_present) {
                sprintf(str_val, "TSCAL%d", i+1) ;
                sprintf(str_val2, "%f", curr_col->scale) ;
                qfits_header_append(fh, str_val, str_val2, "Scaling applied", 
                        NULL);
            }
            curr_col++ ;
        }
        qfits_header_append(fh, "END", NULL, NULL, NULL);

    } else {
		qfits_error("Table type not known") ;
        qfits_header_destroy(fh) ;
		return NULL ;
    }
    return fh ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Table object constructor
  @param	filename	Name of the FITS file associated to the table
  @param	table_type	Type of the table (QFITS_ASCIITABLE or QFITS_BINTABLE)
  @param    table_width Width in bytes of the table
  @param	nb_cols		Number of columns
  @param	nb_raws		Number of raws
  @return	The table object
  The columns are also allocated. The object has to be freed with 
  qfits_table_close()
 */
/*----------------------------------------------------------------------------*/
qfits_table * qfits_table_new(
		char	*	filename,
		int			table_type,
		int			table_width,
		int			nb_cols,
        int         nb_raws)
{
	qfits_table	*	qt ;
	qt = malloc(sizeof(qfits_table)) ;
	(void)strcpy(qt->filename, filename) ;
	qt->tab_t = table_type ;
	qt->nc = nb_cols ;
	qt->nr = nb_raws ;
	qt->col = calloc(qt->nc, sizeof(qfits_col)) ;
    qt->tab_w = table_width ;
    
	return qt ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Fill a column object with some provided informations
  @param	qc		Pointer to the column that has to be filled
  @param	unit	Unit of the data 
  @param	label	Label of the column 
  @param	disp	Way to display the data 
  @param    nullval Null value
  @param	atom_nb	Number of atoms per field. According to the type, an atom 
  					is a double, an int, a char, ... 
  @param    atom_dec_nb Number of decimals as specified in TFORM 
  @param	atom_size	Size in bytes of the field for ASCII tables, and of 
  						an atom for BIN tables. ASCII tables only contain 1 
						atom per field (except for A type where you can of
						course have more than one char per field)
  @param	atom_type	Type of data (11 types for BIN, 5 for ASCII)
  @param    zero_present    Flag to use or not zero
  @param    zero            Zero value
  @param    scale_present   Flag to use or not scale
  @param    scale           Scale value
  @param    offset_beg  Gives the position of the column
  @return 	-1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
int qfits_col_fill(
		qfits_col	*	qc,
		int				atom_nb,
		int				atom_dec_nb,
		int				atom_size,
		tfits_type		atom_type,
		char		*	label,
		char		*	unit,
		char		*	nullval,
		char		*	disp,
        int             zero_present,
        float           zero,
        int             scale_present,
        float           scale,
		int				offset_beg)
{
    /* Number of atoms per column */
	qc->atom_nb = atom_nb ;
   
    /* Number of decimals in a field in ASCII table (0 in BINTABLE) */
    qc->atom_dec_nb = atom_dec_nb ;
    
    /* Size in bytes of an atom  */
	qc->atom_size = atom_size ;
    
    /* Data type in the column */
	qc->atom_type = atom_type ;
    
    /* Label of the column */
	(void)strcpy(qc->tlabel, label) ;
   
    /* Unit of the column data */
	(void)strcpy(qc->tunit, unit) ;
    
    /* Null value*/
	(void)strcpy(qc->nullval, nullval) ;

    /* How to display the data */
	(void)strcpy(qc->tdisp, disp) ;

    /* Default values for zero and scales */
    qc->zero_present = zero_present ;
    qc->scale_present = scale_present ;
    qc->zero = zero ;
    qc->scale = scale ;

    /* Number of bytes between two consecutive fields of the same column */
	qc->off_beg = offset_beg ;
    
    /* A column is a priori readable */
    qc->readable = 1 ;

	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Read a FITS extension.
  @param	filename	Name of the FITS file to examine.
  @param	xtnum		Extension number to read (starting from 1).
  @return	Pointer to newly allocated qfits_table structure.

  Read a FITS table from a given file name and extension, and return a
  newly allocated qfits_table structure. 
 */
/*----------------------------------------------------------------------------*/
qfits_table * qfits_table_open(
		char	*	filename, 
		int 		xtnum)
{
	qfits_table		*	tload ;
	qfits_col		*	curr_col ;
	char			*	str_val ;
	char				keyword[FITSVALSZ] ;
    /* Table infos  */
    int                 table_type ;
    int                 nb_col ;
    int                 table_width ;
	int					nb_raws ;
    /* Column infos */
    char                label[FITSVALSZ] ;
    char                unit[FITSVALSZ] ;
    char                disp[FITSVALSZ] ;
    char                nullval[FITSVALSZ] ;
    int                 atom_nb ;
    int                 atom_dec_nb ;
    int                 atom_size ;
    tfits_type          atom_type ;
	int					offset_beg ;
    int                 data_size ;
    int                 theory_size ;
    int                 zero_present ;
    int                 scale_present ;
    float               zero ;
    float               scale ;
    
    /* For ASCII tables */
	int					col_pos ;	
	int					next_col_pos ;
    
    /* For X type */
	int					nb_bits ;
        
	int					i ;
	
	 /* See if 'filename' is a fits file  */
	if (is_fits_file(filename) != 1) {
		qfits_error("[%s] is not FITS", filename) ;
		return NULL ;
	}
		
	/* Identify a table and get the table type : ASCII or BIN */
	if ((table_type = qfits_is_table(filename, xtnum))==0) {
		qfits_error("[%s] extension %d is not a table", filename, xtnum) ;
		return NULL ;
	}
	
	/* Get number of columns and allocate them: nc <-> TFIELDS */
	if ((str_val = qfits_query_ext(filename, "TFIELDS", xtnum)) == NULL) {
		qfits_error("cannot read TFIELDS in [%s]:[%d]", filename, xtnum) ;
		return NULL ;
	}
	nb_col = atoi(str_val) ;

    /* Get the width in bytes of the table */
    if ((str_val = qfits_query_ext(filename, "NAXIS1", xtnum)) == NULL) {
		qfits_error("cannot read NAXIS1 in [%s]:[%d]", filename, xtnum) ;
		return NULL ;
	}
	table_width = atoi(str_val) ;
    
	/* Get the number of raws */
	if ((str_val = qfits_query_ext(filename, "NAXIS2", xtnum)) == NULL) {
		qfits_error("cannot read NAXIS2 in [%s]:[%d]", filename, xtnum) ;
		return NULL ;
	}
	nb_raws = atoi(str_val) ;

    /* Create the table object */
    tload = qfits_table_new(filename, table_type, table_width, nb_col, nb_raws);
    
	/* Initialize offset_beg */
	if (qfits_get_datinfo(filename, xtnum, &offset_beg, &data_size)!=0) {
		qfits_error("cannot find data start in [%s]:[%d]", filename, xtnum);
		qfits_table_close(tload);
		return NULL ;
	}
	
	/* Loop on all columns and get column descriptions  */
    curr_col = tload->col ;
	for (i=0 ; i<tload->nc ; i++) {
		/* label <-> TTYPE	 */
		sprintf(keyword, "TTYPE%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum)) == NULL) {
            label[0] = (char)0 ;
        } else strcpy(label, qfits_pretty_string(str_val)) ;
		
		/* unit <-> TUNIT */
		sprintf(keyword, "TUNIT%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum)) == NULL) {
			unit[0] = (char)0 ;
		} else strcpy(unit, qfits_pretty_string(str_val)) ;

		/* disp <-> TDISP */
		sprintf(keyword, "TDISP%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum)) == NULL) {
			disp[0] = (char)0 ;
		} else strcpy(disp, qfits_pretty_string(str_val)) ;

		/* nullval <-> TNULL */
		sprintf(keyword, "TNULL%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum)) == NULL) {
		    nullval[0] = (char)0 ;
        } else strcpy(nullval, qfits_pretty_string(str_val)) ;
	
		/* atom_size, atom_nb, atom_dec_nb, atom_type	<-> TFORM */
		sprintf(keyword, "TFORM%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum))==NULL) {
			qfits_error("cannot read [%s] in [%s]:[%d]", keyword, filename, 
					xtnum);
			qfits_table_close(tload);
			return NULL ;
		}
		/* Interpret the type in header */
		if (qfits_table_interpret_type(qfits_pretty_string(str_val), 
						&(atom_nb), 
                        &(atom_dec_nb),
						&(atom_type), 
						table_type) == -1) {
			qfits_error("cannot interpret the type: %s", str_val) ;
			qfits_table_close(tload) ;
			return NULL ;
		}
		
        /* Set atom_size */
        switch (atom_type) {
            case TFITS_BIN_TYPE_A:
            case TFITS_BIN_TYPE_L:
            case TFITS_BIN_TYPE_B:
                atom_size = 1 ;
                break ;
            case TFITS_BIN_TYPE_I:
                atom_size = 2 ;
                break ;
            case TFITS_BIN_TYPE_J:
            case TFITS_BIN_TYPE_E:
            case TFITS_ASCII_TYPE_I:
            case TFITS_ASCII_TYPE_E:
            case TFITS_ASCII_TYPE_F:
                atom_size = 4 ;
                break ;
            case TFITS_BIN_TYPE_C:
            case TFITS_BIN_TYPE_P:
                atom_size = 4 ;
				atom_nb *= 2 ;
                break ;
            case TFITS_BIN_TYPE_D:
            case TFITS_ASCII_TYPE_D:
                atom_size = 8 ;
                break ;
            case TFITS_BIN_TYPE_M:
                atom_size = 8 ;
                atom_nb *= 2 ;
                break ;
            case TFITS_BIN_TYPE_X:
                atom_size = 1 ;
                nb_bits = atom_nb ;
                atom_nb = (int)((nb_bits - 1)/ 8) + 1 ;
                break ;
            case TFITS_ASCII_TYPE_A:
				atom_size = atom_nb ;
				break ;
            default:
				qfits_error("unrecognized type") ;
                qfits_table_close(tload) ;
                return NULL ;
                break ;
        }
	
		/* zero <-> TZERO */
		sprintf(keyword, "TZERO%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum)) != NULL) {
			zero = (float)atof(str_val) ;
			zero_present = 1 ;	
		} else {
			zero = (float)0.0 ;
			zero_present = 0 ;	
		}
		
		/* scale <-> TSCAL */
		sprintf(keyword, "TSCAL%d", i+1) ;
		if ((str_val=qfits_query_ext(filename, keyword, xtnum)) != NULL) {
			scale = (float)atof(str_val) ;
			scale_present = 1 ;
		} else {
			scale = (float)1.0 ;
			scale_present = 0 ;
		}

        /* Fill the current column object */
        qfits_col_fill(curr_col, atom_nb, atom_dec_nb, atom_size, atom_type, 
                label, unit, nullval, disp, zero_present, zero, scale_present, 
                scale, offset_beg) ;
        
        /* Compute offset_beg but for the last column */
        if (i < tload->nc - 1) {
            if (table_type == QFITS_ASCIITABLE) {
		    	/* column width <-> TBCOLi and TBCOLi+1 */
		    	sprintf(keyword, "TBCOL%d", i+1) ;
		    	if ((str_val=qfits_query_ext(filename, keyword, xtnum))==NULL) {
		    		qfits_error("cannot read [%s] in [%s]", keyword, filename);
		    		qfits_table_close(tload);
		    		return NULL ;
		    	}
		    	col_pos = atoi(qfits_pretty_string(str_val)) ;
		    	
		    	sprintf(keyword, "TBCOL%d", i+2) ;
		    	if ((str_val=qfits_query_ext(filename, keyword, xtnum))==NULL){
		    		qfits_error("cannot read [%s] in [%s]", keyword, filename) ;
		    		qfits_table_close(tload) ;
		    		return NULL ;
		    	}
		    	next_col_pos = atoi(qfits_pretty_string(str_val)) ;
		    	offset_beg += (int)(next_col_pos - col_pos) ;
            } else if (table_type == QFITS_BINTABLE) {
                offset_beg += atom_nb * atom_size ;
            }
        }
        curr_col++ ;
	}

    /* Check that the theoritical data size is not far from the measured */
    /* one by more than 2880 */
    theory_size = qfits_compute_table_width(tload)*tload->nr ;
    if (data_size < theory_size) {
        qfits_error("Uncoherent data sizes") ;
        qfits_table_close(tload) ;
        return NULL ;
    }
    
	/* Return  */
	return tload ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Free a FITS table and associated pointers
  @param	t qfits_table to free
  @return	void
  Frees all memory associated to a qfits_table structure.
 */
/*----------------------------------------------------------------------------*/
void qfits_table_close(qfits_table * t)
{
	if (t==NULL) return ;
	if (t->nc>0) if (t->col!=NULL) free(t->col) ;
	free(t);
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Extract data from a column in a FITS table
  @param	th		Allocated qfits_table
  @param	colnum	Number of the column to extract (from 0 to colnum-1)
  @param    selection  boolean array to define the selected rows
  @return	unsigned char array

  If selection is NULL, select the complete column.
  
  Extract a column from a FITS table and return the data as a bytes 
  array. The returned array type and size are determined by the
  column object in the qfits_table and by the selection parameter.

  Returned array size in bytes is:
  nbselected * col->natoms * col->atom_size

  Numeric types are correctly understood and byte-swapped if needed,
  to be converted to the local machine type.

  NULL values have to be handled by the caller.

  The returned buffer has been allocated using one of the special memory
  operators present in xmemory.c. To deallocate the buffer, you must call
  the version of free() offered by xmemory, not the usual system free(). It
  is enough to include "xmemory.h" in your code before you make calls to
  the pixel loader here.

 */
/*----------------------------------------------------------------------------*/
unsigned char * qfits_query_column(
		qfits_table	    *   th,
		int                 colnum,
        int             *   selection)
{
	char			*	start ;
    qfits_col       *   col ;
	int					field_size ;
    unsigned char   *   array ;
	unsigned char   *   r ;
    unsigned char   *   inbuf ;
    int                 table_width ;
    int                 nb_rows ;
    size_t              size ;
	int                 i ;
   
    if (th->tab_w == -1) {
        /* Compute the table width in bytes */
        if ((table_width = qfits_compute_table_width(th)) == -1) {
            qfits_error("cannot compute the table width") ;
            return NULL ;
        }
    } else table_width = th->tab_w ;
   
    /* Compute the number of selected rows */
    nb_rows = 0 ;
    if (selection == NULL) {
        nb_rows = th->nr ;
    } else {
        for (i=0 ; i<th->nr ; i++) if (selection[i] == 1) nb_rows++ ;
    }
    
	/* Pointer to requested column */
	col = th->col + colnum ;

	/* Test if column is empty */
	if (nb_rows * col->atom_size * col->atom_nb == 0) col->readable = 0 ;
	
	/* Test if column is readable */
	if (col->readable == 0)  return NULL ;

	/* Compute the size in bytes of one field stored in the file */
    if ((field_size=qfits_table_get_field_size(th->tab_t,col))==-1) return NULL;
	
	/* Load input file */
    if ((start=falloc(th->filename, 0, &size))==NULL) {
        qfits_error("cannot open table for query [%s]", th->filename);
        return NULL ;
    }
   
	/* Allocate data array */
	array = malloc(nb_rows * field_size * sizeof(char)) ; 
			
    /* Position the input pointer at the begining of the column data */
    r = array ;
    inbuf = (unsigned char*)start + col->off_beg ;
   
    /* Copy the values in array */
	if (selection == NULL) {
        /* No selection : get the complete column */
        for (i=0 ; i<th->nr ; i++) {
            /* Copy all atoms on this field into array */
            memcpy(r, inbuf, field_size);
            r += field_size ;
            /* Jump to next line */
            inbuf += table_width ;
        }
    } else {
        /* Get only the selected rows */
        for (i=0 ; i<th->nr ; i++) {
            if (selection[i] == 1) {
                /* Copy all atoms on this field into array */
                memcpy(r, inbuf, field_size);
                r += field_size ;
            }
            /* Jump to next line */
            inbuf += table_width ;
        }
    }
    fdealloc(start, 0, size) ;

	/* SWAP the bytes if necessary */
#ifndef WORDS_BIGENDIAN
    if (th->tab_t == QFITS_BINTABLE) {
		r = array ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			swap_bytes(r, col->atom_size);
			r += col->atom_size ;
		}
	}
#endif

     /* Return allocated and converted array */
	return array ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Extract consequtive values from a column in a FITS table
  @param	th		Allocated qfits_table
  @param	colnum	Number of the column to extract (from 0 to colnum-1)
  @param    start_ind   Index of the first row (0 for the first)
  @param    nb_rows     Number of rows to extract
  @return	unsigned char array
  Does the same as qfits_query_column() but on a consequtive sequence of rows
  Spares the overhead of the selection object allocation
 */
/*----------------------------------------------------------------------------*/
unsigned char * qfits_query_column_seq(
		qfits_table	    *   th,
		int                 colnum,
        int                 start_ind,
        int                 nb_rows)
{
	char			*	start ;
    qfits_col       *   col ;
	int					field_size ;
    unsigned char   *   array ;
	unsigned char   *   r ;
    unsigned char   *   inbuf ;
    int                 table_width ;
    size_t              size ;
	int                 i ;
   
    if (th->tab_w == -1) {
        /* Compute the table width in bytes */
        if ((table_width = qfits_compute_table_width(th)) == -1) {
            qfits_error("cannot compute the table width") ;
            return NULL ;
        }
    } else table_width = th->tab_w ;
  
    /* Check the validity of start_ind and nb_rows */
    if ((start_ind<0) || (start_ind+nb_rows>th->nr)) { 
        qfits_error("bad start index and number of rows") ;
        return NULL ;
    }
    
	/* Pointer to requested column */
	col = th->col + colnum ;

	/* Test if column is empty */
	if (nb_rows * col->atom_size * col->atom_nb == 0) col->readable = 0 ;
	
	/* Test if column is readable */
	if (col->readable == 0)  return NULL ;

	/* Compute the size in bytes of one field stored in the file */
    if ((field_size=qfits_table_get_field_size(th->tab_t,col))==-1) return NULL;
	
	/* Load input file */
    if ((start=falloc(th->filename, 0, &size))==NULL) {
        qfits_error("cannot open table for query [%s]", th->filename);
        return NULL ;
    }
   
	/* Allocate data array */
	array = malloc(nb_rows * field_size * sizeof(char)) ; 
			
    /* Position the input pointer at the begining of the column data */
    r = array ;
    inbuf = (unsigned char*)start + col->off_beg + table_width * start_ind ;
   
    /* Copy the values in array */
    /* Get only the selected rows */
    for (i=0 ; i<nb_rows ; i++) {
        /* Copy all atoms on this field into array */
        memcpy(r, inbuf, field_size);
        r += field_size ;
        /* Jump to next line */
        inbuf += table_width ;
    }
    fdealloc(start, 0, size) ;

	/* SWAP the bytes if necessary */
#ifndef WORDS_BIGENDIAN
    if (th->tab_t == QFITS_BINTABLE) {
		r = array ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			swap_bytes(r, col->atom_size);
			r += col->atom_size ;
		}
	}
#endif

     /* Return allocated and converted array */
	return array ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Compute the table width in bytes from the columns infos 
  @param	th		Allocated qfits_table
  @return	the width (-1 in error case)
 */
/*----------------------------------------------------------------------------*/
int qfits_compute_table_width(qfits_table * th)
{
    int             width ;
    qfits_col   *   curr_col ;
    int             i ;
    
    /* Initialize */
    width = 0 ;
    
    /* Loop on all columns and get column descriptions  */
    curr_col = th->col ;
    for (i=0 ; i<th->nc ; i++) {
        if (th->tab_t == QFITS_ASCIITABLE) {
            width += curr_col->atom_nb ;
        } else if (th->tab_t == QFITS_BINTABLE) {
            width += curr_col->atom_nb * curr_col->atom_size ;
        }
        curr_col++ ;
    }
    return width ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Extract binary data from a column in a FITS table
  @param	th		Allocated qfits_table
  @param	colnum	Number of the column to extract (from 0 to colnum-1)
  @param    selection  bollean array to identify selected rows
  @param	null_value	Value to return when a NULL value comes
  @return	Pointer to void *

  Extract a column from a FITS table and return the data as a generic
  void* array. The returned array type and size are determined by the
  column object in the qfits_table.
	
  Returned array size in bytes is:
  nb_selected * col->atom_nb * col->atom_size
  
  NULL values are recognized and replaced by the specified value.

  The returned buffer has been allocated using one of the special memory
  operators present in xmemory.c. To deallocate the buffer, you must call
  the version of free() offered by xmemory, not the usual system free(). It
  is enough to include "xmemory.h" in your code before you make calls to
  the pixel loader here.
 */
/*----------------------------------------------------------------------------*/
void * qfits_query_column_data(
		qfits_table	    *   th,
		int                 colnum,
        int             *   selection,
		void			*	null_value)
{
	void			*	out_array ;
	qfits_col       *   col ;
    int                 nb_rows ;
	unsigned char	*	in_array ;
	char			*	field ;

	unsigned char		ucnull ;
	short				snull ;
	int                 inull ;
	double				dnull ;
	float				fnull ;

    int                 i ;
	
    /* Initialize */
	if (null_value == NULL) {
        inull  = (int)0 ;
		snull  = (short)0 ;
		ucnull = (unsigned char)0 ;
		fnull  = (float)0.0 ;
		dnull  = (double)0.0 ;
    } else {
		inull  = *(int*)null_value ;
        snull  = *(short*)null_value ;
		ucnull = *(unsigned char*)null_value ;
        fnull  = *(float*)null_value ;
        dnull  = *(double*)null_value ;
    }

    /* Get the number of selected rows */
    nb_rows = 0 ;
    if (selection == NULL) {
        nb_rows = th->nr ;
    } else {
        for (i=0 ; i<th->nr ; i++) if (selection[i] == 1) nb_rows++ ;
    }

	/* Pointer to requested column */
    col = th->col+colnum ;

    /* Test if column is readable */
    if (col->readable == 0) return NULL ;
	
	/* Handle each type separately */
	switch(col->atom_type) {
		case TFITS_ASCII_TYPE_A:
		out_array = (char*)qfits_query_column(th, colnum, selection) ;
		break ;

		case TFITS_ASCII_TYPE_I:
		in_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
		out_array = malloc(nb_rows*col->atom_size);
		field = malloc((col->atom_nb+1)*sizeof(char)) ;
		for (i=0 ; i<nb_rows ; i++) {
			/* Copy all atoms of the field into 'field' */
			memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
			field[col->atom_nb]=(char)0 ;
			/* Write the data in out_array */
			/* Test if a NULL val is encoutered */
			if (!strcmp(col->nullval, qfits_strstrip(field))) {
				((int*)out_array)[i] = inull ;
			} else {
				((int*)out_array)[i] = (int)atoi(field) ; 
			}
		}
		free(field) ;
		free(in_array) ;
		break ;
			
		case TFITS_ASCII_TYPE_E:
		case TFITS_ASCII_TYPE_F:
		in_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
		out_array = malloc(nb_rows*col->atom_size);
		field = malloc((col->atom_nb+1)*sizeof(char)) ;
		for (i=0 ; i<nb_rows ; i++) {
			/* Copy all atoms of the field into 'field' */
			memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
			field[col->atom_nb]=(char)0 ;
			/* Write the data in out_array */
			/* Test if a NULL val is encoutered */
			if (!strcmp(col->nullval, qfits_strstrip(field))) {
				((float*)out_array)[i] = fnull ;
			} else {
                /* Add the decimal handling */
                ((float*)out_array)[i]=(float)qfits_str2dec(field, 
                                                             col->atom_dec_nb) ;
			}
		}
		free(field) ;
		free(in_array) ;
		break ;
			
        case TFITS_ASCII_TYPE_D:
        in_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
        out_array = malloc(nb_rows*col->atom_size);
        field = malloc((col->atom_nb+1)*sizeof(char)) ;
        for (i=0 ; i<nb_rows ; i++) {
            /* Copy all atoms of the field into 'field' */
            memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
            field[col->atom_nb]=(char)0 ;
            /* Write the data in out_array */
            /* Test if a NULL val is encoutered */
            if (!strcmp(col->nullval, field)) {
                ((double*)out_array)[i] = dnull ;
            } else {
                /* Add the decimal handling */
                ((double*)out_array)[i]=qfits_str2dec(field, col->atom_dec_nb) ;
            }
        }
        free(field) ;
        free(in_array) ;
   
        break ;
		case TFITS_BIN_TYPE_A:
		case TFITS_BIN_TYPE_L:
		out_array = (char*)qfits_query_column(th, colnum, selection) ;
		break ;
			
		case TFITS_BIN_TYPE_D:
		case TFITS_BIN_TYPE_M:
		out_array = (double*)qfits_query_column(th, colnum, selection) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (qfits_isnan(((double*)out_array)[i]) || 
					qfits_isinf(((double*)out_array)[i])) {
				((double*)out_array)[i] = dnull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_E:
		case TFITS_BIN_TYPE_C:
		out_array = (float*)qfits_query_column(th, colnum, selection) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (qfits_isnan(((float*)out_array)[i]) || 
					qfits_isinf(((float*)out_array)[i])) {
				((float*)out_array)[i] = fnull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_X:
		out_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
		break ;
			
		case TFITS_BIN_TYPE_B:
		out_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) && 
                (atoi(col->nullval) == (int)((unsigned char*)out_array)[i])) {
				((unsigned char*)out_array)[i] = ucnull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_I:
		out_array = (short*)qfits_query_column(th, colnum, selection) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==(int)((short*)out_array)[i])) { 	
				((short*)out_array)[i] = snull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_J:
		out_array = (int*)qfits_query_column(th, colnum, selection) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==((int*)out_array)[i])) { 	
				((int*)out_array)[i] = inull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_P:
		out_array = (int*)qfits_query_column(th, colnum, selection) ;
		break ;
			
		default:
		qfits_error("unrecognized data type") ;
		return NULL ;
	}
	return out_array ;	
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Extract binary data from a column in a FITS table
  @param	th		Allocated qfits_table
  @param	colnum	Number of the column to extract (from 0 to colnum-1)
  @param    start_ind   Index of the first row (0 for the first)
  @param    nb_rows     Number of rows to extract
  @param	null_value	Value to return when a NULL value comes
  @return	Pointer to void *
  Does the same as qfits_query_column_data() but on a consequtive sequence 
  of rows.  Spares the overhead of the selection object allocation
 */
/*----------------------------------------------------------------------------*/
void * qfits_query_column_seq_data(
		qfits_table	    *   th,
		int                 colnum,
        int                 start_ind,
        int                 nb_rows,
		void			*	null_value)
{
	void			*	out_array ;
	qfits_col       *   col ;
	unsigned char	*	in_array ;
	char			*	field ;

	unsigned char		ucnull ;
	short				snull ;
	int                 inull ;
	double				dnull ;
	float				fnull ;

    int                 i ;
	
    /* Initialize */
	if (null_value == NULL) {
        inull  = (int)0 ;
		snull  = (short)0 ;
		ucnull = (unsigned char)0 ;
		fnull  = (float)0.0 ;
		dnull  = (double)0.0 ;
    } else {
		inull  = *(int*)null_value ;
        snull  = *(short*)null_value ;
		ucnull = *(unsigned char*)null_value ;
        fnull  = *(float*)null_value ;
        dnull  = *(double*)null_value ;
    }

	/* Pointer to requested column */
    col = th->col+colnum ;

    /* Test if column is readable */
    if (col->readable == 0) return NULL ;
	
	/* Handle each type separately */
	switch(col->atom_type) {
		case TFITS_ASCII_TYPE_A:
		out_array = (char*)qfits_query_column_seq(th, colnum, start_ind, 
                nb_rows) ;
		break ;

		case TFITS_ASCII_TYPE_I:
		in_array = (unsigned char*)qfits_query_column_seq(th, colnum, 
                start_ind, nb_rows) ;
		out_array = malloc(nb_rows*col->atom_size);
		field = malloc((col->atom_nb+1)*sizeof(char)) ;
		for (i=0 ; i<nb_rows ; i++) {
			/* Copy all atoms of the field into 'field' */
			memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
			field[col->atom_nb]=(char)0 ;
			/* Write the data in out_array */
			/* Test if a NULL val is encoutered */
			if (!strcmp(col->nullval, qfits_strstrip(field))) {
				((int*)out_array)[i] = inull ;
			} else {
				((int*)out_array)[i] = (int)atoi(field) ; 
			}
		}
		free(field) ;
		free(in_array) ;
		break ;
			
		case TFITS_ASCII_TYPE_E:
		case TFITS_ASCII_TYPE_F:
		in_array = (unsigned char*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		out_array = malloc(nb_rows*col->atom_size);
		field = malloc((col->atom_nb+1)*sizeof(char)) ;
		for (i=0 ; i<nb_rows ; i++) {
			/* Copy all atoms of the field into 'field' */
			memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
			field[col->atom_nb]=(char)0 ;
			/* Write the data in out_array */
			/* Test if a NULL val is encoutered */
			if (!strcmp(col->nullval, qfits_strstrip(field))) {
				((float*)out_array)[i] = fnull ;
			} else {
                /* Add the decimal handling */
                ((float*)out_array)[i]=(float)qfits_str2dec(field, 
                                                            col->atom_dec_nb) ;
			}
		}
		free(field) ;
		free(in_array) ;
		break ;
			
		case TFITS_ASCII_TYPE_D:
		in_array = (unsigned char*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		out_array = malloc(nb_rows*col->atom_size);
		field = malloc((col->atom_nb+1)*sizeof(char)) ;
		for (i=0 ; i<nb_rows ; i++) {
			/* Copy all atoms of the field into 'field' */
			memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
			field[col->atom_nb]=(char)0 ;
			/* Write the data in out_array */
			/* Test if a NULL val is encoutered */
			if (!strcmp(col->nullval, qfits_strstrip(field))) {
				((double*)out_array)[i] = dnull ;
            } else {
                /* Add the decimal handling */
                ((double*)out_array)[i]=qfits_str2dec(field, col->atom_dec_nb) ;
			}
		}
		free(field) ;
		free(in_array) ;
		break ;
			
		case TFITS_BIN_TYPE_A:
		case TFITS_BIN_TYPE_L:
		out_array = (char*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		break ;
			
		case TFITS_BIN_TYPE_D:
		case TFITS_BIN_TYPE_M:
		out_array = (double*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (qfits_isnan(((double*)out_array)[i]) || 
					qfits_isinf(((double*)out_array)[i])) {
				((double*)out_array)[i] = dnull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_E:
		case TFITS_BIN_TYPE_C:
		out_array = (float*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (qfits_isnan(((float*)out_array)[i]) || 
					qfits_isinf(((float*)out_array)[i])) {
				((float*)out_array)[i] = fnull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_X:
		out_array = (unsigned char*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		break ;
			
		case TFITS_BIN_TYPE_B:
		out_array = (unsigned char*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)== (int)((unsigned char*)out_array)[i])) {
				((unsigned char*)out_array)[i] = ucnull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_I:
		out_array = (short*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==(int)((short*)out_array)[i])) { 	
				((short*)out_array)[i] = snull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_J:
		out_array = (int*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==((int*)out_array)[i])) { 	
				((int*)out_array)[i] = inull ;
			}
		}
		break ;
			
		case TFITS_BIN_TYPE_P:
		out_array = (int*)qfits_query_column_seq(th, colnum,
                start_ind, nb_rows) ;
		break ;
			
		default:
		qfits_error("unrecognized data type") ;
		return NULL ;
	}
	return out_array ;	
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Detect NULL values in a column
  @param	th		Allocated qfits_table
  @param	colnum	Number of the column to check (from 0 to colnum-1)
  @param    selection Array to identify selected rows
  @param	nb_vals Gives the size of the output array
  @param	nb_nulls Gives the number of detected null values
  @return 	array with 1 for NULLs and 0 for non-NULLs	
 */
/*----------------------------------------------------------------------------*/
int * qfits_query_column_nulls(
		qfits_table	    *   th,
		int                 colnum,
        int             *   selection,
		int				*	nb_vals,
		int				*	nb_nulls)
{
	int 			*	out_array ;
	qfits_col       *   col ;
	unsigned char	*	in_array ;
	void			*	tmp_array ;
	char			*	field ;
    int                 nb_rows ;
    int                 i ;

	/* Initialize */
	*nb_nulls = 0 ;
	*nb_vals = 0 ;

    /* Get the number of selected rows */
    nb_rows = 0 ;
    if (selection == NULL) {
        nb_rows = th->nr ;
    } else {
       for (i=0 ; i<th->nr ; i++) if (selection[i] == 1) nb_rows++ ; 
    }
    
	/* Pointer to requested column */
    col = th->col+colnum ;
		
    /* Test if column is readable */
    if (col->readable == 0) return NULL ;

	/* Handle each type separately */
	switch(col->atom_type) {
		case TFITS_ASCII_TYPE_A:
		case TFITS_ASCII_TYPE_D:
		case TFITS_ASCII_TYPE_E:
		case TFITS_ASCII_TYPE_F:
		case TFITS_ASCII_TYPE_I:
		in_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
		out_array = calloc(nb_rows, sizeof(int));
		*nb_vals = nb_rows ;
		field = malloc((col->atom_nb+1)*sizeof(char)) ;
		for (i=0 ; i<nb_rows ; i++) {
			/* Copy all atoms of the field into 'field' */
			memcpy(field, &in_array[i*col->atom_nb], col->atom_nb);
			field[col->atom_nb]=(char)0 ;
			/* Test if a NULL val is encoutered */
			if (!strcmp(col->nullval, qfits_strstrip(field))) {
				out_array[i] = 1 ;	
				(*nb_nulls)++ ;
			} 
		}
		free(field) ;
		if (in_array != NULL) free(in_array) ;
		break ;
			
		case TFITS_BIN_TYPE_A:
		/* No NULL values */
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		break ;
		
		case TFITS_BIN_TYPE_L:
		case TFITS_BIN_TYPE_X:
		case TFITS_BIN_TYPE_P:
		/* No NULL values */
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		break ;
			
		case TFITS_BIN_TYPE_D:
		case TFITS_BIN_TYPE_M:
		tmp_array = (double*)qfits_query_column(th, colnum, selection) ;
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (qfits_isnan(((double*)tmp_array)[i]) || 
				qfits_isinf(((double*)tmp_array)[i])) {
				out_array[i] = 1 ;
				(*nb_nulls)++ ;
			}
		}
		if (tmp_array != NULL) free(tmp_array) ;
		break ;
		
		case TFITS_BIN_TYPE_E:
		case TFITS_BIN_TYPE_C:
		tmp_array = (float*)qfits_query_column(th, colnum, selection) ;
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (qfits_isnan(((float*)tmp_array)[i]) || 
				qfits_isinf(((float*)tmp_array)[i])) {
				out_array[i] = 1 ;
				(*nb_nulls)++ ;
			}
		}
		if (tmp_array != NULL) free(tmp_array) ;
		break ;
		
		case TFITS_BIN_TYPE_B:
		tmp_array = (unsigned char*)qfits_query_column(th, colnum, selection) ;
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==(int)((unsigned char*)tmp_array)[i])) {
				out_array[i] = 1 ;
				(*nb_nulls)++ ;
			}
		}
		if (tmp_array != NULL) free(tmp_array) ;
		break ;
			
		case TFITS_BIN_TYPE_I:
		tmp_array = (short*)qfits_query_column(th, colnum, selection) ;
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==(int)((short*)tmp_array)[i])) { 	
				out_array[i] = 1 ;
				(*nb_nulls)++ ;
			}
		}
		if (tmp_array != NULL) free(tmp_array) ;
		break ;
			
		case TFITS_BIN_TYPE_J:
		tmp_array = (int*)qfits_query_column(th, colnum, selection) ;
		out_array = calloc(nb_rows * col->atom_nb, sizeof(int)) ;
		*nb_vals = nb_rows * col->atom_nb ;
		for (i=0 ; i<nb_rows * col->atom_nb ; i++) {
			if (((col->nullval)[0] != (char)0) &&
                (atoi(col->nullval)==((int*)tmp_array)[i])) { 	
				out_array[i] = 1 ;
				(*nb_nulls)++ ;
			}
		}
		if (tmp_array != NULL) free(tmp_array) ;
		break ;
			
		default:
		qfits_error("unrecognized data type") ;
		return NULL ;
	}
	return out_array ;	
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Save a table to a FITS file with a given FITS header.
  @param    array	        Data array.
  @param	table			table
  @param    fh              FITS header to insert in the output file.
  @return   -1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
int qfits_save_table_hdrdump(
		void			**	array,
		qfits_table		*	table,
		qfits_header	*	fh)
{
	FILE	*	outfile ;
	char	*	md5hash ;
	char		md5card[81];

	/* Open the destination file */
	if ((outfile = fopen(table->filename, "w")) == NULL) {
		qfits_error("cannot open file [%s]", table->filename) ;
		return -1 ;
	}
	/* Write the fits header in the file 'outname' */
	if (qfits_header_dump(fh, outfile) == -1) {
		qfits_error("cannot dump header in file") ;
		fclose(outfile) ;
		return -1 ;
	}
	/* Append the extension */
	if (table->tab_t == QFITS_BINTABLE) {
		if (qfits_table_append_bin_xtension(outfile, table, array) == -1) {
			qfits_error("in writing fits table") ;
			fclose(outfile) ;
			return -1 ;
		}
	} else if (table->tab_t == QFITS_ASCIITABLE) {
		if (qfits_table_append_ascii_xtension(outfile, table, array) == -1) {
			qfits_error("in writing fits table") ;
			fclose(outfile) ;
			return -1 ;
		}
	} else {
		qfits_error("Unrecognized table type") ;
		fclose(outfile) ;
		return -1 ;
	}
	fclose(outfile) ;
	/* Update MD5 keyword */
    if (strcmp(table->filename, "STDOUT")) {
        md5hash = qfits_datamd5(table->filename);
        if (md5hash==NULL) {
            qfits_error("computing MD5 signature for output file %s", 
					table->filename);
            return -1 ;
        }
        sprintf(md5card, "DATAMD5 = '%s' / MD5 checksum", md5hash);
        qfits_replace_card(table->filename, "DATAMD5", md5card);
    }
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Appends a std extension header + data to a FITS table file.
  @param	outfile		Pointer to (opened) file ready for writing.
  @param	t			Pointer to qfits_table
  @param	data		Table data to write
  @return	int 0 if Ok, -1 otherwise

  Dumps a FITS table to a file. The whole table described by qfits_table, and 
  the data arrays contained in 'data' are dumped to the file. An extension 
  header is produced with all keywords needed to describe the table, then the 
  data is dumped to the file.
  The output is then padded to reach a multiple of 2880 bytes in size.
  Notice that no main header is produced, only the extension part.
 */
/*----------------------------------------------------------------------------*/
int qfits_table_append_xtension(
        FILE            *   outfile,
        qfits_table    	*   t,
        void            **  data)
{
	/* Append the extension */
	if (t->tab_t == QFITS_BINTABLE) {
		if (qfits_table_append_bin_xtension(outfile, t, data) == -1) {
			qfits_error("in writing fits table") ;
			return -1 ;
		}
	} else if (t->tab_t == QFITS_ASCIITABLE) {
		if (qfits_table_append_ascii_xtension(outfile, t, data) == -1) {
			qfits_error("in writing fits table") ;
			return -1 ;
		}
	} else {
		qfits_error("Unrecognized table type") ;
		return -1 ;
	}
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Appends a specified extension header + data to a FITS table file.
  @param	outfile		Pointer to (opened) file ready for writing.
  @param	t			Pointer to qfits_table
  @param	data		Table data to write
  @param    hdr         Specified extension header
  @return	int 0 if Ok, -1 otherwise

  Dumps a FITS table to a file. The whole table described by qfits_table, and 
  the data arrays contained in 'data' are dumped to the file following the 
  specified fits header. 
  The output is then padded to reach a multiple of 2880 bytes in size.
  Notice that no main header is produced, only the extension part.
 */
/*----------------------------------------------------------------------------*/
int qfits_table_append_xtension_hdr(
        FILE            *   outfile,
        qfits_table    	*   t,
        void            **  data,
        qfits_header    *   hdr)
{
    /* Write the fits header in the file  */
    if (qfits_header_dump(hdr, outfile) == -1) {
		qfits_error("cannot dump header in file") ;
		return -1 ;
	}

    /* Append the data to the file */
    return qfits_table_append_data(outfile, t, data) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	given a col and a row, find out the string to write for display
  @param	table	table structure
  @param	col_id	col id (0 -> nbcol-1)
  @param	row_id	row id (0 -> nrow-1)
  @param	use_zero_scale	Flag to use or not zero and scale
  @return	the string to write (to be freed)

  This function is highly inefficient, it should not be used in loops to
  display a complete table. It is more to get one field from time to
  time, or for debugging puposes.
 */
/*----------------------------------------------------------------------------*/
char * qfits_table_field_to_string(
        qfits_table    	*   table,
        int                 col_id,
        int                 row_id,
		int					use_zero_scale)
{
	char	*	str ;
	
	switch (table->tab_t) {
		case QFITS_BINTABLE:
			str = qfits_bintable_field_to_string(table, col_id, row_id, 
					use_zero_scale) ;
			break ;

		case QFITS_ASCIITABLE:
			str = qfits_asciitable_field_to_string(table, col_id, row_id, 
					use_zero_scale) ;
			break ;
		default:
			qfits_error("Table type not recognized") ;
			return NULL ;
			break ;
	}
	return str ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	given a col and a row, find out the string to write for display
  @param	table	table structure
  @param	col_id	col id (0 -> nbcol-1)
  @param	row_id	row id (0 -> nrow-1)
  @param	use_zero_scale	Flag to use or not zero and scale
  @return	the string to write (to be freed)
  
  ASCII tables specific
 */
/*----------------------------------------------------------------------------*/
static char * qfits_asciitable_field_to_string(
        qfits_table    	*   table,
        int                 col_id,
        int                 row_id,
		int					use_zero_scale)
{
	qfits_col       *       col ;
    char            *       ccol ;
    int             *       icol ;
    float           *       fcol ;
    double          *       dcol ;
	char                    ctmp[512];
    char            *       stmp ;
    int                     field_size ;
	void			*		field ;
    int             *       selection ;
	
	/* Test inputs */
	if (table->tab_t != QFITS_ASCIITABLE) return NULL ;
    
	/* Initialize */
    ctmp[0] = (char)0 ;

    /* Set selection to select the requested row */
    selection = calloc(table->nr, sizeof(int)) ;
    selection[row_id] = 1 ;
    
	/* Load the column data */
	if ((field = qfits_query_column_data(table, col_id, selection, 
                    NULL)) == NULL) return NULL ;
    free(selection) ;
	
    /* Set reference to the column */
    col = table->col + col_id ;
    
    /* Compute field size and allocate stmp */
    if (col->atom_nb > ELEMENT_MAX_DISPLAY_SIZE) field_size = col->atom_nb + 1 ;
    else field_size = ELEMENT_MAX_DISPLAY_SIZE ;
    stmp = malloc(field_size * sizeof(char)) ;
    stmp[0] = (char)0 ;
 
	/* Get the string to write according to the type */
    switch(col->atom_type) {
		case TFITS_ASCII_TYPE_A:
			ccol = (char*)field ;
			strncpy(ctmp, ccol, col->atom_nb);
			ctmp[col->atom_nb] = (char)0;
			strcpy(stmp, ctmp);
			break ;
            
		case TFITS_ASCII_TYPE_I:
			icol = (int*)field ;
        	/* Two cases: use col->zero and col->scale or not */
        	if (col->zero_present && col->scale_present && use_zero_scale) {
				sprintf(stmp, "%f", (float)(col->zero +
							(float)icol[0] * col->scale)) ;
			} else {	
				sprintf(stmp, "%d", icol[0]);
			}
			break ;
            
		case TFITS_ASCII_TYPE_E:
		case TFITS_ASCII_TYPE_F:
			fcol = (float*)field ;
        	/* Two cases: use col->zero and col->scale or not */
        	if (col->zero_present && col->scale_present && use_zero_scale) {
				sprintf(stmp, "%f", (float)(col->zero +
							fcol[0] * col->scale)) ;
			} else {
				sprintf(stmp, "%f", fcol[0]);
			}
			break ;
            
		case TFITS_ASCII_TYPE_D:
			dcol = (double*)field ;
        	/* Two cases: use col->zero and col->scale or not */
        	if (col->zero_present && col->scale_present && use_zero_scale) {
				sprintf(stmp, "%f", (float)(col->zero +
						(float)dcol[0] * col->scale)) ;
			} else {
				sprintf(stmp, "%g", dcol[0]) ;
			}
			break ;
		default:
			qfits_warning("Type not recognized") ;
			break ;
	}

    /* Free and return */
	free(field) ;
	return stmp ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Given a col and a row, find out the string to write for display
  @param	table	table structure
  @param	col_id	col id (0 -> nbcol-1)
  @param	row_id	row id (0 -> nrow-1)
  @param	use_zero_scale	Flag to use or not zero and scale
  @return	the allocated string to write
  
  BIN tables specific
 */
/*----------------------------------------------------------------------------*/
static char * qfits_bintable_field_to_string(
        qfits_table    	*   table,
        int                 col_id,
        int                 row_id,
		int					use_zero_scale)
{
	qfits_col       *       col ;
    unsigned char   *       uccol ;
    char            *       ccol ;
    int             *       icol ;
    short           *       scol ;
    float           *       fcol ;
    double          *       dcol ;
    char                    ctmp[512];
    char            *       stmp ;
    int                     field_size ;
	void			*		field ;
    int             *       selection ;

    int                     i ;

	/* Test inputs */
	if (table->tab_t != QFITS_BINTABLE) return NULL ;

   /* Initialize */
    ctmp[0] = (char)0 ;
    
    /* Set selection to select the requested row */
    selection = calloc(table->nr, sizeof(int)) ;
    selection[row_id] = 1 ;
	
    /* Load the data column */
	if ((field = qfits_query_column_data(table, col_id, selection, 
                    NULL)) == NULL) {
        free(selection) ;
        return NULL ;
    }
    free(selection) ;
	
    /* Set reference to the column */
    col = table->col + col_id ;

    /* Compute field size and allocate stmp */
    field_size = col->atom_nb * ELEMENT_MAX_DISPLAY_SIZE ;
    stmp = malloc(field_size * sizeof(char)) ;
    stmp[0] = (char)0 ;
 
    /* Get the string to write according to the type */
    switch(col->atom_type) {
        case TFITS_BIN_TYPE_A:
        ccol = (char*)field ;
        strncpy(ctmp, ccol, col->atom_size * col->atom_nb) ;
        ctmp[col->atom_size*col->atom_nb] = (char)0 ;
        strcpy(stmp, ctmp) ;
        break ;

        case TFITS_BIN_TYPE_B:
        uccol = (unsigned char*)field ;
        /* Two cases: use col->zero and col->scale or not */
        if (col->zero_present && col->scale_present && use_zero_scale) {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
				sprintf(ctmp, "%f, ", (float)(col->zero +
                        (float)uccol[i] * col->scale)) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
			sprintf(ctmp, "%f", (float)(col->zero +
				(float)uccol[col->atom_nb-1]*col->scale)) ;
            strcat(stmp, ctmp) ;
        } else {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
				sprintf(ctmp, "%d, ", (int)uccol[i]) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
			sprintf(ctmp,"%d",(int)uccol[col->atom_nb-1]);
            strcat(stmp, ctmp) ;
        }
        break ;

        case TFITS_BIN_TYPE_D:
        case TFITS_BIN_TYPE_M:
        dcol = (double*)field ;
        /* Two cases: use col->zero and col->scale or not */
        if (col->zero_present && col->scale_present && use_zero_scale) {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
                sprintf(ctmp, "%g, ", (double)((double)col->zero +
                        dcol[i] * (double)col->scale)) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
            sprintf(ctmp, "%g", (double)((double)col->zero +
                dcol[col->atom_nb-1] * (double)col->scale));
            strcat(stmp, ctmp) ;
        } else {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
                sprintf(ctmp, "%g, ", dcol[i]) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
            sprintf(ctmp, "%g", dcol[col->atom_nb-1]) ;
            strcat(stmp, ctmp) ;
        }
        break ;

        case TFITS_BIN_TYPE_E:
        case TFITS_BIN_TYPE_C:
        fcol = (float*)field ;
        /* Two cases: use col->zero and col->scale or not */
        if (col->zero_present && col->scale_present && use_zero_scale) {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
                sprintf(ctmp, "%f, ", (float)(col->zero +
                        (float)fcol[i] * col->scale)) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
            sprintf(ctmp, "%f", (float)(col->zero +
                (float)fcol[col->atom_nb-1] * col->scale)) ;
            strcat(stmp, ctmp) ;
        } else {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
                sprintf(ctmp, "%f, ", fcol[i]) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
            sprintf(ctmp, "%f", fcol[col->atom_nb-1]) ;
            strcat(stmp, ctmp) ;
        }
        break ;

        case TFITS_BIN_TYPE_I:
        scol = (short*)field ;
        /* Two cases: use col->zero and col->scale or not */
        if (col->zero_present && col->scale_present && use_zero_scale) {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
				sprintf(ctmp, "%f, ", (float)(col->zero +
                        (float)scol[i] * col->scale)) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
			sprintf(ctmp, "%f", (float)(col->zero +
                (float)scol[col->atom_nb-1] * col->scale)) ;
            strcat(stmp, ctmp) ;
        } else {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
				sprintf(ctmp, "%d, ", (int)scol[i]) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
			sprintf(ctmp, "%d",(int)scol[col->atom_nb-1]);
            strcat(stmp, ctmp) ;
        }
        break ;

        case TFITS_BIN_TYPE_J:
        icol = (int*)field ;
        /* Two cases: use col->zero and col->scale or not */
        if (col->zero_present && col->scale_present && use_zero_scale) {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
				sprintf(ctmp, "%f, ", (float)(col->zero +
                        (float)icol[i] * col->scale)) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
			sprintf(ctmp, "%f", (float)(col->zero +
                (float)icol[col->atom_nb-1] * col->scale)) ;
            strcat(stmp, ctmp) ;
        } else {
            /* For each atom of the column */
            for (i=0 ; i<col->atom_nb-1 ; i++) {
				sprintf(ctmp, "%d, ", (int)icol[i]) ;
                strcat(stmp, ctmp) ;
            }
            /* Handle the last atom differently: no ',' */
			sprintf(ctmp, "%d",(int)icol[col->atom_nb-1]);
            strcat(stmp, ctmp) ;
        }
        break ;

        case TFITS_BIN_TYPE_L:
        ccol = (char*)field ;
        /* For each atom of the column */
        for (i=0 ; i<col->atom_nb-1 ; i++) {
            sprintf(ctmp, "%c, ", ccol[i]) ;
            strcat(stmp, ctmp) ;
        }
        /* Handle the last atom differently: no ',' */
        sprintf(ctmp, "%c", ccol[col->atom_nb-1]) ;
        strcat(stmp, ctmp) ;
        break ;

        case TFITS_BIN_TYPE_X:
        uccol = (unsigned char*)field ;
        /* For each atom of the column */
        for (i=0 ; i<col->atom_nb-1 ; i++) {
            sprintf(ctmp, "%d, ", uccol[i]) ;
            strcat(stmp, ctmp) ;
        }
        /* Handle the last atom differently: no ',' */
        sprintf(ctmp, "%d", uccol[col->atom_nb-1]) ;
        strcat(stmp, ctmp) ;
        break ;

        case TFITS_BIN_TYPE_P:
        icol = (int*)field ;
		/* For each atom of the column */
		for (i=0 ; i<col->atom_nb-1 ; i++) {
			sprintf(ctmp, "%d, ", (int)icol[i]) ;
			strcat(stmp, ctmp) ;
		}
		/* Handle the last atom differently: no ',' */
		sprintf(ctmp, "%d",(int)icol[col->atom_nb-1]);
		strcat(stmp, ctmp) ;
		break ;

        default:
        qfits_warning("Type not recognized") ;
        break ;
    }
	free(field) ;
    return stmp ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Remove blanks at the beginning and the end of a string.
  @param    s   String to parse.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end and the beg. of the string have been removed.
  Do not free or modify the returned string! 
  Since the returned string is statically allocated, it will be modified at 
  each function call (not re-entrant).
 */
/*----------------------------------------------------------------------------*/
static char * qfits_strstrip(char * s)
{
    static char l[ASCIILINESZ+1];
    char * last ;

    if (s==NULL) return NULL ;

    while (isspace((int)*s) && *s) s++;

    memset(l, 0, ASCIILINESZ+1);
    strcpy(l, s);
    last = l + strlen(l);
    while (last > l) {
        if (!isspace((int)*(last-1)))
            break ;
        last -- ;
    }
    *last = (char)0;

    return (char*)l ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Make a double out of a string and a number of decimals	
  @param    to_format   the string to convert
  @param    nb_dec      the number of decimals
  @return   the double
  A field with 123 of type F3.1 actually contains 12.3
  This is handled by this function.
 */
/*----------------------------------------------------------------------------*/
static double qfits_str2dec(
        char    *   to_format,
        int         nb_dec)
{
    double      val ;
    int         i ;
    
    /* Test entries */
    if (to_format == NULL) return 0.00 ;
    
    val = (double)atof(to_format) ;
    /* First handle case where there are no decimals or the dot is there */
    if ((strstr(to_format, ".") == NULL) && (nb_dec > 0)) {
        for (i=0 ; i<nb_dec ; i++) val /=10 ;
    }
    return val ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Parse a FITS type	
  @param	str		string read in the FITS header (e.g. TFORM value)
  @param	nb		pointer to the number
  @param    dec_nb  pointer to the number of decimals
  @param	type	pointer to the type
  @param	table_type	Table type (BIN, ASCII, ...)
  @return	0 if ok, -1 otherwise

  This functions reads the input string and uses it to update nb and type
 */
/*----------------------------------------------------------------------------*/
static int qfits_table_interpret_type(
		char		*	str,
		int			*	nb,
        int         *   dec_nb,
		tfits_type	*	type,
		int				table_type)
{
	char		type_c ;
     
    *dec_nb = 0 ;
	if (table_type == QFITS_BINTABLE) {
		if (sscanf(str, "%d%c", nb, &type_c) == 0) {
			/* nb is 1 by default */
			if (sscanf(str, "%c", &type_c) == 0) {
				qfits_error("cannot interpret this type: %s", str) ;
				return -1 ;
			}
			*nb = 1 ;
		}
		switch(type_c) {
			case 'A': *type = TFITS_BIN_TYPE_A ; break ;
			case 'B': *type = TFITS_BIN_TYPE_B ; break ;
			case 'C': *type = TFITS_BIN_TYPE_C ; break ;
			case 'D': *type = TFITS_BIN_TYPE_D ; break ;
			case 'E': *type = TFITS_BIN_TYPE_E ; break ;
			case 'I': *type = TFITS_BIN_TYPE_I ; break ;
			case 'J': *type = TFITS_BIN_TYPE_J ; break ;
			case 'L': *type = TFITS_BIN_TYPE_L ; break ;
			case 'M': *type = TFITS_BIN_TYPE_M ; break ;
			case 'P': *type = TFITS_BIN_TYPE_P ; break ;
			case 'X': *type = TFITS_BIN_TYPE_X ; break ;
			default: return -1 ; 
		}
	} else if (table_type == QFITS_ASCIITABLE) {
		if (sscanf(str, "%c%d.%d", &type_c, nb, dec_nb) == 0) {
			qfits_error("cannot interpret this type: %s", str) ;
			return -1 ;
		}
		switch(type_c) {
			case 'A': *type = TFITS_ASCII_TYPE_A ; break ;
			case 'D': *type = TFITS_ASCII_TYPE_D ; break ;
			case 'E': *type = TFITS_ASCII_TYPE_E ; break ;
			case 'F': *type = TFITS_ASCII_TYPE_F ; break ;
			case 'I': *type = TFITS_ASCII_TYPE_I ; break ;
			default: return -1 ;
		}
	} else {
		qfits_error("unrecognized table type") ;
		return -1 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a FITS type string
  @param	col		input column
  @return	The string to write to TFORM
 */
/*----------------------------------------------------------------------------*/
static char * qfits_build_format(qfits_col * col)
{
	static char sval[10] ;
    int         nb ;
		
	switch (col->atom_type) {
		case TFITS_ASCII_TYPE_A: 
            nb=sprintf(sval, "A%d.%d", col->atom_nb, col->atom_dec_nb) ; break ;
		case TFITS_ASCII_TYPE_D: 
            nb=sprintf(sval, "D%d.%d", col->atom_nb, col->atom_dec_nb) ; break ;
		case TFITS_ASCII_TYPE_E: 
            nb=sprintf(sval, "E%d.%d", col->atom_nb, col->atom_dec_nb) ; break ;
		case TFITS_ASCII_TYPE_I: 
            nb=sprintf(sval, "I%d.%d", col->atom_nb, col->atom_dec_nb) ; break ;
		case TFITS_ASCII_TYPE_F: 
            nb=sprintf(sval, "F%d.%d", col->atom_nb, col->atom_dec_nb) ; break ;
		case TFITS_BIN_TYPE_D: nb=sprintf(sval, "%dD", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_E: nb=sprintf(sval, "%dE", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_I: nb=sprintf(sval, "%dI", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_A: nb=sprintf(sval, "%dA", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_B: nb=sprintf(sval, "%dB", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_C: nb=sprintf(sval, "%dC", 
                                       (int)(col->atom_nb/2)) ; break ;
		case TFITS_BIN_TYPE_J: nb=sprintf(sval, "%dJ", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_L: nb=sprintf(sval, "%dL", col->atom_nb) ; break ;
		case TFITS_BIN_TYPE_M: nb=sprintf(sval, "%dM", 
                                       (int)(col->atom_nb/2)) ; break ;
		case TFITS_BIN_TYPE_P: nb=sprintf(sval, "%dP", 
                                       (int)(col->atom_nb/2)) ; break ;
		case TFITS_BIN_TYPE_X: nb=sprintf(sval, "%dX", 
                                       8*col->atom_nb) ; break ;
		default: return NULL ;
	}
	sval[nb] = (char)0 ;
	return sval ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Appends a std extension header + data to a FITS BIN table file.
  @param	outfile		Pointer to (opened) file ready for writing.
  @param	t			Pointer to qfits_table
  @param	data		Table data to write
  @return	int 0 if Ok, -1 otherwise

  Dumps a FITS table to a file. The whole table described by qfits_table, and 
  the data arrays contained in 'data' are dumped to the file. An extension 
  header is produced with all keywords needed to describe the table, then the 
  data is dumped to the file.
  The output is then padded to reach a multiple of 2880 bytes in size.
  Notice that no main header is produced, only the extension part.
 */
/*----------------------------------------------------------------------------*/
static int qfits_table_append_bin_xtension(
        FILE            *   outfile,
        qfits_table    	*   t,
        void            **  data)
{
	qfits_header	*	fh ;

    if ((fh=qfits_table_ext_header_default(t)) == NULL) {
		qfits_error("cannot create new fits header") ;
		return -1 ;
	}

    /* Write the fits header in the file  */
    if (qfits_header_dump(fh, outfile) == -1) {
		qfits_error("cannot dump header in file") ;
		qfits_header_destroy(fh) ;
		fclose(outfile) ;
		return -1 ;
	}
	qfits_header_destroy(fh) ;

    /* Append the data to the file */
    return qfits_table_append_data(outfile, t, data) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Appends an extension header + data to a FITS ASCII table file.
  @param	outfile		Pointer to (opened) file ready for writing.
  @param	t			Pointer to qfits_table
  @param	data		Table data to write
  @return	int 0 if Ok, -1 otherwise

  Dumps a FITS table to a file. The whole table described by
  qfits_table, and the data arrays contained in 'data' are dumped to
  the file. An extension header is produced with all keywords needed
  to describe the table, then the data is dumped to the file.

  The output is then padded to reach a multiple of 2880 bytes in size.

  Notice that no main header is produced, only the extension part.
 */
/*----------------------------------------------------------------------------*/
static int qfits_table_append_ascii_xtension(
        FILE            *   outfile,
        qfits_table    	*   t,
        void            **  data)
{
	qfits_header	*	fh ;
	
    if ((fh=qfits_table_ext_header_default(t)) == NULL) {
		qfits_error("cannot create new fits header") ;
		return -1 ;
	}

    /* Write the fits header in the file  */
    if (qfits_header_dump(fh, outfile) == -1) {
		qfits_error("cannot dump header in file") ;
		qfits_header_destroy(fh) ;
		return -1 ;
	}
	qfits_header_destroy(fh) ;
    
    /* Append the data to the file */
    return qfits_table_append_data(outfile, t, data) ;
} 

/*----------------------------------------------------------------------------*/
/**
  @brief	Appends data to a FITS table file.
  @param	outfile		Pointer to (opened) file ready for writing.
  @param	t			Pointer to qfits_table
  @param	data		Table data to write
  @return	int 0 if Ok, -1 otherwise

  Dumps the data part of a FITS table to a file. The primary header, as well as
  the extension header are supposed to be already there (and properly padded).
  The output is then padded to reach a multiple of 2880 bytes in size.
 */
/*----------------------------------------------------------------------------*/
static int qfits_table_append_data(
        FILE        *   outfile, 
        qfits_table *   t, 
        void        **  data) 
{
	qfits_col       *   curr_col ;
    char                field[1024] ;
    char            *   line ;
    unsigned char   **  array ;
    unsigned char   *   r ;
    unsigned char   *   inbuf ;
    int                 writt_char ;
    int                 nb_blanks ;
    int                 field_size ;
    int                 i, j ;

    /* Write DATA */
    array = malloc(t->nc*sizeof(unsigned char *)) ;

    curr_col = t->col ;
    for (i=0 ; i<t->nc ; i++) {
        /* Compute the field size */
        field_size = qfits_table_get_field_size(t->tab_t, curr_col) ;

        /* Copy data from data to array (unsigned char) */
        array[i] = malloc(t->nr * field_size) ;
        r = (unsigned char *)array[i] ;
        inbuf = (unsigned char *)(data[i]) ;

        /* Copy the data */
        if (t->tab_t == QFITS_ASCIITABLE) {
            /* ASCII table */
            for (j=0 ; j<t->nr ; j++) {
                switch(curr_col->atom_type) {
                    case TFITS_ASCII_TYPE_A :
                        strncpy(field, (char*)inbuf, curr_col->atom_nb) ;
                        field[curr_col->atom_nb] = (char)0 ;
                        inbuf += curr_col->atom_nb ;
                        break ;
                    case TFITS_ASCII_TYPE_D :
                        memset(field, ' ', curr_col->atom_nb) ;
                        sprintf(field, "%g", ((double*)data[i])[j]) ;
                        field[curr_col->atom_nb] = (char)0 ;
                        break ;
                    case TFITS_ASCII_TYPE_E :
                    case TFITS_ASCII_TYPE_F :
                        memset(field, ' ', curr_col->atom_nb) ;
                        sprintf(field, "%f", ((float*)data[i])[j]) ;
                        field[curr_col->atom_nb] = (char)0 ;
                        break ;
                    case TFITS_ASCII_TYPE_I :
                        memset(field, ' ', curr_col->atom_nb) ;
                        sprintf(field, "%d", ((int*)data[i])[j]) ;
                        field[curr_col->atom_nb] = (char)0 ;
                        break ;
                    default:
                        break ;
                }
                memcpy(r, field, curr_col->atom_nb) ;
                r += (curr_col->atom_nb) ;
            }
        } else if (t->tab_t == QFITS_BINTABLE) {
            /* BINARY table */
            for (j=0 ; j<t->nr ; j++) {
                memcpy(r, inbuf, field_size) ;
                inbuf += field_size ;
                r += field_size ;
            }

            /* Byte swapping needed if on a little-endian machine */
#ifndef WORDS_BIGENDIAN
            r = array[i] ;
            for (j=0 ; j<t->nr * curr_col->atom_nb ; j++) {
                swap_bytes(r, curr_col->atom_size);
                r += curr_col->atom_size ;
            }
#endif
        } else return -1 ;
        curr_col++ ;
    }

    /* Write to the outfile */
    writt_char = 0 ;
    for (i=0 ; i<t->nr ; i++) {
        curr_col = t->col ;
        for (j=0 ; j<t->nc ; j++) {
            field_size = qfits_table_get_field_size(t->tab_t, curr_col) ;
            r = array[j] + field_size * i ;
            line = (char *)calloc (field_size+1, sizeof (char)) ;
            memcpy(line, r, field_size) ;
            line[field_size] = (char)0 ;
            fwrite(line, 1, field_size, outfile) ;
            writt_char += field_size ;
            curr_col++ ;
            free(line) ;
        }
    }

    /* Complete with blanks to FITS_BLOCK_SIZE characters */
    if (writt_char % FITS_BLOCK_SIZE) {
        nb_blanks = FITS_BLOCK_SIZE - (writt_char%FITS_BLOCK_SIZE) ;
        for (i=1 ; i<=nb_blanks ; i++) fwrite(" ", 1, 1, outfile) ;
    }

    /* Free and return  */
    for(i=0 ; i<t->nc ; i++) {
        if (array[i] != NULL) free(array[i]) ;
    }
    free(array) ;
    return  0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Get the size in bytes of a field
  @param    type    table type
  @param    col     a column
  @return   the size
 */
/*----------------------------------------------------------------------------*/
static int qfits_table_get_field_size(
        int             type,
        qfits_col   *   col)
{
    int     field_size ;

	switch (type) {
		case QFITS_BINTABLE:
			field_size = col->atom_nb * col->atom_size ;
			break ;
		case QFITS_ASCIITABLE:
			field_size = col->atom_nb ;
			break ;
		default:
			qfits_warning("unrecognized table type") ;
			field_size = -1 ;
	}
    return field_size ;
}
