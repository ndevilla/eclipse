/*----------------------------------------------------------------------------*/
/**
   @file    jconfig.c
   @author
   @date    March 2002
   @version	$Revision: 1.27 $
   @brief   Jitter configuration handling.
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: jconfig.c,v 1.27 2004/02/09 16:11:04 yjung Exp $
	$Author: yjung $
	$Date: 2004/02/09 16:11:04 $
	$Revision: 1.27 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "pfitspro.h"
#include "jtypes.h"

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Jitter config constructor
  @return   1 newly allocated jitter config.

  Simple constructor, uses calloc() so all fields are set to zero.
 */
/*----------------------------------------------------------------------------*/
jitter_config_t * jitter_config_new(void)
{
    return calloc(1, sizeof(jitter_config_t)) ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Jitter config destructor.
  @param    jc  Current jitter config.
  @return   void

  Safe destructor, will delete all possibly allocated fields if non-NULL
  and free the top structure.
 */
/*----------------------------------------------------------------------------*/
void jitter_config_del(jitter_config_t * jc)
{
    int i ;

    if (jc==NULL) return ;

    /* Section: Frames */
    if (jc->nframes>0) {
        for (i=0 ; i<jc->nframes ; i++) {
            if (jc->frame[i].image != NULL) {
                image_del(jc->frame[i].image);
            }
            if (jc->frame[i].docatg!=NULL) {
                free(jc->frame[i].docatg);
            }
        }
        free(jc->frame);
    }

    /* Free x-correlation places */
    if (jc->saa_xcorrp_x!=NULL)
        free(jc->saa_xcorrp_x);
    if (jc->saa_xcorrp_y!=NULL)
        free(jc->saa_xcorrp_y);

    /* Free final image */
    if (jc->final!=NULL)
        image_del(jc->final);

    free(jc);

    return ;
}

/*-----------------------------------------------------------------------------
  Convert enums to strings
 -----------------------------------------------------------------------------*/
char * jconv_ftype(jframe_type t)
{
    char * s ;
    switch (t) {
        case type_obj:        s="obj" ;        break ;
        case type_sky:        s="sky" ;        break ;
        case type_rej:        s="rej" ;        break ;
        case type_hc:         s="half-cycle" ; break;
        case type_subtracted: s="subtracted" ; break ;
        default: s="XXX" ; break ;
    }
    return s ;
}

char * jconv_skymethod(jsky_method m)
{
    char * s ;
    switch (m) {
        case skymethod_auto: s="auto" ; break ;
        case skymethod_combine: s="combine" ; break ;
        case skymethod_medianframe: s="medframe" ; break ;
        case skymethod_combine_mc: s="combine_mc" ; break ;
        default: s="XXX" ; break ;
    }
    return s ;
}

char * jconv_algo(jalgo_status t)
{
    char * s ;
    switch (t) {
        case ALGO_NOTREACHED: s="not_reached" ; break ;
        case ALGO_OK: s="ok" ; break ;
        case ALGO_FAILED: s="failed" ; break ;
        case ALGO_SKIPPED: s="skipped" ; break ;
        default: s="XXX" ; break ;
    }
    return s ;
}

char * jconv_ins(instrument_t i)
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
        case instrument_naco:
            switch (i.mode) {
                case insmode_nochop: s="naco-nochop" ; break ;
                default: s="naco"; break ;
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
  @brief	Jitter config dump
  @param    jc      Current jitter config.
  @param    out     Opened file pointer for output.
  @return   void

  This function dumps the status of the current jitter config
  to the provided file pointer. It is Ok to pass stdout or stderr
  as file pointers.
 */
/*----------------------------------------------------------------------------*/
void jitter_config_dump(jitter_config_t * jc, FILE * out)
{
    int i ;

    if (jc==NULL || out==NULL) return ;

    fprintf(out,
    "#\n"
    "# jitter status pid %ld\n"
    "# %s\n"
    "#\n"
    "\n",
    (long)getpid(),
    create_timestamp());

    fprintf(out,
    "[Summary]\n"
    "Load              = %s\n"
    "Calibration       = %s\n"
    "SkyFilter         = %s\n"
    "ShiftAndAdd       = %s\n"
    "PostProcessing    = %s\n"
    "Save              = %s\n"
    "\n",
    jconv_algo(jc->status_load),
    jconv_algo(jc->status_calib),
    jconv_algo(jc->status_sky),
    jconv_algo(jc->status_saa),
    jconv_algo(jc->status_postproc),
    jconv_algo(jc->status_save));

    fprintf(out,
    "[Input]\n"
    "Name              = %s\n"
    "NFrames           = %d\n"
    "RejectZone        = %d - %d - %d - %d\n"
    "FrameSize         = %d x %d\n"
    "TotalPixelsIn     = %ld\n"
    "Algorithm         = %s\n"
    "\n",
    jc->in_name,
    jc->nframes,
    jc->zone.bottom, jc->zone.top, jc->zone.left, jc->zone.right,
    jc->lx, jc->ly,
    jc->total_pixin,
    jconv_ins(jc->algo)
    );

    fprintf(out,
    "[Frames]\n"
    "# rank type (p=pnum/x=xtnum) - basename\n"
    "#-----------------------------------------------------------------\n");

    for (i=0 ; i<jc->nframes ; i++) {
        fprintf(out,
                "%02d = %s (p=%02d/x=%d) - %s\n",
                i+1,
                jconv_ftype(jc->frame[i].type),
                jc->frame[i].pnum,
                jc->frame[i].xtnum,
                get_basename(jc->frame[i].name));
    }
    fprintf(out,
    "#-----------------------------------------------------------------\n"
    "\n");

    fprintf(out,
    "[Calibration]\n"
    "Status            = %s\n"
    "\n"
    "Dark              = %s\n"
    "FlatField         = %s\n"
    "BadPixMap         = %s\n"
    "\n",
    jconv_algo(jc->status_calib),
    jc->dark_sub    ? jc->dark_name : "none",
    jc->ff_div      ? jc->ff_name   : "none",
    jc->badpix_rep  ? jc->badpixmap : "none");

    fprintf(out,
    "[SkyEngine]\n"
    "Status            = %s\n"
    "FoundSkyFrames    = %s\n"
    "OutDiff           = %s\n"
    "\n"
    "Method            = %s\n"
    "MethodUsed        = %s\n"
    "\n",
    jconv_algo(jc->status_sky),
    jc->sky_ispresent ? "yes" : "no",
    jc->sky_outdiff ? "yes" : "no",
    jconv_skymethod(jc->sky_method),
    jconv_skymethod(jc->sky_method_used));

    fprintf(out,
    "[SkyCombine]\n"
    "MinNumberOfFrames = %d\n"
    "RejectHalfWidth   = %d\n"
    "RejectMin         = %d\n"
    "RejectMax         = %d\n"
    "SeparateQuadrants = %s\n"
    "\n",
    jc->skyfilter_minframes,
    jc->skyfilter_rejhw,
    jc->skyfilter_rejmin,
    jc->skyfilter_rejmax,
    jc->skyfilter_sepquad ? "yes" : "no");

    fprintf(out,
    "[ShiftAndAdd]\n"
    "Status            = %s\n"
    "\n"
    "ObjectSource      = %s\n"
    "AutoDetectImage   = %s\n"
    "AutoThreshold     = %g\n"
    "AutoMinPoints     = %d\n"
    "AutoMaxPoints     = %d\n"
    "ObjectFileName    = %s\n"
    "\n",
    jconv_algo(jc->status_saa),
    jc->saa_objsource == objsource_auto ? "auto" :
    jc->saa_objsource == objsource_file ? "file" :
    "XXX",
    jc->saa_detectim == detectim_diff ? "diff" :
    jc->saa_detectim == detectim_first ? "first" :
    "XXX",
    jc->saa_detectk,
    jc->saa_detectminp,
    jc->saa_detectmaxp,
    jc->saa_objfile);

    fprintf(out,
    "OffsetSource      = %s\n"
    "\n",
    jc->saa_offsource == offsource_header ? "header" :
    jc->saa_offsource == offsource_file   ? "file" :
    jc->saa_offsource == offsource_blind  ? "blind" :
    "XXX");

    fprintf(out,
    "OffsetInputFile   = %s\n"
    "\n",
    jc->saa_offfilename);

    fprintf(out,
    "OffsetRefine      = %s\n"
    "OffsetSearchSizeX = %d\n"
    "OffsetSearchSizeY = %d\n"
    "OffsetMeasureSizeX= %d\n"
    "OffsetMeasureSizeY= %d\n"
    "\n",
    jc->saa_xcorractive ? "yes" : "no",
    jc->saa_xcorrsx,
    jc->saa_xcorrsy,
    jc->saa_xcorrhx,
    jc->saa_xcorrhy);

    fprintf(out,
    "AverageRejectMin  = %d\n"
    "AverageRejectMax  = %d\n"
    "UnionFrame        = %s\n"
    "\n",
    jc->saa_3drejmin,
    jc->saa_3drejmax,
    jc->saa_union ? "yes" : "no");

    fprintf(out,
    "[Objects]\n"
    "# rank      X        Y\n"
    "#--------------------------------------------------------------------\n");
    for (i=0 ; i<jc->saa_xcorrp_n ; i++) {
        fprintf(out, "%02d = %8.2f %8.2f\n",
                i+1,
                jc->saa_xcorrp_x[i],
                jc->saa_xcorrp_y[i]);
    }
    fprintf(out,
    "#--------------------------------------------------------------------\n"
    "\n");

    fprintf(out,
    "[Offsets]\n"
    "# rank - in_x     in_y    out_x    out_y    err_x    err_y  (dist)\n"
    "#--------------------------------------------------------------------\n");
    
    for (i=0 ; i<jc->nframes ; i++) {
        fprintf(out, "%02d = ", i+1);
        switch (jc->frame[i].type) {
            case type_obj:
            fprintf(out,
                    "%8.2f %8.2f %8.2f %8.2f %8.2f %8.2f  (%g)\n",
                    jc->frame[i].off_x,
                    jc->frame[i].off_y,
                    jc->frame[i].off_cor_x,
                    jc->frame[i].off_cor_y,
                    jc->frame[i].off_err_x,
                    jc->frame[i].off_err_y,
                    jc->frame[i].off_dist);
            break;

            case type_sky:
            fprintf(out, "(sky)\n");
            break ;

            case type_hc:
            fprintf(out, "(half-cycle)\n");
            break ;

            case type_subtracted:
            fprintf(out, "(subtracted)\n");
            break ;
            
            case type_rej:
            fprintf(out, "%8.2f %8.2f (rej)\n",
                    jc->frame[i].off_x,
                    jc->frame[i].off_y);
            break ;
        }
    }
    fprintf(out,
    "#--------------------------------------------------------------------\n");

    fprintf(out,
    "[Output]\n"
    "BaseName          = %s\n",
    jc->output_basename);

    if (jc->final!=NULL) {
        fprintf(out,
                "Size              = %d x %d\n",
                jc->final->lx,
                jc->final->ly);
    } else {
        fprintf(out,
                "Size              = %d x %d\n",
                jc->lx,
                jc->ly);
    }

    fprintf(out,
    "\n"
    "[PostProcessing]\n"
    "Active            = %s\n"
    "RowMedianSub      = %s\n"
    "StartViewer       = %s\n"
    "StartCommand      = %s\n",
    jc->pproc_active ? "yes" : "no",
    jc->pproc_rowmediansub ? "yes" : "no",
    jc->pproc_startviewer ? "yes" : "no",
    jc->pproc_viewer);

    fprintf(out, "\n\n# end of file\n");

    return ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Select planes in the config and build a cube from them.
  @param    jc      Current jitter config.
  @param    t       Frame type to select.
  @return   1 newly allocated integer array.

  This function examines the passed jitter config and extracts
  all planes which type corresponds to the passed type. It builds
  an integer array of size jc->nframes in which selected frames
  are assigned 1 and non-selected are assigned 0.

  This function returns NULL if no plane can be selected.
 */
/*----------------------------------------------------------------------------*/
int * jitter_cubeselect(jitter_config_t * jc, jframe_type t)
{
    int *   sel ;
    int     i ;

    sel = malloc(jc->nframes * sizeof(int));
    for (i=0 ; i<jc->nframes ; i++) {
        if ((jc->frame[i].type==t) || ((int)t==-1)) {
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
  @param    jc      Current jitter config.
  @param    sel     Selection array, see jitter_cubeselect()
  @return   1 newly allocated cube.

  This function selects planes in a jitter config and builds a new
  cube containing only the selected planes, or NULL if no plane is
  selected.

  The returned cube should be deallocated using cube_del_shallow().

  If the passed selection list is NULL, all planes are selected.
 */
/*----------------------------------------------------------------------------*/
cube_t * jitter_cubeget(jitter_config_t * jc, int * sel)
{
    cube_t  *   csel ;
    int         nsel ;
    int         i, j ;

    /* Count selected frames */
    if (sel==NULL) {
        /* All frames are selected */
        nsel = jc->nframes ;
    } else {
        /* Count selected frames only */
        nsel=0 ;
        for (i=0 ; i<jc->nframes ; i++) {
            if (sel[i])
                nsel ++ ;
        }
    }
    /* If no frame was selected, return NULL */
    if (nsel==0)
        return NULL ;

    /* Build a new cube structure and copy relevant pointers to it */
    csel = cube_new(jc->lx, jc->ly, nsel);

    if (sel==NULL) {
        /* Copy all plane pointers */
        for (i=0 ; i<jc->nframes ; i++) {
            csel->plane[i] = jc->frame[i].image ;
        }
    } else {
        /* Copy only selected plane pointers */
        j=0 ;
        for (i=0 ; i<jc->nframes ; i++) {
            if (sel[i]) {
                csel->plane[j] = jc->frame[i].image ;
                j++ ;
            }
        }
    }
    return csel ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Put planes back into jitter config.
  @param    jc      Current jitter config.
  @param    sel     Selection array.
  @param    c       Cube to copy to jitter config.
  @return   void

  This function copies plane pointers back into a jitter config,
  according to a selection array. If sel is NULL, all plane
  pointers are copied to the jitter config.
 */
/*----------------------------------------------------------------------------*/
void jitter_cubeput(jitter_config_t * jc, int * sel, cube_t * c)
{
    int i, j ;

    if (sel==NULL) {
        for (i=0 ; i<jc->nframes ; i++) {
            jc->frame[i].image = c->plane[i];
        }
    } else {
        j=0 ;
        for (i=0 ; i<jc->nframes ; i++) {
            if (sel[i]) {
                jc->frame[i].image = c->plane[j];
                j++ ;
            }
        }
    }
    return ;
}

