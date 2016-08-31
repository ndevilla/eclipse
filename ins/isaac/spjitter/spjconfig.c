/*----------------------------------------------------------------------------*/
/**
   @file    spjconfig.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.7 $
   @brief   Spectroscopic jitter configuration handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjconfig.c,v 1.7 2003/11/19 12:02:52 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/19 12:02:52 $
	$Revision: 1.7 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "pfitspro.h"
#include "spjtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Spectroscopic jitter config constructor
  @return   1 newly allocated spjitter config.

  Simple constructor, uses calloc() so all fields are set to zero.
 */
/*----------------------------------------------------------------------------*/
spjitter_config_t * spjitter_config_new(void)
{
    return calloc(1, sizeof(spjitter_config_t)) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Spectroscopic jitter config destructor.
  @param    spjc  Current spjitter config.
  @return   void

  Safe destructor, will delete all possibly allocated fields if non-NULL
  and free the top structure.
 */
/*----------------------------------------------------------------------------*/
void spjitter_config_del(spjitter_config_t * spjc)
{
    int i ;

    if (spjc==NULL) return ;

    /* Section: Frames */
    if (spjc->nframes>0) {
        for (i=0 ; i<spjc->nframes ; i++) {
            if (spjc->frame[i].image != NULL) {
                image_del(spjc->frame[i].image);
            }
            if (spjc->frame[i].docatg!=NULL) {
                free(spjc->frame[i].docatg);
            }
        }
        free(spjc->frame);
    }
    if (spjc->sky_lines != NULL) image_del(spjc->sky_lines) ;
    if (spjc->combined != NULL) image_del(spjc->combined) ;
    
    if (spjc->wavecal_disprel != NULL) {
        if ((spjc->wavecal_disprel)->poly != NULL)
            free((spjc->wavecal_disprel)->poly) ;
        free(spjc->wavecal_disprel) ;
    }
    if (spjc->main_offset_diff != NULL) free(spjc->main_offset_diff) ;
    if (spjc->extracted_values != NULL) free(spjc->extracted_values) ;
    if (spjc->extr_x_coordinate != NULL) free(spjc->extr_x_coordinate) ;
    if (spjc->sky_signal != NULL) free(spjc->sky_signal) ;

    free(spjc);

    return ;
}

/*-----------------------------------------------------------------------------
  Convert enums to strings
 -----------------------------------------------------------------------------*/
char * spjconv_ftype(spjframe_type t)
{
    char * s ;
    switch (t) {
        case type_obj:        s="obj" ;        break ;
        case type_averaged:   s="averaged" ;   break ;
        case type_rej:        s="rej" ;        break ;
        case type_hc:         s="half-cycle" ; break ;
        case type_subtracted: s="subtracted" ; break ;
        case type_combined:   s="combined" ;   break ;
        default: s="XXX" ; break ;
    }
    return s ;
}

char * spjconv_algo(spjalgo_status t)
{
    char * s ;
    switch (t) {
        case NOTREACHED: s="not_reached" ; break ;
        case OK:         s="ok" ; break ;
        case FAILED:     s="failed" ; break ;
        case SKIPPED:    s="skipped" ; break ;
        default:         s="XXX" ; break ;
    }
    return s ;
}

char * spjconv_offsource(spjoff_source o)
{
    char * s ;
    switch (o) {
        case offsets_unknown: s="unknown" ; break ;
        case offsets_header:  s="header"  ; break ;
        case offsets_file:    s="file"    ; break ;
        case offsets_blind:   s="blind"   ; break ;
        default:              s="XXX"     ; break ;
    }
    return s ;
}

char * spjconv_diffmeth(spjdiff_meth m)
{   
    char * s ;
    switch (m) {
        case diff_unknown:  s="unknown" ;   break ;
        case diff_all:      s="all"     ;   break ;
        case diff_half:     s="half"    ;   break ;
        default:            s="XXX"     ;   break ;
    }
    return s ;
}

char * spjconv_combmeth(spjcomb_meth m)
{   
    char * s ;
    switch (m) {
        case combine_unknown:   s="unknown" ;   break ;
        case combine_median:    s="median"  ;   break ;
        case combine_rejection: s="rejection" ; break ;
        case combine_linear:    s="linear" ;    break ;
        default:                s="XXX" ;       break ;
    }
    return s ;
}

char * spjconv_ins(instrument_t i)
{
    char * s ;

    switch (i.ins) {
        case instrument_isaac:
        switch (i.mode) {
            case insmode_nochop: s="isaac-nochop"; break ;
            case insmode_chop:   s="isaac-chop"; break ;
            default: s="isaac"; break ;
        }
        break ;

        default:
        s="XXX";
        break ;
    }
    return s ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Spectroscopic jitter config dump
  @param    spjc    Current spjitter config.
  @param    out     Opened file pointer for output.
  @return   void
  This function dumps the status of the current spjitter config to the provided
  file pointer. It is Ok to pass stdout or stderr as file pointers.
 */
/*----------------------------------------------------------------------------*/
void spjitter_config_dump(spjitter_config_t * spjc, FILE * out)
{
    int i ;

    if (spjc==NULL || out==NULL) return ;

    fprintf(out,
    "#\n"
    "# spjitter status pid %ld\n"
    "# %s\n"
    "#\n"
    "\n",
    (long)getpid(),
    create_timestamp());

    fprintf(out,
    "[Summary]\n"
    "Load                   = %s\n"
    "Classification         = %s\n"
    "Wl calibration (sky)   = %s\n"
    "Wl calibration (arc)   = %s\n"
    "Wl calibration         = %s\n"
    "Differences            = %s\n"
    "Distortion (arc)       = %s\n"
    "Distortion (startrace) = %s\n"
    "Combination            = %s\n"
    "Extraction             = %s\n"
    "Save                   = %s\n"
    "\n",
    spjconv_algo(spjc->status_load),
    spjconv_algo(spjc->status_classification),
    spjconv_algo(spjc->status_wavecal_sky),
    spjconv_algo(spjc->status_wavecal_arc),
    spjconv_algo(spjc->status_wavecal_done),
    spjconv_algo(spjc->status_differences),
    spjconv_algo(spjc->status_disto_slit_curv),
    spjconv_algo(spjc->status_disto_startrace),
    spjconv_algo(spjc->status_combination),
    spjconv_algo(spjc->status_extraction),
    spjconv_algo(spjc->status_save)) ;

    fprintf(out,
    "[Input]\n"
    "Name              = %s\n"
    "NFrames           = %d\n"
    "NObjFrames        = %d\n"
    "FrameSize         = %d x %d\n"
    "TotalPixelsIn     = %ld\n"
    "Algorithm         = %s\n"
    "DataType          = %s\n"
    "\n",
    spjc->in_name,
    spjc->nframes,
    spjc->nobjframes,
    spjc->lx, spjc->ly,
    spjc->total_pixin,
    spjconv_ins(spjc->algo),
    spjconv_ins(spjc->data_type)); 

    fprintf(out,
    "[Frames]\n"
    "# rank type (p=pnum/x=xtnum) - basename\n"
    "#-----------------------------------------------------------------\n");

    for (i=0 ; i<spjc->nframes ; i++) {
        fprintf(out,
                "%02d = %s (p=%02d/x=%d) - %s\n",
                i+1,
                spjconv_ftype(spjc->frame[i].type),
                spjc->frame[i].pnum,
                spjc->frame[i].xtnum,
                get_basename(spjc->frame[i].name));
    }
    fprintf(out,
    "#-----------------------------------------------------------------\n"
    "\n");

    fprintf(out,
    "[Calibration]\n"
    "Arc                    = %s\n"
    "Startrace              = %s\n"
    "FlatField              = %s\n"
    "\n",
    spjc->cal_arc_active ? spjc->cal_arc_name : "none",
    spjc->cal_startrace_active ? spjc->cal_startrace_name : "none",
    spjc->cal_spflat_active ? spjc->cal_spflat_name : "none") ;

    fprintf(out,
    "[Classification]\n"
    "DividedByFlat          = %s\n"
    "OffsetsSource          = %s\n"
    "OffsetFile             = %s\n"
    "NbClassifiedCubes      = %d\n"
    "\n",
    spjc->divided_by_flat ? "yes" : "no",
    spjconv_offsource(spjc->offsets_source),
    spjc->offsets_file,
    spjc->nb_classified_cubes) ;

    fprintf(out,
    "# Classification results :\n"
    "#-----------------------------------------------------------------\n");
    for (i=0 ; i<spjc->nframes ; i++) {
        fprintf(out,
                "%02d - offset = %g -> cube %d\n",
                i+1,
                spjc->frame[i].offset,
                spjc->frame[i].cube_id) ;
    }
    fprintf(out,
    "#-----------------------------------------------------------------\n"
    "\n");
    
    fprintf(out,
    "[WavelengthCalibration]\n"
    "WavecalActive          = %s\n"
    "WavecalArcActive       = %s\n"
    "WavecalArcFile         = %s\n"
    "WavecalDiscard Hi Lo Le ri = %d %d %d %d\n"
    "WavecalNbCoeff         = %d\n",
    spjc->wavecal_active ? "yes" : "no",
    spjc->wavecal_arc_active ? "yes" : "no",
    spjc->wavecal_arcfile,
    spjc->wavecal_discard_hi,
    spjc->wavecal_discard_lo,
    spjc->wavecal_discard_le,
    spjc->wavecal_discard_ri,
    spjc->wavecal_nb_coeff) ;

    if (spjc->wavecal_disprel == NULL) {
        fprintf(out, "No wavelength calibration computed\n") ;
    } else {
        fprintf(out, "Wavelength calibration: wave(pix)=Sum(a[n].pix^n)\n");
        for (i=0 ; i<spjc->wavecal_nb_coeff ; i++) {
            fprintf(out, "a[%d]=%g  ", i, (spjc->wavecal_disprel)->poly[i]) ;
        }
        fprintf(out, "\n") ;
    }
    fprintf(out, "\n") ;
 
    fprintf(out,
    "[Differences]\n"
    "Method                  = %s\n"
    "\n",
    spjconv_diffmeth(spjc->diff_method)) ;
    
    fprintf(out,
    "[Distortion]\n"
    "DistortionActive        = %s\n"
    "AutoDarkSubtraction     = %s\n"
    "DistorXMin              = %d\n"
    "DistorYMin              = %d\n"
    "DistorXMax              = %d\n"
    "DistorYMax              = %d\n"
    "\n",
    spjc->distortion_active ? "yes" : "no",
    spjc->auto_dark_subtraction ? "yes" : "no",
    spjc->distor_xmin, spjc->distor_ymin, spjc->distor_xmax, spjc->distor_ymax);

    fprintf(out,
    "[Combination]\n"
    "CircularShift          = %s\n"
    "RefineOffsets          = %s\n"
    "CombineMethod          = %s\n"
    "AverageHiRejection     = %g\n"
    "AverageLoRejection     = %g\n",
    spjc->circular_shift ? "yes" : "no",
    spjc->refine_offsets ? "yes" : "no",
    spjconv_combmeth(spjc->combine_method),
    spjc->average_hi_rejection, 
    spjc->average_lo_rejection) ;
    for (i=0 ; i<spjc->nb_classified_cubes ; i++) {
        /* fprintf(out, "cube %d -> main offset = %g\n",  */
                /* i+1, spjc->main_offset_diff[i]) ; */
    }
    fprintf(out, "\n") ;

    fprintf(out,
    "[SpectrumExtract]\n"
    "SpectrumExtrActive      = %s\n"
    "DetectBadLeft           = %d\n"
    "DetectBadRight          = %d\n"
    "DetectBadTop            = %d\n"
    "DetectBadBot            = %d\n"
    "SpectrumDetected        = %s\n"
    "SpectrumPosition        = %d\n"
    "SpectrumWidth           = %d\n"
    "ResSkyHiWidth           = %d\n"
    "ResSkyLoWidth           = %d\n"
    "ResSkyHiDist            = %d\n"
    "ResSkyLoDist            = %d\n"
    "ApplyFilter             = %s\n"
    "SpectrumExtracted       = %s\n"
    "\n",
    spjc->spectrum_extr_active ? "yes" : "no",
    spjc->detect_bad_left, spjc->detect_bad_right,
    spjc->detect_bad_top, spjc->detect_bad_bot,
    spjc->spectrum_detected ? "yes" : "no",
    spjc->spectrum_position,
    spjc->spectrum_width,
    spjc->res_sky_hi_width, spjc->res_sky_lo_width,
    spjc->res_sky_hi_dist, spjc->res_sky_lo_dist,
    spjc->apply_filter ? "yes" : "no",
    spjc->spectrum_extracted ? "yes" : "no") ;

    fprintf(out,
    "[Output]\n"
    "OutputBasename          = %s\n"
    "OutputStartViewer       = %s\n"
    "OutputViewer            = %s\n"
    "OutputGnuplot           = %s\n"
    "OutputStatusReport      = %s\n" 
    "\n",
    spjc->output_basename,
    spjc->output_startviewer ? "yes" : "no",
    spjc->output_viewer,
    spjc->output_gnuplot ? "yes" : "no",
    spjc->output_statusreport ? "yes" : "no") ;

    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Select planes in the config and build a cube from them.
  @param    spjc    Current spjitter config.
  @param    t       Frame type to select.
  @return   1 newly allocated integer array.

  This function examines the passed spjitter config and extracts
  all planes which type corresponds to the passed type. It builds
  an integer array of size spjc->nframes in which selected frames
  are assigned 1 and non-selected are assigned 0.

  This function returns NULL if no plane can be selected.
 */
/*----------------------------------------------------------------------------*/
int * spjitter_cubeselect(spjitter_config_t * spjc, spjframe_type t)
{
    int *   sel ;
    int     i ;

    sel = malloc(spjc->nframes * sizeof(int));
    for (i=0 ; i<spjc->nframes ; i++) {
        if ((spjc->frame[i].type==t) || ((int)t==-1)) {
            sel[i] = 1 ;
        } else {
            sel[i] = 0 ;
        }
    }
    return sel ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Get planes from a config and build a cube.
  @param    spjc    Current spjitter config.
  @param    sel     Selection array, see spjitter_cubeselect()
  @return   1 newly allocated cube.

  This function selects planes in a spjitter config and builds a new
  cube containing only the selected planes, or NULL if no plane is selected.

  The returned cube should be deallocated using cube_del_shallow().

  If the passed selection list is NULL, all planes are selected.
 */
/*----------------------------------------------------------------------------*/
cube_t * spjitter_cubeget(spjitter_config_t * spjc, int * sel)
{
    cube_t  *   csel ;
    int         nsel ;
    int         i, j ;

    /* Count selected frames */
    if (sel==NULL) {
        /* All frames are selected */
        nsel = spjc->nframes ;
    } else {
        /* Count selected frames only */
        nsel=0 ;
        for (i=0 ; i<spjc->nframes ; i++) {
            if (sel[i])
                nsel ++ ;
        }
    }
    /* If no frame was selected, return NULL */
    if (nsel==0) return NULL ;

    /* Build a new cube structure and copy relevant pointers to it */
    csel = cube_new(spjc->lx, spjc->ly, nsel);

    if (sel==NULL) {
        /* Copy all plane pointers */
        for (i=0 ; i<spjc->nframes ; i++) {
            csel->plane[i] = spjc->frame[i].image ;
        }
    } else {
        /* Copy only selected plane pointers */
        j=0 ;
        for (i=0 ; i<spjc->nframes ; i++) {
            if (sel[i]) {
                csel->plane[j] = spjc->frame[i].image ;
                j++ ;
            }
        }
    }
    return csel ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Put planes back into spjitter config.
  @param    spjc    Current spjitter config.
  @param    sel     Selection array.
  @param    c       Cube to copy to spjitter config.
  @return   void

  This function copies plane pointers back into a spitter config, according to 
  a sel array. If sel is NULL, all plane pointers are copied to the spjitter.
 */
/*----------------------------------------------------------------------------*/
void spjitter_cubeput(spjitter_config_t * spjc, int * sel, cube_t * c)
{
    int i, j ;

    if (sel==NULL) {
        for (i=0 ; i<spjc->nframes ; i++) {
            spjc->frame[i].image = c->plane[i];
        }
    } else {
        j=0 ;
        for (i=0 ; i<spjc->nframes ; i++) {
            if (sel[i]) {
                spjc->frame[i].image = c->plane[j];
                j++ ;
            }
        }
    }
    return ;
}

