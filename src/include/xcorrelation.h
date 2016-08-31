/*-------------------------------------------------------------------------*/
/**
   @file    xcorrelation.h
   @author  N. Devillard & Y. Jung
   @date    November 2000
   @version $Revision: 1.9 $
   @brief   Cross correlation fonctions
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: xcorrelation.h,v 1.9 2002/03/31 22:21:33 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/03/31 22:21:33 $
    $Revision: 1.9 $
*/

#ifndef _X_CORRELATION_H_
#define _X_CORRELATION_H_

/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#include "comm.h"
#include "static_sz.h"
#include "xmemory.h"
#include "cube_handling.h"
#include "image_stats.h"
#include "resampling.h"
#include "doubles.h"
#include "intimage.h"

/*---------------------------------------------------------------------------
                                Defines
 ---------------------------------------------------------------------------*/

/* Correlation area definition  */
#define CORR_DX_MAX     5
#define CORR_DY_MAX     5

/* Surface of measurement   */
#define CORR_HX         25
#define CORR_HY         25

#define	DEFAULT_SIGMA_THRESHOLD	2.0

/*---------------------------------------------------------------------------
                                New types
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	xcorr2d_zone object
 */
/*-------------------------------------------------------------------------*/
typedef struct _xcorr2d_zone_ {
    int xmin ;
    int xmax ;
    int ymin ;
    int ymax ;
	int	size_x ;
	int size_y ;
	int n ;
    int valid ;
} xcorr2d_zone ;

/*---------------------------------------------------------------------------
   						Function ANSI C prototypes	
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Find correlating regions in an image.
  @param    in_image            Image to search for correlating regions.
  @param    edge_x              Edge definition in X.
  @param    edge_y              Edge definition in Y.
  @param    sigma_threshold     Sigma threshold for kappa-sigma clipping.
  @param    min_points          Min # of points to detect.
  @param    max_points          Max # of points to detect.
  @return   Newly allocated list of points in the image.
 
  This function is responsible for searching into an image for
  interesting points for cross-correlation. A point is interesting for
  cross-correlation if it has a clear disymmetry in X and Y, which
  allows to locate it to subpixel precision. Point-like sources are
  okay.
 
  The detected objects are required to be at least edge_x and edge_y
  pixels from the edges of the image. This is done to avoid
  cross-correlating on the edges of an image, which is always tricky
  (loss of precision mostly, deformations of the detector in these
  regions, etc.).
 
  The detection method is currently trying to locate point-like
  sources. It should be enhanced in future versions to locate points
  with a more "cross-correlating" way of behaving.
 
  min_points and max_points can be given as -1, in which case default
  values are used (min=1, max=100).

  The returned list of points must be deallocated using double3_del().
 */
/*--------------------------------------------------------------------------*/
double3 * get_xcorrelation_points(
        image_t     *   in_image,
        int             edge_x,
        int             edge_y,
        double          sigma_threshold,
        int             min_points,
        int             max_points) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Get points to blindly estimate the offsets
  @param    inimage             input image
  @param    sigma_threshold     threshold
  @param    edge_x              X edge size to reject
  @param    edge_y              Y edge size to reject
  @param    min_points          Minimum nb of points to detect
  @param    max_points          Maximum nb of points to detect
  @return   List of points in the image
  This is the engine of get_xcorrelation_points()
 */
/*--------------------------------------------------------------------------*/
double3 * get_points_engine(
        image_t     *   inimage,
        double          sigma_threshold,
        int             edge_x,
        int             edge_y,
        int             min_points,
        int             max_points) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find out points within bounds in a given list.
  @param    peaks       List of input points to scrutinize.
  @param    lx          Image size in x.
  @param    ly          Image size in y.
  @param    edge_x      Edge condition on x.
  @param    edge_y      Edge condition on y.
  @param    nvalid      Number of valid points in the list.
  @param    valid_flags Flag array indicating which points are within bounds.
  @return   void

  This function is used in the context of get_xcorrelation_points. A
  peak detector is applied in the previous function to locate all
  point-like objects. Some of these objects will be too close to the
  edges and need to be rejected. That is what this function does.

  Provide the size of the image to which the points belong (you do not
  need to provide the image itself), and the edge conditions. An array
  of boolean flags (integer type) will be updated to say for each
  point if it is within bounds or not.
 */
/*--------------------------------------------------------------------------*/
void localize_xcorr_centers(
        double3 *   peaks,
        int         lx,
        int         ly,
        int         edge_x,
        int         edge_y,
        int     *   nvalid,
        int     *   valid_flags) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compare a reference plane to every plane in the cube.
  @param    to_compare      Cube to be compared plane by plane.
  @param    pattern         Reference image to compare with.
  @param    estimates       First-guess estimation of the offsets.
  @param    xcorr_p         List of anchor points.
  @param    search_width    Half-width of search area.
  @param    search_height   Half-height of search area.
  @param    hx              Half-width of measurement area.
  @param    hy              Half-height of measurement area.
  @return   A newly allocated offset measurement as a double3 object.
 
  This function expects in input a reference frame (pattern) against
  which every plane in the input cube (to_compare) will be matched.
  The matching is performed using a 2d cross-correlation, using a
  minimal squared differences criterion. One measurement is performed
  per input anchor point, and the median offset is returned together
  with a measure of similarity for each plane.
 
  The search is performed over 2*search_width+1 times
  2*search_height+1 and at every point 2*hx+1 times 2*hy+1
  measurements are taken.
 
  See other functions in this module to learn more about how the
  cross-correlation is computed.

  The returned object must be deallocated using double3_del().
 */
/*--------------------------------------------------------------------------*/
double3 * xcorr_with_objs(
        cube_t      *   to_compare,
        image_t     *   pattern,
        double3     *   estimates,
        double3     *   xcorr_p,
        int             search_width,
        int             search_height,
        int             hx,
        int             hy) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the median offset between 2 images.
  @param    reference       Reference image.
  @param    compared        Compared image.
  @param    estimate        First-guess estimates of the offsets.
  @param    xcorr_p         List of anchor points.
  @param    search_width    Search width in pixels (+/-)
  @param    search_height   Search height in pixels (+/-)
  @param    hx              Measurement area half-width.
  @param    hy              Measurement area half-height.
  @return   Newly allocated list of offset measurements as a double3.
 
  This function compares two images using a 2d cross-correlation. The
  input images are expected to be more or less the same, up to a shift
  in x and y. The returned offset measures the translation of the
  'compared' image with respect to the 'reference' image.
 
  The cross-correlation is carried out for each anchor point given in
  the list. It is Ok to provide only one anchor point in xcorr_p.
 
  For each point, a cross-correlation criterion will be computed on a
  grid of 2hx+1 by 2hy+1, on the search area defined by search_width
  and search_height. The total search size is 2*search_width+1 by
  2*search_height+1. The total number of pixel operations is quite
  high: number of anchor points times number of search area pixels
  times number of measurement area pixels.
 
  The returned measurement is stored into a double3 object.
  This returned object must be freed using double3_del().
 */
/*--------------------------------------------------------------------------*/
double3 * xcorr_get_median_offset(
        image_t         *   reference,
        image_t         *   compared,
        double3         *   estimate,
        double3         *   xcorr_p,
        int                 search_width,
        int                 search_height,
        int                 hx,
        int                 hy);




/*-------------------------------------------------------------------------*/
/**
  @brief    Get a list of offset measurements from an ASCII file.
  @param    filename        Name of the ASCII file to read.
  @return   A newly allocated list of offset measurements as a double3.
 
  The input ASCII file is expected to contain three columns of
  numbers. The first column contains the plane number (integer), the
  second one contains the offset in X (floating-point or integer), the
  third one contains the offset in Y (floating-point or integer).
 
  The first line in the ASCII file indicates what kind of offsets are
  provided in the file. If the first pair of offsets is (0,0) the offset
  are considered as relative offsets since no object can be at that
  position, i.e. offsets are all relative to the first frame.
  Otherwise the offsets are considered absolute, i.e. they indicate
  the position of the same object in all frames.
 
  Here is an example of a valid offset file:
  \begin{verbatim}
  # Comment lines start with a hash
  1   0   0
  2  -38  45
  3  3.4  2.3
  \end{verbatim}
 
  Notice that plane numbers start at 1.

  The returned object is a double3. At this stage, only the x and y fields
  are used, the z field is left initialized to 0.00 because it has not
  been measured but read from a file. The returned object must be
  deallocated using double3_del().
 */
/*--------------------------------------------------------------------------*/
double3 * load_offsets_from_txtfile(char * filename) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Find offsets in an image sequence without a priori information.
  @param    cube_i      Input cube   
  @param    pattern     Pattern to compare all planes in the cube.
  @return   1 newly allocated double3 object containing offset estimates.

  This function applies the image processing textbook method to
  detect simple shifts between frames. It is very sensitive to offset
  and gain variations between images, so be careful.

  The returned object is a newly allocated double3, to be deallocated
  using double3_del().

  The input images can have any size > 512x512.
 */
/*--------------------------------------------------------------------------*/
double3 * cube_blindoffsets(cube_t * cube_i, image_t * pattern);

#endif
