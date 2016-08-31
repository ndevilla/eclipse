/*-------------------------------------------------------------------------*/
/**
   @file	image_filters.c
   @author	Nicolas Devillard
   @date	Aug 29, 1995
   @version	$Revision: 1.23 $
   @brief	various image filters in spatial domain
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: image_filters.c,v 1.23 2003/10/24 08:40:14 yjung Exp $
	$Author: yjung $
	$Date: 2003/10/24 08:40:14 $
	$Revision: 1.23 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image_filters.h"
#include "image_handling.h"
#include "image_intops.h"
#include "cube_defs.h"
#include "cube_handling.h"
#include "pixel_handling.h"
#include "function_1d.h"
#include "extraction.h"
#include "fourier.h"
#include "xmemory.h"

/*---------------------------------------------------------------------------
                        Static pre-defined filters
 ---------------------------------------------------------------------------*/

static double _filter_mean3[9]={1,1,1,1,1,1,1,1,1};
static double _filter_dx[9]={-1,0,1,-1,0,1,-1,0,1};
static double _filter_dy[9]={-1,-1,-1,0,0,0,1,1,1};
static double _filter_dx2[9]={1,-2,1,1,-2,1,1,-2,1};
static double _filter_dy2[9]={1,1,1,-2,-2,-2,1,1,1};
static double _filter_contour1[9]={1,0,-1,0,0,0,-1,0,1};
static double _filter_contour2[9]={-1,0,1,2,0,-2,-1,0,1};
static double _filter_contour3[9]={-1,2,-1,0,0,0,1,-2,1};
static double _filter_contrast1[9]={1,1,1,1,4,1,1,1,1};
static double _filter_mean5[25]=
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static double _filter_morphomin[9]={1,0,0,0,0,0,0,0,0};
static double _filter_morphomax[9]={0,0,0,0,0,0,0,0,1};
static double _filter_morphomaxmin[9]={-1,0,0,0,0,0,0,0,1};

static struct {
    char    *   name ;
    int         nval ;
    int         morpho ;
    double  *   ker ;
} _filter_defs[] = {
    {"mean3",     9, 0, _filter_mean3},
    {"dx",        9, 0, _filter_dx},
    {"dy",        9, 0, _filter_dy},
    {"dx2",       9, 0, _filter_dx2},
    {"dy2",       9, 0, _filter_dy2},
    {"contour1",  9, 0, _filter_contour1},
    {"contour2",  9, 0, _filter_contour2},
    {"contour3",  9, 0, _filter_contour3},
    {"contrast1", 9, 0, _filter_contrast1},
    {"mean5",    25, 0, _filter_mean5},
    {"min",       9, 1, _filter_morphomin},
    {"max",       9, 1, _filter_morphomax},
    {"max-min",   9, 1, _filter_morphomaxmin},
    {NULL, 0, 0, NULL}
} ;

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Get a pre-defined filter kernel definition.
  @param    name    Name of the filter.
  @param    nval    Output number of values in the array.
  @param    morpho  Flag to set to 1 if required filter is morpho.
  @return   1 pointer to a list of doubles.

  This function takes as input the name of a filter, and returns
  the filter definition as an array of doubles. The array is
  statically allocated inside this module and should not be modified
  or freed by the caller of this function.

  The number of values in the filter is returned in 'nval'. If 'nval'
  is passed as a NULL pointer, it is not updated but the function
  still returns a valid pointer to an array (if one can be found).

  The 'morpho' flag is updated to contain 1 if the filter is
  morphological, 0 if not. If 'morpho' is set to NULL it is not
  updated.

  Valid filters are:

  - "mean3"     3x3 mean (flat)
  - "mean5"     5x5 mean (flat)
  - "dx"        3x3 derivative in x
  - "dy"        3x3 derivative in y
  - "d2x"       3x3 second derivative in x
  - "d2y"       3x3 second derivative in y
  - "contour1"  3x3 contour detector
  - "contour2"  3x3 contour detector
  - "contour3"  3x3 contour detector
  - "contrast1" 3x3 contrast enhancement
  - "min"       3x3 morphological min
  - "max"       3x3 morphological max
  - "median"    3x3 morphological median
  - "max-min"   3x3 morphological max - min

 */
/*--------------------------------------------------------------------------*/

double * image_filter_getkernel(char * name, int * nval, int * morpho)
{
    double * ker ;
    int      i ;

    if (name==NULL) return NULL ;
    ker=NULL ;
    i=0 ;
    while (_filter_defs[i].name) {
        if (!strcmp(name, _filter_defs[i].name)) {
            if (nval!=NULL) {
                *nval = _filter_defs[i].nval ;
            }
            if (morpho!=NULL) {
                *morpho = _filter_defs[i].morpho ;
            }
            ker = _filter_defs[i].ker ;
            break ;
        }
        i++ ;
    }
    return ker ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	Filter an image in spatial domain with a 3x3 kernel.
  @param	image_in	Image to filter.
  @param	filter		Filter definition.
  @return	1 newly allocated image.

  The input filter is defined by a 3x3 double matrix, given as an array
  of 9 doubles. If the matrix is:

  @verbatim
  f7 f8 f9
  f4 f5 f6
  f1 f2 f3
  @endverbatim

  Then the filter is given as an array: {f1, f2, ... f9}.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter3x3(  
		image_t	*	image_in, 
		double	*	filter) 
{
	image_t		*	image_out ;
	int				i, j ;
	double			sum_pix ;
	double			filter_norm ;
	int				curr_pos ;
	int				im_width ;

	if ((image_in==NULL) || (filter==NULL)) return NULL ;

	image_out = image_new(image_in->lx, image_in->ly) ;
	/* precompute inverse sum of filters coeffs	*/
	filter_norm = 0.0 ;
	for (i=0 ; i<9 ; i++)
		filter_norm += filter[i] ;
	
	if (fabs(filter_norm)<1e-6) {
		filter_norm = 1.0 ;
	} else {
		filter_norm = 1.0 / filter_norm ;
	}

	/* Main filter loop	*/
	im_width = image_in->lx ;
	for (j=1 ; j<image_in->ly-1 ; j++) {
		for (i=1 ; i<image_in->lx-1 ; i++) {
			/* Go into upper left corner of current pixel	*/
			curr_pos = i + j*image_in->lx ;

			sum_pix = 0.0 ;
			/* Compute filter combination on first line	*/
			sum_pix += filter[0] * (double)image_in->data[curr_pos-1-im_width] ; 
			sum_pix += filter[1] * (double)image_in->data[curr_pos-im_width] ; 
			sum_pix += filter[2] * (double)image_in->data[curr_pos+1-im_width] ; 
			/* Go on central line, compute filter		*/
			sum_pix += filter[3] * (double)image_in->data[curr_pos-1] ; 
			sum_pix += filter[4] * (double)image_in->data[curr_pos] ; 
			sum_pix += filter[5] * (double)image_in->data[curr_pos+1] ; 
			/* Go on last line, compute filter			*/
			sum_pix += filter[6] * (double)image_in->data[curr_pos-1+im_width] ; 
			sum_pix += filter[7] * (double)image_in->data[curr_pos+im_width] ; 
			sum_pix += filter[8] * (double)image_in->data[curr_pos+1+im_width] ; 

			/* Normalize output	*/
			sum_pix *= filter_norm ; 
			/* Assign value to image_out	*/
			image_out->data[curr_pos] = (pixelvalue)sum_pix ;
		}
	}
	return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Filter an image in spatial domain with a 3x1 kernel.
  @param	image_in	Image to filter.
  @param	filter		Filter definition.
  @return	1 newly allocated image.

  The input filter is defined by a 3x1 double matrix, given as an array
  of 3 doubles: {f1, f2, f3}.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter3x1(  
		image_t	*	image_in, 
		double	*	filter) 
{
	image_t	*	image_out ;
	pixelvalue	*	line_i ;
	pixelvalue	*	line_o ;
	int				i, j ;
	double			sumpix ;
	double			norm[3] ;
	int				lx ;

	if ((image_in==NULL) || (filter==NULL)) return NULL ;

	image_out = image_new(image_in->lx, image_in->ly) ;
	lx = image_in->lx ;

	/* Precompute normalization factors */

	/* Left-side normalization factor */
	norm[0] = filter[1]+filter[2];
	if (fabs(norm[0])<1e-6) {
		norm[0] = 1.0 ;
	} else {
		norm[0] = 1.0 / norm[0] ;
	}
	/* Central normalization factor */
	norm[1] = filter[0]+filter[1]+filter[2];
	if (fabs(norm[1])<1e-6) {
		norm[1] = 1.0 ;
	} else {
		norm[1] = 1.0 / norm[1] ;
	}
	/* Right-side normalization factor */
	norm[2] = filter[0]+filter[1];
	if (fabs(norm[2])<1e-6) {
		norm[2] = 1.0 ;
	} else {
		norm[2] = 1.0 / norm[2] ;
	}

	/* Main filter loop	*/
	line_i = image_in->data ;
	line_o = image_out->data ;
	for (j=0 ; j<image_in->ly ; j++) {

		/* Compute first pixel */
		sumpix = norm[0] *
					(filter[1] * (double)line_i[0] +
					 filter[2] * (double)line_i[1]) ;
		line_o[0] = (pixelvalue)sumpix ;

		/* Compute central pixels */
		for (i=1 ; i<image_in->lx-1 ; i++) {
			sumpix = norm[1] *
					 (filter[0] * (double)line_i[i-1] +
					  filter[1] * (double)line_i[i]   +
					  filter[2] * (double)line_i[i+1]) ;
			
			line_o[i] = (pixelvalue)sumpix ;
		}

		/* Compute last pixel */
		sumpix = norm[2] *
					(filter[0] * (double)line_i[lx-2] +
					 filter[1] * (double)line_i[lx-1]) ;
		line_o[lx-1] = (pixelvalue)sumpix ;

		/* Update line pointers */
		line_i += lx ;
		line_o += lx ;
	}
	return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Filter an image in spatial domain with a 5x5 kernel.
  @param	image_in	Image to filter.
  @param	filter		Filter definition.
  @return	1 newly allocated image.

  The input filter is defined by a 5x5 double matrix, given as an array
  of 25 doubles. If the matrix is:

  @verbatim
  f21  f22  f23  f24  f25
  f16  f17  f18  f19  f20
  f11  f12  f13  f14  f15
  f6   f7   f8   f9   f10
  f1   f2   f3   f4   f5
  @endverbatim

  Then the filter is given as an array: {f1, f2, ... f25}.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter5x5(  
		image_t 	*	image_in, 
		double      *	filter) 
{
    image_t     *	image_out ;
    int				i, j ;
    double          sum_pix ;
    pixelvalue   *	cur_pix ;
    double          filter_norm ;
 

	if ((image_in==NULL) || (filter==NULL)) return NULL ;
    image_out = image_new(image_in->lx, image_in->ly) ;
    /* precompute inverse sum of filters coeffs */  
    filter_norm = 0.0 ;   
    for (i=0 ; i<25 ; i++) 
        filter_norm += filter[i] ; 
         
    if (fabs(filter_norm) < 1e-6) {
        filter_norm = 1.0 ;
	} else {
		filter_norm = 1.0 / filter_norm ;
	}
    
    /* Main filter loop */ 
    for (j=2 ; j<image_in->ly-2 ; j++)   {
        for (i=2 ; i<image_in->lx-2 ; i++) { 
            /* Go into upper left corner of current pixel   */ 
            cur_pix = image_in->data + (i-2) + (j-2) * image_in->lx ; 
             
            sum_pix = 0.0 ;  
            /* Compute filter combination on first line */ 
            sum_pix += filter[0] * (double)cur_pix[0] ; 
            sum_pix += filter[1] * (double)cur_pix[1] ;
            sum_pix += filter[2] * (double)cur_pix[2] ;
            sum_pix += filter[3] * (double)cur_pix[3] ;
            sum_pix += filter[4] * (double)cur_pix[4] ;
            /* Go on second line, compute filter       */
            cur_pix += image_in->lx ;    
            sum_pix += filter[5] * (double)cur_pix[0] ;  
            sum_pix += filter[6] * (double)cur_pix[1] ;
            sum_pix += filter[7] * (double)cur_pix[2] ;
            sum_pix += filter[8] * (double)cur_pix[3] ;
            sum_pix += filter[9] * (double)cur_pix[4] ;
            /* Go on third line, compute filter       */
            cur_pix += image_in->lx ;    
            sum_pix += filter[10] * (double)cur_pix[0] ;  
            sum_pix += filter[11] * (double)cur_pix[1] ;
            sum_pix += filter[12] * (double)cur_pix[2] ;
            sum_pix += filter[13] * (double)cur_pix[3] ;
            sum_pix += filter[14] * (double)cur_pix[4] ;
            /* Go on fourth line, compute filter       */
            cur_pix += image_in->lx ;    
            sum_pix += filter[15] * (double)cur_pix[0] ;  
            sum_pix += filter[16] * (double)cur_pix[1] ;
            sum_pix += filter[17] * (double)cur_pix[2] ;
            sum_pix += filter[18] * (double)cur_pix[3] ;
            sum_pix += filter[19] * (double)cur_pix[4] ;
            /* Go on last line, compute filter          */
            cur_pix += image_in->lx ;    
            sum_pix += filter[20] * (double)cur_pix[0] ;  
            sum_pix += filter[21] * (double)cur_pix[1] ;
            sum_pix += filter[22] * (double)cur_pix[2] ;
            sum_pix += filter[23] * (double)cur_pix[3] ;
            sum_pix += filter[24] * (double)cur_pix[4] ;
            
            /* Normalize output */
            sum_pix *= filter_norm ; 
            /* Assign value to image_out */
            image_out->data[i+j*image_out->lx] = (pixelvalue)sum_pix ;             
        }            
    }         
    return image_out ; 
}    


/*-------------------------------------------------------------------------*/
/**
  @brief	Filter an image in spatial domain with a 3x3 morpho kernel.
  @param	image_in	Image to filter.
  @param	filter		Filter definition.
  @return	1 newly allocated image.

  The input filter is defined by a 3x3 double matrix, given as an array
  of 9 doubles. The first array element will be applied to the min pixel
  in the 3x3 neighborhood, the second to the second-to-min, etc. and
  the last coefficient is applied to the max pixel.

  The returned image is a newly allocated object, it must be freed using
  image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_morpho(  
		image_t 	*	image_in,   
		double       *	filter)
{
    image_t     *	image_out ;
    int	         	i, j, k ;
    double          sum_pix ;
    pixelvalue      curr_3x3[9] ;
    double          filter_norm ;
	int				curr_pos, im_width ;

	if ((image_in==NULL) || (filter==NULL)) return NULL ;
    image_out = image_new(image_in->lx, image_in->ly) ;

    /* precompute inverse sum of filters coeffs */
    filter_norm = 0.0 ;
    for (i=0 ; i<9 ; i++)
        filter_norm += filter[i] ;

    if (fabs(filter_norm) < 1e-6) {
        filter_norm = 1.0 ;
	} else {
		filter_norm = 1.0 / filter_norm ;
	}

    /* Main filter loop */
	im_width = image_in->lx ;
    for (j=1 ; j<image_in->ly-1 ; j++) {
        for (i=1 ; i<image_in->lx-1 ; i++) {
			curr_pos = i + j*im_width ;
			/* Store all relevant pixels in an array for sorting	*/
            curr_3x3[0] = image_in->data[curr_pos - 1 - im_width] ;
            curr_3x3[1] = image_in->data[curr_pos - im_width] ;
            curr_3x3[2] = image_in->data[curr_pos + 1 - im_width] ;

            curr_3x3[3] = image_in->data[curr_pos - 1] ;
            curr_3x3[4] = image_in->data[curr_pos] ;
            curr_3x3[5] = image_in->data[curr_pos + 1] ;

            curr_3x3[6] = image_in->data[curr_pos - 1 + im_width] ;
            curr_3x3[7] = image_in->data[curr_pos + im_width] ;
            curr_3x3[8] = image_in->data[curr_pos + 1 + im_width] ;

			/* Sort array with standard sorting routine	*/
			pixel_qsort(curr_3x3, 9) ;
            sum_pix = 0.0 ;
			for (k=0 ; k<9 ; k++)
				sum_pix += filter[k] * (double)curr_3x3[k] ; 

            /* Normalize output */
            sum_pix *= filter_norm ;
            /* Assign value to image_out */
            image_out->data[curr_pos] = (pixelvalue)sum_pix ;
        }
    }    
    return image_out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a spatial 3x3 median filter to an image.
  @param	in	Image to filter.
  @return	1 newly allocated image.

  Apply a spatial 3x3 median filter to an image, return a newly allocated
  image which must be freed using image_del(). This is an optimized
  version.
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_median(image_t * in)
{
	image_t		*	out ;
	int				i, j ;
	int				curpos ;
	int				width ;
	pixelvalue		current3x3[9] ;

	if (in==NULL) return NULL ;
    out = image_new(in->lx, in->ly) ;

	/* Main filter loop	*/
	width = in->lx ;
	for (j=1 ; j<in->ly-1 ; j++) {
		for (i=1 ; i<in->lx-1 ; i++) {
			curpos = i+j*width ;
			/* store all relevant pixels in an array for sorting	*/

            current3x3[0] = in->data[curpos - 1 - width] ;
            current3x3[1] = in->data[curpos - width] ;
            current3x3[2] = in->data[curpos + 1 - width] ;

            current3x3[3] = in->data[curpos - 1] ;
            current3x3[4] = in->data[curpos] ;
            current3x3[5] = in->data[curpos + 1] ;

            current3x3[6] = in->data[curpos - 1 + width] ;
            current3x3[7] = in->data[curpos + width] ;
            current3x3[8] = in->data[curpos + 1 + width] ;

			/* find median by optimized median find	*/	
			out->data[i+j*width] = opt_med9(current3x3) ;
		}
	}
	return out ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a vertical median filter to an image.
  @param	in			Image to filter.
  @param	filtsize	Size of the median kernel.
  @return	1 newly allocated image.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_vertical_median(image_t * in, int filtsize) 
{
	image_t	    *	filt_img;
	int 			col,
					row;
	pixelvalue	*	slit,
				*	save_column;	
	int				maxrow ;
	int				rowdif,
					f2;

	if (in==NULL || filtsize<1) return NULL ;
	maxrow = in->ly;
	if (maxrow <filtsize) return NULL;

	f2 = filtsize/2;
	filt_img = image_new( in->lx, maxrow );
	save_column= calloc(in->ly,sizeof(pixelvalue));
	slit = calloc(filtsize,sizeof(pixelvalue));
	for (col=0;col<in->lx;col++){
		for (row=0;row<in->ly;row++)
            save_column[row] = in->data[col+row*in->lx];
		for (row=0;row<in->ly;row++){
			rowdif= f2-row;
			if (rowdif>0)
				memcpy(slit,save_column,(filtsize-rowdif)*sizeof(pixelvalue));
			else{
				rowdif= row-in->ly+f2+1;
				if (rowdif<0)
					rowdif=0;
				memcpy(slit,&save_column[row-f2],
						(filtsize-rowdif)*sizeof(pixelvalue));
			}
			filt_img->data[col+row*filt_img->lx]=
				median_pixelvalue(slit,filtsize-rowdif);
		}
	}
	free(slit);
	free(save_column);
	return filt_img;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply a spatial median filter to an image, with rect kernel.
  @param	in			Image to filter
  @param	filtsizex	Size of the filter box in x.
  @param	filtsizey	Size of the filter box in y.
  @return	1 newly allocated image.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_large_median(image_t * in, int filtsizex, int filtsizey) 
{
	image_t		*	filt_img;
	int 			col,
					row;
	pixelvalue	*	buf;	
	int				coldif,
					rowdif,
					f2x,
					f2y;
	int				medsize,
					upright_x,
					loleft_x,
					upright_y,
					loleft_y;
	int				outlx,
					outly ;
	int				i, j;
    pixelvalue  *	inpt,
				*	outpt ;

    if (in==NULL) return NULL ;
    if ((in->lx<=filtsizex) || (in->ly<=filtsizey)) return NULL ;

	f2x = filtsizex/2;
	f2y = filtsizey/2;
	filt_img = image_new( in->lx, in->ly);
    buf = malloc(filtsizex*filtsizey*sizeof(pixelvalue)); 
	for (row=0;row<in->ly;row++){
		rowdif= f2y-row;
		if (rowdif<0){
			rowdif= row-in->ly+f2y+1;
			if (rowdif<0)
				rowdif=0;
		}
		loleft_y = row - f2y ;
		if (loleft_y <0)
			loleft_y = 0;
		upright_y= row + f2y ;
		if (upright_y >= in->ly)
			 upright_y= in->ly;
    	outly = upright_y - loleft_y ; 
		for (col=0;col<in->lx;col++){
			coldif= f2x-col;
			if (coldif<0){
				coldif= col-in->lx+f2x+1;
				if (coldif<0)
					coldif=0;
			}
			medsize = (filtsizex-coldif) * (filtsizey-rowdif);
			loleft_x = col - f2x ;
			if (loleft_x <0)
				loleft_x = 0;
			upright_x= col + f2x ;
			if (upright_x >= in->lx)
				 upright_x= in->lx;
			/* inline version of extract_slit_from_image... */			
			outlx = upright_x - loleft_x ;
			/* Optimized extraction */
    		for (j=0; j<outly ; j++) {
        		inpt = in->data+loleft_x + (j+loleft_y)*in->lx ;
        		outpt = buf + j*outlx ;
        		for (i=0; i<outlx ; i++) 
            		*outpt++ = *inpt++ ;
    		}
			filt_img->data[col+row*filt_img->lx]=
				median_pixelvalue(buf,medsize );
		}
	}
	free(buf);
	return filt_img;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Apply an horizontal median filter to an image.
  @param	in			Image to filter.
  @param	filtsize	Size of the filter to apply.
  @return	1 newly allocated image.

  The returned image must be freed using image_del().
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_horizontal_median(image_t * in, int filtsize) 
{
	image_t		*	filt_img;
	int 			col,
					row;
	pixelvalue	*	save_slit;	
	int				maxcol;
	int				f2 ;
	int				coldif;

	maxcol = in->lx;
	f2 = filtsize/2;
	if (maxcol <filtsize) return NULL;
	filt_img = image_new( maxcol,in->ly );
	save_slit = calloc(filtsize,sizeof(pixelvalue));
	for (row=0;row<in->ly;row++){
		for (col=0;col<maxcol;col++){
			coldif= f2-col;
			if (coldif>0)
				memcpy(save_slit,&(in->data[0+row*in->lx]),
						(filtsize-coldif)*sizeof(pixelvalue));
			else{
				coldif= col-in->lx+f2+1;
				if (coldif<0)
					coldif=0;
					memcpy(save_slit,&(in->data[col-f2+row*in->lx]),
						(filtsize-coldif)*sizeof(pixelvalue));
			}
			filt_img->data[col+row*filt_img->lx]=
				median_pixelvalue(save_slit,filtsize-coldif);
		}
	}
	free(save_slit);
	return filt_img;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Filter an image with a flat kernel of given size.
  @param	im		Image to filter.
  @param	ksize	Half-size of the filtering kernel.
  @return	1 newly allocated image.

  Apply a flat filter of given size. A flat filter is defined by a
  convolution matrix filled with 1's everywhere. The matrix is always
  odd-sized and square. The given parameter defines the half-size of the
  filter to apply.

  The filter is applied in one pass to avoid memory overflows. Applying it
  in two passes would be faster but requires twice the amount of memory.
  This is an issue, especially with WFI frames.

  Example: applying a 9x9 flat filter would be done by setting ksize to 4.
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_flat(image_t * im, int ksize)
{
	image_t	    *	filt ;
	int				i, j, k, l ;
	double			acc ;
	int				nacc ;
	register
	pixelvalue	*	out_p ;

	if (im==NULL || ksize<1) return NULL ;
	if (ksize>im->lx || ksize>im->ly) return NULL ;

	filt = image_new(im->lx, im->ly);

	/*
	 * Following is an optimized quadruple loop.
	 * Not the most efficient way of implementing a flat filter, but
	 * fast enough for most purposes.
	 */

	/* Loop over all image pixels */
	out_p = filt->data ;
	for (j=0 ; j<filt->ly ; j++) {
		for (i=0 ; i<filt->lx ; i++) {
			/* Loop over convolution matrix */
			acc  = 0 ;
			nacc = 0 ;
			for (l=-ksize ; l<=ksize ; l++) {
				if (((j+l)>=0) && ((j+l)<im->ly)) {
					for (k=-ksize ; k<=ksize ; k++) {
						/* Test how many pixels are visible from here */
						if (((i+k)>=0) && ((i+k)<im->lx)) {
							nacc ++ ;
							acc += (double)im->data[(i+k)+(j+l)*im->lx];
						}
					}
				}
			}

			/* Average accumulator */
			acc /= (double)nacc ;
			*out_p++ = (pixelvalue)acc ;
		}
	}
	return filt ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Filter an image with a flat rectangular kernel of given size.
  @param	in		Image to filter.
  @param	hx		Horizontal half-size of the kernel
  @param	hy		Vertical half-size of the kernel
  @return	1 newly allocated image.
 */
/*--------------------------------------------------------------------------*/
image_t * image_rectangle_filter_flat(image_t * in, int hx, int hy)
{
    image_t    *   filtered ;
    pixelvalue      curr_flux ;
    int             xmin, xmax, ymin, ymax ;
    int             i, j, k, l ;

    if (in==NULL || hx<1 || hy<1) return NULL ;
    filtered = image_new(in->lx, in->ly) ;

    /* Define the analysis zone */
    xmin = hx ;
    xmax = in->lx - hx - 1 ;
    ymin = hy ;
    ymax = in->ly - hy - 1 ;

    for (i=xmin ; i<xmax ; i++) {
        for (j=ymin ; j<ymax ; j++) {
            curr_flux = 0.0 ;
            for (k=i-hx ; k<i+hx+1 ; k++) {
                for (l=j-hy ; l<j+hy+1 ; l++) {
                    curr_flux += in->data[k+l*in->lx] ;
                }
            }
            filtered->data[i+j*in->lx] =
                (pixelvalue) (curr_flux / ((2*hx+1)*(2*hy+1)));
        }
    }
    return filtered ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Standard deviation filter
  @param	in		input image
  @param	hx		horizontal zone half size
  @param	hy		vertical zone half size
  @return	filtered image
  
  For each pixel, compute the standard deviation of a local zone.
  The image borders are set to 0.
 */
/*--------------------------------------------------------------------------*/
image_t * image_filter_stdev(image_t * in, int hx, int hy)
{
	image_t			*	filtered ;
	int					xmin, xmax, ymin, ymax ;
	register double		pix ;
	double				sum ;
	double				sq_sum ;
	double				inv_surf ;				
	int					i, j, k, l ;

	if (in == NULL) return NULL ;
	if (hx < 1 || hy < 1) return NULL ;

	/* Set the computed zone */
	xmin = hx ;
	xmax = in->lx - 1 - hx ;
	ymin = hy ;
	ymax = in->ly - 1 - hy ;
	if ((xmin >= xmax) || (ymin >= ymax)) {
		return NULL ;
	}

	/* Set inv_srf */
	inv_surf = 1.00 / (double)((2*hx+1)*(2*hy+1)) ;
	
	/* Filtering */
	filtered = image_new(in->lx, in->ly) ;
	for (j=ymin ; j<=ymax ; j++) {
		/* Beginning of the line, compute the initial sum */
		i = xmin ;
		sum = 0.00 ;
		sq_sum = 0.00 ;
		for (k=i-hx ; k<=i+hx ; k++) {
			for (l=j-hy ; l<=j+hy ; l++) {
				pix = (double)in->data[k+l*in->lx];
				sum += pix ;
				sq_sum += pix * pix ;
			}
		}
		filtered->data[i+j*in->lx] = sqrt((sq_sum*inv_surf) - 
				(sum*sum*inv_surf*inv_surf)) ;
	
		/* Compute the rest of the line using the initial sum */
		for (i=xmin+1 ; i<=xmax ; i++) {
			/* Remove the previous first column */
			k = i - hx - 1 ;
			for (l=j-hy ; l<=j+hy ; l++) {
				pix = (double)in->data[k+l*in->lx];
				sum -= pix ;
				sq_sum -= pix * pix ;
			}
			/* Add the new last column */
			k = i + hx ;
			for (l=j-hy ; l<=j+hy ; l++) {
				pix = (double)in->data[k+l*in->lx];
				sum += pix ;
				sq_sum += pix * pix ;
			}
			filtered->data[i+j*in->lx] = sqrt((sq_sum*inv_surf) - 
					(sum*sum*inv_surf*inv_surf)) ;
		}
	}
	return filtered ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief	50 Herz correction filter
  @param	in		input image
  @return   0 if ok, -1 otherwise	
  The input image is modified
 */
/*--------------------------------------------------------------------------*/
#define FIFTY_HZ_HW_SMOOTH    20
#define FIFTY_HZ_REJECTED_LOW_PIXELS    0
#define FIFTY_HZ_REJECTED_HIGH_PIXELS 900
#define FIFTY_HZ_THRESHOLD 5.0
int image_remove_fiftyhertz(image_t * in)
{
    image_t     *   collapsed ;
    image_t     *   highfreq ;
    pixelvalue  *   lowfreq ;
    int             j ;

    /* Collapse the current image */
    collapsed = image_collapse_median(in, 1, FIFTY_HZ_REJECTED_LOW_PIXELS,
            FIFTY_HZ_REJECTED_HIGH_PIXELS);

    /* Extract the low frequency signal */
    lowfreq = function1d_median_smooth(collapsed->data, collapsed->ly,
            FIFTY_HZ_HW_SMOOTH) ;

    /* Subtract the lowfreq to the collapsed image */
    /* Set to 0 everything above 5 or under -5 in high frequency sig */
    highfreq = image_copy(collapsed) ;
    image_del(collapsed) ;
    for (j=0 ; j<highfreq->ly ; j++) {
        highfreq->data[j] -= lowfreq[j] ;
        if (fabs(highfreq->data[j]) > FIFTY_HZ_THRESHOLD) 
            highfreq->data[j] = 0.0 ;
    }
    free(lowfreq) ; 

    /* Correct the input cube */
    image_sub_1d_local(in, highfreq) ;
    image_del(highfreq) ;

    return 0 ;
}
#undef FIFTY_HZ_HW_SMOOTH
#undef FIFTY_HZ_REJECTED_LOW_PIXELS 
#undef FIFTY_HZ_REJECTED_HIGH_PIXELS
#undef FIFTY_HZ_THRESHOLD

/*----------------------------------------------------------------------------*/
/**
  @brief    Remove the odd-even effect for each quadrant separately.
  @param    im  Image to process
  @return   1 newly allocated image.

  This function tries to remove the odd-even effect from an image, applying 
  image_de_oddeven() to each quadrant in sequence. The input image size must 
  be a power of 2 and the image must be square.
 */
/*----------------------------------------------------------------------------*/
image_t * image_de_oddeven_byquad(image_t * im)
{
    image_t *   quad ;
    image_t *   f_quad ;
    image_t *   cleaned ;
    image_t *   cleaned_tmp ;

    cleaned = image_new(im->lx, im->ly);

    compute_status("filtering odd-even effect", 0, 4, 1);
    quad = image_getvig(im, 1, 1, im->lx/2, im->ly/2);
    f_quad = image_de_oddeven(quad);
    image_del(quad);
    cleaned_tmp = image_paste_vig(cleaned, f_quad, 1, 1, 1, 1, f_quad->lx, 
            f_quad->ly);
    image_del(f_quad);
    image_del(cleaned) ;
    cleaned = cleaned_tmp ;

    compute_status("filtering odd-even effect", 1, 4, 1);
    quad = image_getvig(im, 1+im->lx/2, 1, im->lx, im->ly/2);
    f_quad = image_de_oddeven(quad);
    image_del(quad);
    cleaned_tmp = image_paste_vig(cleaned, f_quad, 1+im->lx/2, 1, 1, 1, 
            f_quad->lx, f_quad->ly);
    image_del(f_quad);
    image_del(cleaned);
    cleaned = cleaned_tmp ;

    compute_status("filtering odd-even effect", 2, 4, 1);
    quad = image_getvig(im, 1, 1+im->ly/2, im->lx/2, im->ly);
    f_quad = image_de_oddeven(quad);
    image_del(quad);
    cleaned_tmp = image_paste_vig(cleaned, f_quad, 1, 1+im->ly/2, 1, 1, 
            f_quad->lx, f_quad->ly);
    image_del(f_quad);
    image_del(cleaned);
    cleaned = cleaned_tmp ;

    compute_status("filtering odd-even effect", 3, 4, 1);
    quad = image_getvig(im, 1+im->lx/2, 1+im->ly/2, im->lx, im->ly);
    f_quad = image_de_oddeven(quad);
    image_del(quad);
    cleaned_tmp = image_paste_vig(cleaned, f_quad, 1+im->lx/2, 1+im->ly/2, 1, 
            1, f_quad->lx, f_quad->ly);
    image_del(f_quad);
    image_del(cleaned);
    cleaned = cleaned_tmp ;

    return cleaned ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief    Remove the odd-even effect inside an image.
  @param    im  Image to process
  @return   1 newly allocated image.

  This function tries to remove the odd-even effect. The input image size 
  must be a power of 2 and the image must be square.
 */
/*----------------------------------------------------------------------------*/
image_t * image_de_oddeven(image_t * im)
{
    image_t *   cleaned ;
    cube_t  *   freq_i ;
    cube_t  *   freq_o ;
    cube_t  *   freq_i_amp ;
    int         lx, ly ;

    if (im==NULL) return NULL ;

    lx = im->lx ;
    ly = im->ly ;

    /* Apply FFT to input image */
    freq_i = image_fft(im, NULL, FFT_FORWARD);
    /* Convert to amplitude/phase */
    freq_i_amp = cube_conv_xy_rtheta(freq_i);
    cube_del(freq_i);
    /* Nullify the odd-even frequencies */
    freq_i_amp->plane[0]->data[lx/2] = 0 ;
    freq_i_amp->plane[0]->data[lx/2 + (ly-1)*lx] = 0 ;
    /* Convert to X/Y */
    freq_i = cube_conv_rtheta_xy(freq_i_amp);
    cube_del(freq_i_amp);
    /* FFT back to image space */
    freq_o = image_fft(freq_i->plane[0], freq_i->plane[1], FFT_INVERSE);
    cube_del(freq_i);
    cleaned = freq_o->plane[0] ;
    freq_o->plane[0] = NULL ;
    cube_del(freq_o);
    return cleaned ;
}

