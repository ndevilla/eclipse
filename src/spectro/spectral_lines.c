/*-------------------------------------------------------------------------*/
/**
   @file	spectral_lines.c
   @author	N. Devillard
   @date	November 1999
   @version	$Revision: 1.46 $
   @brief	spectrum line handling routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: spectral_lines.c,v 1.46 2003/11/20 14:12:44 llundin Exp $
	$Author: llundin $
	$Date: 2003/11/20 14:12:44 $
	$Revision: 1.46 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <math.h>

#include "spectral_lines.h"
#include "comm.h"
#include "emission_lines.h"

/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#ifndef ASCIILINESZ
#define ASCIILINESZ		1024
#endif

/*---------------------------------------------------------------------------
  							Function prototypes
 ---------------------------------------------------------------------------*/

static int emission_lines_compare(const void * e1, const void * e2)  ;

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Initialize a spectral line table.
  @param	path	Name of the table to initialize.
  @return	1 newly allocated spectral_table object.

  The received path string indicates where the spectral table should be
  loaded from.

  \begin{tabular}{ll}
  "oh"			&	OH table \\
  "Xe"   		&	Xenon table \\
  "Ar"   		&	Argon table \\
  "Xe+Ar"	    &	Xenon+Argon table \\
  /path/file	&	User-specified external table
  \end{tabular}

  If the given table name corresponds to a user-specified table, it is 
  loaded from the disk and dynamically allocated. 
  See spectral_table_parse_list() for the acceptable
  The returned table must be deallocated using spectral_table_destroy()
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_init(char * path)
{
	spectral_table *	spt1 ;
	spectral_table *	spt2 ;
	spectral_table *	spt ;

	if (!strcmp(path, "oh")) {
		spt=spectral_table_select((spectral_table*)&emission_lines_table,"oh");
	} else if (!strcmp(path, "Xe")) {
		spt=spectral_table_select((spectral_table*)&emission_lines_table,"Xe");
	} else if (!strcmp(path, "Ar")) {
		spt=spectral_table_select((spectral_table*)&emission_lines_table,"Ar");
	} else if (!strcmp(path, "Xe+Ar")) {
		spt1=spectral_table_select((spectral_table*)&emission_lines_table,"Xe");
		spt2=spectral_table_select((spectral_table*)&emission_lines_table,"Ar");
		spt = spectral_table_merge(spt1, spt2) ;
		spectral_table_destroy(spt1) ;
		spectral_table_destroy(spt2) ;
	} else {
		/* The specified list exists: load it */
		spt = spectral_table_parse_list(path);
		if (spt == NULL) {
			e_error("parsing file [%s]", path);
		}
	}
	spectral_table_sort(spt) ;
        if (spt->nlines < 1) {
            spectral_table_destroy(spt) ;
            return NULL;
        }
	return spt ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Create a spectral table
  @param	size	Size of the created table
  @return	the allocated spectral table
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_create(int size)
{
	spectral_table	*	spt ;

	spt = malloc(sizeof(spectral_table)) ;
	spt->nlines = size ;
	spt->lines = malloc(size * sizeof(emission_line)) ;
	return spt ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Deallocate a spectral table.
  @param	spt	Spectral table to deallocate.
  @return	void
 */
/*--------------------------------------------------------------------------*/
void spectral_table_destroy(spectral_table * spt)
{
	if (spt == NULL) return ;
	free(spt->lines) ;
	free(spt) ;
	return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Sort a spectral table 
  @param	table	spectral table to sort (modified) 
  @return	0 if Ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int spectral_table_sort(spectral_table * table)
{
	if (table == NULL) return -1 ;

	qsort((void*)table->lines, (size_t)table->nlines, sizeof(emission_line),
			emission_lines_compare) ;
	return 0 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Compare two emission lines 
  @param	e1	first emission line
  @param	e2	second emission line
  @return	int 1 if e1->wavel < e2->wavel, -1 otherwise.
 */
/*--------------------------------------------------------------------------*/
static int emission_lines_compare(const void * e1, const void * e2) 
{
	if (((emission_line*)e1)->wavel < ((emission_line*)e2)->wavel) return -1 ;
    else return 1 ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Dump a spectral table
  @param	table	spectral table to dump
  @param	out     Opened file pointer
  @return	0 if Ok, -1 otherwise
 */
/*--------------------------------------------------------------------------*/
int spectral_table_dump(
		spectral_table	*	table,
		FILE			*	out)
{
	int		i ;
		
	/* Check input parameters */
    if (table==NULL) return -1 ;
    if (out==NULL) out=stdout ;

	for (i=0 ; i<table->nlines ; i++) {
		fprintf(out, "%g\t%g\t%s\n", (table->lines[i]).wavel,
				(table->lines[i]).intens, (table->lines[i]).type) ;
	}
    return 0 ;  
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Merge two spectral tables
  @param	spt1	First spectral table
  @param	spt2	Second spectral table
  @return	a spectral table composed with the two input ones
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_merge(
		spectral_table	*	spt1,
		spectral_table	*	spt2)
{
	spectral_table	*	spt ;
	int					i ;

	spt = spectral_table_create(spt1->nlines + spt2->nlines) ;
	
	/* Merge the tw input tables */
	if (spt != NULL) {
		for (i=0 ; i<spt1->nlines ; i++) {
			(spt->lines[i]).wavel = (spt1->lines[i]).wavel ;
			(spt->lines[i]).intens = (spt1->lines[i]).intens ;
			strcpy((spt->lines[i]).type, (spt1->lines[i]).type) ;
		}
		for (i=0 ; i<spt2->nlines ; i++) {
			(spt->lines[i+spt1->nlines]).wavel = (spt2->lines[i]).wavel ;
			(spt->lines[i+spt1->nlines]).intens = (spt2->lines[i]).intens ;
			strcpy((spt->lines[i+spt1->nlines]).type, (spt2->lines[i]).type) ;
		}
	}
	return spt ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Select lines in a spectral table
  @param	ref		reference spectral table
  @param	type	type of lines selected
  @return	a spectral table created with selected lines
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_select(
		spectral_table	*	ref,
		char			*	type)
{
	spectral_table	*	selected ;
	int					nb_selected ;
	int					i, j ;

	/* Initialize */
	nb_selected = 0 ;

	/* Count the number of selected lines */
	for (i=0 ; i<ref->nlines ; i++) {
		if (!strcmp(type, (ref->lines[i]).type)) nb_selected ++ ;
	}
	
	if (nb_selected == 0) return NULL ;
	
	/* Allocate the output table */
	selected = spectral_table_create(nb_selected) ;

	/* Copy the selected lines in the output spectral table */
	j = 0 ;
	for (i=0 ; i<ref->nlines ; i++) {
		if (!strcmp(type, (ref->lines[i]).type)) {
			(selected->lines[j]).wavel = (ref->lines[i]).wavel ;
			(selected->lines[j]).intens = (ref->lines[i]).intens ;
			strcpy((selected->lines[j]).type, (ref->lines[i]).type) ;
			j++ ;
		}
	}
	return selected ;
}
		
/*-------------------------------------------------------------------------*/
/**
  @brief	Read a spectral table from an external file.
  @param	path	Full path name of the file to load.
  @return	1 pointer to newly allocated spectral table.

  This function loads an external spectral table into a spectral_table
  object. The file must be an ASCII file, following these properties:

  \begin{itemize}
  \item Lines starting with a '#' are comments and ignored.
  \item Blank lines are ignored.
  \item Spectral lines are given as two values per line, separated by any
  number of blanks or tabs. The first value gives the wavelength in
  Angstroems, the second gives the relative intensity, which must be
  consistent with all lines in the same table.
  \end{itemize}

  The returned object must be deallocated using spectral_table_destroy().
 */
/*--------------------------------------------------------------------------*/
spectral_table * spectral_table_parse_list(char * path)
{
	spectral_table 	*	spt ;
	FILE	 		*	listfile ;
	int					i ;
	int					values_on_line ;
	int					nlines ;
	double				wave,
						irel ;
	int					lineno ;
	char				line[ASCIILINESZ];

	if ((listfile=fopen(path, "r"))==NULL) {
		e_error("cannot open file [%s]", path);
		return NULL ;
	}

	/* Count how many lines in the given file */
	nlines = 0 ;
	lineno = 0 ;
	while (fgets(line, ASCIILINESZ, listfile)!=NULL) {
		lineno++ ;
		if (line[0]!='#') {
			values_on_line = sscanf(line, "%lg %lg", &wave, &irel);
			if (values_on_line != 2) {
				e_error("in file %s (%d): expected two values",
						path, lineno);
				return NULL ;
			}
			nlines++ ;
		}
	}

	/* Allocate output table */
	spt = malloc(sizeof(spectral_table));
	spt->nlines = nlines ;
	spt->lines = malloc(nlines * sizeof(emission_line)) ;

	/* Now load line information */
	rewind(listfile);
	i=0 ;
	while (fgets(line, ASCIILINESZ, listfile)!=NULL) {
		if (line[0]!='#') {
			values_on_line = sscanf(line, "%lg %lg", &wave, &irel);
			if (values_on_line==2) {
				(spt->lines[i]).wavel = wave ;
				(spt->lines[i]).intens = irel ;
				strcpy((spt->lines[i]).type, "EF") ;
				i++ ;
			}
		}
	}
	fclose(listfile);
	return spt ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Build a 1d signal from a spectral table 
  @param    spt         Spectral table to use.
  @param    disprel     4 coeffs of the wavelength calibration polynomial
  @param    order       Order used in the spectral table look-up
  @param    slit_width  Width in pixels of the slit used
  @param    size        Size of the signal to generate.
  @param    found       Number of non-zero samples in signal
  @return   1 pointer to a newly allocated array of 'size' doubles.

  Provide an allocated spectral table and a 3rd deegree wavelength calibration
  polynomial, and the size in pixels of the signal to generate.
  The returned array is a 1d signal (of doubles) containing the spectral
  lines as pixels.

  The spectral lines are smoothed (with a gaussian). Spectral lines just outside
  the range of the wavelength calibration polynomial, i.e. lines just below
  wl_low = WAVELEN(1-0.5) and just above wl_high = WAVELEN(size+0.5) are thus
  also used to generate the spectrum.

  The intensities of the spectrum are transformed with log(1+I).

  Returns NULL in case of error (i.e. no emission lines found,
  or non-positive slit width).
 */
/*--------------------------------------------------------------------------*/
double * spectral_table_build_signal(
        const spectral_table * spt,
        const double         * disprel,
        const int              order,
        const double           slit_width,
        const int              size,
        int                  * found)
{
    double         wl_high;

    /* Represent each line as a gaussian with sigma = slit_width/4 */
    const double sigma  = slit_width * SLITWIDTH_TO_SIGMA;
    double f1, f2;

    double * smooth ;

    const int gwidth = 6*sigma; /* cut-off below exp(-6*6/2) */
    int i, ii;
    int j       = 0;
    int lines   = 0;
    int ilines;


    *found = 0;

    if (slit_width <= 0) {
        e_error("non-positive slit_width, aborting");
        return NULL;
    }

    /* Prepare the Gaussian smoothing */
    f1 = 1   /(sigma*sqrt(2*4*atan(1)));
    f2 = -0.5/(sigma*sigma);

    smooth = calloc(size, sizeof(double));

    wl_high = WAVELEN(disprel, 0.5 - gwidth);
    /* At this point wl_high is the highest wavelength
       _not_ relevant for the first sample, namely WAVELEN(0.5 - gwidth) */

    /* Find first emission line in range */
    while (j < spt->nlines && order * (spt->lines[j]).wavel < wl_high) j++ ;

    /* Build up a signal from the list of lines */
    for (i=0-gwidth ; i<size+gwidth ; i++) {
        /* The upper boundary for the previous sample
           becomes the lower boundary for this sample */
        const double wl_low = wl_high;
        const int istart = i - gwidth < 0     ? 0      : i - gwidth;
        const int istop  = i + gwidth >= size ? size-1 : i + gwidth;

        if (j == spt->nlines) break;

        wl_high = WAVELEN(disprel, i+1 + 0.5);

        /* sample nr. x (with index i = x-1) has wavelengths
           from p(x-0.5) to p(x+0.5) */

        ilines = 0;
        while (j < spt->nlines && order * (spt->lines[j]).wavel < wl_high) {
          if ((spt->lines[j]).intens > 0) {
            const double intens = f1 * (spt->lines[j]).intens;
            /* Assume a first order dispersion relation between neighbouring
               pixel boundaries - the error is less than 1e-5 pixel ...
               isub == 0 means the line is in the center */
            const double isub   = 0.5 - (order * (spt->lines[j]).wavel - wl_low)
                                      / (wl_high - wl_low);
            /* Evaluate the Gaussian at a location with sub-pixel precision */
            double xsub = istart - i + isub;

            ilines++;

            /* Apply the gaussian filter */
            for (ii=istart ; ii<=istop ; ii++) {
                smooth[ii] += intens*exp(xsub*xsub*f2);
                xsub++;
            }
          }
          j++ ;
        }
        lines += ilines;
        if (ilines) (*found)++;
    }

    if (*found < 1) {
        e_warning("No emission lines with disprel [%g %g %g %g] (%d)",
                disprel[0], disprel[1], disprel[2], disprel[3], spt->nlines);
        free(smooth);
        return NULL;
    }

    if (debug_active() > 2) e_comment(2,
        "%d emission lines with disprel [%g %g %g %g] placed in %d samples",
             lines, disprel[0], disprel[1], disprel[2], disprel[3], *found);

    /* Put less weight on the intensity by taking the logarithm 
       - add 1 to ensure continuity around zero */
    for (i=0 ; i<size ; i++) if (smooth[i] > 0)
        smooth[i] = log(1 + smooth[i]);

    return smooth ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Output a list of table lines as a signal to a file.
  @param    table_name   Name of the table to output.
  @param    outfilename  Name of the output file.
  @param    disprel      Coeffs of the wavelength calibration polynomial
  @param    order        Order used in the spectral table look-up
  @param    slit_width   Width in pixels of the slit used
  @param    size         Number of samples to produce.
  @return   void

  This function builds a 1d signal based on the requested table name and
  wavelength range, and outputs it to an ASCII file. This is useful to dump
  out lists of lines for e.g. debugging purposes, to compare a calibrated
  signal with a list of reference lines.

  Provide NULL or the character string "STDOUT" to output the list to
  stdout. Any other name specifies a file. If another file by the same name
  already exists, it will be overwritten.
 */
/*--------------------------------------------------------------------------*/
void spectral_table_build_spectrum(
  const char    *    table_name,
  const char    *    outfilename,
  const double  *    disprel,
  const int          order,
  const double       slit_width,
  const int          size)
{
    spectral_table * spt;
    FILE         * out ;
    double       * spectrum ;
    int            found;
    int            i ;


    spt = spectral_table_init((char *) table_name);

    spectrum = spectral_table_build_signal(spt, disprel,
                                                  order, slit_width,
                                                  size, &found);

    spectral_table_destroy(spt);

    if (spectrum == NULL) {
        e_error("cannot build the signal") ;
        return ;
    }

    if (outfilename == NULL) out = stdout ;
    else if (!strcmp(outfilename, "STDOUT")) out = stdout ;
    else out = fopen(outfilename, "w");
    if (out==NULL) {
        e_error("cannot create file [%s]", outfilename);
        free(spectrum);
        return ;
    }
    /* The spectrum intensities are in the log(I+1) domain */
    for (i=0 ; i<size ; i++) fprintf(out, "%g\t%g\n",
        WAVELEN(disprel, i+1), exp(spectrum[i])-1);

    if (out!=stdout) fclose(out);
    free(spectrum);
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Count the positive-intensity lines in a given wavelength range
  @param    spt         Spectral table
  @param    wave_min    minimum wavelength
  @param    wave_max    maximum wavelength
  @param    order       order
  @return   the number of lines, or -1 on error (invalid spectral table)
 */
/*--------------------------------------------------------------------------*/
int spectral_table_count_lines(
        spectral_table  *   spt,
        double              wave_min,
        double              wave_max,
        int                 order) 
{
    int     nb_lines ;
    int     i ;

    if (spt == NULL) return -1;
    if (order < 1) return 0;

    /* Initialize  */
    nb_lines = 0 ;
    i = 0;

    wave_max /= order;
    wave_min /= order;

    while (i<spt->nlines && (spt->lines[i]).wavel < wave_min) i++;
    while (i<spt->nlines && (spt->lines[i]).wavel < wave_max)
      if ((spt->lines[i++]).intens > 0) nb_lines++ ;

    return nb_lines ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Count all lines in a given wavelength range
  @param    spt         Spectral table
  @param    wave_min    minimum wavelength
  @param    wave_max    maximum wavelength
  @param    order       order
  @return   the number of lines, or -1 on error (invalid spectral table)
 */
/*--------------------------------------------------------------------------*/
int spectral_table_count_linez(
        spectral_table  *   spt,
        double              wave_min,
        double              wave_max,
        int                 order) {


    int     nb_lines ;
    int     i ;

    if (spt == NULL) return -1;
    if (order < 1) return 0;

    /* Initialize  */
    nb_lines = 0 ;
    i = 0;

    wave_max /= order;
    wave_min /= order;

    while (i<spt->nlines && (spt->lines[i]).wavel < wave_min) i++;
    while (i<spt->nlines && (spt->lines[i++]).wavel < wave_max) nb_lines++ ;
            
    return nb_lines ;

}

