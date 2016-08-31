
/*-------------------------------------------------------------------------*/
/**
  @file		dstats.c
  @author	N. Devillard
  @date		Mar 2001
  @version	$Revision: 1.7 $
  @brief	Statistics on arrays of doubles

  This module complements the C library by offering various statistical
  operations on arrays of doubles.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: dstats.c,v 1.7 2003/11/28 09:33:11 llundin Exp $
	$Author: llundin $
	$Date: 2003/11/28 09:33:11 $
	$Revision: 1.7 $
*/

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dstats.h"

/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*/
/**
  @brief	Find the median of an array of doubles.
  @param	a	Array to consider for median search.
  @param	n	Number of elements in the array.
  @return	1 double (an element of the array).

  This function computes the median of an array of doubles.
  NB: THE INPUT ARRAY IS MODIFIED.
 */
/*--------------------------------------------------------------------------*/

double
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
double_median(double * a, int n)
{
	return ((n)&1) ? double_kth_smallest(a, (n), ((n)-1)/2) :
					 double_kth_smallest(a, (n), (n)/2) ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief    Find the kth smallest element in a double array.
  @param    a   Array to consider.
  @param    n   Number of elements in the array.
  @param    k   Rank of the element to find (between 0 and n-1).
  @return   One element from the array.

  Get the kth smallest element in the input array.
  NB: THE INPUT ARRAY IS MODIFIED.
 */
/*--------------------------------------------------------------------------*/


#define DBL_SWAP(a,b) { register double t=(a);(a)=(b);(b)=t; }
double
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
double_kth_smallest(double * a, int n, int k)
{
    register int i,j,l,m ;
    register double x ;

    l=0 ; m=n-1 ;
    while (l<m) {
        x=a[k] ;
        i=l ;
        j=m ;
        do {
            while (a[i]<x) i++ ;
            while (x<a[j]) j-- ;
            if (i<=j) {
                DBL_SWAP(a[i],a[j]) ;
                i++ ; j-- ;
            }
        } while (i<=j) ;
        if (j<k) l=i ;
        if (k<i) m=j ;
    }
    return a[k] ;
}
#undef DBL_SWAP



/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the rms on an array of doubles.
  @param	a	Array of doubles to consider.
  @param	n	Number of doubles in the array.
  @return	1 double

  This function computes the RMS of the given array.
 */
/*--------------------------------------------------------------------------*/

double
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
double_rms(double * a, int n)
{
	int		i ;
	double	sum, sqsum ;
    double var ;

	if (a==NULL || n<1) return 0 ;
	sum=0.0 ;
	sqsum=0.0 ;
	for (i=0 ; i<n ; i++) {
		sum += a[i] ;
		sqsum += a[i] * a[i] ;
	}

    /* Rounding errors can cause the variance to be negative */
    var = (sqsum-((sum*sum)/(double)n))/((double)n-1.0);
    return var > 0 ? sqrt(var) : 0;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the average on an array of doubles.
  @param	a 	Array of doubles to consider.
  @param	n	Number of doubles in the array.
  @return	1 double

  This function computes the average of the given array.
 */
/*--------------------------------------------------------------------------*/

double
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
double_avg(double * a, int n)
{
	int		i ;
	double	sum ;

	if (a==NULL || n<1) return 0 ;
	sum=0.0;
	for (i=0 ; i<n ; i++) {
		sum += a[i];
	}
	return sum / (double)n ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Sort an array of doubles by increasing value.
  @param	arr	Array of doubles to consider
  @param	n	Number of doubles in the array.
  @return	void

  Fast sort on an array of doubles.
 */
/*--------------------------------------------------------------------------*/

void
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
double_qsort(double * arr, int n)
{
#define DBL_SWAP(a,b) { double temp=(a);(a)=(b);(b)=temp; }
#define DBL_STACK_SIZE 50
    int         i,
                ir,
                j,
                k,
                l;
    int *       i_stack ;
    int         j_stack ;
    double		a ;

    ir = n ;
    l = 1 ;
    j_stack = 0 ;
    i_stack = malloc(DBL_STACK_SIZE * sizeof(double)) ;
    for (;;) {
        if (ir-l < 7) {
            for (j=l+1 ; j<=ir ; j++) {
                a = arr[j-1];
                for (i=j-1 ; i>=1 ; i--) {
                    if (arr[i-1] <= a) break;
                    arr[i] = arr[i-1];
                }
                arr[i] = a;
            }
            if (j_stack == 0) break;
            ir = i_stack[j_stack-- -1];
            l  = i_stack[j_stack-- -1];
        } else {
            k = (l+ir) >> 1;
            DBL_SWAP(arr[k-1], arr[l])
            if (arr[l] > arr[ir-1]) {
                DBL_SWAP(arr[l], arr[ir-1])
            }
            if (arr[l-1] > arr[ir-1]) {
                DBL_SWAP(arr[l-1], arr[ir-1])
            }
            if (arr[l] > arr[l-1]) {
                DBL_SWAP(arr[l], arr[l-1])
            }
            i = l+1;
            j = ir;
            a = arr[l-1];
            for (;;) {
                do i++; while (arr[i-1] < a);
                do j--; while (arr[j-1] > a);
                if (j < i) break;
                DBL_SWAP(arr[i-1], arr[j-1]);
            }
            arr[l-1] = arr[j-1];
            arr[j-1] = a;
            j_stack += 2;
            if (j_stack > DBL_STACK_SIZE) {
                fprintf(stderr, "stack too small in pixel_qsort: aborting");
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
    free(i_stack) ;
#undef DBL_STACK_SIZE
#undef DBL_SWAP
}



