/*----------------------------------------------------------------------------*/
/**
   @file	slitposition.c
   @author	Yves Jung
   @date	28 Mar 2003
   @version	$Revision: 1.2 $
   @brief	Slit position computation
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: slitposition.c,v 1.2 2003/03/28 14:46:17 yjung Exp $
	$Author: yjung $
	$Date: 2003/03/28 14:46:17 $
	$Revision: 1.2 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "eclipse.h"
#include "slitposition.h"

/*-----------------------------------------------------------------------------
                                Defines
 -----------------------------------------------------------------------------*/

#define MAX_NB_EROSIONS     1024
#define KERNEL_SIZE_Y       5

/*-----------------------------------------------------------------------------
  							Function prototypes
 -----------------------------------------------------------------------------*/

static int slitpos_find_edges_one_line(image_t *, int, int *, int *) ;
static int slitpos_find_vert_slit_ends(image_t *, int, int *, int *) ;
static int slitpos_find_vert_pos(image_t *, int *) ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief    Detect the slit position, detect its ends, extract a thin image
            containing only the slit and find its edges
  @param    inimage input image
  @param    slit_max_width maximum slit width
  @param    slit_angle  pointer to the angle horizontal-slit
  @param    slit_length pointer to the slit length
  @return   ptr to 3 double3 objects.

  This function can be used for vertical slits.

  This function returns 3 double3 objects:

  - Left or Lower edge of the slit
  - Center of the slit
  - Right or Upper edge of the slit

  NB: Coordinates use FITS convention.
 */
/*----------------------------------------------------------------------------*/
double3 ** slitpos_analysis(
        image_t *   inimage,
        int             slit_max_width,
        double      *   slit_angle,
        int         *   slit_length)
{
    image_t        *   filtered ;
    image_t        *   extr_fits ;
    int                 slit_pos ;
    int                 spec_size ;
    int                 slit_size ;
    int                 slit_top_y ;
    int                 slit_bot_y ;
    double3         **  out ;
    double3         *   slit_l,
                    *   slit_c,
                    *   slit_r ;
    int                 right_pos ;
    int                 left_pos ;
    double          *   coeff_r ;
    double          *   coeff_l ;

    int                 i ;

    /* Flip the image or not to have a vertical slit */
    spec_size = inimage->lx ;
    slit_size = inimage->ly ;

    /* Find the position of the slit */
    if ((slitpos_find_vert_pos(inimage, &slit_pos)) == -1) {
        e_error("cannot find the slit position") ;
        return NULL ;
    }

    /* Filter the input image to 'erase' the bad pixels */
    if ((filtered = image_filter_median(inimage)) == NULL) {
        e_error("unable to filter the image") ;
        return NULL ;
    }

    /* Extract a thin image containing the slit */
    if ((extr_fits = image_getvig(filtered,
                            (int)(slit_pos-(int)(slit_max_width/2)),
                            1,
                            (int)(slit_pos+(int)(slit_max_width/2)),
                            (int)(slit_size))) == NULL) {
        e_error("unable to extract the thin image") ;
        image_del(filtered) ;
        return NULL ;
    }

    /* Find the ends of the slit */
    if ((slitpos_find_vert_slit_ends(extr_fits,
                            KERNEL_SIZE_Y,
                            &slit_bot_y,
                            &slit_top_y)) == -1) {
        e_error("cannot find the ends of the slit") ;
        image_del(filtered) ;
        image_del(extr_fits) ;
        return NULL ;
    }
    image_del(extr_fits) ;
    *slit_length = slit_top_y - slit_bot_y ;

    /* Extract an image with exactly the slit */
    if ((extr_fits = image_getvig(filtered,
                            (int)(slit_pos-(int)(slit_max_width/2)),
                            slit_bot_y,
                            (int)(slit_pos+(int)(slit_max_width/2)),
                            slit_top_y)) == NULL) {
        e_error("cannot extract a thin image") ;
        image_del(filtered) ;
        return NULL ;
    }
    image_del(filtered) ;

    /* Allocate the slit position arrays */
    slit_l = double3_new(*slit_length);
    slit_c = double3_new(*slit_length);
    slit_r = double3_new(*slit_length);

    /* Find the edges of the slit */
    for (i=0 ; i<*slit_length ; i++) {
        if ((slitpos_find_edges_one_line(extr_fits,
                                i,
                                &left_pos,
                                &right_pos)) == -1) {
            e_error("cannot find the edges of the [%d]th line", i+1) ;
            image_del(extr_fits) ;
            double3_del(slit_l);
            double3_del(slit_c);
            double3_del(slit_r);
            return NULL ;
        }
        /* Store the edges as horizontal lines for the fit */

        slit_l->y[i] = (double)left_pos ;
        slit_l->x[i] = (double)(i+slit_bot_y-1) ;

        slit_r->y[i] = (double)right_pos ;
        slit_r->x[i] = (double)(i+slit_bot_y-1) ;
    }
    image_del(extr_fits) ;

    /* Linear regression to find the edges */
    coeff_l = fit_slope_robust(slit_l);
    coeff_r = fit_slope_robust(slit_r);

    /* Rewrite the edges in the out table, and write the center */
    for (i=0 ; i<*slit_length ; i++) {
        slit_l->y[i] = slit_c->y[i] = slit_r->y[i] = (double)(i+slit_bot_y);
        slit_l->x[i] =
            coeff_l[0]+coeff_l[1]*slit_l->y[i]+(slit_pos-slit_max_width/2);
        slit_r->x[i] =
            coeff_r[0]+coeff_r[1]*slit_r->y[i]+(slit_pos-slit_max_width/2);
        slit_c->x[i] = (double)((slit_l->x[i] + slit_r->x[i])/2.) ;
    }
    free(coeff_r) ;
    free(coeff_l) ;

    /* Find the slit angle in degrees with the horizontal axis   */
    *slit_angle = (double)(atan((slit_c->y[*slit_length-1] -
                        slit_c->y[0]) / (slit_c->x[*slit_length-1] -
                        slit_c->x[0]))) ;
    *slit_angle *= 180.0/M_PI ;

    /* Free and return   */
    out = malloc(3*sizeof(double3*)) ;
    out[0] = slit_l ;
    out[1] = slit_c ;
    out[2] = slit_r ;
    return out ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Return the first pixel higher than avg starting from the
            left and from the right of the line 
  @param    inimage input image
  @param    line_pos    line position
  @param    left_pos    pointer to left position
  @param    right_pos   pointer to right position
  @return   -1 in error case, 0 otherwise
  
  Positions in C coordinates (first pixel position is 0)
 */
/*----------------------------------------------------------------------------*/
static int slitpos_find_edges_one_line(
        image_t    *   inimage,
        int             line_pos,
        int         *   left_pos,
        int         *   right_pos)
{
    image_stats *   statistics ;
    pixelvalue      threshold ;
    int             zone[4] ;

    int             i ;
    
    /* Find the threshold */
    zone[0] = 1 ;
    zone[1] = inimage->lx ;
    zone[2] = line_pos+1 ; 
    zone[3] = line_pos+1 ;

    if ((statistics = image_getstats_opts(inimage,
                                           NULL,
                                           NULL,
                                           zone))==NULL) {
        e_error("cannot get the statistics of [%d]th line",
                line_pos+1);
        return -1 ;
    }
    threshold = statistics->avg_pix ;
    free(statistics) ;
    
    /* Detect the left edge */
    i = 0 ;
    while ((inimage->data[line_pos*inimage->lx+i] < threshold)
            && (i < inimage->lx)) i++ ;
    *left_pos = i ;

    /* Detect the right edge */
    i = inimage->lx - 1 ;
    while ((inimage->data[line_pos*inimage->lx+i] < threshold)
            && (i >= 0 )) i-- ;
    *right_pos = i ;

    return 0 ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Find the ends of a vertical slit (y coordinates in FITS
            convention)
  @param    in  input image
  @param    kernel_size vertical kernel size
  @param    bot_slit_y  bottom slit y position
  @param    top_slit_y  top slit y position
  @return   -1 in error case, 0 otherwise
  
  The input image as to be as thin as possible to contain only the slit
 */
/*----------------------------------------------------------------------------*/
static int slitpos_find_vert_slit_ends(
        image_t    *   in,
        int             kernel_size,
        int         *   bot_slit_y,
        int         *   top_slit_y)
{
    image_stats     *   statistics ;
    pixelmap        *   binary ;
    pixelmap        *   kernel ;
    intimage        *   label_image ;
    int                 nobj ;
    int                 erosions_nb ;
    int                 i ;

    /* Threshold to have a binary image */
    if ((statistics = image_getstats(in)) == NULL) {
        e_error("unable to get the image stats") ;
        return -1 ;
    }
    binary = image_threshold2pixelmap(in,
                                     (double)statistics->avg_pix,
                                     (double)statistics->max_pix);
    free(statistics) ;
    if (binary==NULL) {
        e_error("failed while binarizing the image") ;
        return -1 ;
    }

    /* Define the kernel for morpho operations */
    if ((kernel = pixelmap_new(1,kernel_size)) == NULL) {
        e_error("cannot create the kernel") ;
        pixelmap_del(binary) ;
        return -1 ;
    }

    /* Erode until there is 1 object left in the image */
    i = 0 ;
    label_image = intimage_labelize_pixelmap(binary, &nobj) ;
    intimage_del(label_image);
    while (nobj>1) {
        if (pixelmap_morpho_erosion_k(binary, kernel) != 0) {
            e_error("cannot erode") ;
            pixelmap_del(binary) ;
            pixelmap_del(kernel) ;
            return -1 ;
        }
        i++ ;
        if (i >= MAX_NB_EROSIONS) {
            e_error("max number of erosions reached : [%d] - aborting",
                    i) ;
            pixelmap_del(binary) ;
            pixelmap_del(kernel) ;
            return -1 ;
        }
        label_image = intimage_labelize_pixelmap(binary, &nobj) ;
        intimage_del(label_image);
    }
    if (nobj<=0) {
        e_error("no detected slit") ;
        pixelmap_del(binary) ;
        pixelmap_del(kernel) ;
        return -1 ;
    }
    erosions_nb = i ;

    /* Reconstruct the slit with dilatations */
    if (erosions_nb > 0) {
        pixelmap_del(kernel) ;
        if ((kernel = pixelmap_new(1,
                            (kernel_size-1)*erosions_nb+1)) == NULL) {
            e_error("cannot create the kernel") ;
            pixelmap_del(binary) ;
            return -1 ;
        }
        if (pixelmap_morpho_dilation_k(binary, kernel) != 0) {
            e_error("cannot dilate") ;
            pixelmap_del(binary) ;
            pixelmap_del(kernel) ;
            return -1 ;
        }
    }
    pixelmap_del(kernel) ;

    /* Find the ends of the slit */
    i = 0 ;
    while (binary->data[i] != 1) i++ ;
    *bot_slit_y = (int)(i/binary->lx + 1) ;
    i = (binary->lx * binary->ly) - 1 ;
    while (binary->data[i] != 1) i-- ;
    *top_slit_y = (int)(i/binary->lx + 1) ;

    /* Free and return */
    pixelmap_del(binary) ;
    return 0 ;
}


/*----------------------------------------------------------------------------*/
/**
  @brief    Find a vertical slit position (x coordinate of the slit)
  @param    in  input image
  @param    slit_pos    pointre to the searched position
  @return   -1 in error case, 0 otherwise
  
  Coordinate given in FITS convention (ll is (1,1))
 */
/*----------------------------------------------------------------------------*/
static int slitpos_find_vert_pos(
        image_t    *   in,
        int         *   slit_pos)
{
    image_t    *   filtered ;
    image_t    *   image1D ;
    image_stats *   statistics ;

    /* Filter the input image to 'erase' the bad pixels */
    if ((filtered = image_filter_median(in)) == NULL) {
        e_error("unable to filter the image") ;
        return -1 ;
    }

    /* Collapse the image to a horizontal 1D image */
    if ((image1D = image_collapse(filtered, 0)) == NULL) {
        e_error("unable to collapse the image") ;
        image_del(filtered) ;
        return -1 ;
    }

    /* Search the max of the 1D image to identify the slit position */
    if ((statistics = image_getstats(image1D)) == NULL) {
        e_error("unable to get image statistics") ;
        image_del(filtered) ;
        image_del(image1D) ;
        return -1 ;
    }
    *slit_pos = statistics->max_x + 1;

    /* Free and return */
    free(statistics) ;
    image_del(image1D) ;
    image_del(filtered) ;
    return 0 ;
}

