/*----------------------------------------------------------------------------*/
/**
   @file    spjini.c
   @author  Y. Jung
   @date    Dec. 2002
   @version $Revision: 1.11 $
   @brief   Spectroscopic jitter ini file handling
*/
/*----------------------------------------------------------------------------*/

/*
    $Id: spjini.c,v 1.11 2003/11/19 12:02:52 yjung Exp $
    $Author: yjung $
    $Date: 2003/11/19 12:02:52 $
    $Revision: 1.11 $
*/

/*-----------------------------------------------------------------------------
                                Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "pfitspro.h"
#include "spjtypes.h"

/*-----------------------------------------------------------------------------
                                Define
 -----------------------------------------------------------------------------*/

#define WAVECAL_DISHIGH         50
#define WAVECAL_DISLOW          50

#define DISTO_XMIN              1 
#define DISTO_YMIN              50
#define DISTO_XMAX              1024
#define DISTO_YMAX              975 

#define SPECTRACT_BADLEFT       50
#define SPECTRACT_BADRIGHT      50
#define SPECTRACT_BADTOP        0
#define SPECTRACT_BADBOT        0
#define SPECTRACT_SPECWIDTH     10  

/*-----------------------------------------------------------------------------
                                New types
 -----------------------------------------------------------------------------*/

/* Internal to this module only: set of default parameters for the algorithm */
typedef struct spjparams {

    /* Algorithm name */
    char    *   algo_name ;
    
    /* Difference method used */
    char    *   diff_method ;

} spjparams ;

/*-----------------------------------------------------------------------------
                            Private functions
 -----------------------------------------------------------------------------*/

/* Generation functions */
static void spjitter_ini_gen_calib(FILE*, char*);

/* Parsing functions */
static int spjitter_ini_parse_general(dictionary * ini,spjitter_config_t *spjc);
static int spjitter_ini_parse_frames(dictionary * ini, spjitter_config_t *spjc);
static int spjitter_ini_parse_calib(dictionary * ini, spjitter_config_t * spjc);
static int spjitter_ini_parse_classif(dictionary * ini,spjitter_config_t *spjc);
static int spjitter_ini_parse_wavecal(dictionary * ini,spjitter_config_t *spjc);
static int spjitter_ini_parse_diff(dictionary *  ini, spjitter_config_t * spjc);
static int spjitter_ini_parse_disto(dictionary * ini, spjitter_config_t * spjc);
static int spjitter_ini_parse_combine(dictionary * ini,spjitter_config_t *spjc);
static int spjitter_ini_parse_extract(dictionary * ini,spjitter_config_t *spjc);
static int spjitter_ini_parse_output(dictionary * ini,spjitter_config_t * spjc);

/*-----------------------------------------------------------------------------
                            Private variables
 -----------------------------------------------------------------------------*/

/* Default values set for NO-CHOPPING mode */
static spjparams spjparams_nochop = {
    /* Algorithm name */
    "nochop",
    /* Difference method used */
    "all"
};

/* Default values set for CHOPPING mode */
static spjparams spjparams_chop = {
    /* Algorithm name */
    "chop",
    /* Difference method used */
    "half"
};

/* Default values for no instrument */
static spjparams spjparams_auto = {
    "auto",
    /* Difference method used */
    "all"
};

/* Pointer to static array holding default values */
static spjparams * spjparams_defaults ;

/*-----------------------------------------------------------------------------
                                Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Generate a default ini file for the spjitter command.
  @param    ininame     Name of ini file to generate.
  @param    name_i      Name of input file
  @param    name_o      Name of output file
  @param    name_c      Name of calibration file
  @param    algo        Name of the algorithm to use.
  @return   int 0 if Ok, -1 otherwise

  This function generates a default ini file for the spjitter command. The
  generated file will have the requested name. If you do not want to provide 
  names for the input/output/calib files or for the instrument, feed NULL
  pointers or character strings starting with (char)0.
 */
/*----------------------------------------------------------------------------*/
int spjitter_ini_generate(
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
            spjparams_defaults = &spjparams_auto ;
        } else if (!strcasecmp(algo, "nochop")) {
            /* Non-chopping Algorithm */
            spjparams_defaults = &spjparams_nochop ;
        } else if (!strcasecmp(algo, "chop")) {
            /* Chopping Algorithm */
            spjparams_defaults = &spjparams_chop ;
        } else if (!strcasecmp(algo, "auto")) {
            /* Auto algorithm */
            spjparams_defaults = &spjparams_auto ;
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
"# Configuration file for spectroscopic jitter reduction\n"
"# %s\n"
"#\n", create_timestamp());

    fprintf(ini,
"#\n"
"# Check out the following pages regularly for updates:\n"
"#\n"
"#     eclipse main WWW site:\n"
"#     http://www.eso.org/eclipse\n"
"#\n\n");

    fprintf(ini,
"#\n"
"# -------------------- General\n"
"#\n"
"# Algorithm can be any of the following:\n"
"#\n"
"# auto        - Let spjitter find out according the instrument used\n"
"# nochop      - Non chopping algorithm\n"
"# chop        - Chopping algorithm\n"
"[General]\n"
"Eclipse         = %s ;\n"
"Algorithm       = %s ;\n"
"\n", get_eclipse_version(), spjparams_defaults->algo_name) ;

    fprintf(ini,
"#\n"
"# -------------------- Frames\n"
"#\n"
"# Frame names in the input file are expected in same order as they were \n"
"# generated, one file name per line, no comments allowed.\n"
"[Frames]\n"
"FileList         = %s ; contains the list of frames to process\n"
"\n", name_i) ;

    spjitter_ini_gen_calib(ini, name_c) ;

    fprintf(ini,
"#\n"
"# -------------------- Classification\n"
"#\n"
"# The classification is done according the offsets read in the\n"
"# header of the input files or in a file provided by the user\n"
"# OffsetFile is the ascii file name with offsets (one value per line,\n"
"# and as many lines as the number of input frames).\n"
"#\n"
"# The object frames are first classified in two different categories A or B\n"
"# depending in which zone their offset is.\n"
"# Example :\n"
"#      frame1 - offset: -250.4    ---> cat. A\n"
"#      frame2 - offset: +120.6    ---> cat. B\n"
"#      frame3 - offset: +120.6    ---> cat. B\n"
"#      frame4 - offset: -250.4    ---> cat. A\n"
"#\n"
"# These frames are then grouped in cubes according the category order.\n"
"# Each cube is then averaged in a single image.\n"
"# Example : \n"
"#      frame1 -  cat A    -> cube_id 1  ---> averaged1\n"
"#      frame2 -  cat A    -> cube_id 1\n"
"#      frame3 -  cat B    -> cube_id 2  ---> averaged2\n"
"#      frame4 -  cat B    -> cube_id 2\n"
"#      frame5 -  cat B    -> cube_id 3  ---> averaged3\n"
"#      frame6 -  cat B    -> cube_id 3\n"
"#      frame7 -  cat A    -> cube_id 4  ---> averaged4\n"
"#      frame8 -  cat A    -> cube_id 4\n"
"#      frame9 -  cat A    -> cube_id 5  ---> averaged5\n"
"#      frame10 - cat A    -> cube_id 5\n"
"#      frame11 - cat B    -> cube_id 6  ---> averaged6\n"
"#      frame12 - cat B    -> cube_id 6\n"
"#      frame13 - cat B    -> cube_id 6\n"
"#      frame14 - cat B    -> cube_id 6\n"
"[Classification]\n"
"Select = header  ;  header / file\n"
"OffsetFile = none\n"
"\n") ;

    fprintf(ini,
"#\n"
"# -------------------- WavelengthCalibration\n"
"#\n"
"# If an arc file (produced by the 'isaacp arc' recipe and contains a\n"
"# wavelength calibration solution) is provided, this calibration is used.\n"
"#\n"
"# If not, a rough wavelength calibration is computed with the physical\n"
"# model, and used as a first estimate for a more accurate calibration\n"
"# using the sky lines and an internal oh lines catalog. In 'chop' mode,\n"
"# no sky lines are visible, the physical model solution is used.\n"
"# In all cases, a third degree polynomial is computed\n"
"[WavelengthCalibration]\n"
"Select = yes\n"
"WavecalArcFile = none\n"
"DiscardHigh = 50 ; number of pixels to discard at the top and\n"
"DiscardLow = 50 ; the bottom of the image used for calibration\n"
"DiscardLeft = -1 ; left columns set to 0 before lines matching\n"
"DiscardRight = -1 ; same as left. -1 for automatic mode\n"
"\n");

    fprintf(ini,
"#\n"
"# -------------------- Differences\n"
"#\n"
"# Depending if we are in chopping or non-chopping mode, the frames\n"
"# combination is not exactly the same. In both cases, the frames are\n"
"# reduced by pairs. At this stage, we have an even number of frames.\n"
"#\n"
"# In non-chopping mode, each pair (frame n, frame n+1) will generate\n"
"# two difference frames (frame n-frame n+1) and (frame n+1-frame n).\n"
"# Example : \n"
"#      averaged1 - cat A  ---> averaged1 - averaged2 = difference1\n"
"#      averaged2 - cat B  ---> averaged2 - averaged1 = difference2\n"
"#      averaged3 - cat B  ---> averaged3 - averaged4 = difference3\n"
"#      averaged4 - cat A  ---> averaged4 - averaged3 = difference4\n"
"#      averaged5 - cat A  ---> averaged5 - averaged6 = difference5\n"
"#      averaged6 - cat B  ---> averaged6 - averaged5 = difference6\n"
"# This is the 'all' method because all differences are computed.\n"
"# This is the default for the 'nochop' algorithm.\n"
"#\n"
"# In chopping mode, as the difference is already done by the chopping,\n"
"# the pairs are directly combined.\n"
"# Example : \n"
"#      averaged1 - cat A ---> (averaged1 - averaged2)/2 = combined1\n"
"#      averaged2 - cat B\n"
"#      averaged3 - cat B\n"
"#      averaged4 - cat A ---> (averaged4 - averaged3)/2 = combined2\n"
"#      averaged5 - cat A ---> (averaged5 - averaged6)/2 = combined3\n"
"#      averaged6 - cat B\n"
"# This is the 'half' method because we end with half the number of frames.\n"
"# This is the default for the 'chop' algorithm.\n"
"[Differences]\n"
"Method = %s ; all / half\n"
"\n", spjparams_defaults->diff_method) ;

    fprintf(ini,
"#\n"
"# -------------------- Distortion\n"
"#\n"
"# There are two kind of distortions: the slit curvature and the startrace\n"
"# distortion.\n"
"#\n"
"# The startrace distortion is corrected if a startrace TFITS calibration\n"
"# file (produced by the 'isaacp startrace' recipe and contains a 2d\n"
"# distortion polynomial) is provided in the [Calibration] section. This\n"
"# distortion is the one that makes the horizontal lines (spectra) appear\n"
"# curved.\n"
"#\n"
"# The slit curvature distortion is the distortion that makes the vertical\n"
"# lines (sky lines) appear curved. This one is corrected using an arc\n"
"# TFITS calibration file (produced by the 'isaacp arc' recipe and contains\n"
"# a 2d distortion polynomial) if provided in [Calibration] section. If not,\n"
"# the sky lines are used to try to estimate this distortion and then correct\n"
"# it. In 'chop' mode, as no sky lines are visible, no correction without\n"
"# calibration file.\n"
"[Distortion]\n"
"Select = yes ; activate the distortion correction\n"
"AutoDarkSubtraction = yes ; auto. dark subt. before sky lines detection.\n"
"XMin   = 1\n"
"YMin   = 50\n"
"XMax   = 1024\n"
"YMax   = 975\n"
"\n");

    fprintf(ini,
"#\n"
"# -------------------- Combination\n"
"#\n"
"# In 'nochop' mode, the frames have first two be combined 2 by 2 together\n"
"# like this :\n"
"#      difference1 ---> mean(difference1, shift(difference2)) = combined1\n"
"#      difference2\n"
"#      difference3 ---> mean(difference3, shift(difference4)) = combined2\n"
"#      difference4\n"
"#      difference5 ---> mean(difference5, shift(difference6)) = combined3\n"
"#      difference6\n"
"#\n"
"# The final combination shifts the combinedx images and stack them with\n"
"# the specified method.\n"
"[Combination]\n"
"CircularShift = no\n"
"RefineOffsets = yes\n"
"Method = median ; median - rejection - linear\n"
"AverageHiRejection = 0.1 ; high rejection rate for averaging\n"
"AverageLoRejection = 0.1 ; low rejection rate for averaging\n"
"\n") ;

    fprintf(ini,
"#\n"
"# -------------------- SpectrumExtraction\n"
"#\n"
"# Either you specify the position of the spectrum you want to extract, or\n"
"# you leave -1 as its position (Y in pixels), and the brightest one is\n"
"# detected and extracted.\n"
"[SpectrumExtraction]\n"
"Select = yes\n"
"SpectrumWidth = 10\n"
"BadTop = 0\n"
"BadLeft = 50\n"
"BadRight = 50\n"
"BadBot = 0\n"
"ResSkyHiWidth = -1 ; residual sky width above the spectrum \n"
"ResSkyHiDist = -1 ; residual sky width above the spectrum \n"
"ResSkyLoWidth = -1 ; residual sky width below the spectrum \n"
"ResSkyLoDist = -1 ; residual sky width below the spectrum \n"
"ApplyFilter = no ; to apply a median filter before extraction\n"
"SpectrumPosition = -1\n"
"\n");

    fprintf(ini,
"#\n"
"# -------------------- Output\n"
"#\n"
"[Output]\n"
"BaseName =    %s\n"
"ProduceStatusReport = yes ; to produce global status report\n"
"PlotSpectrum = no ; to plot the extracted spectrum\n"
"StartViewer = no ; to launch a viewer when finished\n"
"StartCommand = ""saoimage -fits %%s""\n"
"\n", name_o);

    fprintf(ini,
"#\n"
"# ----- end of file\n"
"#\n") ;

    fclose(ini) ;
    return 0 ;
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
static void spjitter_ini_gen_calib(FILE * ini, char * name_c)
{
    framelist   *   cal_list ;
    char            arc_name[FILENAMESZ] ;
    char            sttr_name[FILENAMESZ] ;
    char            flat_name[FILENAMESZ] ;
    char        *   arc_type ;
    char        *   sttr_type ;
    char        *   flat_type ;
    instrument_t    ins_for_pfits ;
    int             i ;
    
    /* Initialize     */
    strcpy(arc_name,  "none") ;
    strcpy(sttr_name, "none") ;
    strcpy(flat_name, "none") ;

    /* Load the calib file */
    if ((cal_list = framelist_load(name_c)) != NULL) {
        /* Define the used ins data from the INSTRUME of the first file  */
        ins_for_pfits = pfits_identify_ins(cal_list->name[0]) ;

        /* Get the calibration frames expected types (from DO_CATG) */
        arc_type =  pfits_getdocat_value(ins_for_pfits, docat_spec_arc) ;
        sttr_type = pfits_getdocat_value(ins_for_pfits, docat_spec_sttr) ;
        flat_type = pfits_getdocat_value(ins_for_pfits, docat_spec_flat) ;
 
        for (i=0 ; i<cal_list->n ; i++) {
            if (arc_type != NULL) {
                if (!strcmp(cal_list->type[i], strlwc(arc_type))) { 
                    strcpy(arc_name, cal_list->name[i]);
                } else if (strstr(cal_list->type[i], "arc") != NULL) {
                    e_warning("%s should be used instead of %s in 2nd col.",
                        arc_type, cal_list->type[i]) ;
                    strcpy(arc_name, cal_list->name[i]);
                }
            }
            if (sttr_type != NULL) {
                if (!strcmp(cal_list->type[i], strlwc(sttr_type))) {
                    strcpy(sttr_name, cal_list->name[i]) ;
                } else if (strstr(cal_list->type[i], "trace") != NULL) {
                    e_warning("%s should be used instead of %s in 2nd col.",
                        sttr_type, cal_list->type[i]) ;
                    strcpy(sttr_name, cal_list->name[i]) ;
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
        }
    }

    fprintf(ini,
"#\n"
"# -------------------- Calibration\n"
"#\n"
"# Arc and startrace distortion corrections and flat-field division\n"
"# can be activated here. Provide a file name containing the correct\n"
"# calibration data in each case.\n"
"# If you do not provide these files, specify 'none' as filename\n"
"[CalibrationData]\n"
"ArcTable          = %s ; arc table name\n"
"StarTraceTable    = %s ; startrace table name\n"
"MasterSpFlat      = %s ; Master flat name\n"
"\n", arc_name, sttr_name, flat_name) ;

    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the spjitter.ini file and fill up the config.
  @param    ininame     Name of the input ini file
  @param    spjc        spjitter_config_t to fill up.
  @return   int 0 if Ok, anything else if error occurred.
 */
/*----------------------------------------------------------------------------*/
int spjitter_ini_parse(char * ininame, spjitter_config_t * spjc)
{
    dictionary  *   ini ;
    int             err ;

    /* Bulletproof */
    if (ininame==NULL || spjc==NULL) return -1 ;
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

    err += spjitter_ini_parse_general (ini, spjc);
    err += spjitter_ini_parse_frames  (ini, spjc);
    err += spjitter_ini_parse_calib   (ini, spjc);
    err += spjitter_ini_parse_classif (ini, spjc);
    err += spjitter_ini_parse_wavecal (ini, spjc);
    err += spjitter_ini_parse_diff    (ini, spjc);
    err += spjitter_ini_parse_disto   (ini, spjc);
    err += spjitter_ini_parse_combine (ini, spjc);
    err += spjitter_ini_parse_extract (ini, spjc);
    err += spjitter_ini_parse_output  (ini, spjc);

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
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_general(
        dictionary          *   ini, 
        spjitter_config_t   *   spjc)
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
        spjc->algo.ins = instrument_auto ;
        spjc->algo.mode = insmode_none ;
    } else if (!strcasecmp(sval, "nochop")) {
        spjc->algo.ins = instrument_isaac ;
        spjc->algo.mode = insmode_nochop ;
    } else if (!strcasecmp(sval, "chop")) {
        spjc->algo.ins = instrument_isaac ;
        spjc->algo.mode = insmode_chop ;
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
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_frames(
        dictionary          *   ini, 
        spjitter_config_t   *   spjc)
{
    int         err ;
    char    *   sval ;
    
    err=0 ;
    sval = iniparser_getstr(ini, "frames:filelist", NULL);
    if (sval==NULL) {
        e_error("missing [Frames]:Filelist");
        spjc->in_name[0] = 0 ;
        err++ ;
    } else {
        strcpy(spjc->in_name, sval);
    }

    return err ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the calibration section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_calib(
        dictionary          *   ini, 
        spjitter_config_t   *   spjc)
{
    char * sval ;
    int    err ;

    err=0 ;
    /* Arc handling */
    sval = iniparser_getstr(ini, "calibrationdata:arctable", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, "none")) {
            if (file_exists(sval)!=1) {
                e_error("cannot find arc: %s", sval);
                err++ ;
                spjc->cal_arc_active=0 ;
            } else {
                spjc->cal_arc_active=1 ;
                strcpy(spjc->cal_arc_name, sval);
            }
        }
    }
    /* Startrace handling */
    sval = iniparser_getstr(ini, "calibrationdata:startracetable", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, "none")) {
            if (file_exists(sval)!=1) {
                e_error("cannot find startrace: %s", sval);
                err++ ;
                spjc->cal_startrace_active=0 ;
            } else {
                spjc->cal_startrace_active=1 ;
                strcpy(spjc->cal_startrace_name, sval);
            }
        }
    }
    /* Flatfield handling */
    sval = iniparser_getstr(ini, "calibrationdata:masterspflat", NULL);
    if (sval!=NULL) {
        if (strcmp(sval, "none")) {
            if (file_exists(sval)!=1) {
                e_error("cannot find flatfield: %s", sval);
                err++ ;
                spjc->cal_spflat_active=0 ;
            } else {
                spjc->cal_spflat_active=1 ;
                strcpy(spjc->cal_spflat_name, sval);
            }
        }
    }
    return err ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the classification section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_classif(
        dictionary          *   ini, 
        spjitter_config_t   *   spjc)
{
    char * sval ;
    int    err ;

    err=0 ;

    sval = iniparser_getstr(ini, "classification:select", NULL) ;
    if (sval==NULL) {
        e_warning("no source specified: switching to header classification");
        spjc->offsets_source = offsets_header ;
    } else {
        if (!strcmp(sval, "header")) {
            spjc->offsets_source = offsets_header ;
        } else if (!strcmp(sval, "file")) {
            spjc->offsets_source = offsets_file ;
            
            
            
            sval = iniparser_getstr(ini, "classification:offsetfile", NULL);
            if (sval==NULL) {
                e_error("An offset file has to be provided");
                err++ ;
            } else if (!strcmp(sval, "none")) {
                e_error("An offset file has to be provided");
                err++ ;
            } else {
                strcpy(spjc->offsets_file, sval);
            }
        }
    }
    spjc->divided_by_flat = 0 ;
    return err ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the wavelength calibration section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_wavecal(
        dictionary          *   ini,
        spjitter_config_t   *   spjc)
{
    int         ival ;
    char    *   sval ;

    if (iniparser_getboolean(ini, "wavelengthcalibration:select",0) == 1) {
        spjc->wavecal_active = 1 ;
    }

    sval = iniparser_getstr(ini, "wavelengthcalibration:wavecalarcfile", NULL);
    if (sval != NULL) {
        if (strcmp(sval, "none")) {
            strcpy(spjc->wavecal_arcfile, sval) ;
            spjc->wavecal_arc_active = 1 ;
        } else {
            strcpy(spjc->wavecal_arcfile, "none") ;
        }
    }

    ival = iniparser_getint(ini, "wavelengthcalibration:discardhigh", -1) ;
    if (ival < 0) {
        e_warning("illegal [WavelengthCalibration]:DiscardHigh") ;
        e_warning("using default DiscardHigh [%d]", WAVECAL_DISHIGH) ;
        ival = WAVECAL_DISHIGH ;
    }
    spjc->wavecal_discard_hi = ival ;

    ival = iniparser_getint(ini, "wavelengthcalibration:discardlow", -1) ;
    if (ival < 0) {
        e_warning("illegal [WavelengthCalibration]:DiscardLow") ;
        e_warning("using default DiscardLow [%d]", WAVECAL_DISLOW) ;
        ival = WAVECAL_DISLOW ;
    }
    spjc->wavecal_discard_lo = ival ;

    spjc->wavecal_discard_le=iniparser_getint(ini,
                                    "wavelengthcalibration:discardleft", -1) ;

    spjc->wavecal_discard_ri=iniparser_getint(ini,
                                    "wavelengthcalibration:discardright", -1) ;

    spjc->wavecal_nb_coeff = 4 ;

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the differences section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_diff(
        dictionary          *   ini,
        spjitter_config_t   *   spjc)
{
    char    *   sval ;

    sval = iniparser_getstr(ini, "differences:method", NULL) ;
    if (sval==NULL) {
        e_warning("default differences method used: [all]");
        spjc->diff_method = diff_all ;
    } else {
        if (!strcmp(sval, "all")) { 
            spjc->diff_method = diff_all ;
        } else if (!strcmp(sval, "half")) {
            spjc->diff_method = diff_half ;
        } else {
            e_warning("Unknown differences method") ;
            spjc->diff_method = diff_unknown ;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the distortion section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_disto(
        dictionary          *   ini,
        spjitter_config_t   *   spjc)
{
    int     ival ;

    if (iniparser_getboolean(ini, "distortion:select",0) == 1) {
        spjc->distortion_active = 1 ;
    }

    if (iniparser_getboolean(ini, "distortion:autodarksubtraction",0) == 1) {
        spjc->auto_dark_subtraction = 1 ;
    }

    ival = iniparser_getint(ini, "distortion:xmin", -1) ;
    if (ival < 0) {
        e_warning("illegal or no value for [Distortion]:XMin") ;
        e_warning("using default XMin [%d]", DISTO_XMIN) ;
        ival = DISTO_XMIN ;
    }
    spjc->distor_xmin = ival ;

    ival = iniparser_getint(ini, "distortion:ymin", -1) ;
    if (ival < 0) {
        e_warning("illegal or no value for [Distortion]:YMin") ;
        e_warning("using default YMin [%d]", DISTO_YMIN) ;
        ival = DISTO_YMIN ;
    }
    spjc->distor_ymin = ival ;

    ival = iniparser_getint(ini, "distortion:xmax", -1) ;
    if (ival < 0) {
        e_warning("illegal or no value for [Distortion]:XMax") ;
        e_warning("using default XMax [%d]", DISTO_XMAX) ;
        ival = DISTO_XMAX ;
    }
    spjc->distor_xmax = ival ;

    ival = iniparser_getint(ini, "distortion:ymax", -1) ;
    if (ival < 0) {
        e_warning("illegal or no value for [Distortion]:YMax") ;
        e_warning("using default YMax [%d]", DISTO_YMAX) ;
        ival = DISTO_YMAX ;
    }
    spjc->distor_ymax = ival ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the combine section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_combine(
        dictionary          *   ini,
        spjitter_config_t   *   spjc)
{
    double      dval ;
    char    *   sval ;
    
    if (iniparser_getboolean(ini, "combination:circularshift", 0) == 1) {
        spjc->circular_shift = 1 ;
    }

    if (iniparser_getboolean(ini, "combination:refineoffsets",1) == 1) {
        spjc->refine_offsets = 1 ;
    }

    sval = iniparser_getstr(ini, "combination:method", NULL) ;
    if (sval==NULL) {
        e_warning("default final combination method used: [median]");
        spjc->combine_method = combine_median ;
    } else {
        if (!strcmp(sval, "median")) { 
            spjc->combine_method = combine_median ;
        } else if (!strcmp(sval, "rejection")) {
            spjc->combine_method = combine_rejection ;
        } else if (!strcmp(sval, "linear")) {
            spjc->combine_method = combine_linear ;
        } else {
            e_warning("Unknown combination method") ;
            spjc->combine_method = combine_unknown ;
        }
    }

    dval = iniparser_getdouble(ini, "combination:averagehirejection", -1.0) ;
    if (dval<= 0.0) spjc->average_hi_rejection = 0.0 ;
    else spjc->average_hi_rejection = dval ;

    dval = iniparser_getdouble(ini, "combination:averagelorejection", -1.0) ;
    if (dval<= 0.0) spjc->average_lo_rejection = 0.0 ;
    else spjc->average_lo_rejection = dval ;

    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the extraction section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_extract(
        dictionary          *   ini,
        spjitter_config_t   *   spjc)
{
    int         ival ;

    if (iniparser_getboolean(ini, "spectrumextraction:select",0) == 1) {
        spjc->spectrum_extr_active = 1 ;
    }
    
    ival = iniparser_getint(ini, "spectrumextraction:badleft", -1) ;
    if (ival < 0) {
        e_warning("illegal [SpectrumExtraction]:BadLeft") ;
        e_warning("using default BadLeft [%d]", SPECTRACT_BADLEFT) ;
        ival = SPECTRACT_BADLEFT ;
    }
    spjc->detect_bad_left = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:badright", -1) ;
    if (ival < 0) {
        e_warning("illegal [SpectrumExtraction]:BadRight") ;
        e_warning("using default BadRight [%d]", SPECTRACT_BADRIGHT) ;
        ival = SPECTRACT_BADRIGHT ;
    }
    spjc->detect_bad_right = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:badtop", -1) ;
    if (ival < 0) {
        e_warning("illegal [SpectrumExtraction]:BadTop") ;
        e_warning("using default BadTop [%d]", SPECTRACT_BADTOP) ;
        ival = SPECTRACT_BADTOP ;
    }
    spjc->detect_bad_top = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:badbot", -1) ;
    if (ival < 0) {
        e_warning("illegal [SpectrumExtraction]:BadBot") ;
        e_warning("using default BadBot [%d]", SPECTRACT_BADBOT) ;
        ival = SPECTRACT_BADBOT ;
    }
    spjc->detect_bad_bot = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:spectrumposition", -1) ;
    if (ival < 0) ival = -1 ;
    spjc->spectrum_position = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:spectrumwidth", -1) ;
    if (ival < 0) {
        e_warning("illegal [SpectrumExtraction]:SpectrumWidth") ;
        e_warning("using default SpectrumWidth [%d]", SPECTRACT_SPECWIDTH) ;
        ival = SPECTRACT_SPECWIDTH ;
    }
    spjc->spectrum_width = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:resskyhiwidth", -1) ;
    if (ival < 0) ival = -1 ;
    spjc->res_sky_hi_width = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:resskylowidth", -1) ;
    if (ival < 0) ival = -1 ;
    spjc->res_sky_lo_width = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:resskyhidist", -1) ;
    if (ival < 0) ival = -1 ;
    spjc->res_sky_hi_dist = ival ;

    ival = iniparser_getint(ini, "spectrumextraction:resskylodist", -1) ;
    if (ival < 0) ival = -1 ;
    spjc->res_sky_lo_dist = ival ;

    if (iniparser_getboolean(ini, "spectrumextraction:applyfilter",0) == 1) {
        spjc->apply_filter = 1 ;
    }
    return 0 ;
}



/*----------------------------------------------------------------------------*/
/**
  @brief    Parse the output section
  @param    dictionary  dictionary
  @param    spjc        spjitter configuration
  @return   0 if ok 
 */
/*----------------------------------------------------------------------------*/
static int spjitter_ini_parse_output(
        dictionary          *   ini, 
        spjitter_config_t   *   spjc)
{
    char    *   sval ;
    int         ival ;
    int         err ;

    err=0 ;
    sval = iniparser_getstr(ini, "output:basename", NULL);
    if (sval==NULL) {
        e_error("missing [Output]:Basename");
        err++ ;
        spjc->output_basename[0]=0 ;
    } else {
        strcpy(spjc->output_basename, sval);
    }

    ival = iniparser_getboolean(ini, "output:startviewer", 0);
    spjc->output_startviewer = ival ;

    if (spjc->output_startviewer) {
        sval = iniparser_getstr(ini, "output:startcommand", NULL);
        if (sval==NULL) {
            e_error("missing [Output]:StartCommand");
            err++ ;
            spjc->output_viewer[0] = 0 ;
        } else {
            strcpy(spjc->output_viewer, sval);
        }
    }

    if (iniparser_getboolean(ini, "output:producestatusreport",0)==1) {
        spjc->output_statusreport = 1 ;
    }
    if (iniparser_getboolean(ini, "output:plotspectrum", 0) == 1) {
        spjc->output_gnuplot = 1 ;
    }
    
    return err ;
}
