/*----------------------------------------------------------------------------*/
/**
   @file	pixel_handling.c
   @author	Nicolas Devillard
   @date	March 04, 1997
   @version	$Revision: 1.9 $
   @brief	Functions processing arrays of pixels
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: pixel_handling.c,v 1.9 2003/04/22 07:07:53 yjung Exp $
	$Author: yjung $
	$Date: 2003/04/22 07:07:53 $
	$Revision: 1.9 $
*/

/*-----------------------------------------------------------------------------
  								Includes
 -----------------------------------------------------------------------------*/

#include "static_sz.h"
#include "pixel_handling.h"
#include "comm.h"

/*-----------------------------------------------------------------------------
								Function codes
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
  @brief	Compare two pixelvalues.
  @param	pix1	First pixel value.
  @param	pix2	Second pixel value.
  @return	1 if (pix1>pix2), -1 otherwise.

  This function is meant to be used with qsort().
 */
/*----------------------------------------------------------------------------*/
int
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
pixel_compare(
		const void * pix1,
		const void * pix2)
{
	return (*(pixelvalue*)pix1 > *(pixelvalue*)pix2) ? 1 : -1 ; 
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Sort an array of pixels by increasing pixelvalue.
  @param	pix_arr		Array to sort.
  @param	npix		Number of pixels in the array.
  @return	void
  Optimized implementation of a fast pixel sort. The input array is modified.
 */
/*----------------------------------------------------------------------------*/
#define PIX_SWAP(a,b) { pixelvalue temp=(a);(a)=(b);(b)=temp; }
#define PIX_STACK_SIZE 50
void
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
pixel_qsort(
		pixelvalue	*	pix_arr, 
		int 			npix)
{
    int         i,
                ir,
                j,
                k,
                l;
    int         i_stack[PIX_STACK_SIZE] ;
    int         j_stack ;
    pixelvalue  a ;

    ir = npix ;
    l = 1 ;
    j_stack = 0 ;
    for (;;) {
        if (ir-l < 7) {
            for (j=l+1 ; j<=ir ; j++) {
                a = pix_arr[j-1];
                for (i=j-1 ; i>=1 ; i--) {
                    if (pix_arr[i-1] <= a) break;
                    pix_arr[i] = pix_arr[i-1];
                }
                pix_arr[i] = a;
            }
            if (j_stack == 0) break;
            ir = i_stack[j_stack-- -1];
            l  = i_stack[j_stack-- -1];
        } else {
            k = (l+ir) >> 1;
            PIX_SWAP(pix_arr[k-1], pix_arr[l])
            if (pix_arr[l] > pix_arr[ir-1]) {
                PIX_SWAP(pix_arr[l], pix_arr[ir-1])
            }
            if (pix_arr[l-1] > pix_arr[ir-1]) {
                PIX_SWAP(pix_arr[l-1], pix_arr[ir-1])
            }
            if (pix_arr[l] > pix_arr[l-1]) {
                PIX_SWAP(pix_arr[l], pix_arr[l-1])
            }
            i = l+1;
            j = ir;
            a = pix_arr[l-1];
            for (;;) {
                do i++; while (pix_arr[i-1] < a);
                do j--; while (pix_arr[j-1] > a);
                if (j < i) break;
                PIX_SWAP(pix_arr[i-1], pix_arr[j-1]);
            }
            pix_arr[l-1] = pix_arr[j-1];
            pix_arr[j-1] = a;
            j_stack += 2;
            if (j_stack > PIX_STACK_SIZE) {
                e_error("stack too small in pixel_qsort: aborting");
                exit(-2001) ;
            }
            if (ir-i+1 >= j-l) {
                i_stack[j_stack-1] = ir;
                i_stack[j_stack-2] = i;
                ir = j-1;
            } else {
                i_stack[j_stack-1] = j-1;
                i_stack[j_stack-2] = l;
                l = i;
            }
        }
    }
}
#undef PIX_STACK_SIZE
#undef PIX_SWAP

/*----------------------------------------------------------------------------*/
/**
  @brief	Convert an array from pixelvalue to double.
  @param	arr		Input pixelvalue array.
  @param	n		Number of values in the array.
  @return	1 newly allocated array of doubles.

  Convert an array of pixelvalues to a newly allocated array of doubles.
  The returned array must be deallocated using free().
 */
/*----------------------------------------------------------------------------*/
double * pixel2double_array(
		pixelvalue	*	arr, 
		int 			n)
{
	double	*	d ;
	int			i ;

	if ((arr==NULL) || (n<1)) return NULL ;
	d = malloc(n * sizeof(double));
	for (i=0 ; i<n ; i++) {
		d[i] = (double)arr[i];
	}
	return d ;
}

/*----------------------------------------------------------------------------*/
/**
  @brief	Convert an array from double to pixelvalue.
  @param	arr		Input double array.
  @param	n		Number of values in the array.
  @return	1 newly allocated array of pixelvalues.

  Convert an array of doubles to a newly allocated array of pixelvalues.
  The returned array must be deallocated using free().
 */
/*----------------------------------------------------------------------------*/
pixelvalue * double2pixel_array(
		double	*	arr, 
		int 		n)
{
	pixelvalue	*	d ;
	int				i ;

	if ((arr==NULL) || (n<1)) return NULL ;
	d = malloc(n * sizeof(pixelvalue));
	for (i=0 ; i<n ; i++) {
		d[i] = (pixelvalue)arr[i];
	}
	return d ;
}

