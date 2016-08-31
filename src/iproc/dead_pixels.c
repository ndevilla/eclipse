/*-------------------------------------------------------------------------*/
/**
   @file	dead_pixels.c
   @author	Nicolas Devillard	
   @date	Sept 15, 1995
   @version	$Revision: 1.22 $
   @brief	dead pixel localization/elimination
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: dead_pixels.c,v 1.22 2003/11/06 09:59:34 yjung Exp $
	$Author: yjung $
	$Date: 2003/11/06 09:59:34 $
	$Revision: 1.22 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "dead_pixels.h"
#include "image_stats.h"

/*---------------------------------------------------------------------------
  								Defines
 ---------------------------------------------------------------------------*/

#define MAX_DEVIATION               500

/*---------------------------------------------------------------------------
 							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Clean an image of its dead pixels.
  @param	dirty		Image to clean.
  @param	deadpixmap	Dead pixel map.
  @return	Newly allocated, clean image.

  Replace dead pixels by an average of the correct neighbors in the
  3x3 neighborhood around each pixel. If no correct pixel can be found
  in the 8 neighbors, the pixel is set to zero.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_clean_deadpix(image_t * dirty, pixelmap * deadpixmap)
{
    image_t     *   cleaned ;
    int             i, j, k, l ;
    int             nx, ny ;
    double          replace ;
    int             pixelpos ;
    int             good_neighbors ;

    if (dirty==NULL || deadpixmap==NULL) return NULL ;
    cleaned = image_new(dirty->lx, dirty->ly) ;

    /* Replace bad pixels by interpolated value */
    for (j=0 ; j < dirty->ly ; j++) {
        for (i=0 ; i<dirty->lx ; i++) {
            pixelpos = i + j*dirty->lx ;
            if (deadpixmap->data[pixelpos] == PIXELMAP_1) {
                cleaned->data[pixelpos] = dirty->data[pixelpos] ;
            } else {
                good_neighbors=0 ;
                replace = 0 ;
                for (l=-1 ; l<=1 ; l++) {
                    ny = j+l ;
                    for (k=-1 ; k<=1 ; k++) {
                        nx = i+k ;
                        /*
                         * Check neighbor is inside image
                         * and pixel is valid in pixelmap.
                         */
                        if ((nx>=0) &&
                            (nx<dirty->lx) &&
                            (ny>=0) &&
                            (ny<dirty->ly) &&
                            (deadpixmap->data[nx+ny*deadpixmap->lx]
                             ==PIXELMAP_1)) {
                            good_neighbors++ ;
                            replace += (double)dirty->data[nx+ny*dirty->lx];
                        }
                    }
                }
                /* Take the mean value over neighbors   */
                if (good_neighbors == 0) {
                    cleaned->data[pixelpos] = (pixelvalue)0;
                } else {
                    cleaned->data[pixelpos] =
                        (pixelvalue)(replace/(double)good_neighbors) ;
                }
            }
        } /* end loop on row    */
    } /* end loop on columns    */
    return cleaned ;
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Clean out a cube of its bad pixels.
  @param	in			Cube to clean.
  @param	deadpixmap	Dead pixel map.
  @return	int 0 if Ok, -1 otherwise.

  Applies a dead pixel cleaning function, according to the passed
  pixelmap, to all planes in the input cube. The input cube is
  modified.
 */
/*--------------------------------------------------------------------------*/
int cube_clean_deadpix(
		cube_t		*	in,
		pixelmap	*	deadpixmap)
{
	int				p ;
	image_t	*	cleaned ;

	if (in==NULL || deadpixmap==NULL) return -1 ;
	for (p=0 ; p<in->np ; p++) {
		cleaned = image_clean_deadpix(in->plane[p], deadpixmap) ;
		image_del(in->plane[p]) ;
		in->plane[p] = cleaned ;
	}
	return 0 ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Detect bad pixels in a single image by median filtering.
  @param	dirty				Image to use for detection.
  @param	median_threshold	Threshold for detection.
  @return	1 newly allocated pixelmap.

  The list of bad pixels is detected by thresholding the difference
  between the original image and a median-filtered version of it. This
  method is extremely sensitive to the input signal and is likely to
  require interaction with a user to iterate until an acceptable pixel
  map is found. A robust dead pixel detection should not be based on
  this method.
 */
/*--------------------------------------------------------------------------*/
pixelmap * image_detect_deadpix_median(
		image_t		*	dirty,
		pixelvalue		median_threshold)
{
	pixelmap	*	badpixmap ;
	int				i ;
	image_t	*	filtered_img ;
	image_t	*	diff_img ;
	image_t	*	abs_img ;

	if (dirty == NULL) return NULL;

	filtered_img = image_filter_median(dirty) ;
	if (filtered_img==NULL) {
		e_error("filter failed: aborting median detection") ;
		return NULL ;
	}

	diff_img = image_sub(dirty, filtered_img) ;
	image_del(filtered_img) ;
	if (diff_img == NULL) {
		e_error("difference failed : aborting median detection") ;
		return NULL ;
	}

	abs_img = image_abs(diff_img) ;
	image_del(diff_img) ;
	if (abs_img == NULL) {
		e_error("absolute value failed : aborting median detection") ;
		return NULL ;
	}

	badpixmap = pixelmap_new(dirty->lx, dirty->ly) ;
	for (i=0 ; i<(abs_img->lx * abs_img->ly) ; i++) {
		if (abs_img->data[i] > median_threshold) {
			badpixmap->data[i] = PIXELMAP_0 ;
			badpixmap->ngoodpix -- ;
		}
	}
	image_del(abs_img) ;
	return badpixmap ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Detect bad pixels in a cube using a median method.
  @param	skyname				Name of the cube to use for detection.
  @param	median_threshold	Threshold for detection.
  @return	1 newly allocated pixelmap.

  A median detection is used on every plane of the input cube. The
  final pixel map is an AND of all pixel maps found (1 for each input
  plane). This means that for a pixel to be declared good, it has to
  be declared good in all produced pixelmaps.

  This method is as unreliable in automatic mode as the
  image_detect_deadpix_median() method. It is likely to require interaction
  with a user to reach an acceptable threshold. The AND condition is
  maybe too restrictive to get usable pixel maps in output.
 */
/*--------------------------------------------------------------------------*/
pixelmap * cube_detect_deadpix_median(
		char 		*	skyname, 
		pixelvalue 		median_threshold)
{
	cube_t		*	sky ;
	int				p ;
	pixelmap	*	last_map ;
	pixelmap	*	final_map ;

	if (!skyname || !*skyname) return NULL ;

	sky = cube_load(skyname) ;
	if (sky == NULL) {
		e_error("cannot load file [%s]: aborting median detection", skyname) ;
		return NULL ;
	}

	final_map = pixelmap_new(sky->lx, sky->ly) ;
	for (p=0 ; p<sky->np ; p++) {
		last_map = image_detect_deadpix_median(sky->plane[p], median_threshold) ;
		if (last_map == NULL) {
			cube_del(sky) ;
			e_error("cannot extract median threshold: aborting") ;
			return NULL ;
		}
		pixelmap_update(final_map, last_map) ;
		pixelmap_del(last_map) ;
	}
	cube_del(sky) ;
	return final_map ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Find out bad pixels by observing pixel behaviour in time.
  @param	skyname			Input cube name.
  @param	sigma_width		Sigma threshold.
  @return	1 newly allocated pixelmap.

  An image of the standard deviations of the pixels along time is
  computed. This image expresses the variability of the pixels along
  time in the cube. This standard deviation image is then thresholded
  using the provided threshold to yield a map of the most agitated
  pixels. These pixels are declared bad.

  This method is not reliable, do not use it in automatic mode.
 */
/*--------------------------------------------------------------------------*/
pixelmap * cube_detect_deadpix_z(
		char 	*	skyname, 
		double 		sigma_width) 
{
	pixelmap	*	map1  ;
	cube_t		*	sky_cube ;
	image_t		*	stdev_img ;
	image_t		*	thresh_stdev_img ;
	double			mean_pix,
					sigma2,
					sigma ;
	double			min_stdev_skycube,
					max_stdev_skycube ;

    /*
     * Sky processing: the sky is a slowly varying signal.
     * Compute for each pixel location the standard deviation along
     * with planes.
     * A low stdev means the pixel is blind.
     * A high stdev means the pixel's response to a slowly varying signal
     * is too high, therefore must be rejected.
     */

	if (!skyname || !*skyname) return NULL ;

	sky_cube = cube_load(skyname) ;
	if (sky_cube == NULL) {
		e_error("loading [%s]: aborting", skyname) ;
		return NULL ;
	}

	/* Get standard deviation along time for the cube	*/
	stdev_img = cube_stdev_z(sky_cube) ;
	cube_del(sky_cube) ;
	if (stdev_img == NULL) {
		e_error("image time stdev failed: aborting pixelmap update") ;
		return NULL ;
	}

	if (debug_active()) {
		image_save_fits(stdev_img, "stdev.fits", BPP_DEFAULT) ;
		e_warning("saving standard deviation image in ./stdev.fits") ;
	}

	/* Get Mean and sigma of standard deviation image */
    /* Threshold to avoid screwing up statistics    */
	thresh_stdev_img = image_threshold(	stdev_img, 
									0, MAX_DEVIATION, 
									0, MAX_DEVIATION) ;
	if (debug_active()>1) {
		image_save_fits(thresh_stdev_img, "tstdev.fits", BPP_DEFAULT) ;
		e_warning("saving thresholded std deviation image in ./tstdev.fits") ;
	}

    /* Get mean and standard deviation of this image    */
	image_del(stdev_img) ;
	mean_pix = image_getmean(thresh_stdev_img) ;
	sigma2 = image_getstdev(thresh_stdev_img) ;
	sigma = sqrt(sigma2) ;

    /* Threshold the image at Width sigma (+/- 1.5) */
	min_stdev_skycube = mean_pix - (sigma_width * 0.5 * sigma) ;
	max_stdev_skycube = mean_pix + (sigma_width * 0.5 * sigma) ;

	map1 = image_threshold2pixelmap(thresh_stdev_img, 
									min_stdev_skycube, 
									max_stdev_skycube) ;
	image_del(thresh_stdev_img) ;
	return map1 ;
}
