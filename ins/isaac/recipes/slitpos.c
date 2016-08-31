/*----------------------------------------------------------------------------*/
/**
   @file    slitpos.c
   @author  Y. Jung
   @date    Feb. 2001
   @version	$Revision: 1.49 $
   @brief   Slit position 
*/  
/*----------------------------------------------------------------------------*/

/*
	$Id: slitpos.c,v 1.49 2003/03/28 14:46:17 yjung Exp $
	$Author: yjung $
	$Date: 2003/03/28 14:46:17 $
	$Revision: 1.49 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int slitpos_engine(char *, char *, int) ;
static int slitpos_write_paffile(char *, char *, double, double, double) ;
static int slitpos_write_outfile(char *, int, double, double3 **, 
		framelist *, int) ;

/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int isaac_slitpos_main(void * dict)
{
	dictionary	*	d ;
		
	int				slit_max_width ;
	
	char			argname[10] ;
	char    	*	name_i ;
    char    	*	name_o ;
    int     		nfiles ;

	int				errors ;
	int				i ;
	 
	d = (dictionary*)dict ;
   	/* Get options */
	slit_max_width   = dictionary_getint(d, "arg.max_width", 20) ;

	/* Get input/output file names */
    nfiles = dictionary_getint(d, "arg.n", -1) ;
    if (nfiles<0) {
        e_error("missing input file name(s): aborting");
        return -1 ;
    }
    /* Loop on input file names */
	errors = 0 ;
	for (i=1 ; i<nfiles ; i++) {
		sprintf(argname, "arg.%d", i);
		name_i = dictionary_get(d, argname, NULL) ;
		name_o = dictionary_get(d, "arg.output", NULL) ;
		if (name_o == NULL) name_o = strdup(get_rootname(get_basename(name_i)));
		else name_o = strdup(get_rootname(name_o)) ;
		
		/* Once command-line options have been cleared out, call the engine */
    	errors += slitpos_engine(name_i, name_o, slit_max_width) ;
		free(name_o) ;
	}
	return errors ;
}

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/**
  @brief    engine for the is_spec_slitpos recipe
  @param    inname  input file
  @param    outname output file name
  @param    slit_max_width  maximum slit width
  @return   int -1 in error case
  
  Find out if the first frame is a dark and subtract it from slit images.
  Then detect the slit on each slit images
 */
/*----------------------------------------------------------------------------*/
static int slitpos_engine(
        char    *   inname,
        char    *   outname,
        int         slit_max_width)
{
	cube_t		*	images ;
	framelist	*	filenames ;
	char		*	current_file ;
	char			output_name[FILENAMESZ] ;
	char		*	mode ;
	int				dark_present ;
	image_t		*	dark ;
	int				first_frame_ind ;
	double				slit_angle ;
	int				slit_length ;
	double			xcenter,
					ycenter ;

	double3		**	out_table ;				
    instrument_t    ins ;
	
	int				i,
					j ;
	
	/* Initialize */
    ins = pfits_identify_insstr("isaac") ;
	dark = NULL ;
	dark_present = 0 ;
	first_frame_ind = 0 ;
	 
	/* Prepare the cube for reduction */

	/* If the input file is a FITS file */
	if (is_fits_file(inname)) {
		images = cube_load(inname) ;
		if (images == NULL) {
			e_error("cannot load FITS file [%s]", inname) ;
			return -1 ;
		}
	
		/* Fill filenames */
		filenames = framelist_new(1) ;
		filenames->name[0] = strdup(inname) ;
		
	/* If the input file is an ASCII file */
	} else {
	
		/* Read the input ASCII file  */
		filenames = framelist_load(inname) ;
		if (filenames == NULL) {
			e_error("cannot read the input ASCII file: [%s]", inname) ;
			return -1 ;
		}
	
		/* Find out if the first frame is a dark and load it */
		current_file = filenames->name[0] ;
		mode = pfits_get(ins, current_file, "mode") ;
		if (!strcmp(mode, "SW_DARK") || !strcmp(mode, "LW_DARK")) {
			e_comment(1, "dark present: %s", current_file) ;
			dark_present = 1 ;
			dark = image_load(current_file) ;
			first_frame_ind = 1 ;
			
			/* Test if there is not only the dark */
			if (filenames->n < 2) {
				e_error("only a DARK frame in the list - abort") ;
				framelist_del(filenames) ;
				image_del(dark) ;
				return -1 ;
			}
		} else {
			e_comment(1, "No dark present") ;
		}
		
		/* Load the cube */
		images = cube_load(inname) ;
		if (images == NULL) {
			e_error("cannot load ASCII file [%s]", inname) ;
			framelist_del(filenames) ;
			if (dark_present) image_del(dark) ;
			return -1 ;
		}

		/* Subtract the dark if present */
		if (dark_present) {
			if (cube_sub_im(images, dark) == -1) {
				e_error("cannot subtract dark - abort") ;
				image_del(dark) ;
				cube_del(images) ;
				framelist_del(filenames) ;
				return -1 ;
			}
			image_del(dark) ;
		}
	}
	
	/* Loop on all the slit images */
	for (i=first_frame_ind ; i<images->np ; i++) {
		e_comment(1, "Slit image no %d", i+1) ;
		/* Slit analysis */
		if ((out_table = slitpos_analysis(images->plane[i],
								slit_max_width,
								&slit_angle,
								&slit_length)) == NULL) {
			e_error("in slit position analysis: [%s]", filenames->name[i]) ;
		} else {
			sprintf(output_name, "%s_%d.tfits", outname, i+1) ;
			/* Write the output files (TFITS & PAF) */
			if (slitpos_write_outfile(output_name,
										slit_length,
										slit_angle,
										out_table,
										filenames, 
										i) == -1) {
				e_error("in writing the output FITS table") ;
				framelist_del(filenames) ;
				cube_del(images) ;
				for (j=0 ; j<3 ; j++) {
					double3_del(out_table[j]) ;
				}
				free(out_table) ;
				return -1 ;
			}
			
			xcenter = (out_table[1]->x[0]+out_table[1]->x[slit_length-1])/2.0 ;
			ycenter = (out_table[1]->y[0]+out_table[1]->y[slit_length-1])/2.0 ;
			for (j=0 ; j<3 ; j++) {
				double3_del(out_table[j]) ;
			}
			free(out_table) ;

			/* Write the output PAF file */
			sprintf(output_name, "%s.paf", get_rootname(output_name)) ;
			if (slitpos_write_paffile(output_name,
									filenames->name[i],
									xcenter,
									ycenter,
									slit_angle) == -1) {
				e_error("in writing the output PAF file") ;
				framelist_del(filenames) ;
				cube_del(images) ;
				return -1 ;
			}
		}
	}

	/* Free and return */
	framelist_del(filenames) ;
	cube_del(images) ;
	return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief	Write the output PAF file 
  @param	outname	name of the out file
  @param	inname	name of the input FITS file
  @param	xcenter	X coordinate of the slit center
  @param	ycenter Y coordinate of the slit center
  @param	slit_angle	slit angle with the horizontal in degrees
  @return	-1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int slitpos_write_paffile(
		char	*	outname,
		char	*	inname,
		double		xcenter,
		double		ycenter,
		double		slit_angle)
{
	FILE	    *	paf ;
	char	    *	mjd_obs ;
	char	    *	strvar ;
    instrument_t    ins ;
	
	/* Initialize */
    ins = pfits_identify_insstr("isaac") ;
    
    paf = qfits_paf_print_header( outname,
                            "ISAAC/slitpos",
                            "Slit position recipe results",
                            get_login_name(),
                            get_datetime_iso8601()) ;
   if (paf == NULL) {
       e_warning("cannot output PAF file") ;
   } else {
	   fprintf(paf, "\n");
	   /* ARCFILE */
	   strvar = pfits_get(ins, inname, "arcfile") ;
	   if (strvar != NULL) {
		   fprintf(paf, "ARCFILE   \"%s\"    \n", strvar) ;
	   }
       /* MJD-OBS */
	   mjd_obs = pfits_get(ins, inname, "mjdobs") ;
       if (mjd_obs!=NULL) {
           fprintf(paf, "MJD-OBS  %s; # Obs start\n\n", mjd_obs);
       } else {
           fprintf(paf, "MJD-OBS  0.0; # Obs start unknown\n\n");
       }
	   /* INSTRUME keyword  */
	   strvar = pfits_get(ins, inname, "instrument") ;
	   if (strvar != NULL) {
		   fprintf(paf, "INSTRUME \"%s\" \n", strvar) ; 
	   } 
	   /* TPL.ID  */
	   strvar = pfits_get(ins, inname, "templateid") ;
	   if (strvar != NULL) {
		   fprintf(paf, "TPL.ID  \"%s\" \n", strvar) ;
	   }
	   /* TPL.NEXP */
	   strvar = pfits_get(ins, inname, "numbexp") ;
	   if (strvar != NULL) {
		   fprintf(paf, "TPL.NEXP  %s \n", strvar) ;
	   } 
	   /* DPR.CATG */
	   strvar = pfits_get(ins, inname, "dpr_catg") ;
	   if (strvar != NULL) {
		   fprintf(paf, "DPR.CATG  \"%s\" \n", strvar) ;
	   }
	   /* DPR.TYPE */
	   strvar = pfits_get(ins, inname, "dpr_type") ;
	   if (strvar != NULL) {
		   fprintf(paf, "DPR.TYPE  \"%s\" \n", strvar) ;
	   }
	   /* DPR.TECH */
	   strvar = pfits_get(ins, inname, "dpr_tech") ;
	   if (strvar != NULL) {
		   fprintf(paf, "DPR.TECH  \"%s\" \n", strvar) ;
	   }
        /* Add PRO.CATG */
        fprintf(paf, "PRO.CATG \"%s\" ;# Product category\n",
                pfits_getprokey(ins, procat_spec_slitpos_qc)) ;
        /* Add the date */
        fprintf(paf, "DATE-OBS \"%s\" ;# Date\n",
                pfits_get(ins, inname, "date_obs")) ;
	   /* INS.OPTI1.ID */
	   strvar = pfits_get(ins, inname, "optical_id") ;
	   if (strvar != NULL) {
		   fprintf(paf, "INS.OPTI1.ID  \"%s\" \n", strvar) ;
	   }
	   /* QC.SLIT.XPOS  */
	   fprintf(paf, "QC.SLIT.XPOS  %g \n", xcenter) ;
	   
	   /* QC.SLIT.YPOS */
	   fprintf(paf, "QC.SLIT.YPOS  %g \n", ycenter) ;
	   
	   /* QC.SLIT.ANGLE */
	   fprintf(paf, "QC.SLIT.POSANG  %g \n", slit_angle) ;
	  
	   fclose(paf) ;
	   e_comment(0, "file [%s] produced", outname) ; 	
   }

   return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Write the output FITS table		
  @param	outname	name of out file	
  @param	slit_length	length of the out table
  @param	slit_angle	angle of the slit
  @param	out_table	result of slitpos_analysis()
  @param	filenames	list of input file names (charmatrix)
  @param	int			file id 
  @return	-1 in error case, 0 otherwise
 */
/*----------------------------------------------------------------------------*/
static int slitpos_write_outfile(
	char		*	outname,
	int				slit_length,
	double			slit_angle,
	double3		**	out_table,
	framelist	*	filenames,
	int				file_id)
{
	qfits_header	*	fh ;
	qfits_table		*	table ;
	qfits_col		*	col ;
	int					i, j ;
	double			**	data ;
	
	/* Write the output qfits_table table (informations) */
	table = qfits_table_new(outname, QFITS_BINTABLE, -1, 4, slit_length) ;
    col = table->col ;
    for (i=0 ; i<table->nc ; i++) {
    	qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, "pixel", 
                " ", " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
		col++ ;
    }
	col = table->col ;
	strcpy(col->tlabel, "Y") ;
	col++ ;
	strcpy(col->tlabel, "LEFT_POSITION") ;
	col++ ;
	strcpy(col->tlabel, "CENTER_POSITION") ;
	col++ ;
	strcpy(col->tlabel, "RIGHT_POSITION") ;

    /* Put out_table in data */
    data = malloc(table->nc * sizeof(double*)) ;
    for (i=0 ; i<table->nc ; i++) {
        data[i] = malloc(table->nr * sizeof(double)) ;
    }
	for (j=0 ; j<table->nr ; j++) {
		data[0][j] = out_table[0]->y[j] ;
		data[1][j] = out_table[0]->x[j] ;
		data[2][j] = out_table[1]->x[j] ;
		data[3][j] = out_table[2]->x[j] ;
	}

	/* WRITE THE OUTPUT FILE */
	/* Read the input header */
	if ((fh = qfits_header_read(filenames->name[file_id])) == NULL) {
        e_error("in writing the output fits file") ;
		for (i=0 ; i<table->nc ; i++) free(data[i]) ;
        free(data) ;
		qfits_table_close(table) ;
        return -1 ;
    }

	/* Prepare it for table output */
    if (isaac_header_for_table(fh) == -1) {
        e_error("in writing the output fits file") ;
		for (i=0 ; i<table->nc ; i++) free(data[i]) ;
        free(data) ;
        qfits_header_destroy(fh) ;
		qfits_table_close(table) ;
        return -1 ;
    }

	/* Write the PRO keywords in the header */
    if (isaac_pro_fits(fh,
                        outname,
                        "REDUCED",
                        NULL,
                        procat_spec_slitpos_table,
                        "OK",
                        "img_tec_slitposition",
                        filenames->n,
						filenames,
						NULL) == -1) {
        e_error("in writing PRO keywords in output file") ;
		for (i=0 ; i<table->nc ; i++) free(data[i]) ;
        free(data) ;
        qfits_header_destroy(fh) ;
		qfits_table_close(table) ;
        return -1 ;
    }

	/* Write the HISTORY keywords with the input file names */
    if (isaac_add_files_history(fh, filenames) == -1) {
        e_warning("cannot write HISTORY keywords in out file") ;
    }

	/* Write the file on disk */
    if (qfits_save_table_hdrdump((void**)data, table, fh) == -1) {
        e_error("cannot write file: %s", outname) ;
        qfits_header_destroy(fh) ;
		qfits_table_close(table) ;
		for (i=0 ; i<table->nc ; i++) free(data[i]) ;
        free(data) ;
        return -1 ;
    }
	qfits_table_close(table) ;
    qfits_header_destroy(fh) ;
	for (i=0 ; i<table->nc ; i++) free(data[i]) ;
    free(data) ;

    e_comment(0, "File [%s] produced", outname) ;

    /* Write slit_angle slit_length and the slit center coordinates */ 
	/* on the screen */
	e_comment(0,"Slit angle with horizontal (in deg): %g\n", slit_angle) ;
	e_comment(0,"Slit center coordinates: (%g, %g)\n", 
					(out_table[1]->x[0]+out_table[1]->x[slit_length-1])/2.0,
				(out_table[1]->y[0]+out_table[1]->y[slit_length-1])/2.0) ;
	e_comment(0,"Slit length in pixels: %d\n", slit_length) ;
	e_comment(0,"file [%s] produced", outname) ;

	return 0 ;
}

