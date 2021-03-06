/*----------------------------------------------------------------------------*/
/**
   @file    ieeefp-compat.c
   @author  N. Devillard
   @date    Feb 2002
   @version	$Revision: 2.7 $
   @brief   This module implements the isnan() and isinf() macros.
  
   The isnan() and isinf() macros are unfortunately not yet part of
   the standard C math library everywhere. They can usually be found
   in different places, if they are offered at all, and require the
   application to link against the math library. To avoid portability
   problems and linking against -lm, this module implements a fast
   and portable way of finding out whether a floating-point value
   (float or double) is a NaN or an Inf.

   Instead of calling isnan() and isinf(), the programmer including
   this file should call qfits_isnan() and qfits_isinf().
*/
/*----------------------------------------------------------------------------*/

/*
	$Id: ieeefp-compat.c,v 2.7 2003/01/23 09:27:55 yjung Exp $
	$Author: yjung $
	$Date: 2003/01/23 09:27:55 $
	$Revision: 2.7 $
*/

/*-----------------------------------------------------------------------------
   								Includes
 -----------------------------------------------------------------------------*/

#include "config.h"

/*-----------------------------------------------------------------------------
   								New types
 -----------------------------------------------------------------------------*/

#ifndef WORDS_BIGENDIAN
/* Little endian ordering */
typedef union _ieee_double_pattern_ {
    double d ;
    struct {
        unsigned int lsw ;
        unsigned int msw ;
    } p ;
} ieee_double_pattern ;
#else
/* Big endian ordering */
typedef union _ieee_double_pattern_ {
    double d ;
    struct {
        unsigned int msw ;
        unsigned int lsw ;
    } p ;
} ieee_double_pattern ;
#endif

typedef union _ieee_float_pattern_ {
    float f ;
    int   i ;
} ieee_float_pattern ;

/*-----------------------------------------------------------------------------
  							Function codes
 -----------------------------------------------------------------------------*/

int _qfits_isnanf(float f)
{
    ieee_float_pattern ip ;
    int ix ;

    ip.f = f ;
    ix = ip.i ;
    ix &= 0x7fffffff ;
    ix = 0x7f800000 - ix ;
    return (int)(((unsigned int)(ix))>>31);
}

int _qfits_isinff(float f)
{
    ieee_float_pattern ip ;
    int ix, t ;

    ip.f = f ;
    ix = ip.i ;
    t = ix & 0x7fffffff;
    t ^= 0x7f800000;
    t |= -t;
    return ~(t >> 31) & (ix >> 30);
}

int _qfits_isnand(double d)
{
    ieee_double_pattern id ;
    int hx, lx ;

    id.d = d ;
    lx = id.p.lsw ;
    hx = id.p.msw ;

    hx &= 0x7fffffff;
    hx |= (unsigned int)(lx|(-lx))>>31;
    hx = 0x7ff00000 - hx;
    return (int)(((unsigned int)hx)>>31);
}

int _qfits_isinfd(double d)
{
    ieee_double_pattern id ;
    int hx, lx ;

    id.d = d ;
    lx = id.p.lsw ;
    hx = id.p.msw ;

    lx |= (hx & 0x7fffffff) ^ 0x7ff00000;
    lx |= -lx;
    return ~(lx >> 31) & (hx >> 30);
}


/*
 * Test program to validate the above functions
 * Compile with:
 * % cc -o ieeefp-compat ieeefp-compat.c -DTEST
 */

#ifdef TEST
#include <stdio.h>
#include <string.h>
#include "ieeefp-compat.h"

#ifndef WORDS_BIGENDIAN
/* Little endian patterns */
static unsigned char fnan_pat[] = {0, 0, 0xc0, 0x7f};
static unsigned char dnan_pat[] = {0, 0, 0, 0, 0, 0, 0xf8, 0x7f};
static unsigned char finf_pat[] = {0, 0, 0x80, 0x7f};
static unsigned char dinf_pat[] = {0, 0, 0, 0, 0, 0, 0xf0, 0x7f};
static unsigned char fminf_pat[] = {0, 0, 0x80, 0xff};
/* static unsigned char dminf_pat[] = {0, 0, 0, 0, 0, 0, 0xf0, 0xff}; */
static unsigned char dminf_pat[] = {0, 0, 0, 0, 0, 0, 0xf0, 0x7f};
#else
/* Big endian patterns */
static unsigned char fnan_pat[] = {0x7f, 0xc0, 0, 0};
static unsigned char dnan_pat[] = {0x7f, 0xf8, 0, 0, 0, 0, 0, 0};
static unsigned char finf_pat[] = {0x7f, 0x80, 0, 0};
static unsigned char dinf_pat[] = {0x7f, 0xf0, 0, 0, 0, 0, 0, 0};
static unsigned char fminf_pat[] = {0xff, 0x80, 0, 0};
static unsigned char dminf_pat[] = {0xff, 0xf0, 0, 0, 0, 0, 0, 0};
#endif

static void hexdump(void * p, int s)
{
	unsigned char * c ;
	int i ;

	c=(unsigned char*)p ;
#ifndef WORDS_BIGENDIAN
	for (i=s-1 ; i>=0 ; i--) {
#else
	for (i=0 ; i<s ; i++) {
#endif
		printf("%02x", c[i]);
	}
	printf("\n");
}


int main(void)
{
	float	f ;
	double	d ;

	printf("Testing Nan...\n");
	memcpy(&f, fnan_pat, 4);
	memcpy(&d, dnan_pat, 8);
	printf("f=%g d=%g\n", f, d);
	hexdump(&f, sizeof(float));
	hexdump(&d, sizeof(double));

    if (qfits_isnan(f)) {
        printf("f is NaN\n");
    }
    if (qfits_isnan(d)) {
        printf("d is NaN\n");
    }

	printf("Testing +Inf...\n");
    memcpy(&f, finf_pat, 4);
    memcpy(&d, dinf_pat, 8);
	printf("f=%g d=%g\n", f, d);
	hexdump(&f, sizeof(float));
	hexdump(&d, sizeof(double));

    if (qfits_isinf(f)) {
        printf("f is Inf\n");
    }
    if (qfits_isinf(d)) {
        printf("d is Inf\n");
    }

	printf("Testing -Inf...\n");
    memcpy(&f, fminf_pat, 4);
    memcpy(&d, dminf_pat, 8);
	printf("f=%g d=%g\n", f, d);
	hexdump(&f, sizeof(float));
	hexdump(&d, sizeof(double));

    if (qfits_isinf(f)) {
        printf("f is (-)Inf\n");
    }
    if (qfits_isinf(d)) {
        printf("d is (-)Inf\n");
    }

	return 0 ;
}
#endif
/* vim: set ts=4 et sw=4 tw=75 */
