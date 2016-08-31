/*----------------------------------------------------------------------------*/
/**
   @file    jini.c
   @author
   @date    March 2002
   @version	$Revision: 1.52 $
   @brief   Jitter ini file handling
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jini.c,v 1.52 2004/02/09 16:11:04 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:11:04 $
	$Revision: 1.52 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "pfitspro.h"
#include "jtypes.h"

/*-----------------------------------------------------------------------------
                                New types
 -----------------------------------------------------------------------------*/

/*
 * Internal to this module only: set of default parameters for the
 * algorithm, one set per known instrument mode.
 */
typedef struct jparams {

    /* Algorithm name */
    char    *   algo_name ;

    /* Rejected zone */
    int         reject_bottom ;
    int         reject_top ;
    int         reject_left ;
    int         reject_right ;

    /* Pre-processing */
    int         preproc_oddeven ;
    int         preproc_fiftyhertz ;
    
    /* Sky filter */
    int         sky_activate ;
    char    *   sky_method ;
    int         skyfilter_minframes ;
    int         skyfilter_rejhw ;
    int         skyfilter_rejmin ;
    int         skyfilter_rejmax ;
    int         skyfilter_quadsep ;

    /* X-correlation */
    char    *   detect_frame ;
    char    *   offsets_in ;
    int         refine ;
    int         xcorr_sx ;
    int         xcorr_sy ;
    int         xcorr_hx ;
    int         xcorr_hy ;

    /* Frames stacking */
    int         stack_rejmin ;
    int         stack_rejmax ;
    int         stack_unionframe ;

    /* Post-processing */
    int         row_submedian ;

} jparams ;

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

/* Generation functions */
static void jitter_ini_gen_input(FILE*, char*);
static void jitter_ini_gen_calib(FILE*, char*);
static void jitter_ini_gen_output(FILE*, char*);

/* Parsing functions */
static int jitter_ini_parse_general(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_frames(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_preproc(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_calib(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_sky(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_saa(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_pproc(dictionary * ini, jitter_config_t * jc);
static int jitter_ini_parse_output(dictionary * ini, jitter_config_t * jc);

/*-----------------------------------------------------------------------------
  							Private variables
 -----------------------------------------------------------------------------*/

/* Default values set for ISAAC-SW */
static jparams jparams_isaac_sw = {
    "isaac-sw",
    /* Rejected zone */
    0, 0, 0, 0,
    /* Pre-processing */
    0, 0, 
    /* Sky filter */
    1, "auto", 10, 7, 3, 3, 0,
    /* X-correlation */
    "diff", "header", 1, 10, 10, 45, 45,
    /* Frames stacking */
    3, 3, 1,
    /* Post-processing */
    1
};

/* Default values set for ISAAC-LW */
static jparams jparams_isaac_lw = {
    "isaac-lw",
    /* Rejected zone */
    0, 0, 0, 0,
    /* Pre-processing */
    0, 0,
    /* Sky filter */
    0, "median", 10, 7, 3, 3, 0,
    /* X-correlation */
    "first", "header", 1, 10, 10, 45, 45,
    /* Frames stacking */
    3, 3, 1,
    /* Post-processing */
    0
};

/* Default values set for NACO-SW */
static jparams jparams_naco_sw = {
    "naco-sw",
    /* Rejected zone */
    100, 0, 0, 0,
    /* Pre-processing */
    0, 0,
    /* Sky filter */
    1, "median", 10, 7, 3, 3, 0,
    /* X-correlation */
    "diff", "header", 1, 40, 40, 65, 65,
    /* Frames stacking */
    2, 2, 1,
    /* Post-processing */
    0
};

/* Default values for no instrument */
static jparams jparams_auto = {
    "auto",
    /* Rejected zone */
    0, 0, 0, 0,
    /* Pre-processing */
    0, 0,
    /* Sky filter */
    1, "auto", 10, 7, 3, 3, 0,
    /* X-correlation */
    "diff", "header", 1, 20, 20, 65, 65,
    /* Frames stacking */
    3, 3, 1,
    /* Post-processing */
    0
};

/* Pointer to static array holding default values */
static jparams * jparams_defaults ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Generate a default ini file for the jitter command.
  @param    ininame     Name of ini file to generate.
  @param    name_i      Name of input file
  @param    name_o      Name of output file
  @param    name_c      Name of calibration file
  @param    algo        Name of the algorithm to use.
  @return   int 0 if Ok, -1 otherwise

  This function generates a default ini file for the jitter command. The
  generated file will have the requested name. If you do not want to
  provide names for the input/output/calib files or for the instrument,
  feed NULL pointers or character strings starting with (char)0.
 */
/*----------------------------------------------------------------------------*/
int jitter_ini_generate(
        char    *   ininame,
        char    *   name_i,
        char    *   name_o,
        char    *   name_c,
        char    *   algo)
{
	FILE    *   ini ;
    
    /* Test entries */
    if ((ininame==NULL) ||
        (name_i==NULL)  ||
        (name_o==NULL)  ||
        (name_c==NULL)  ||
        (algo==NULL)) {
        return -1 ;
    }
    
    /* Define here algorithm-specific parameters */
    if (algo!=NULL) {
        if (algo[0] == (char)0) {
            /* Auto algorithm */
            jparams_defaults = &jparams_auto ;
        } else if (!strcasecmp(algo, "isaac-sw")) {
            /* ISAAC SW Algorithm */
            jparams_defaults = &jparams_isaac_sw ;
        } else if (!strcasecmp(algo, "isaac-lw")) {
            /* ISAAC LW */
            jparams_defaults = &jparams_isaac_lw ;
        } else if (!strcasecmp(algo, "naco-sw")) {
            /* NACO SW */
            jparams_defaults = &jparams_naco_sw ;
        } else if (!strcasecmp(algo, "auto")) {
            /* Auto algorithm */
            jparams_defaults = &jparams_auto ;
        } else {
            /* Unknown */
            e_warning("unknown algorithm: %s\n", algo) ;
            return -1 ;
        }
    }

	if (file_exists(ininame)) {
		e_warning("overwriting %s", ininame) ;
	}
	if ((ini = fopen(ininame, "w"))==NULL) {
		e_error("cannot create .ini file %s", ininame) ;
		return 1 ;
	}

	fprintf(ini,
"#\n"
"# Configuration file for jitter imaging reduction\n"
"# %s\n"
"#\n", create_timestamp());

	fprintf(ini,
"#\n"
"# Check out the following pages regularly for updates:\n"
"#\n"
"#     Infrared jitter imaging data reduction algorithms\n"
"#     http://www.eso.org/projects/dfs/papers/jitter99/\n"
"#\n"
"#     Frequently Asked Questions about eclipse,\n"
"#     has a dedicated section for the 'jitter' command.\n"
"#     http://www.eso.org/eclipse/faq\n"
"#\n"
"#     eclipse main WWW site:\n"
"#     http://www.eso.org/eclipse\n"
"#\n"
"# Please read the algorithmic manual BEFORE you start using\n"
"# this software.\n"
"#\n");

	fprintf(ini,
"\n"
"[General]\n"
"Eclipse         = %s ;\n"
"\n"
"# Algorithm can be any of the following:\n"
"#\n"
"# auto        - Let jitter find out according the instrument used\n"
"# isaac-sw    - ISAAC SW algorithm\n"
"# isaac-lw    - ISAAC LW algorithm\n"
"# naco-sw     - NAOS/CONICA SW algorithm\n"
"\n"
"Algorithm       = %s ;\n"
"\n"
"\n", get_eclipse_version(), jparams_defaults->algo_name) ;

	jitter_ini_gen_input(ini, name_i);

    fprintf(ini,
"#\n"
"# -------------------- Pre-processing\n"
"#\n"
"\n"
"[PreProcessing]\n"
"Activate             = yes ;       activate pre-processing\n"
"OddEvenCorrection    = %s ;        activate odd-even correction\n"
"FiftyHertzCorrection = %s ;        activate 50Hz correction\n"
"\n",
    jparams_defaults->preproc_oddeven ? "yes" : "no",
    jparams_defaults->preproc_fiftyhertz ? "yes" : "no") ;
    
	jitter_ini_gen_calib(ini, name_c);	

	fprintf(ini,
"#\n"
"# -------------------- Sky subtraction\n"
"#\n"
"\n"
"[SkyEngine]\n"
"EstimateSky         = %s  ;        activate sky estimation\n"
"OutputDiff          = no ;          activate output (object - sky)\n"
"Method              = %s ;          auto / combine / combine_mc / median\n"
"\n",
    jparams_defaults->sky_activate ? "yes" : "no",
    jparams_defaults->sky_method);
    fprintf(ini,
"\n"
"[SkyCombine]\n"
"MinNumberOfFrames   = %d ;          min # of frames to run sky estimation\n"
"RejectHalfWidth     = %d ;          rejection halfwidth (int)\n"
"RejectMin           = %d ;          rejection min (int)\n"
"RejectMax           = %d ;          rejection max (int)\n"
"SeparateQuadrants   = %s ;          separate quadrants for sky subtraction\n"
"\n"
"\n",
    jparams_defaults->skyfilter_minframes,
    jparams_defaults->skyfilter_rejhw,
    jparams_defaults->skyfilter_rejmin,
    jparams_defaults->skyfilter_rejmax,
    jparams_defaults->skyfilter_quadsep ? "yes" : "no"
    ) ;

	fprintf(ini,
"#\n"
"# -------------------- Shift and add\n"
"#\n"
"# Shift and add is separated into the following sections:\n"
"# -> object acquisition (detection or file read)\n"
"# -> offset detection/estimation\n"
"# -> plane registration and stacking\n"
"#\n");

    fprintf(ini,
"\n"
"[ShiftAndAdd]\n"
"Activate            = yes ;         activate shift and add\n"
"\n"
"# Identify source of cross-correlating objects: auto or file\n"
"ObjectSource        = auto ;        auto/file\n"
"\n");

    fprintf(ini,
"# Only valid if ObjectSource is 'auto'\n"
"AutoDetectImage     = %s   ;        diff/first\n"
"AutoThreshold       = 2.0 ;         peak detection sigma threshold\n"
"AutoMinPoints       = 1 ;           min # of peaks to detect\n"
"AutoMaxPoints       = 1 ;           max # of peaks to detect\n"
"AutoOutputObjects   = no ;          dump objects to separate file\n"
"\n",
    jparams_defaults->detect_frame) ;

    fprintf(ini,
"# Only valid if ObjectSource is 'file'.\n"
"# The provided file is an ASCII file containing as many lines as\n"
"# provided objects, each line simply contains the x and y coordinates\n"
"# separated by a space.\n"
"ObjectFileName      = objects.in ;  name of the input object file\n"
"\n"
"# Identify source of offsets between frames\n"
"OffsetInput         = %s ;          header/file/blind\n"
"\n",
    jparams_defaults->offsets_in) ;

    fprintf(ini,
"# Only valid if Input is file.\n"
"# The provided file is an ASCII file containing as many lines as\n"
"# input frames, each line simply contains the x and y offsets\n"
"# separated by a space.\n"
"OffsetInputFile     = offsets.in\n"
"\n"
"# These parameters specify the cross-correlation search\n"
"OffsetRefine        = %s ;  activate offset refining\n"
"OffsetSearchSizeX   = %d ;   search halfsize  (int)\n"
"OffsetSearchSizeY   = %d ;   search halfsize  (int)\n"
"OffsetMeasureSizeX  = %d ;   measure halfsize (int)\n"
"OffsetMeasureSizeY  = %d ;   measure halfsize (int)\n"
"\n",
    jparams_defaults->refine ? "yes" : "no",
    jparams_defaults->xcorr_sx,
    jparams_defaults->xcorr_sy,
    jparams_defaults->xcorr_hx,
    jparams_defaults->xcorr_hy
    );
    
    fprintf(ini,
"# Frame averaging is done with a 3d filter rejection\n"
"# Specify here the number of min and max pixels to reject\n"
"AverageRejectMin    = %d ;   Number of min pixels to reject in stacking\n"
"AverageRejectMax    = %d ;   Number of max pixels to reject in stacking\n"
"UnionFrame          = %s ;  Compute Union frame (no for intersection)\n"
"\n"
"\n",
    jparams_defaults->stack_rejmin,
    jparams_defaults->stack_rejmax,
    jparams_defaults->stack_unionframe ? "yes" : "no") ;

	fprintf(ini,
"#\n"
"# -------------------- Post-processing\n"
"#\n"
"# RowSubtractMedian will compute the median pixel value for all\n"
"# rows in the image and subtract this value from all pixels in the\n"
"# row. It is a very efficient algorithm to remove saturation effects\n"
"# and it does not affect \"normal\" lines.\n"
"#\n");

    fprintf(ini,
"[PostProcessing]\n"
"Activate              = yes ; if not set none of the following occurs\n"
"RowSubtractMedian     = %s ; to remove row saturation effects\n"
"\n"
"# Included as 'post-processing' is the ability to start\n"
"# an image viewer to see the results when 'jitter' has\n"
"# finished working. Specify the command-line to start it,\n"
"# %%s being the name of the output file\n"
"\n",
    jparams_defaults->row_submedian ? "yes" : "no");

    fprintf(ini,
"StartViewer         = no  ; to launch a viewer when finished\n"
"StartCommand        = \"saoimage -fits %%s\" ;\n"
"#\n"
"# Examples:\n"
"#\n"
"# StartCommand        = \"saoimage -fits %%s\" ;\n"
"# StartCommand        = \"rtd %%s\" ;\n"
"# StartCommand        = \"xv %%s\" ;\n"
"#\n"
"\n"
"\n") ;

	jitter_ini_gen_output(ini, name_o);
	fprintf(ini,
"#\n"
"# ----- end of file\n"
"#\n") ;

	fclose(ini) ;
	return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Generate input section
  @param    ini     File object where the ini file is written
  @param    name_i  Input name
  @return   void
 */
/*----------------------------------------------------------------------------*/
static void	jitter_ini_gen_input(FILE * ini, char * name_i)
{
	fprintf(ini,
"#\n"
"# -------------------- Input files\n"
"#\n"
"# Input file names are stored in a separate file.\n"
"# The input frame list is an ASCII file containing\n"
"# the file name in first column and the frame type\n"
"# in second column. The frame type is indicating if\n"
"# the frame is an object or a sky.\n"
"#\n");

    fprintf(ini,
"# Actually, if the string in second column contains\n"
"# anywhere the string 'sky' (case insensitive) then\n"
"# the frame is taken as a sky, otherwise as an object.\n"
"#\n"
"# Example:\n"
"# file1         object\n"
"# file2         sky\n"
"# file3         object\n"
"# file4         sky\n"
"#\n");

    fprintf(ini,
"# is similar to:\n"
"# file1\n"
"# file2         SKY_FRAME\n"
"# file3         this is an object frame\n"
"# file4         sky\n"
"#\n");

    fprintf(ini,
"# Frame names in the input file are expected in\n"
"# same order as they were generated, one file name\n"
"# per line, no comments allowed.\n"
"#\n"
"#\n"
"\n"
"[Frames]\n"
"FileList         = %s ; contains the list of frames to process\n"
"\n"
"RejectBottom     = %d ; Number of pixels to reject at the bottom\n"
"RejectTop        = %d ; Number of pixels to reject at the top\n"
"RejectLeft       = %d ; Number of pixels to reject at the left\n"
"RejectRight      = %d ; Number of pixels to reject at the right\n\n\n",
			name_i,
            jparams_defaults->reject_bottom,
            jparams_defaults->reject_top,
            jparams_defaults->reject_left,
            jparams_defaults->reject_right);
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate output section
  @param    ini     File where the ini file is written
  @param    name_o  Output name
  @return   void
 */
/*----------------------------------------------------------------------------*/
static void	jitter_ini_gen_output(FILE * ini, char * name_o)
{
	fprintf(ini,
"#\n"
"# -------------------- Saving results\n"
"#\n"
"# All files created by 'jitter' will be named according to the\n"
"# following convention: basename_[type].[extension]\n"
"# where basename is declared in the following section,\n"
"# [type] depends on the frame type\n"
"# and [extension] depends on the file format (fits, tfits, or paf)\n"
"#\n"
"\n"
"[Output]\n"
"BaseName      = "
"%s ;\n\n\n", name_o);
	return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate calibration section	
  @param    ini     File where the ini file is written
  @param    name_c  Calibration ascii file
  @return   void
  Use the DO_CATG keyword written in the calib ascii file to recognize the
  calibration data. 
 */
/*----------------------------------------------------------------------------*/
static void	jitter_ini_gen_calib(FILE * ini, char * name_c)
{
    framelist   *   cal_list ;
    char            dark_name[FILENAMESZ] ;
    char            flat_name[FILENAMESZ] ;
    char            bpm_name[FILENAMESZ] ;
    char        *   dark_type ;
    char        *   flat_type ;
    char        *   bpm_type ;
    instrument_t    ins_for_pfits ;
    int             i ;
    
    /* Initialize     */
    strcpy(dark_name, "none") ;
    strcpy(flat_name, "none") ;
    strcpy(bpm_name,  "none") ;

    /* Load the calib file */
    if ((cal_list = framelist_load(name_c)) != NULL) {
        /* Define the used ins data from the INSTRUME of the first file  */
        ins_for_pfits = pfits_identify_ins(cal_list->name[0]) ;

        /* Get the calibration frames expected types (from DO_CATG) */
        dark_type = pfits_getdocat_value(ins_for_pfits, docat_imag_dark) ;
        flat_type = pfits_getdocat_value(ins_for_pfits, docat_imag_flat) ;
        bpm_type  = pfits_getdocat_value(ins_for_pfits, docat_imag_badpix) ;
 
        for (i=0 ; i<cal_list->n ; i++) {
            if (dark_type != NULL) {
                if (!strcmp(cal_list->type[i], strlwc(dark_type))) { 
                    strcpy(dark_name, cal_list->name[i]);
                } else if (strstr(cal_list->type[i], "dark") != NULL) {
                    e_warning("%s should be used instead of %s in 2nd col.",
                        dark_type, cal_list->type[i]) ;
                    strcpy(dark_name, cal_list->name[i]);
                }
            }
            if (flat_type != NULL) {
                if (!strcmp(cal_list->type[i], strlwc(flat_type))) {
                    strcpy(flat_name, cal_list->name[i]) ;
                } else if (strstr(cal_list->type[i], "flat") != NULL) {
                    e_warning("%s should be used instead of %s in 2nd col.",
                        flat_type, cal_list->type[i]) ;
                    strcpy(flat_name, cal_list->name[i]) ;
                }
            }
            if (bpm_type != NULL) {
                if (!strcmp(cal_list->type[i], strlwc(bpm_type))) {
                    strcpy(bpm_name, cal_list->name[i]) ;
                } else if (strstr(cal_list->type[i], "bad") != NULL) {
                    e_warning("%s should be used instead of %s in 2nd col.",
                        bpm_type, cal_list->type[i]) ;
                    strcpy(bpm_name, cal_list->name[i]) ;
                }
            }
        }
    }

    fprintf(ini,
"#\n"
"# -------------------- Calibration\n"
"#\n"
"# Dark subtraction, flat-field division and bad pixel replacement\n"
"# can be activated here. Provide a file name containing the correct\n"
"# calibration data in each case.\n"
"# If you do not provide these files, specify 'none' as filename\n"
"#\n"
"\n"
"[CalibrationData]\n"
"Dark          = %s ;           name of the dark file\n"
"FlatField     = %s ;           name of the flatfield file\n"
"BadPixelMap   = %s ;           name of a bad pixel map\n"
"\n", dark_name, flat_name, bpm_name) ;

    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Parse the jitter.ini file and fill up the config.
  @param    ininame     Name of the input ini file
  @param    jc          jitter_config_t to fill up.
  @return   int 0 if Ok, anything else if error occurred.

  This function tries to fill in as much as possible inside a
  jitter_config_t struct. It is verbose and will print out detailed
  messages in cases of errors.
 */
/*----------------------------------------------------------------------------*/
int jitter_ini_parse(char * ininame, jitter_config_t * jc)
{
    dictionary  *   ini ;
    int             err ;

    /* Bulletproof */
    if (ininame==NULL || jc==NULL) return -1 ;
    if (!file_exists(ininame)) {
        e_error("cannot find %s", ininame);
        return -1 ;
    }
    /* Load ini file */
    ini = iniparser_load(ininame);
    if (ini==NULL) {
        e_error("loading ini file %s", ininame);
        return -1 ;
    }
    /* Fill up the structure part by part */
    err=0 ;

    err += jitter_ini_parse_general (ini, jc);
    err += jitter_ini_parse_frames  (ini, jc);
    err += jitter_ini_parse_preproc (ini, jc);
    err += jitter_ini_parse_calib   (ini, jc);
    err += jitter_ini_parse_sky     (ini, jc);
    err += jitter_ini_parse_saa     (ini, jc);
    err += jitter_ini_parse_pproc   (ini, jc);
    err += jitter_ini_parse_output  (ini, jc);

    iniparser_freedict(ini);
    if (err!=0) {
        e_error("total: %d error(s) found in %s", err, ininame);
    }
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the general section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_general(dictionary * ini, jitter_config_t * jc)
{
    char *  sval ;
    int     err ;

    err=0 ;
    sval = iniparser_getstr(ini, "general:eclipse", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, get_eclipse_version())) {
            e_warning("this ini file produced by eclipse %s\n"
                      "you are running version %s",
                      sval,
                      get_eclipse_version());
        }
    } else {
        e_warning("no eclipse version number found in ini file");
    }

    sval = iniparser_getstr(ini, "general:algorithm", NULL);
    if (sval==NULL) {
        e_error("missing [General]:Algorithm");
        err++ ;
    } else if (!strcasecmp(sval, "auto")) {
        jc->algo.ins = instrument_auto ;
        jc->algo.mode = insmode_none ;
    } else if (!strcasecmp(sval, "isaac-sw")) {
        jc->algo.ins = instrument_isaac ;
        jc->algo.mode = insmode_nochop ;
    } else if (!strcasecmp(sval, "isaac-lw")) {
        jc->algo.ins = instrument_isaac ;
        jc->algo.mode = insmode_chop ;
    } else if (!strcasecmp(sval, "naco-sw")) {
        jc->algo.ins = instrument_naco ;
        jc->algo.mode = insmode_nochop ;
    } else {
        e_error("illegal value for [General]:Algorithm: %s", sval);
        err++ ;
    }
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the frames section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_frames(dictionary * ini, jitter_config_t * jc)
{
    int         err ;
    char    *   sval ;
    int         ival ;
    
    err=0 ;
    sval = iniparser_getstr(ini, "frames:filelist", NULL);
    if (sval==NULL) {
        e_error("missing [Frames]:Filelist");
        jc->in_name[0] = 0 ;
        err++ ;
    } else {
        strcpy(jc->in_name, sval);
    }

    /* Rejected parts */
    ival = iniparser_getint(ini, "frames:rejectbottom", 0);
    if (ival<0) {
        e_error("missing or illegal [Frames]:RejectBottom");
        err++ ;
    } else {
        jc->zone.bottom = ival ;
    }
    ival = iniparser_getint(ini, "frames:rejecttop", 0);
    if (ival<0) {
        e_error("missing or illegal [Frames]:RejectTop");
        err++ ;
    } else {
        jc->zone.top = ival ;
    }
    ival = iniparser_getint(ini, "frames:rejectleft", 0);
    if (ival<0) {
        e_error("missing or illegal [Frames]:RejectLeft");
        err++ ;
    } else {
        jc->zone.left = ival ;
    }
    ival = iniparser_getint(ini, "frames:rejectright", 0);
    if (ival<0) {
        e_error("missing or illegal [Frames]:RejectRight");
        err++ ;
    } else {
        jc->zone.right = ival ;
    }
    
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the pre-processing section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_preproc(dictionary * ini, jitter_config_t * jc)
{
    int    err ;

    err=0 ;
    /* [PreProcessing] */
    if (iniparser_getboolean(ini, "preprocessing:activate", 0)==0) {
        /* No sky filtering requested */
        jc->preproc_active = 0 ;
        return 0 ;
    }
    jc->preproc_active = 1 ;

    /* Read oddeven flag */
    jc->preproc_oddeven = iniparser_getboolean(ini, 
            "preprocessing:oddevencorrection", 0) ;

    /* Read 50 herz flag */
    jc->preproc_fiftyhertz = iniparser_getboolean(ini, 
            "preprocessing:fiftyhertzcorrection", 0) ;
    
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the calibration section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_calib(dictionary * ini, jitter_config_t * jc)
{
    char * sval ;
    int    err ;

    err=0 ;
    /* Dark handling */
    sval = iniparser_getstr(ini, "calibrationdata:dark", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, "none")) {
            if (file_exists(sval)!=1) {
                e_error("cannot find dark: %s", sval);
                err++ ;
                jc->dark_sub=0 ;
            } else {
                /* Cannot subtract dark in chopping mode */
                if (jc->algo.mode == insmode_chop) {
                    e_warning("cannot use dark subtraction in chopping mode") ;
                    jc->dark_sub=0 ;
                } else {
                    jc->dark_sub=1 ;
                    strcpy(jc->dark_name, sval);
                }
            }
        }
    }
    /* Flat field handling */
    sval = iniparser_getstr(ini, "calibrationdata:flatfield", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, "none")) {
            if (file_exists(sval)!=1) {
                e_error("cannot find flat-field: %s", sval);
                err++ ;
                jc->ff_div=0 ;
            } else {
                jc->ff_div=1 ;
                strcpy(jc->ff_name, sval);
            }
        }
    }
    /* Bad pixel map handling */
    sval = iniparser_getstr(ini, "calibrationdata:badpixelmap", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, "none")) {
            if (file_exists(sval)!=1) {
                e_error("cannot find bad pixel map: %s", sval);
                err++ ;
                jc->badpix_rep=0 ;
            } else {
                jc->badpix_rep=1 ;
                strcpy(jc->badpixmap, sval);
            }
        }
    }
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the sky section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_sky(dictionary * ini, jitter_config_t * jc)
{
    char * sval ;
    int    ival ;
    int    err ;

    err=0 ;
    /* [SkyEngine] */
    if (iniparser_getboolean(ini, "skyengine:estimatesky", 0)==0) {
        /* No sky filtering requested */
        jc->sky_active=0 ;
        return 0 ;
    }

    jc->sky_active=1 ;
    /* Read diff flag */
    jc->sky_outdiff = iniparser_getboolean(ini, "skyengine:outputdiff", 0);

    /* Read sky method */
    sval = iniparser_getstr(ini, "skyengine:method", NULL);
    if (sval==NULL) {
        jc->sky_method = skymethod_auto ;
    } else if (!strcasecmp(sval, "auto")) {
        jc->sky_method = skymethod_auto ;
    } else if (!strcasecmp(sval, "combine")) {
        jc->sky_method = skymethod_combine ;
    } else if (!strcasecmp(sval, "combine_mc")) {
        jc->sky_method = skymethod_combine_mc ;
    } else if (!strcasecmp(sval, "median")) {
        jc->sky_method = skymethod_medianframe ;
    } else {
        e_error("illegal value for [SkyEngine]:Method: %s\n"
                "expected one of: auto filter median\n",
                sval);
        err++ ;
    }
    
    /* Sky filter parameters */
    if (jc->sky_method != skymethod_medianframe) {
        ival = iniparser_getint(ini, "skycombine:minnumberofframes", -1);
        if (ival<0) {
            e_error("missing or illegal [SkyCombine]:MinNumberOfFrames");
            err++ ;
        } else {
            jc->skyfilter_minframes = ival ;
        }
        ival = iniparser_getint(ini, "skycombine:rejecthalfwidth", -1);
        if (ival<0) {
            e_error("missing or illegal [SkyCombine]:RejectHalfWidth");
            err++ ;
        } else {
            jc->skyfilter_rejhw = ival ;
        }
        ival = iniparser_getint(ini, "skycombine:rejectmin", -1);
        if (ival<0) {
            e_error("missing or illegal [SkyCombine]:RejectMin");
            err++ ;
        } else {
            jc->skyfilter_rejmin = ival ;
        }
        ival = iniparser_getint(ini, "skycombine:rejectmax", -1);
        if (ival<0) {
            e_error("missing or illegal [SkyCombine]:RejectMax");
            err++ ;
        } else {
            jc->skyfilter_rejmax = ival ;
        }

        jc->skyfilter_sepquad =
            iniparser_getboolean(ini, "skycombine:separatequadrants", 0);
    }
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the shiftandadd section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_saa(dictionary * ini, jitter_config_t * jc)
{
    char * sval ;
    int    ival ;
    double dval ;
    int    err ;

    err=0 ;
    jc->saa_active = iniparser_getboolean(ini, "shiftandadd:activate", 0);
    if (jc->saa_active==0) {
        return 0 ;
    }

    sval = iniparser_getstr(ini, "shiftandadd:objectsource", NULL);
    if (sval==NULL) {
        jc->saa_objsource = objsource_auto ;
    } else if (!strcasecmp(sval, "auto")) {
        jc->saa_objsource = objsource_auto ;
    } else if (!strcasecmp(sval, "file")) {
        jc->saa_objsource = objsource_file ;
    } else {
        e_error("illegal [ShiftAndAdd]:ObjectSource: %s", sval);
        err++ ;
    }

    switch (jc->saa_objsource) {
        case objsource_auto:
        sval = iniparser_getstr(ini, "shiftandadd:autodetectimage", NULL);
        if (sval==NULL) jc->saa_detectim = detectim_invalid ;
        else if (!strcasecmp(sval, "diff"))  jc->saa_detectim = detectim_diff ;
        else if (!strcasecmp(sval, "first")) jc->saa_detectim = detectim_first ;
        else jc->saa_detectim = detectim_invalid ;

        dval = iniparser_getdouble(ini, "shiftandadd:autothreshold", -1.0);
        if (dval<0.0) {
            e_error("missing of illegal [ShiftAndAdd]:AutoThreshold: %g",
                    dval);
            err++ ;
        } else {
            jc->saa_detectk = dval ;
        }
        ival = iniparser_getint(ini, "shiftandadd:autominpoints", -1);
        if (ival<0) {
            e_error("missing of illegal [ShiftAndAdd]:AutoMinPoints: %d",
                    ival);
            err++ ;
        } else {
            jc->saa_detectminp = ival ;
        }
        ival = iniparser_getint(ini, "shiftandadd:automaxpoints", -1);
        if (ival<0) {
            e_error("missing of illegal [ShiftAndAdd]:AutoMaxPoints: %d",
                    ival);
            err++ ;
        } else {
            jc->saa_detectmaxp = ival ;
        }
        break ;

        case objsource_file:
        sval = iniparser_getstr(ini, "shiftandadd:objectfilename", NULL);
        if (sval==NULL) {
            e_error("missing [ShiftAndAdd]:ObjectFileName");
            err++ ;
        } else {
            if (file_exists(sval)!=1) {
                e_error("cannot find object source file: %s", sval);
                err++ ;
            } else {
                strcpy(jc->saa_objfile, sval);
            }
        }
        break ;

        default:
        break ;
    }

    sval = iniparser_getstr(ini, "shiftandadd:offsetinput", NULL);
    if (sval==NULL) {
        e_error("missing [ShiftAndAdd]:OffsetInput");
        err++ ;
        jc->saa_offsource = offsource_unknown ;
    } else if (!strcasecmp(sval, "header")) {
        jc->saa_offsource = offsource_header ;
    } else if (!strcasecmp(sval, "file")) {
        jc->saa_offsource = offsource_file ;
    } else if (!strcasecmp(sval, "blind")) {
        jc->saa_offsource = offsource_blind ;
    } else {
        e_error("illegal [ShiftAndAdd]:OffsetInput: %s", sval);
        err++ ;
        jc->saa_offsource = offsource_unknown ;
    }

    switch (jc->saa_offsource) {
        case offsource_header:
        break ;

        case offsource_file:
        sval = iniparser_getstr(ini, "shiftandadd:offsetinputfile", NULL);
        if (sval==NULL) {
            e_error("missing [ShiftAndAdd]:OffsetInputFile");
            err++ ;
        } else {
            strcpy(jc->saa_offfilename, sval);
        }
        break ;

        case offsource_blind:
        break ;

        case offsource_unknown:
        default:
        break ;
    }

    ival = iniparser_getboolean(ini, "shiftandadd:offsetrefine", -1);
    if (ival<0) {
        e_error("missing [ShiftAndAdd]:OffsetRefine");
        err++ ;
    } else {
        jc->saa_xcorractive = ival ;
    }

    if (jc->saa_xcorractive) {
        ival = iniparser_getint(ini, "shiftandadd:offsetsearchsizex",-1);
        if (ival<0) {
            e_error("missing [ShiftAndAdd]:OffsetSearchSizeX");
            err++ ;
        } else {
            jc->saa_xcorrsx = ival ;
        }
        ival = iniparser_getint(ini, "shiftandadd:offsetsearchsizey",-1);
        if (ival<0) {
            e_error("missing [ShiftAndAdd]:OffsetSearchSizeY");
            err++ ;
        } else {
            jc->saa_xcorrsy = ival ;
        }
        ival = iniparser_getint(ini, "shiftandadd:offsetmeasuresizex",-1);
        if (ival<0) {
            e_error("missing [ShiftAndAdd]:OffsetMeasureSizeX");
            err++ ;
        } else {
            jc->saa_xcorrhx = ival ;
        }
        ival = iniparser_getint(ini, "shiftandadd:offsetmeasuresizex",-1);
        if (ival<0) {
            e_error("missing [ShiftAndAdd]:OffsetMeasureSizeX");
            err++ ;
        } else {
            jc->saa_xcorrhy = ival ;
        }
    }

    ival = iniparser_getint(ini, "shiftandadd:averagerejectmin", -1);
    if (ival<0) {
        e_error("missing [ShiftAndAdd]:AverageRejectMin");
        err++ ;
    } else {
        jc->saa_3drejmin = ival ;
    }
    ival = iniparser_getint(ini, "shiftandadd:averagerejectmax", -1);
    if (ival<0) {
        e_error("missing [ShiftAndAdd]:AverageRejectMax");
        err++ ;
    } else {
        jc->saa_3drejmax = ival ;
    }
    jc->saa_union = iniparser_getboolean(ini, "shiftandadd:unionframe", 1);

    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the postprocessing section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_pproc(dictionary * ini, jitter_config_t * jc)
{
    char * sval ;
    int    ival ;
    int    err ;

    err=0 ;
    ival = iniparser_getboolean(ini, "postprocessing:activate", -1);
    if (ival<0) {
        e_error("missing [PostProcessing]:Activate");
        return 1 ;
    } else {
        jc->pproc_active = ival ;
    }
    if (jc->pproc_active==0)
        return 0 ;

    ival = iniparser_getboolean(ini, "postprocessing:rowsubtractmedian", 0);
    jc->pproc_rowmediansub = ival ;

    ival = iniparser_getboolean(ini, "postprocessing:startviewer", 0);
    jc->pproc_startviewer = ival ;

    if (jc->pproc_startviewer) {
        sval = iniparser_getstr(ini, "postprocessing:startcommand", NULL);
        if (sval==NULL) {
            e_error("missing [PostProcessing]:StartCommand");
            err++ ;
            jc->pproc_viewer[0] = 0 ;
        } else {
            strcpy(jc->pproc_viewer, sval);
        }
    }
    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the output section
  @param    dictionary  dictionary
  @param    jc          jitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int jitter_ini_parse_output(dictionary * ini, jitter_config_t * jc)
{
    char * sval ;
    int    err ;

    err=0 ;
    sval = iniparser_getstr(ini, "output:basename", NULL);
    if (sval==NULL) {
        e_error("missing [Output]:Basename");
        err++ ;
        jc->output_basename[0]=0 ;
    } else {
        strcpy(jc->output_basename, sval);
    }
    return err ;
}
