/*----------------------------------------------------------------------------*/
/**
   @file    arc.c
   @author  Y. Jung
   @date    May 2002
   @version	$Revision: 1.29 $
   @brief   ISAAC arc recipe
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: arc.c,v 1.29 2005/03/10 16:02:22 yjung Exp $
	$Author: yjung $
	$Date: 2005/03/10 16:02:22 $
	$Revision: 1.29 $
 */

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "isaacp_lib.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define         HAWAI_FWHM_YMIN         420
#define         HAWAI_FWHM_YMAX         460
#define         ALLADIN_FWHM_YMIN       400
#define         ALLADIN_FWHM_YMAX       600
#define         LINE_HALF_LENGTH        10

/*-----------------------------------------------------------------------------
                            Function prototypes
 -----------------------------------------------------------------------------*/

static int arc_engine(char *, char *, char *, int, int, int, int, int) ;
static int arc_engine_ascii(char *, char *, char *, int, int, int, int, int) ;
static int arc_engine_fits(char *, char *, char *, int, int, int, int, int) ;
static int arc_reduce_one_hawai_setting(framelist *, int, char *, char *, int, 
        int, int, int, int) ;
static int arc_reduce_one_alladin_setting(framelist *, int, char *, char *, int,
        int, int, int, int) ;
static double ** compute_arc_reduction(image_t *, char *, framelist *, int *,
        int, int, int, char *, int, char *, int, computed_disprel **,
        double3 **, int *) ;
static int arc_write_outfile(char *, int, double **, char *, framelist *, 
        char *, computed_disprel *, double3 *, int) ;
static int * arc_find_activated_lamps(framelist * lnames) ;
    
/*-----------------------------------------------------------------------------
                            Main code
 -----------------------------------------------------------------------------*/
int isaac_arc_main(void * dict)
{
	dictionary	*	d ;
	char		*	catalog ;
	int				rejected_end ;
	int				rej_left ;
	int				rej_right ;
	int				auto_dark_subtraction ;
	int				out_corrected ;
	
	char			argname[10] ;
	char    	*	name_i ;
    char    	*	name_o ;
    int     		nfiles ;
	
	int				errors ;
	int				i ;
	 
	d = (dictionary*)dict ;
   	/* Get options */
	rejected_end          = dictionary_getint(d, "arg.rejected_ends", 100) ;
	rej_left              = dictionary_getint(d, "arg.reject_left", -1) ;
	rej_right             = dictionary_getint(d, "arg.reject_right", -1) ;
	auto_dark_subtraction = dictionary_getint(d, "arg.subdark", 0) ;
	out_corrected         = dictionary_getint(d, "arg.out_corr", 0) ;
	catalog               = dictionary_get(d, "arg.catalog", NULL) ;
	
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
		
		/*
		 * Once command-line options have been cleared out, call the main
		 * computing function.
		 */
    	errors += arc_engine(name_i, 
                name_o,
                catalog,
                rejected_end,
                rej_left,
                rej_right,
                auto_dark_subtraction,
                out_corrected) ;
		free(name_o) ;
	}
	return errors ;
}

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	ARC reduction 
  @param	inname					input file (ascii or fits)
  @param	outname					outname
  @param	catalog					catalog
  @param	rejected_end			number of pix. to reject on image borders
  @param    rej_left                nb of pix to rej on the left for lines id.
  @param    rej_right               on the right...
  @param	auto_dark_subtraction	flag to subtract the dark
  @param	out_corrected			flag to output corrected frames
  @return	0 if ok, -1 otherwise
  Execute arc_engine_fits() if the input file is a fits file and 
  arc_engine_ascii() if it is an ASCII list
 */
/*----------------------------------------------------------------------------*/
static int arc_engine(
		char	*	inname,
		char	*	outname,
		char	*	catalog,
		int			rejected_end,
		int			rej_left,
		int			rej_right,
		int			auto_dark_subtraction,
		int			out_corrected)
{
    /* Test if the input file is a fits or ascii one */
    if (is_fits_file(inname) == 1) {
        if (arc_engine_fits(inname, 
                outname, 
                catalog, 
                rejected_end, 
                rej_left, 
                rej_right, 
                auto_dark_subtraction, 
                out_corrected) == -1) {
            e_error("cannot reduce the fits file: %s", inname) ;
            return -1 ;
        }
    } else if (is_ascii_list(inname) == 1) {
	    if (arc_engine_ascii(inname,
                outname,
                catalog,
                rejected_end,
                rej_left,
                rej_right,
                auto_dark_subtraction,
                out_corrected) == -1) {
            e_error("cannot reduce the ascii file: %s", inname) ;
            return -1 ;
        }
    } else {
        e_error("input file should be either a fits or an ascii file") ;
        return -1 ;
    }
	return 0 ;
}
 
/*----------------------------------------------------------------------------*/
/**
  @brief	ARC reduction for an ASCII list of FITS files
  @param	inname					input file (ascii or fits)
  @param	outname					outname
  @param	catalog					catalog
  @param	rejected_end			number of pix. to reject on image borders
  @param    rej_left                nb of pix to rej on the left for lines id.
  @param    rej_right               on the right...
  @param	auto_dark_subtraction	flag to subtract the dark
  @param	out_corrected			flag to output corrected frames
  @return	0 if ok, -1 otherwise
  For each identified setting, reduce it.
 */
/*----------------------------------------------------------------------------*/
static int arc_engine_ascii(         
        char	*	inname,
		char	*	outname,
		char	*	catalog,
		int			rejected_end,
		int			rej_left,
		int			rej_right,
		int			auto_dark_subtraction,
		int			out_corrected)
{
	framelist	*	lnames ;
	framelist	*	lnames_set ;
	int				nsettings ;
    char        *   sval ;
    instrument_t    ins ;
	int				i ;

    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;
    
    /* Read the in ascii file */
    if ((lnames=framelist_load(inname)) == NULL) {
        e_error("cannot read the ascii input file") ;
        return -1 ;
    }

    /* Number of different settings */
    if ((nsettings=framelist_labelize(lnames, compare_settings)) == -1) {
        e_error("in getting the number of different settings") ;
        framelist_del(lnames) ;
        return -1 ;
    }
    e_comment(1, "there are %d different setting(s)", nsettings) ;
    
    /* For each setting */
    for (i=0 ; i<nsettings ; i++) {
        e_comment(1, "reduction for setting no. %d", i+1) ;
        /* Get the files for current setting		 */
        if ((lnames_set=framelist_select(lnames, i)) == NULL) {
            e_error("cannot get files for current setting") ;
            framelist_del(lnames) ;
            return -1 ;
        }
    
        /* Check which arm has been used */
        if ((sval=pfits_get(ins, lnames_set->name[0], "arm")) != NULL) {
            if (toupper(sval[0])=='S') {
                /* Reduce the current setting */
                if (arc_reduce_one_hawai_setting(lnames_set,
                            i,
                            outname,
                            catalog,
                            rejected_end,
                            rej_left,
                            rej_right,
                            auto_dark_subtraction,
                            out_corrected) == -1) {
                    e_warning("cannot reduce the setting: %d", i+1) ;
                }
                framelist_del(lnames_set) ;
            } else if (toupper(sval[0])=='L') {
                /* Reduce the current setting */
                if (arc_reduce_one_alladin_setting(lnames_set,
                            i,
                            outname,
                            catalog,
                            rejected_end,
                            rej_left,
                            rej_right,
                            auto_dark_subtraction,
                            out_corrected) == -1) {
                    e_warning("cannot reduce the setting: %d", i+1) ;
                }
                framelist_del(lnames_set) ;
            }
        } else {
            e_warning("unrecognized arm: %s in setting %d", sval, i+1) ;
            framelist_del(lnames_set) ;
        }
    }
    
    /* Free and return */
    framelist_del(lnames) ;
	return 0 ;
}
  
/*----------------------------------------------------------------------------*/
/**
  @brief	ARC reduction for one ALLADIN detector setting
  @param    lnames                  file names of the setting
  @param    setid                   setting id
  @param	outname					outname
  @param	catalog					catalog
  @param	rejected_end			number of pix. to reject on image borders
  @param    rej_left                nb of pix to rej on the left for lines id.
  @param    rej_right               on the right...
  @param	auto_dark_subtraction	flag to subtract the dark
  @param	out_corrected			flag to output corrected frames
  @return	0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int arc_reduce_one_alladin_setting(
        framelist   *   lnames,
        int             setid,
        char    	*	outname,
		char	    *	catalog,
		int			    rejected_end,
		int			    rej_left,
		int			    rej_right,
		int			    auto_dark_subtraction,
		int			    out_corrected)
{
    int                 *   lamps ;   
	char			        lines_table[FILENAMESZ] ;
	double	            **  out_table ;
	image_t		        *   to_compute ;
	image_t		        *   dark ;
    double                  dark_dit, 
                            lamp_dit ;
	double3	            *	arcs_fwhm ;
	int				        nb_coeffs ;	
	char			        outfile_name[FILENAMESZ] ;
    instrument_t            ins ;
    computed_disprel    *   disprel ;
    int                     nb_saturated ;
	int	                    i, j ;

    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;
    disprel = NULL ; 
    nb_saturated = 0 ;
    
    /* Write out the files of this setting */
    e_comment(2, "concerned files:") ;
    for (i=0 ; i<lnames->n ; i++) e_comment(2, "%s", lnames->name[i]) ;
 
    /* There should be an even number of files */
    if (lnames->n % 2) {
        e_error("Odd nb of frames (%d) for setting %d", lnames->n, setid+1) ;
        return -1 ;
    } 
  
    /* Look for each file which lamp is activated */
    /* lamps[i]==0   --->   frame i : lamps off */
    /* lamps[i]==1   --->   frame i : Xenon lamp on */
    /* lamps[i]==2   --->   frame i : Argon lamp on */
    /* lamps[i]==3   --->   frame i : Arg and Xe lamps on  */
    if ((lamps=arc_find_activated_lamps(lnames)) == NULL) {
        e_error("in finding the activated lamps") ;
        return -1 ;
    }

    /* For each pair */
    for (i=0 ; i<(lnames->n)/2 ; i++) {
        e_comment(1, "Pair %d: Lamp and dark identification", i+1) ;

        /* Get the DIT keywords */
        lamp_dit = (double)atof(pfits_get(ins, lnames->name[2*i], "dit")) ;
        dark_dit = (double)atof(pfits_get(ins, lnames->name[2*i+1], "dit"));

        /* Check if the dark is there */
        if (lamps[2*i+1] == 0) {
            dark = image_load(lnames->name[2*i+1]) ;
            e_comment(2, "Dark image: [%s]", lnames->name[2*i+1]) ;
        } else {
            dark = NULL ;
            e_comment(2, "No dark frame") ;
        }

        /* Identify the lamp */
        switch (lamps[2*i]) {
            
            /* XENON lamp */
            case 1:
            strcpy(lines_table, "Xe") ;
            to_compute = image_load(lnames->name[2*i]) ;
            e_comment(2, "Xenon lamp: [%s]", lnames->name[2*i]) ;
            sprintf(outfile_name, "%s_set%d_pair%d_%s.tfits", outname, setid+1, 
                    i+1, lines_table) ;
            break ;

            /* ARGON lamp */
            case 2:
            strcpy(lines_table, "Ar") ;
            to_compute = image_load(lnames->name[2*i]) ;
            e_comment(2, "Argon lamp: [%s]", lnames->name[2*i]) ;
            sprintf(outfile_name, "%s_set%d_pair%d_%s.tfits", outname, setid+1,
                    i+1, lines_table) ;
            break ;

            /* XENON+ARGON lamp */
            case 3:
            strcpy(lines_table, "Xe+Ar") ;
            to_compute = image_load(lnames->name[2*i]) ;
            e_comment(2, "Xenon+Argon lamp: [%s]", lnames->name[2*i]) ;
            sprintf(outfile_name, "%s_set%d_pair%d_%s.tfits", outname, setid+1,
                    i+1, lines_table) ;
            break ;

            default :
            to_compute = NULL ;
            e_comment(2, "Lamps are off. Next pair...") ;
            break ;
        }

        /* Subtract the dark if the DIT is ok */
        if ((dark != NULL) && (to_compute != NULL)) {
            if (fabs(dark_dit-lamp_dit) < 1e-4) {
                image_sub_local(to_compute, dark) ;
            } else {
                e_comment(2, "Dark not used (bad DIT)") ;
            }
        }

        /* Free the dark */
        if (dark != NULL) image_del(dark) ;
        dark = NULL ;

        /* Compute the reduction     */
        if (to_compute != NULL) {
            e_comment(1, "Reduction procedure...") ;
            if ((out_table = compute_arc_reduction(to_compute,
                            lnames->name[2*i],
                            lnames,
                            &nb_coeffs,
                            rejected_end,
                            rej_left,
                            rej_right,
                            lines_table,
                            auto_dark_subtraction,
                            outfile_name,
                            out_corrected,
                            &disprel,
                            &arcs_fwhm,
                            &nb_saturated)) == NULL) {
                e_warning("arc reduction computation failed");
                image_del(to_compute) ;
            } else {
                image_del(to_compute) ;

                /* Write the fits table  */
                if (arc_write_outfile(outfile_name,
                                        nb_coeffs,
                                        out_table,
                                        lnames->name[2*i],
                                        lnames,
                                        lines_table,
                                        disprel,
                                        arcs_fwhm,
                                        nb_saturated) == -1) {
                    e_warning("cannot write the output file: [%s]",
                                outfile_name) ;
                } else {
                    e_comment(2, "file [%s] produced", outfile_name) ;
                    e_comment(2, "file [%s.paf] produced", 
                            get_rootname(outfile_name)) ;
                }
                for (j=0 ; j<4 ; j++) free(out_table[j]) ;
                free(out_table) ;
                if (arcs_fwhm != NULL) double3_del(arcs_fwhm) ;
            }
            /* Free disprel if necessary */
            if (disprel != NULL) {
                if (disprel->poly != NULL) free(disprel->poly) ;
                free(disprel) ;
            }
        }
    }

    /* Free and return */
    free(lamps) ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	ARC reduction for one HAWAI detector setting
  @param    lnames                  file names of the setting
  @param    setid                   setting id
  @param	outname					outname
  @param	catalog					catalog
  @param	rejected_end			number of pix. to reject on image borders
  @param    rej_left                nb of pix to rej on the left for lines id.
  @param    rej_right               on the right...
  @param	auto_dark_subtraction	flag to subtract the dark
  @param	out_corrected			flag to output corrected frames
  @return	0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int arc_reduce_one_hawai_setting(
        framelist   *   lnames,
        int             setid,
        char    	*	outname,
		char	    *	catalog,
		int			    rejected_end,
		int			    rej_left,
		int			    rej_right,
		int			    auto_dark_subtraction,
		int			    out_corrected)
{
    int                 *   lamps ;   
	char			        lines_table[FILENAMESZ] ;
	double		        **	out_table ;
	image_t		        *	to_compute ;
	image_t		        *	dark,
                        *   xenon,
                        *   argon,
                        *   xenon_argon ;
    double                  dark_dit, xenon_dit, argon_dit, xenon_argon_dit ;
    int                     dark_found, xenon_found, argon_found, 
                            xenon_argon_found ;
	double3		        *   arcs_fwhm ;
	int				        nb_coeffs ;	
	char			        outfile_name[FILENAMESZ] ;
    char                *   sval ;
    instrument_t            ins ;
    computed_disprel    *   disprel ;
    int                     nb_saturated ;
	int				        i ;

    /* Initialise */
    disprel = NULL ; 
    nb_saturated = 0 ;

    /* Write out the files of this setting */
    e_comment(2, "concerned files:") ;
    for (i=0 ; i<lnames->n ; i++) e_comment(2, "%s", lnames->name[i]) ;
   
    /* Initialize    */
    ins = pfits_identify_insstr("isaac") ;

    /* Look for each file which lamp is activated */
    /* lamps[i]==0   --->   frame i : lamps off */
    /* lamps[i]==1   --->   frame i : Xenon lamp on */
    /* lamps[i]==2   --->   frame i : Argon lamp on */
    /* lamps[i]==3   --->   frame i : Arg and Xe lamps on  */
    if ((lamps=arc_find_activated_lamps(lnames)) == NULL) {
        e_error("in finding the activated lamps") ;
        return -1 ;
    }

    /* Find the dark */
    dark_found = 0 ;
    for (i=0 ; i<lnames->n ; i++) {
        if (lamps[i] == 0) { 
            dark = image_load(lnames->name[i]) ;
            dark_found = 1 ;
            dark_dit=(double)atof(pfits_get(ins, lnames->name[i], "dit"));
            e_comment(2, "Dark image: [%s]", lnames->name[i]) ;
            break ;
        }
    }                      

    /* Find the xenon */   
    xenon_found = 0 ;
    for (i=0 ; i<lnames->n ; i++) {
        if (lamps[i] == 1) {
            xenon = image_load(lnames->name[i]) ;
            xenon_found = 1 ;
            xenon_dit=(double)atof(pfits_get(ins, lnames->name[i], "dit"));
            e_comment(2, "Xenon lamp: [%s]", lnames->name[i]) ;
            break ;
        }
    }

    /* Find the argon */
    argon_found = 0 ;
    for (i=0 ; i<lnames->n ; i++) {
        if (lamps[i] == 2) {
            argon = image_load(lnames->name[i]) ;
            argon_found = 1 ;       
            argon_dit=(double)atof(pfits_get(ins, lnames->name[i], "dit"));
            e_comment(2, "Argon lamp: [%s]", lnames->name[i]) ;
            break ;
        }
    }
    
    /* Find the xenon+argon */
    xenon_argon_found = 0 ;
    for (i=0 ; i<lnames->n ; i++) {
        if (lamps[i] == 3) {
            xenon_argon = image_load(lnames->name[i]) ;
            xenon_argon_found = 1 ;
            xenon_argon_dit = (double)atof(pfits_get(ins, lnames->name[i], 
                        "dit")) ;
            e_comment(2, "Xenon+Argon lamp: [%s]", lnames->name[i]) ;
            break ;
        }
    }
    free(lamps) ;
   
    /* All lamps are switched off */
    if ((!xenon_found) && (!argon_found) && (!xenon_argon_found)) {
        e_error("neither xenon nor argon lamp activated") ;
        if (dark_found) image_del(dark) ;
        return -1 ;
    }
   
    /* Check the used resolution */
    if ((sval=pfits_get(ins, lnames->name[0], "resolution")) != NULL) {
        if (toupper(sval[0])=='L') {
            /* Reduction in Low Resolution */
            e_comment(2, "low resolution") ;	

            /* Reduce the XENON frame if there is one */
            if (xenon_found) {
                if ((dark_found) && (dark_dit == xenon_dit)) {
                    to_compute = image_sub(xenon, dark) ;
                } else {
                    to_compute = image_copy(xenon) ;
                }
                image_del(xenon) ;

                /* Specify the used lines table */
                strcpy(lines_table, "Xe") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_LR_%s.tfits", outname, setid+1,
                        lines_table) ;

                /* Compute the arc calibration */
                if ((out_table = compute_arc_reduction(to_compute,
                                    lnames->name[0],
                                    lnames,
                                    &nb_coeffs,
                                    rejected_end,
                                    rej_left,
                                    rej_right,
                                    lines_table,
                                    auto_dark_subtraction,
                                    outfile_name,
                                    out_corrected,
                                    &disprel,
                                    &arcs_fwhm,
                                    &nb_saturated)) == NULL) {
                    e_warning("arc reduction computation failed");
                    image_del(to_compute) ;
                } else {
                    image_del(to_compute) ;

                    /* Write the fits table  */
                    if (arc_write_outfile(outfile_name,
                                nb_coeffs,
                                out_table,
                                lnames->name[0],
                                lnames,
                                lines_table,
                                disprel,
                                arcs_fwhm,
                                nb_saturated) == -1) {
                        e_warning("cannot write the output file: [%s]",
                                    outfile_name) ;
                    } else {
                        e_comment(2, "file [%s] produced", outfile_name);
                        e_comment(2, "file [%s.paf] produced",
                                get_rootname(outfile_name)) ;
                    }
                    for (i=0 ; i<4 ; i++) free(out_table[i]) ;
                    free(out_table) ;
                    if (arcs_fwhm != NULL) double3_del(arcs_fwhm) ;
                }
                /* Free disprel if necessary */
                if (disprel != NULL) {
                    if (disprel->poly != NULL) free(disprel->poly) ;
                    free(disprel) ;
                }
            }
            
            /* Reduce the ARGON frame if there is one */
            if (argon_found) {
                if ((dark_found) && (dark_dit == argon_dit)) {
                    to_compute = image_sub(argon, dark) ;
                } else {
                    to_compute = image_copy(argon) ;
                }
                image_del(argon) ;

                /* Specify the used lines table */
                strcpy(lines_table, "Ar") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_LR_%s.tfits", outname, setid+1, 
                        lines_table) ;

                /* Compute the arc calibration */
                if ((out_table = compute_arc_reduction(to_compute,
                                    lnames->name[0],
                                    lnames,
                                    &nb_coeffs,
                                    rejected_end,
                                    rej_left,
                                    rej_right,
                                    lines_table,
                                    auto_dark_subtraction,
                                    outfile_name,
                                    out_corrected,
                                    &disprel,
                                    &arcs_fwhm,
                                    &nb_saturated)) == NULL) {
                    e_warning("arc reduction computation failed");
                    image_del(to_compute) ;
                } else {
                    image_del(to_compute) ;

                    /* Write the fits table  */
                    if (arc_write_outfile(outfile_name,
                                nb_coeffs,
                                out_table,
                                lnames->name[0],
                                lnames,
                                lines_table,
                                disprel,
                                arcs_fwhm,
                                nb_saturated) == -1) {
                        e_warning("cannot write the output file: [%s]",
                                    outfile_name) ;
                    } else {
                        e_comment(2, "file [%s] produced",outfile_name);
                        e_comment(2, "file [%s.paf] produced",
                                get_rootname(outfile_name)) ;
                    }
                    for (i=0 ; i<4 ; i++) free(out_table[i]) ;
                    free(out_table) ;
                    if (arcs_fwhm != NULL) double3_del(arcs_fwhm) ;
                }
                /* Free disprel if necessary */
                if (disprel != NULL) {
                    if (disprel->poly != NULL) free(disprel->poly) ;
                    free(disprel) ;
                }
            }
            
            /* Reduce the XENON+ARGON frame if there is one */
            if (xenon_argon_found) {
                if ((dark_found) && (dark_dit == xenon_argon_dit)) {
                    to_compute = image_sub(xenon_argon, dark) ;
                } else {
                    to_compute = image_copy(xenon_argon) ;
                }
                image_del(xenon_argon) ;
                
                /* Specify the used lines table */
                strcpy(lines_table, "Xe+Ar") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_LR_%s.tfits", outname, setid+1, 
                        lines_table) ;

                /* Compute the arc calibration */
                if ((out_table = compute_arc_reduction(to_compute,
                                lnames->name[0],
                                lnames,
                                &nb_coeffs,
                                rejected_end,
                                rej_left,
                                rej_right,
                                lines_table,
                                auto_dark_subtraction,
                                outfile_name,
                                out_corrected,
                                &disprel,
                                &arcs_fwhm,
                                &nb_saturated)) == NULL) {
                    e_warning("arc reduction computation failed");
                    image_del(to_compute) ;
                } else {
                    image_del(to_compute) ;

                    /* Write the fits table  */
                    if (arc_write_outfile(outfile_name,
                                nb_coeffs,
                                out_table,
                                lnames->name[0],
                                lnames,
                                lines_table,
                                disprel,
                                arcs_fwhm,
                                nb_saturated) == -1) {
                        e_warning("cannot write the output file: [%s]",
                                    outfile_name) ;
                    } else {
                        e_comment(2, "file [%s] produced", outfile_name);
                        e_comment(2, "file [%s.paf] produced",
                                get_rootname(outfile_name)) ;
                    }
                    for (i=0 ; i<4 ; i++) free(out_table[i]) ;
                    free(out_table) ;
                    if (arcs_fwhm != NULL) double3_del(arcs_fwhm) ;
                }
                /* Free disprel if necessary */
                if (disprel != NULL) {
                    if (disprel->poly != NULL) free(disprel->poly) ;
                    free(disprel) ;
                }
            }

            /* Free the DARK image if there is one */
            if (dark_found) image_del(dark) ;
            
        } else if (toupper(sval[0])=='M') {
            e_comment(2, "medium resolution") ;
            
            if ((xenon_found) && (argon_found)) {
                /* Specify the used lines table */
                strcpy(lines_table, "Xe+Ar") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_MR_%s.tfits", outname, setid+1,
                        lines_table) ;

                if ((dark_found)&&(dark_dit==xenon_dit)&&(dark_dit==argon_dit)){
                    image_sub_local(xenon, dark) ;
                    image_sub_local(argon, dark) ;
                    to_compute = image_add(argon, xenon) ;
                } else {
                    to_compute = image_add(argon, xenon) ;
                }
            } else if (xenon_found) {
                /* Specify the used lines table */
                strcpy(lines_table, "Xe") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_MR_%s.tfits", outname, setid+1,
                        lines_table) ;

                if ((dark_found) && (dark_dit == xenon_dit)) {
                    to_compute = image_sub(xenon, dark) ;
                } else {
                    to_compute = image_copy(xenon) ;
                }
            } else if (argon_found) {
                /* Specify the used lines table */
                strcpy(lines_table, "Ar") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_MR_%s.tfits", outname, setid+1,
                        lines_table) ;

                if ((dark_found) && (dark_dit == argon_dit)) {
                    to_compute = image_sub(argon, dark) ;
                } else {
                    to_compute = image_copy(argon) ;
                }
            } else if (xenon_argon_found) {
                /* Specify the used lines table */
                strcpy(lines_table, "Xe+Ar") ;

                /* Output file name */
                sprintf(outfile_name, "%s_set%d_MR_%s.tfits", outname, setid+1,
                        lines_table) ;

                if ((dark_found) && (dark_dit==xenon_argon_dit)) {
                    to_compute = image_sub(xenon_argon, dark) ;
                } else {
                    to_compute = image_copy(xenon_argon) ;
                }
            }

            /* Free */
            if (xenon_argon_found) image_del(xenon_argon) ;
            if (xenon_found) image_del(xenon) ;
            if (argon_found) image_del(argon) ;
            if (dark_found) image_del(dark) ;

            /* Compute the arc calibration */
            if ((out_table = compute_arc_reduction(to_compute,
                                lnames->name[0],
                                lnames,
                                &nb_coeffs,
                                rejected_end,
                                rej_left,
                                rej_right,
                                lines_table,
                                auto_dark_subtraction,
                                outfile_name,
                                out_corrected,
                                &disprel,
                                &arcs_fwhm,
                                &nb_saturated)) == NULL) {
                e_warning("arc reduction computation failed");
                image_del(to_compute) ;
            } else {
                image_del(to_compute) ;

                /* Write the fits table  */
                if (arc_write_outfile(outfile_name,
                            nb_coeffs,
                            out_table,
                            lnames->name[0],
                            lnames,
                            lines_table,
                            disprel,
                            arcs_fwhm,
                            nb_saturated) == -1) {
                    e_warning("cannot write the output fits file: [%s]",
                            outfile_name) ;
                } else {
                    e_comment(2, "file [%s] produced", outfile_name) ;
                    e_comment(2, "file [%s.paf] produced",
                            get_rootname(outfile_name)) ;
                }
                for (i=0 ; i<4 ; i++) free(out_table[i]) ;
                free(out_table) ;
                if (arcs_fwhm != NULL) double3_del(arcs_fwhm) ;
            }
            /* Free disprel if necessary */
            if (disprel != NULL) {
                if (disprel->poly != NULL) free(disprel->poly) ;
                free(disprel) ;
            }
        } else {
            e_error("Unrecognized resolution : %c", toupper(sval[0])) ;
            if (dark_found) image_del(dark) ;
            if (argon_found) image_del(argon) ;
            if (xenon_found) image_del(xenon) ;
            if (xenon_argon_found) image_del(xenon_argon) ;
            return -1 ;
        }
    } else {
        e_error("Cannot read resolution") ;
        if (dark_found) image_del(dark) ;
        if (argon_found) image_del(argon) ;
        if (xenon_found) image_del(xenon) ;
        if (xenon_argon_found) image_del(xenon_argon) ;
        return -1 ;
    }
   
    /* Free and return */
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	ARC reduction for one FITS file
  @param	inname					input file (ascii or fits)
  @param	outname					outname
  @param	catalog					catalog
  @param	rejected_end			number of pix. to reject on image borders
  @param    rej_left                nb of pix to rej on the left for lines id.
  @param    rej_right               on the right...
  @param	auto_dark_subtraction	flag to subtract the dark
  @param	out_corrected			flag to output corrected frames
  @return	0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int arc_engine_fits(
 		char	*	inname,
		char	*	outname,
		char	*	catalog,
		int			rejected_end,
		int			rej_left,
		int			rej_right,
		int			auto_dark_subtraction,
		int			out_corrected)
{
	framelist	        *	lnames_set ;
    int                 *   lamps ; /* lamps[i]==0 : frame i : lamps off */
                                    /* lamps[i]==1 : frame i : Xenon lamp on */
                                    /* lamps[i]==2 : frame i : Argon lamp on */
                               /* lamps[i]==3 : frame i : Arg and Xe lamps on */
	char			        lines_table[FILENAMESZ] ;
	double		        **	out_table ;
	image_t		        *	to_compute ;
	int				        xenon_found ;
	int				        argon_found ;
	double3		        *   arcs_fwhm ;
	int				        nb_coeffs ;	
	char			        outfile_name[FILENAMESZ] ;
    computed_disprel    *   disprel ;
    int                     nb_saturated ;
	int	                    i ;

	/* Initialize */
	xenon_found = 0 ;
	argon_found = 0 ;
    nb_saturated = 0 ;
      
    /* Outfile name */
    sprintf(outfile_name, "%s.tfits", outname) ;	

    /* Input file name written as list of 1 name */
    lnames_set = framelist_new(1) ;
    lnames_set->name[0] = strdup(inname) ;
    
    /* Identify the lines catalog to be used */
    if (catalog == NULL) {	
        /* No catalog specified - read the header */
        
        /* Look for each file which lamp is activated */
        if ((lamps=arc_find_activated_lamps(lnames_set)) == NULL) {
            e_error("in finding the activated lamps") ;
            framelist_del(lnames_set) ;
            return -1 ;
        }
        if (lamps[0] == 1) {
            xenon_found = 1 ;
            /* Specify the used lines table */
            strcpy(lines_table, "Xe") ;
            e_comment(2, "Xenon lamp: [%s]", lnames_set->name[0]) ;
        } else if (lamps[0] == 2) {
            argon_found = 1 ; 
            /* Specify the used lines table */
            strcpy(lines_table, "Ar") ;
            e_comment(2, "Argon lamp: [%s]", lnames_set->name[0]) ;
        } else if (lamps[0] == 3) {
            /* Specify the used lines table */
             strcpy(lines_table, "Xe+Ar") ;
             e_comment(2, "Xenon+Argon lamp: [%s]", lnames_set->name[0]) ;
        } else {
            e_error("neither argon nor xenon lamp activated") ;
            free(lamps) ;
            framelist_del(lnames_set) ;
            return -1 ;
        }
        free(lamps) ;
    } else {
        /* Use the specified catalog */
        strcpy(lines_table, catalog) ;
        e_comment(2, "%s catalog used for the lines match", catalog) ;
    }
    
    /* Test if we have a catalog */
    if ((!xenon_found) && (!argon_found) && (catalog == NULL)) {
        e_error("neither xenon nor argon lamp activated in header") ;
        e_error("and no lines catalog specified") ;
        framelist_del(lnames_set) ;
        return -1 ;
    }
    
    /* Load the input frame */
    to_compute = image_load(inname) ;
    
    /* Compute the arc calibration */
    if ((out_table = compute_arc_reduction(to_compute,
                                    lnames_set->name[0],
                                    lnames_set,
                                    &nb_coeffs,
                                    rejected_end,
                                    rej_left,
                                    rej_right,
                                    lines_table,
                                    auto_dark_subtraction,
                                    outfile_name,
                                    out_corrected,
                                    &disprel,
                                    &arcs_fwhm,
                                    &nb_saturated)) == NULL) {
        e_error("arc reduction computation failed");
        image_del(to_compute) ;
        framelist_del(lnames_set) ;
        return -1 ;
    } 
    image_del(to_compute) ;

    /* Write the fits table  */
    if (arc_write_outfile(outfile_name,
                        nb_coeffs,
                        out_table,
                        inname,
                        lnames_set,
                        lines_table,
                        disprel,
                        arcs_fwhm,
                        nb_saturated) == -1) {
        e_warning("cannot write the output FITS file: [%s]", outfile_name) ;
    } else {
        e_comment(2, "file [%s] produced", outfile_name) ;
        e_comment(2, "file [%s.paf] produced", get_rootname(outfile_name)) ;
    }

    /* Free and return  */
    for (i=0 ; i<4 ; i++) free(out_table[i]) ;
    free(out_table) ;
    if (arcs_fwhm != NULL) double3_del(arcs_fwhm) ;
    framelist_del(lnames_set) ;
    /* Free disprel if necessary */
    if (disprel != NULL) {
        if (disprel->poly != NULL) free(disprel->poly) ;
        free(disprel) ;
    }

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Determine the distortion, correct, and wl calibrate
  @param    in                      image object
  @param    inimage_name            input image name
  @param    listnames               input files names
  @param    nb_coeffs               number of polynomial coefficients
  @param    rejected_ends           number of pix. to reject on image borders
  @param    rej_left                nb of pix to rej. on the left (lines id.)
  @param    rej_right               on the right ...
  @param    line_table              catalog
  @param    auto_dark_subtraction   flag to get rid of dark
  @param    file_name               output file name
  @param    out_corrected           flag to output corrected frames
  @param    disprel                 ptr to the wl calib informations struct
  @param    arcs_fwhm               array with arcs pos and fwhm 
  @param    nb_saturated            Number of saturated pixels
  @return   3 columns with the 2d poly and the last column with the
            1D polynomial describing the dispersion
 */
/*----------------------------------------------------------------------------*/
static double ** compute_arc_reduction(
        image_t             *   in,
        char                *   inimage_name,
        framelist           *   listnames,
        int                 *   nb_coeffs,
        int                     rejected_ends,
        int                     rej_left,
        int                     rej_right,
        char                *   line_table,
        int                     auto_dark_subtraction,
        char                *   file_name,
        int                     out_corrected,
        computed_disprel    **  disprel,
        double3             **  arcs_fwhm,
        int                 *   nb_saturated)
{
    image_t             *   corrected ;
    image_t             *   collapsed ;
    poly2d              *   coeffs ;
    poly2d              *   poly_id ;
    int                     xmin, ymin, xmax, ymax ;
    double              *   phdisprel ;
    int                     order ;
    int                     spec_length ;
    int                     discard_lo, discard_hi ;
    char                    name[FILENAMESZ] ;
    qfits_header        *   fh ;
    int                     nb_arcs ;
    double              *   arcs ;
    pixelvalue          *   line ;
    int                     line_start,
                            line_stop ;
    int                     maxpos ;
    double              **  arc_array ;
    int                     fwhm_ymin,
                            fwhm_ymax ;
    procat                  pro_category ;
    char                *   sval ;
    instrument_t            ins ;
    double                  slit_width ;
    pixelmap            *   saturation_map ;
    int                     i, j ;

    /* Initialize */
    xmin = 0 ;
    xmax = in->lx - 1 ;
    ymin = rejected_ends ;
    ymax = in->ly - 1 - rejected_ends ;
    ins = pfits_identify_insstr("isaac") ;

    /* Compute the number of saturated pixels */
    saturation_map = image_threshold2pixelmap(in, 
            ISAAC_ARC_SATURATION, MAX_PIX_VALUE) ;
    if (saturation_map != NULL) {
        *nb_saturated = pixelmap_getselected(saturation_map) ;
        pixelmap_del(saturation_map) ;
    } else *nb_saturated = 0 ;
    
    /* Compute the slit_width */
    slit_width = isaac_get_slitwidth(inimage_name) ;
    if (slit_width == -1) {
        e_error("cannot get the slit width") ;
        return NULL ;
    }

    /* Identify the used arm */
    if ((sval=pfits_get(ins, inimage_name, "arm")) != NULL) {
        if (toupper(sval[0])=='S') {
            fwhm_ymin = HAWAI_FWHM_YMIN ;
            fwhm_ymax = HAWAI_FWHM_YMAX ;
            pro_category = procat_spec_sw_arc_corr ;
        } else if (toupper(sval[0])=='L') {
            fwhm_ymin = ALLADIN_FWHM_YMIN ;
            fwhm_ymax = ALLADIN_FWHM_YMAX ;
            pro_category = procat_spec_lw_arc_corr ;
        } else {
            e_error("Cannot identify the used arm") ;
            return NULL ;
        }
    } else {
        e_error("Cannot identify the used arm") ;
        return NULL ;
    }

    /* Distortion estimation */
    e_comment(1, "estimate the distortion") ;
    spec_length = in->lx ;

    if ((coeffs=isaac_compute_distortion(in,
                                        xmin,
                                        ymin,
                                        xmax,
                                        ymax,
                                        auto_dark_subtraction,
                                        &nb_arcs,
                                        &arcs)) == NULL) {
        e_error("in compute distortion") ;
        return NULL ;
    }
    *nb_coeffs = coeffs->nc ;

    /* Correction of the distorsion */
    e_comment(1, "correct the distortion of the input image") ;
    if ((poly_id=poly2d_build_from_string("0 1 1.0")) == NULL) {
        e_error("in buiding a poly2d object") ;
        poly2d_free(coeffs) ;
        free(arcs) ;
        return NULL ;
    }
    if ((corrected=image_warp_generic(in,
                                "default",
                                coeffs,
                                poly_id)) == NULL) {
        e_error("in the correction of the distorsion") ;
        poly2d_free(coeffs) ;
        poly2d_free(poly_id) ;
        free(arcs) ;
        return NULL ;
    }

    /* Find out the FWHM of the used arcs */
    (*arcs_fwhm) = double3_new(nb_arcs) ;
    if ((collapsed = image_collapse_vig(corrected,
            1, fwhm_ymin, corrected->lx, fwhm_ymax, 0)) == NULL) {
        e_error("cannot create collapsed image") ;
        double3_del(*arcs_fwhm) ;
        *arcs_fwhm = NULL ;
        free(arcs) ;
    } else {
        for (i=0 ; i<nb_arcs ; i++) {
            /* Position */
            (*arcs_fwhm)->x[i] = arcs[i] ;
            /* FWHM */
            line = malloc((2*LINE_HALF_LENGTH+1)*sizeof(pixelvalue)) ;
            line_start = arcs[i] - LINE_HALF_LENGTH ;
            line_stop = arcs[i] + LINE_HALF_LENGTH ;
            maxpos = LINE_HALF_LENGTH ;
            if (line_start < 1) {
                line_start = 1 ;
                line_stop = 2*LINE_HALF_LENGTH + 1 ;
            }
            if (line_stop > collapsed->lx) {
                line_start = collapsed->lx - 2*LINE_HALF_LENGTH - 1 ;
                line_stop = collapsed->lx ;
            }
            for (j=0 ; j<2*LINE_HALF_LENGTH+1 ; j++) {
                line[j] = collapsed->data[line_start + j] ;
            }
            (*arcs_fwhm)->y[i] = function1d_get_fwhm(line,
                                            2*LINE_HALF_LENGTH+1,
                                            &maxpos,
                                            NULL) ;
            free(line) ;
            /* Arc flux */
            (*arcs_fwhm)->z[i] = (double)image_getsumpix_vig(corrected, 
                                    line_start, 1, line_stop, corrected->ly) ;
        }
        free(arcs) ;
        image_del(collapsed) ;
    }

    /* Allocate the output array */
    arc_array = malloc(4*sizeof(double*)) ;
    for (i=0 ; i<4 ; i++) {
        arc_array[i] = malloc(*nb_coeffs*sizeof(double)) ;
    }

    /* Fill the 3 first columns of the output array */
    for (i=0 ; i<*nb_coeffs ; i++) {
        arc_array[0][i] = (double)coeffs->px[i] ;
        arc_array[1][i] = (double)coeffs->py[i] ;
        arc_array[2][i] = (double)coeffs->c[i] ;
    }
    poly2d_free(coeffs) ;
    poly2d_free(poly_id) ;

    /* Output the corrected images if required */
    if (out_corrected) {
        sprintf(name, "%s_corrected.fits", get_rootname(file_name)) ;
        /* Read the FITS header of the input file    */
        fh = qfits_header_read(inimage_name) ;
        isaac_header_for_image(fh) ;
        isaac_pro_fits(fh,
                   name,
                   "REDUCED",
                   NULL,
                   pro_category,
                   "OK",
                   "spec_tec_arc",
                   1,
                   listnames,
                   NULL) ;
        /* Write the used line table in the header */
        qfits_header_add(fh, "HIERARCH ESO PRO CATALOG", line_table,
                "Catalog used", NULL) ;
        /* Write HISTORY keywords in the header */
        if (isaac_add_files_history(fh, listnames) == -1) {
            e_warning("cannot write HISTORY keywords in out file") ;
        }
        image_save_fits_hdrdump(corrected, name, fh, BPP_DEFAULT) ;
        qfits_header_destroy(fh) ;
        e_comment(0, "Arc corrected image produced: [%s]", name) ;
    }

    /* Wavelength calibration */
    e_comment(1, "Wavelength calibration on the corrected image") ;

    /* First get the wavelength order */
    if ((order = isaac_find_order(inimage_name)) == -1) {
        e_warning("cannot find order") ;
        order = 1 ;
    }

    /* First estimation using a physical model */
    if ((phdisprel = isaac_get_disprel_estimate(inimage_name, 3)) == NULL) {
        e_error("cannot estimate the dispersion relation") ;
        image_del(corrected) ;
        for (i=0 ; i<4 ; i++) free(arc_array[i]) ;
        free(arc_array) ;
        double3_del(*arcs_fwhm) ;
        return NULL ;
    }

    /* Fill the 4th column of the output array with 0*/
    for (i=0 ; i<*nb_coeffs ; i++) arc_array[3][i] = 0;

    discard_lo = discard_hi = rejected_ends ;
    if ((*disprel=spectro_compute_disprel(corrected,
                                        discard_lo,
                                        discard_hi,
                                        rej_left,
                                        rej_right,
                                        isaac_has_thermal(inimage_name) > 0,
                                        line_table,
                                        slit_width,
                                        order,
                                        phdisprel)) == NULL) {
        e_warning("cannot compute the dispersion relation") ;
    } else {
        /* Fill the 4th column of the output array */
        for (i=0 ; i<*nb_coeffs && i < 4; i++) 
            arc_array[3][i] = (*disprel)->poly[i];
        /* Display the wavelength calibration solution */
		e_comment(1, "Cross-correlation quality: %g\n", (*disprel)->cc) ;
        e_comment(1, "Wavelength calib.: wave = f(pix), pix in [1 1024] with:");
        e_comment(1, "    f(x) = %g + %g*x + %g*x^2 + %g*x^3",
                (*disprel)->poly[0], (*disprel)->poly[1], (*disprel)->poly[2], 
                (*disprel)->poly[3]) ;
    }
    free(phdisprel);

    /* Free and return */
    image_del(corrected) ;
    return arc_array ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Write the out FITS table 
  @param    outname         outfile name    
  @param    nb_coeffs       nb of coeffs
  @param    out_table       result of engine
  @param    inimage_name    one input image name
  @param    lnames          input files list
  @param    lines_type      used lines type ("argon", "xenon", argon+xenon")
  @param    disprel         struct with wl_calib results
  @param    arcs_fwhm       array with arcs positions and fwhm
  @param    nb_saturated    number of saturated pixels
  @return   error code: 0 ok, -1 in error case
 */
/*----------------------------------------------------------------------------*/
static int arc_write_outfile(
        char                *   outname,
        int                     nb_coeffs,
        double              **  out_table,
        char                *   inimage_name,
        framelist           *   lnames,
        char                *   lines_type,
        computed_disprel    *   disprel,
        double3             *   arcs_fwhm,
        int                     nb_saturated)
{
    qfits_header    *   fh ;
    qfits_table     *   table ;
    qfits_col       *   col ;
    FILE            *   paf ;
    char            *   mjd_obs ;
    char                pafname[FILENAMESZ] ;
    char            *   strvar ;
    double              fwhm_med ;
    double          *   fwhm_valid ;
    int                 nb_valid ;
    procat              pro_category_tab ;
    procat              pro_category_qc ;
    instrument_t        ins ;
    int                 i ;

    /* Initialize */
    ins = pfits_identify_insstr("isaac") ;

    /* Identify the used arm */
    if ((strvar=pfits_get(ins, inimage_name, "arm")) != NULL) {
        if (toupper(strvar[0])=='S') {
            pro_category_tab = procat_spec_sw_arc_coef ;
            pro_category_qc  = procat_spec_sw_arc_qc ;
        } else if (toupper(strvar[0])=='L') {
            pro_category_tab = procat_spec_lw_arc_coef ;
            pro_category_qc  = procat_spec_lw_arc_qc ;
        } else {
            e_error("Cannot identify the used arm") ;
            return -1 ;
        }
    } else {
        e_error("Cannot identify the used arm") ;
        return -1 ;
    }

   /* Write the output qfits_table table (informations) */
    table = qfits_table_new(outname, QFITS_BINTABLE, -1, 4, nb_coeffs) ;
    col = table->col ;
    for (i=0 ; i<table->nc ; i++) {
        qfits_col_fill(col, 1, 0, sizeof(double), TFITS_BIN_TYPE_D, " ", " ",
                " ", " ", 0, 0.0, 0, 1.0, i*sizeof(double)) ;
        col++ ;
    }
    /* Update the columns labels */
    col = table->col ;
    sprintf(col->tlabel, "Degree_of_x") ;
    col++ ;
    sprintf(col->tlabel, "Degree_of_y") ;
    col++ ;
    sprintf(col->tlabel, "poly2d_coef") ;
    col++ ;
    sprintf(col->tlabel, "WL_coefficients") ;

    /* WRITE THE OUTPUT FITS TABLE */
    /* Read the input header */
    if ((fh = qfits_header_read(inimage_name)) == NULL) {
        e_error("in writing the output fits file") ;
        qfits_table_close(table) ;
        return -1 ;
    }

    /* Prepare it for table output */
    if (isaac_header_for_table(fh) == -1) {
        e_error("in writing the output fits file") ;
        qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
        return -1 ;
    }

    /* Write the PRO keywords in the header */
    if (isaac_pro_fits(fh,
                        outname,
                        "REDUCED",
                        NULL,
                        pro_category_tab,
                        "OK",
                        "spec_tec_arc",
                        lnames->n,
                        lnames,
                        NULL) == -1) {
        e_error("in writing PRO keywords in output file") ;
        qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
        return -1 ;
    }

    /* Write the used CATALOG in the header as PRO keyword*/
    qfits_header_add(fh, "HIERARCH ESO PRO CATALOG", lines_type, "lines", NULL);

    /* Write the HISTORY keywords with the input file names */
    if (isaac_add_files_history(fh, lnames) == -1) {
        e_warning("cannot write HISTORY keywords in out file") ;
    }

    /* Write the file on disk */
    if (qfits_save_table_hdrdump((void**)out_table, table, fh) == -1) {
        e_error("cannot write file: %s", outname) ;
        qfits_header_destroy(fh) ;
        qfits_table_close(table) ;
        return -1 ;
    }
    qfits_table_close(table) ;
    qfits_header_destroy(fh) ;

    /* WRITE THE OUTPUT PAF FILE */
    sprintf(pafname, "%s.paf", get_rootname(outname)) ;
    paf = qfits_paf_print_header( pafname,
                            "ISAAC/arcs",
                            "Arc recipe results",
                            get_login_name(),
                            get_datetime_iso8601()) ;
    if (paf == NULL) {
        e_warning("cannot output PAF file") ;
    } else {
        fprintf(paf, "\n");
        /* ARCFILE */
        strvar = pfits_get(ins, inimage_name, "arcfile") ;
        if (strvar != NULL) {
            fprintf(paf, "ARCFILE \"%s\" \n", strvar) ;
        }
        /* MJD-OBS */
        mjd_obs = pfits_get(ins, inimage_name, "mjdobs") ;
        if (mjd_obs!=NULL) {
            fprintf(paf, "MJD-OBS  %s; # Obs start\n\n", mjd_obs);
        } else {
            fprintf(paf, "MJD-OBS  0.0; # Obs start unknown\n\n");
        }
        /* INSTRUME keyword  */
        strvar = pfits_get(ins, inimage_name, "instrument") ;
        if (strvar != NULL) {
            fprintf(paf, "INSTRUME \"%s\" \n", strvar) ;
        }
        /* TPL.ID  */
        strvar = pfits_get(ins, inimage_name, "templateid") ;
        if (strvar != NULL) {
            fprintf(paf, "TPL.ID  \"%s\" \n", strvar) ;
        }
        /* TPL.NEXP */
        strvar = pfits_get(ins, inimage_name, "numbexp") ;
        if (strvar != NULL) {
            fprintf(paf, "TPL.NEXP  %s \n", strvar) ;
        }
        /* DPR.CATG */
        strvar = pfits_get(ins, inimage_name, "dpr_catg") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.CATG  \"%s\" \n", strvar) ;
        }
        /* DPR.TYPE */
        strvar = pfits_get(ins, inimage_name, "dpr_type") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.TYPE  \"%s\" \n", strvar) ;
        }
        /* DPR.TECH */
        strvar = pfits_get(ins, inimage_name, "dpr_tech") ;
        if (strvar != NULL) {
            fprintf(paf, "DPR.TECH  \"%s\" \n", strvar) ;
        }
        /* Add PRO.CATG */
        fprintf(paf, "PRO.CATG  \"%s\" ;# Product category\n",
                pfits_getprokey(ins, pro_category_qc)) ;
        /* Add the date */
        fprintf(paf, "DATE-OBS  \"%s\" ;# Date\n",
                pfits_get(ins, inimage_name, "date_obs")) ;
        /* INS.GRAT.NAME */
        strvar = pfits_get(ins, inimage_name, "resolution") ;
        if (strvar != NULL) {
            fprintf(paf, "INS.GRAT.NAME  \"%s\" \n", strvar) ;
        }
        /* INS.GRAT.WLEN */
        fprintf(paf, "INS.GRAT.WLEN  %g \n",
                isaac_get_central_wavelength(inimage_name)) ;
        /* INS.GRAT.ORDER */
        fprintf(paf, "INS.GRAT.ORDER %s \n",
                pfits_get(ins, inimage_name, "order")) ;
        /* INS.MODE */
        fprintf(paf, "INS.MODE       %s \n",
                pfits_get(ins, inimage_name, "mode"));
        /* INS.OPTI1.ID */
        fprintf(paf, "INS.OPTI1.ID   %s \n",
                pfits_get(ins, inimage_name, "optical_id")) ;
        /* QC.LAMP */
        fprintf(paf, "QC.LAMP  \"%s\" \n", lines_type) ;
        /* QC.CENWL */
        fprintf(paf, "QC.WLEN     %g \n",
                (double)(out_table[3][0] + out_table[3][1]*512+
                         out_table[3][2]*512*512+out_table[3][3]*512*512*512));
        /* QC.DISPCO1 */
        fprintf(paf, "QC.DISPCO1  %g \n", (double)out_table[3][0]);
        /* QC.DISPCO2 */
        fprintf(paf, "QC.DISPCO2  %g \n", (double)out_table[3][1]);
        /* QC.DISPCO3 */
        fprintf(paf, "QC.DISPCO3  %g \n", (double)out_table[3][2]);
        /* QC.DISPCO4 */
        fprintf(paf, "QC.DISPCO4  %g \n", (double)out_table[3][3]);
        /* QC.DISP.XCORR */
        fprintf(paf, "QC.DISP.XCORR    %g \n", disprel->cc) ;
        /* QC.DISP.NUMCAT */
        fprintf(paf, "QC.DISP.NUMCAT   %d \n", disprel->clines) ;
        /* QC.DISP.NUMMATCH */
        fprintf(paf, "QC.DISP.NUMMATCH %d \n", disprel->dlines) ;
        /* QC.DISP.STDEV */
        fprintf(paf, "QC.DISP.STDEV    %g \n", disprel->rms) ;
        /* QC.DIST1 */
        fprintf(paf, "QC.DIST1   %g \n", (double)out_table[2][0]) ;
        /* QC.DISTX */
        fprintf(paf, "QC.DISTX   %g \n", (double)out_table[2][1]) ;
        /* QC.DISTY */
        fprintf(paf, "QC.DISTY   %g \n", (double)out_table[2][2]) ;
        /* QC.DISTXY */
        fprintf(paf, "QC.DISTXY  %g \n", (double)out_table[2][3]) ;
        /* QC.DISTXX */
        fprintf(paf, "QC.DISTXX  %g \n", (double)out_table[2][4]) ;
        /* QC.DISTYY */
        fprintf(paf, "QC.DISTYY  %g \n", (double)out_table[2][5]) ;
        /* QC.FILTER.OBS */
        strvar = pfits_get(ins, inimage_name, "filter") ;
        if (strvar != NULL) {
            fprintf(paf, "QC.FILTER.OBS        \"%s\" ;\n", strvar) ;
        }
        /* QC.SATUR.NBPIX */
        fprintf(paf, "QC.SATUR.NBPIX %d \n", nb_saturated) ;

        if (arcs_fwhm != NULL) {
            /* QC.ARCS.NUM */
            fprintf(paf, "QC.ARCS.NUM    %d \n", arcs_fwhm->n);
            /* ARCS POS - FWHM - FLUX */
            for(i=0 ; i<arcs_fwhm->n ; i++) {
                fprintf(paf, "QC.ARCS%d.XPOS  %.1f \n", i+1, arcs_fwhm->x[i]) ;
                fprintf(paf, "QC.ARCS%d.FWHM  %.2f \n", i+1, arcs_fwhm->y[i]);
                fprintf(paf, "QC.ARCS%d.FLUX  %.2f \n\n", i+1, arcs_fwhm->z[i]);
            }
            /* Purge the -1 values   */
            nb_valid = 0 ;
            for (i=0 ; i<arcs_fwhm->n ; i++) if (arcs_fwhm->y[i]>0) nb_valid++;
            if (nb_valid>0) {
                fwhm_valid = malloc(nb_valid*sizeof(double)) ;
                nb_valid = 0 ;
                for (i=0 ; i<arcs_fwhm->n ; i++)
                    if (arcs_fwhm->y[i] > 0) {
                        fwhm_valid[nb_valid] = arcs_fwhm->y[i] ;
                        nb_valid++ ;
                    }
                fwhm_med = double_median(fwhm_valid, nb_valid) ;
                fprintf(paf, "QC.FWHM.MED      %.2f \n", fwhm_med) ;
                fprintf(paf, "QC.ARCS.NUMGOOD  %d \n", nb_valid) ;
                free(fwhm_valid) ;
            }
        }
        fclose(paf) ;
    }
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    identify the activated lamps in a frames list   
  @param    lnames  list of frames
  @return   array: array[i]: 0 if no activated lamps in lnames[i], 1
  if xenon lamp is activated, 2 if argon lamp is activated, 3 if both
  lamps are activated
 */
/*----------------------------------------------------------------------------*/
static int * arc_find_activated_lamps(framelist * lnames)
{
    int     *   lamps ;

    int         i ;

    /* Allocate the output array */
    lamps = malloc(lnames->n*sizeof(int)) ;

    for (i=0 ; i<lnames->n ; i++) {
        if (isaac_is_xenon_lamp_active(lnames->name[i]) == 1) {
            if (isaac_is_argon_lamp_active(lnames->name[i]) == 1) {
                lamps[i] = 3 ;
            } else if (isaac_is_argon_lamp_active(lnames->name[i]) == 0) {
                lamps[i] = 1 ;
            } else {
                e_error("cannot check if argon lamp is activated") ;
                free(lamps) ;
                return NULL ;
            }
        } else {
            if (isaac_is_argon_lamp_active(lnames->name[i]) == 1) {
                lamps[i] = 2 ;
            } else if (isaac_is_argon_lamp_active(lnames->name[i]) == 0) {
                lamps[i] = 0 ;
            } else {
                e_error("cannot check if xenon lamp is activated") ;
                free(lamps) ;
                return NULL ;
            }
        }
    }

    /* Free and return */
    return lamps ;
}


