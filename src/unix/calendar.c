
/*-------------------------------------------------------------------------*/
/**
   @file	calendar.c
   @author	N. Devillard
   @date	Dec 2000
   @version	$Revision: 1.4 $
   @brief	Calendar routines
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: calendar.c,v 1.4 2002/01/15 10:05:06 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2002/01/15 10:05:06 $
	$Revision: 1.4 $
*/



/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

/** Number of days in each month */
static int day_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; 

static int is_leap_year(int year)
{ return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0); }


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the date of yesterday for a given date.
  @param	day		Day
  @param	month	Month
  @param	year	Year
  @return	void

  Computes the date for "yesterday", for the given date. The given
  parameters are modified. Year is expected with 4 digits.
  Example (dates are given as DD/MM/YYYY):

  - yesterday for 01.01.2000 is 31.12.1999
  - yesterday for 01.03.2000 is 29.02.1999

  This function handles leap years (yes, 2000 was a leap year).
 */
/*--------------------------------------------------------------------------*/

void calendar_getprev(int * day, int * month, int * year)
{
    /* Simple case: day is not the first of the month */
    if ((*day)>1) {
        (*day)-- ;
        return ;
    }
    /*
     * Day is first of the month.
     */
    if ((*month)==1) {
        /* Date is 1st of January, yesterday is 31 Dec of year before */
        (*day) = 31 ;
        (*month) = 12 ;
        (*year)-- ;
        return ;
    }
 
    /* Month is not january */
    (*month)-- ;
 
    /* Day is last day of the month before */
    (*day) = day_month[(*month)-1];
    /* Handle leap years */
    if ((*month)==2 && is_leap_year((*year))) {
        (*day)++;
    }
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Compute the date of tomorrow for a given date.
  @param	day		Day
  @param	month	Month
  @param	year	Year
  @return	void

  Computes the date for "tomorrow", for the given date. The given
  parameters are modified. Year is expected with 4 digits.
  Example (dates are given as DD/MM/YYYY):

  - tomorrow for 31.12.1999 is 01.01.2000
  - tomorrow for 28.02.2000 is 29.02.2000

  This function handles leap years (yes, 2000 was a leap year).
 */
/*--------------------------------------------------------------------------*/

void calendar_getnext(int * day, int * month, int * year)
{
	int		dtm ;

	/* Compute how many days this month */
	dtm = day_month[(*month)-1] ;
	/* Handle leap years */
	if ((*month)==2 && is_leap_year(*year)) {
		dtm ++ ;
	}
	/* Simple case: day is below dtm */
	if ((*day)<dtm) {
		(*day)++ ;
		return ;
	}
	/* Month change */
	(*day)=1 ;
	/* See if month is december */
	if ((*month)==12) {
		(*month)=1 ;
		(*year)++;
	} else {
		(*month)++;
	}
	return ;
}

/* vim: set ts=4 et sw=4 tw=75 */
