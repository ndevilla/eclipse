/*-------------------------------------------------------------------------*/
/**
   @file	ipow.c
   @author	N. Devillard
   @date	June 1999
   @version	$Revision: 1.4 $
   @brief	integer powers

   This function is so generic and used everywhere, it diserves its
   own source file.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: ipow.c,v 1.4 2001/11/07 13:46:15 yjung Exp $
	$Author: yjung $
	$Date: 2001/11/07 13:46:15 $
	$Revision: 1.4 $
*/

/*---------------------------------------------------------------------------
                            Function code
 ---------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/**
  @brief	Same as pow(x,y) but for integer values of y only (faster).
  @param	x	A double number.
  @param	p	An integer power.
  @return	x to the power p.

  This is much faster than the math function due to the integer. Some
  compilers make this optimization already, some do not.

  p can be positive, negative or null.
 */
/*--------------------------------------------------------------------------*/
double 
#ifdef __GNUC__
__attribute__((__no_instrument_function__))
#endif
ipow(double x, int p)
{
	double r, recip ;

	/* Get rid of trivial cases */
	switch (p) {
		case 0:
		return 1.00 ;

		case 1:
		return x ;

		case 2:
		return x*x ;

		case 3:
		return x*x*x ;

		case -1:
		return 1.00 / x ;

		case -2:
		return (1.00 / x) * (1.00 / x) ;
	}
	if (p>0) {
		r = x ;
		while (--p) r *= x ;
	} else {
		r = recip = 1.00 / x ;
		while (++p) r *= recip ;
	}
	return r;
}


