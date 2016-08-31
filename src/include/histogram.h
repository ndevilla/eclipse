
/*-------------------------------------------------------------------------*/
/**
   @file    histogram.h
   @author  Yves Jung
   @date    Nov 2000
   @version $Revision: 1.6 $
   @brief   Histogram related functions
*/
/*--------------------------------------------------------------------------*/

/*
    $Id: histogram.h,v 1.6 2002/04/29 07:29:45 ndevilla Exp $
    $Author: ndevilla $
    $Date: 2002/04/29 07:29:45 $
    $Revision: 1.6 $
*/

#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "image_handling.h"
#include "gnuplot_i.h"

/*---------------------------------------------------------------------------
   								New type
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Histogram object

  This object contains histogram information about an image. There
  are various kinds of histograms, see the corresponding functions
  in this module.
 */
/*-------------------------------------------------------------------------*/
typedef struct _HISTOGRAM_
{
    /** Array of bins */
	int			*	array ;
    /** Number of bins in the array */
	int				nbin ;
    /** Min value used for sampling */
	pixelvalue		min ;
    /** Max value used for sampling */
	pixelvalue		max ;
    /** Size of each bin */
    double          binsize ;
} histogram ;

/*----------------------------------------------------------------------------
                            Function prototypes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute an image histogram.
  @param    in      input image
  @param    nbin    number of samples
  @param    min     min value (give MIN_PIX_VALUE if you want the im. min)
  @param    max     max value (give MAX_PIX_VALUE if you want the im. max)
  @return   A newly allocated histogram object.

  This function computes the histogram of a given image, considering
  only pixels between @c min and @c max, and sampling over @c nbin
  bins. It returns a newly allocated histogram object, which must
  be deallocated using histogram_del().
 */
/*--------------------------------------------------------------------------*/

histogram * histogram_compute(
        image_t     *   in,
        int             nbin,
        pixelvalue      min,
        pixelvalue      max) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the cumulative histogram for an image
  @param    in      Input image
  @param    nbin    Number of samples
  @param    min     min value (give MIN_PIX_VALUE if you want the im. min)
  @param    max     max value (give MAX_PIX_VALUE if you want the im. max)
  @return   A newly allocated histogram object.

  This function computes the cumulative histogram of a given image,
  considering only pixels between @c min and @c max, and sampling over
  @c nbin bins. It returns a newly allocated histogram object, which must
  be deallocated using histogram_del().
 */
/*--------------------------------------------------------------------------*/
histogram * histogram_compute_cumulative(
    image_t     *   in,
    int             nbin,
    pixelvalue      min,
    pixelvalue      max);


/*-------------------------------------------------------------------------*/
/**
  @brief    Find the mode of an histogram
  @param    histo       The histogram.
  @param    exclude     Flag to exclude extreme bins.
  @return   1 pixelvalue

  This function finds the mode of a histogram, i.e. the pixelvalue
  associated to the central peak. The mode represents the pixelvalue
  which is the one mostly seen in an image.
 */
/*--------------------------------------------------------------------------*/
pixelvalue histogram_find_mode(histogram * histo) ;



/*-------------------------------------------------------------------------*/
/**
  @brief    Histogram constructor.
  @param    nbin    number of bin
  @param    min     min value
  @param    max     max value
  @return   1 newly allocated histogram.

  This function allocates a new histogram, and also allocates the bin
  array used to store sampled values. It must be deallocated using
  histogram_del().
 */
/*--------------------------------------------------------------------------*/
histogram * histogram_new(
        int         nbin,
        pixelvalue  min,
        pixelvalue  max) ;


/*-------------------------------------------------------------------------*/
/** 
  @brief    Histogram destructor.
  @param    h   histogram
  @return   void

  This function deallocates the bin array and the histogram object.
 */
/*--------------------------------------------------------------------------*/
void histogram_del(histogram * h) ;


/*-------------------------------------------------------------------------*/
/**
  @brief    Histogram dump to a FILE.
  @param    h   Histogram to dump
  @param    fp  Opened file pointer to dump to
  @return   void

  This function dumps a histogram onto an opened file pointer (no check
  is done to ensure this). It is Ok to pass stdout or stderr as file
  pointers.
 */
/*--------------------------------------------------------------------------*/
void histogram_dump(histogram * h, FILE * fp);


/*-------------------------------------------------------------------------*/
/**
  @brief    Histogram plot to gnuplot
  @param    h       Histogram to plot.
  @param    gp      Gnuplot session to plot onto.
  @return   void

  This function assumes the passed gnuplot_ctrl pointer points to a valid
  opened gnuplot session. It sends the histogram information onto the
  session and returns.
 */
/*--------------------------------------------------------------------------*/
void histogram_plot(histogram * h, gnuplot_ctrl * gp);


		
#endif
