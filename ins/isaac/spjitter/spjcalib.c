/*----------------------------------------------------------------------------*/
/**
   @file    spjcalib.c
   @author  Y. Jung
   @date    Dec. 2002
   @version	$Revision: 1.11 $
   @brief   Spectroscopic jitter data calibrations
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: spjcalib.c,v 1.11 2003/11/19 12:02:52 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/19 12:02:52 $
	$Revision: 1.11 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "spjtypes.h"
#include "spjconfig.h"

/*-----------------------------------------------------------------------------
  							Private functions
 -----------------------------------------------------------------------------*/

static int spjitter_wlcalibmod(spjitter_config_t * spjc) ;
static int spjitter_wlcalibsky(spjitter_config_t * spjc)  ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Correct the flatfield on type_obj frames
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_flatfield(spjitter_config_t * spjc)
{
    image_t     *   flat ;
    cube_t      *   cube ;
    int         *   selection ;

    /* Test if Flatfielding is required */
    if (!spjc->cal_spflat_active) {
        e_comment(1, "No flatfield provided - skipping") ;
        return 0 ;
    }

    /* Load the flat */
    if ((flat = image_load(spjc->cal_spflat_name)) == NULL) {
        e_error("Cannot load the flatfield - abort") ;
        return -1 ;
    }
    
    /* Put type_obj images in a cube */
    selection = spjitter_cubeselect(spjc, type_obj) ;
    
    /* Build a cube with all the obj input frames */
    cube = spjitter_cubeget(spjc, selection) ;
        
    /* Divide by flatfield */
    cube_correct_ff_dark_badpix(cube, flat, NULL, NULL) ;
    image_del(flat) ;
                       
    /* Put the images back into spjc */
    spjitter_cubeput(spjc, selection, cube) ;
    free(selection) ;
                        
    /* Destroy the cube */
    cube_del_shallow(cube) ;

    spjc->divided_by_flat = 1 ;
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the wavelength calibration with different methods	
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_wlcalib(spjitter_config_t * spjc)
{
    qfits_table     *   arc_table ;
    double              null_val ;
    double          *   disprel ;

    /* Initialize */
    null_val = 0.0 ;
    spjc->wavecal_disprel = NULL ;

    /* Check if the wl calib has been requested */
    if (!spjc->wavecal_active) {
        e_comment(0, "Wavelength calibration not requested - skipping") ;
        return 0 ;
    }

    /* Try to calibrate using the arc wavelength cal. table if provided */
    if (spjc->wavecal_arc_active) {
        e_comment(0, "Wavelength calibration using arc table") ;
        if ((arc_table=qfits_table_open(spjc->wavecal_arcfile, 1)) == NULL) {
            e_warning("cannot open arc table") ;
            spjc->status_wavecal_arc = FAILED ;
        } else {
            if ((disprel=(double*)qfits_query_column_data(
                            arc_table, 3, NULL, (void*)&null_val)) == NULL) {
                e_warning("cannot query column of arc table") ;
                spjc->status_wavecal_arc = FAILED ;
            } else {
                spjc->status_wavecal_arc = OK ;
                spjc->status_wavecal_done = OK ;
            }
            qfits_table_close(arc_table) ;
        }
    }
    /* Successful wl calibration */
    if (spjc->status_wavecal_done == OK) {
        /* Put the calibration in spjc->wavecal_disprel  */
        spjc->wavecal_disprel = malloc(sizeof(computed_disprel));
        (spjc->wavecal_disprel)->cc = -1.0 ;
        (spjc->wavecal_disprel)->poly = disprel ;
        disprel = NULL ;
        return 0 ;
    }
    
    /* Compute the physical model solution */
    e_comment(0, "Wavelength calibration using the physical model") ;
    if (spjitter_wlcalibmod(spjc) != 0) {
        e_error("cannot compute physical model") ;
        spjc->status_wavecal_done = FAILED ;
        return -1 ;
    } else spjc->status_wavecal_done = OK ;

    /* Improve the calibration with the sky lines */
    e_comment(0, "Wavelength calibration using the sky lines") ;
    if (spjitter_wlcalibsky(spjc) != 0) spjc->status_wavecal_sky = FAILED ;
    else                                spjc->status_wavecal_sky = OK ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute the physical model
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_wlcalibmod(spjitter_config_t * spjc)
{
    double          central_wavelength ;
    int             found ;
    char        *   s ;
    char            objective[32];
    char            resolution[32];
    int             nbpix ;
    double      *   c ;
    double3     *   plist ;
    double      *   disprel ;
    int             i ;

    /* Initialize */
    central_wavelength = 0.0 ;
    
    /* Get various information from the FITS header */
    found = i = 0 ;
    while ((i<spjc->nframes) && (found == 0)) {
        found = 1 ;
        central_wavelength = isaac_get_central_wavelength(spjc->frame[i].name);
        if (central_wavelength < 0.0) {
            e_warning("cannot get central wavelength from [%s]", 
                    spjc->frame[i].name);
            found = 0 ;
        }

        s = pfits_get(spjc->data_type, spjc->frame[i].name, "objective") ;
        if (s==NULL) {
            e_warning("cannot get objective from [%s]", spjc->frame[i].name);
            found = 0 ;
        } else {
            strcpy(objective, strlwc(s));
        }

        s = pfits_get(spjc->data_type, spjc->frame[i].name, "resolution") ;
        if (s==NULL) {
            e_warning("cannot get resolution from [%s]", spjc->frame[i].name);
            found = 0 ;
        } else {
            strcpy(resolution, s);
        }

        s = pfits_get(spjc->data_type, spjc->frame[i].name, "naxis1") ;
        if (s==NULL) {
            e_warning("cannot get x size from [%s]", spjc->frame[i].name) ;
            found = 0 ;
        } else {
            nbpix = (int)atoi(s) ;
        }
        i++ ;
    }

    if (found == 0) {
        e_error("cannot find grating info in header files") ;
        return -1 ;
    }

    /* c is an array of nbpix doubles with the wavelength for each pix. */
    if ((c=isaac_physical_model(central_wavelength,
                                objective,
                                resolution,
                                nbpix)) == NULL) {
            e_error("cannot compute the physical model calibration") ;
            return -1 ;
    }

    /* A polynomial fit is computed  */
    plist = double3_new(nbpix);
    for (i=0 ; i<nbpix ; i++) {
        plist->x[i] = (double)(i+1) ;
        plist->y[i] = c[i] ;
    }
    free(c) ;
    disprel = fit_1d_poly(spjc->wavecal_nb_coeff-1, plist, NULL) ;
    double3_del(plist) ;
    
    /* Display the physical model solution */
    e_comment(1, "Physical model sol.: wave = f(pix), pix in [1 1024] with:") ;
    e_comment(1, "    f(x) = %g + %g*x + %g*x^2 + %g*x^3",
            disprel[0], disprel[1], disprel[2], disprel[3]) ;

    /* Put the physical model solution in spjc */
    if (spjc->wavecal_disprel != NULL) {
        if ((spjc->wavecal_disprel)->poly != NULL) 
            free((spjc->wavecal_disprel)->poly) ;
        free(spjc->wavecal_disprel) ;
    }
    spjc->wavecal_disprel = malloc(sizeof(computed_disprel));
    (spjc->wavecal_disprel)->cc = -1.0 ;
    (spjc->wavecal_disprel)->poly = disprel ;
    disprel = NULL ;
    
    /* Return */
    return 0 ;
}
    
/*----------------------------------------------------------------------------*/
/**
  @brief    Wavelength calibration with sky lines
  @param    spjc    spjitter_config_t object
  @return   0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
static int spjitter_wlcalibsky(spjitter_config_t * spjc) 
{
    int                     order ;
    computed_disprel    *   disprel ;
    double              *   phdisprel ;
    double                  slit_width ;
    int                     i ;
    int remove_thermal = isaac_has_thermal(spjc->frame[0].name) > 0;

    /* Compute the slit_width */
    slit_width = isaac_get_slitwidth(spjc->frame[0].name) ;
    if (slit_width == -1) {
        e_error("cannot get the slit width") ;
        return -1 ;
    }

    /* A first guess should be contained in spjc */
    if (spjc->wavecal_disprel == NULL) {
        e_error("First guess wavelength solution missing") ;
        return -1 ;
    }
    
    /* Test if a sky frame is specified */
    if (spjc->sky_lines == NULL) {
        e_warning("No sky frame specified - use the physical mod.") ;
        return -1 ;
    }

    /* Get the order */
    if ((order = isaac_find_order(spjc->frame[0].name))==-1){
       e_warning("cannot determine order - use 1") ;
       order = 1 ;
    }

    /* Create 1st order model based on wave range */
    phdisprel = malloc(4 * sizeof(double)) ;
    phdisprel[0] = (spjc->wavecal_disprel)->poly[0] ;
    phdisprel[1] = (spjc->wavecal_disprel)->poly[1] ;
    phdisprel[2] = (spjc->wavecal_disprel)->poly[2] ;
    phdisprel[3] = (spjc->wavecal_disprel)->poly[3] ;

    /* Compute the dispersion relation */
    if ((disprel = spectro_compute_disprel(spjc->sky_lines,
                                           spjc->wavecal_discard_lo,
                                           spjc->wavecal_discard_hi,
                                           spjc->wavecal_discard_le,
                                           spjc->wavecal_discard_ri,
                                           remove_thermal,
                                           "oh",
                                           slit_width,
                                           order,
                                           phdisprel)) == NULL) {
        e_warning("cannot compute the dispersion relation on sky") ;
        return -1 ;
    }
    free(phdisprel);

    /* Put the computed solution in the spjc */
    if (spjc->wavecal_disprel) {
        if ((spjc->wavecal_disprel)->poly !=NULL) 
            free((spjc->wavecal_disprel)->poly) ;
        free(spjc->wavecal_disprel) ;
    }
    spjc->wavecal_disprel = disprel ;    
    
    /* Display the physical model solution */
    e_comment(1, "Cross-correlation quality: %g", disprel->cc) ;
    e_comment(1, "Sky lines sol.: wave = f(pix), pix in [1 1024] with:") ;
    e_comment(1, "    f(x) = %g + %g*x + %g*x^2 + %g*x^3",
            disprel->poly[0], disprel->poly[1],
            disprel->poly[2], disprel->poly[3]) ;
    
    /* Return */
    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Compute and correct the distortion for type_obj frames  
  @param    spjc    spjitter_config_t object
  @return   int 0 if ok, -1 otherwise
 */
/*----------------------------------------------------------------------------*/
int spjitter_distortion(spjitter_config_t * spjc)
{
    poly2d          *   correct_arc ;
    poly2d          *   correct_sttr ;
    image_t         *   tmp_image ;
    int                 i, j ;
        
    /* First test if the distortion is requested */
    if (spjc->distortion_active == 0) { 
        spjc->status_disto_slit_curv = SKIPPED ;
        spjc->status_disto_startrace = SKIPPED ;
        return 0 ;
    }
    
    /* Initialize */
    correct_arc = correct_sttr = NULL ;
 
    /* FIND THE COEFFICIENTS OF THE DISTORTION */
    
    /* Test if arc calibr. table is available */
    if (spjc->cal_arc_active) {
        correct_arc = read_poly2d_from_table(spjc->cal_arc_name) ;
        if (correct_arc == NULL) {
            e_error("cannot read the arc table") ;
            spjc->status_disto_slit_curv = FAILED ;
        } else {
            spjc->status_disto_slit_curv = OK ;
        }
    } else if (spjc->sky_lines != NULL) {
        /* Estimate the distortion with the sky frame */
        e_comment(2, "computing distortion coefficients...");
        if ((correct_arc=isaac_compute_distortion(spjc->sky_lines,
                                                spjc->distor_xmin,
                                                spjc->distor_ymin,
                                                spjc->distor_xmax,
                                                spjc->distor_ymax,
                                                spjc->auto_dark_subtraction,
                                                NULL,
                                                NULL)) == NULL) {
            e_error("in distortion computation") ;
            spjc->status_disto_slit_curv = FAILED ;
        } else {
            spjc->status_disto_slit_curv = OK ;
        }
    } else {
        spjc->status_disto_slit_curv = SKIPPED ;
    }

    /* Test if startrace calibration table is available */
    if (spjc->cal_startrace_active) {
        correct_sttr = read_poly2d_from_table(spjc->cal_startrace_name) ;
        if (correct_sttr == NULL) {
            e_error("cannot read the startrace table") ;
            spjc->status_disto_startrace = FAILED ;
        } else {
            spjc->status_disto_startrace = OK ;
        }
    } else {
        spjc->status_disto_startrace = SKIPPED ;
    }

    /* Stop if no distortion solution is there */
    if ((spjc->status_disto_startrace != OK) && 
        (spjc->status_disto_slit_curv != OK)) {
        e_warning("cannot estimate the distortion") ;
        return 0 ;
    }
    
    /* CORRECTION OF THE DISTORTION ON ALL THE DIFFERENCE (type_obj) FRAMES */

    /* Use the identity polynomial in no distortion cases */
    if (spjc->status_disto_startrace != OK) {
        /* Polynomial f(x,y) = y */
        correct_sttr = poly2d_build_from_string("0 1 1.0") ;
    }
    if (spjc->status_disto_slit_curv != OK) {
        /* Polynomial f(x,y) = x */
        correct_arc = poly2d_build_from_string("1 0 1.0") ;
    }

    /* For each type_obj frame */
    j = 0 ;
    for (i=0 ; i<spjc->nframes ; i++) {
        if (spjc->frame[i].type == type_obj) {
            compute_status("warping images", j, spjc->nobjframes, 1) ;
            /* Apply the transformation on the images of the current cube */
            if ((tmp_image = image_warp_generic(spjc->frame[j].image,
                            "default",
                            correct_arc,
                            correct_sttr)) == NULL) {
                e_error("in the distortion correction") ;
                poly2d_free(correct_arc) ;
                poly2d_free(correct_sttr) ;
                if (spjc->status_disto_slit_curv == OK)
                    spjc->status_disto_slit_curv = FAILED ;
                if (spjc->status_disto_startrace == OK)
                    spjc->status_disto_startrace = FAILED ;
                return -1 ;
            }
            /* Put the corrected image in spjc */
            image_del(spjc->frame[j].image) ;
            spjc->frame[j].image = tmp_image ;
            tmp_image = NULL ;
            j++ ;
        }
    }

    /* Free and return  */
    poly2d_free(correct_arc) ;
    poly2d_free(correct_sttr) ;
    return 0 ;
}

