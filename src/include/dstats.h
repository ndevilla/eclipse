
/*-------------------------------------------------------------------------*/
/**
  @file     dstats.h
  @author   N. Devillard
  @date     Mar 2001
  @version  $Revision: 1.3 $
  @brief    Statistics on arrays of doubles

  This module complements the C library by offering various statistical
  operations on arrays of doubles.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: dstats.h,v 1.3 2001/10/23 12:20:47 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2001/10/23 12:20:47 $
	$Revision: 1.3 $
*/

#ifndef _DSTATS_H_
#define _DSTATS_H_


/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief    Find the median of an array of doubles.
  @param    a   Array to consider for median search.
  @param    n   Number of elements in the array.
  @return   1 double (an element of the array).

  This function computes the median of an array of doubles.
  NB: THE INPUT ARRAY IS MODIFIED.
 */
/*--------------------------------------------------------------------------*/
double double_median(double * a, int n);

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
double double_kth_smallest(double * a, int n, int k);

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the rms on an array of doubles.
  @param    a   Array of doubles to consider.
  @param    n   Number of doubles in the array.
  @return   1 double

  This function computes the RMS of the given array.
 */
/*--------------------------------------------------------------------------*/
double double_rms(double * a, int n);

/*-------------------------------------------------------------------------*/
/**
  @brief    Compute the average on an array of doubles.
  @param    a   Array of doubles to consider.
  @param    n   Number of doubles in the array.
  @return   1 double

  This function computes the average of the given array.
 */
/*--------------------------------------------------------------------------*/
double double_avg(double * a, int n);

/*-------------------------------------------------------------------------*/
/**
  @brief    Sort an array of doubles by increasing value.
  @param    arr Array of doubles to consider
  @param    n   Number of doubles in the array.
  @return   void

  Fast sort on an array of doubles.
 */
/*--------------------------------------------------------------------------*/
void double_qsort(double * arr, int n);


#endif
