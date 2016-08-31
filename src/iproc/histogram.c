/*-------------------------------------------------------------------------*/
/**
   @file	histogram.c
   @author	Yves Jung
   @date	Nov 2000
   @version	$Revision: 1.12 $
   @brief	Histogram related functions
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: histogram.c,v 1.12 2002/06/13 13:10:20 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/06/13 13:10:20 $
	$Revision: 1.12 $
*/

/*----------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "histogram.h"

/*----------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute an image histogram.
  @param	in		input image
  @param	nbin	number of samples
  @param	min		min value (give MIN_PIX_VALUE if you want the im. min)
  @param	max		max value (give MAX_PIX_VALUE if you want the im. max)
  @return	A newly allocated histogram object.

  This function computes the histogram of a given image, considering
  only pixels between @c min and @c max, and sampling over @c nbin
  bins. It returns a newly allocated histogram object, which must
  be deallocated using histogram_del().
 */
/*--------------------------------------------------------------------------*/
histogram * histogram_compute(
		image_t		*	in,
		int				nbin,
		pixelvalue		min,
		pixelvalue		max)
{
	histogram	*	h ;
	int				bin_id ;
	int				i ;

	/* Test min # of bins */
	if (nbin < 1) {
		e_error("not enough bins: cannot compute histogram") ;
		return NULL ;
	}
	if (nbin > in->lx * in->ly / 10) {
		e_error("too many bins: cannot compute histogram");
		return NULL ;
	}

	/* Use or not the image min and max	 */
	if (min <= (MIN_PIX_VALUE+1)) min = image_getmin(in) ;
	if (max >= (MAX_PIX_VALUE-1)) max = image_getmax(in) ;

	/* Create the histogram */
	h = histogram_new(nbin, min, max) ;

	/* Compute the histogram */
	for (i=0 ; i<(in->lx * in->ly) ; i++) {
		if ((in->data[i] <= max) && (in->data[i] >= min)) {
			bin_id = (int)((in->data[i] - min) / h->binsize) ;
			if (bin_id == nbin) bin_id-- ;
			(h->array[bin_id]) ++ ;
		}
	}

	/* Return */
	return h ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the cumulative histogram for an image
  @param    in      Input image
  @param    nbin    Number of samples
  @param	min		min value (give MIN_PIX_VALUE if you want the im. min)
  @param	max		max value (give MAX_PIX_VALUE if you want the im. max)
  @return	A newly allocated histogram object.

  This function computes the cumulative histogram of a given image,
  considering only pixels between @c min and @c max, and sampling over
  @c nbin bins. It returns a newly allocated histogram object, which must
  be deallocated using histogram_del().
 */
/*--------------------------------------------------------------------------*/
histogram * histogram_compute_cumulative(
    image_t		*	in,
    int				nbin,
    pixelvalue		min,
    pixelvalue		max)
{
	histogram	*	h ;
	histogram	*	h_cumul ;
	int				i ;
    int             accu ;

    /* Compute normal histogram */
    h = histogram_compute(in, nbin, min, max);
    if (h==NULL) {
        return NULL ;
    }
	/* Create cumulative histogram */
	h_cumul = histogram_new(h->nbin, h->min, h->max) ;

	/* Fill up cumulative histogram */
    accu = 0 ;
	for (i=0 ; i<h->nbin ; i++) {
        accu += h->array[i] ;
        h_cumul->array[i] = accu ;
	}
    histogram_del(h);
	return h_cumul ;
}


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
pixelvalue histogram_find_mode(histogram * histo)
{
	int				mode_id ;
	int				curr_val ;
	pixelvalue		mode ;
	
	int				i ;

	/* Find the histogram maximum */
	mode_id = 0 ;
	curr_val = histo->array[0] ;
	
	for (i=1 ; i<histo->nbin ; i++) {
		if (histo->array[i] > curr_val) {
			curr_val = histo->array[i] ;
			mode_id = i ;
		}
	}

	/* Compute the mode */
	mode = histo->min + (histo->max - histo->min)*mode_id/histo->nbin ;

	/* Return it */
	return mode ;
}


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
        pixelvalue  max)
{
    histogram   *   h ;
    
    h = malloc(sizeof(histogram)) ;
    h->min = min ;
    h->max = max ;
    h->nbin = nbin ;
    h->array = calloc(nbin, sizeof(int)) ;
    h->binsize = (double)(max-min) / (double)nbin ;

    return h ;
}


/*-------------------------------------------------------------------------*/
/** 
  @brief    Histogram destructor.
  @param    h   histogram
  @return   void

  This function deallocates the bin array and the histogram object.
 */
/*--------------------------------------------------------------------------*/
void histogram_del(histogram * h)
{
    if (h==NULL) return ;
    if (h->array!=NULL)
        free(h->array) ;
    free(h) ;
    return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Histogram dump to a FILE.
  @param    h   Histogram to dump
  @param    fp  Opened file pointer to dump to
  @return   void

  This function dumps a histogram onto an opened file pointer (no check
  is done to ensure this). It is Ok to pass stdout or stderr as file
  pointers.
 */
/*--------------------------------------------------------------------------*/
void histogram_dump(histogram * h, FILE * fp)
{
    int i ;
    
    if (h==NULL || fp==NULL) return ;
    for (i=0 ; i<h->nbin ; i++) {
        fprintf(fp, "%g %g\n", (double)h->min + (double)i * h->binsize,
                                (double)h->array[i]);
    }
    return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Histogram plot to gnuplot
  @param    h       Histogram to plot.
  @param    gp      Gnuplot session to plot onto.
  @return   void

  This function assumes the passed gnuplot_ctrl pointer points to a valid
  opened gnuplot session. It sends the histogram information onto the
  session and returns.
 */
/*--------------------------------------------------------------------------*/
void histogram_plot(histogram * h, gnuplot_ctrl * gp)
{
    int         i ;
    double  *   x ;
    double  *   y ;

    if (h==NULL || gp==NULL) return ;
    /* Create the bin (x-axis) part of the histogram */
    x = malloc(h->nbin * sizeof(double));
    y = malloc(h->nbin * sizeof(double));
    for (i=0 ; i<h->nbin ; i++) {
        x[i] = (double)h->min + (double)i * h->binsize ;
        y[i] = (double)h->array[i] ;
    }

    gnuplot_plot_xy(gp, x, y, h->nbin, "Histogram");
    free(x);
    free(y);
    return ;
}
