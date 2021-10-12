static char sccsid[] = "@(#)05	1.15  src/bos/kernel/proc/gettod.c, sysproc, bos411, 9428A410j 4/1/93 13:40:35";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: SECSTOYEAR
 *		date_to_secs
 *		secs_to_date
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/rtc.h>

#define  SECPERMINUTE    (60)
#define  SECPERHOUR      (SECPERMINUTE*60)
#define  SECPERDAY       (SECPERHOUR*24)
#define  SECPERYEAR      (SECPERDAY*365)
#define  NO_MONTHS       (12)
#define  DAYSPERYEAR     (365)
#define  MINPERHOUR      (60)
#define  HOURSPERDAY     (24)
#define  SECSTOYEAR(y)   (((y)*SECPERYEAR)+((((y)+1)/4)*SECPERDAY))

/* The following are used as array indices */
#define  JAN_INDEX   0
#define  FEB_INDEX   (JAN_INDEX + 1)

/* An array of the number of days per month. */
static char dpm[NO_MONTHS] = {
	31,  /* JAN */        /* Note: When used, a copy is */
	28,  /* FEB */        /*   made so that FEB can be  */
	31,  /* MAR */        /*   changed to 29 without    */
	30,  /* APR */        /*   affecting re-entrancy.   */
	31,  /* MAY */
	30,  /* JUN */
	31,  /* JUL */
	31,  /* AUG */
	30,  /* SEP */
	31,  /* OCT */
	30,  /* NOV */
	31   /* DEC */
};


/*
 * NAME:  date_to_secs
 *
 * FUNCTION: Convert MM DD YY to seconds since the Epoch.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called under a process at system initialization to
 *	restore the system time from the time-of-day chip.
 *
 * NOTES:  POSIX 1003.1 defines 'seconds since the Epoch' as "a value to be
 *	interpreted as the number of seconds between a specified time and
 *	the Epoch".  1003.1 further specifies that "a Coordinated Universal
 *	Time name . . . is related to a time represented as seconds since
 *	the Epoch according to the expression:
 *		tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *		(tm_year-70)*31536000 + ((tm_year-69)/4)*86400"
 */
date_to_secs(struct tms *timestruct)
{
	register int	year;
	register int	day;

	year = (((int)timestruct->yrs / 16 * 10) + ((int)timestruct->yrs % 16));
	day = ((int)timestruct->jul_100 * 100) + 
		((int)timestruct->jul_dig / 16 * 10) +
		((int)timestruct->jul_dig % 16);
	/*  Correct for julian day starting at 1 rather than 0.  */
	day--;
	timestruct->secs =	
		((int)timestruct->no_secs / 16 * 10) + 
		((int)timestruct->no_secs % 16) + 
		(((int)timestruct->mins / 16 * 10) * 60) +
		(((int)timestruct->mins % 16) * 60) +
		(((int)timestruct->hrs / 16 * 10) * 3600) +
		(((int)timestruct->hrs % 16) * 3600) +
		(day * 86400) +
		(year * 31536000) +
		(((year + 1)/4) * 86400);
	timestruct->ms = (int)(timestruct->ms / 16 * 10) +
		(int)(timestruct->ms % 16);
}

/*
 * NAME:  secs_to_date
 *
 * FUNCTION: Convert seconds since the Epoch to MM DD YY.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called under a process from settimer() to save the
 *	system time to the time-of-day chip.
 *
 * NOTES:  POSIX 1003.1 defines 'seconds since the Epoch' as "a value to be
 *	interpreted as the number of seconds between a specified time and
 *	the Epoch".  1003.1 further specifies that "a Coordinated Universal
 *	Time name . . . is related to a time represented as seconds since
 *	the Epoch according to the expression:
 *		tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *		(tm_year-70)*31536000 + ((tm_year-69)/4)*86400"
 */
secs_to_date(register struct tms *timestruct)
{
	register unsigned int    i, tempsecs;
	char     tempdpm[NO_MONTHS];

	/* Make a copy of the days per month structure for re-entrancy.*/
	for (i=0; i < NO_MONTHS; i++)  {
		tempdpm[i] = dpm[i];
	}

	/*  Make a copy of the number of seconds since the Epoch.  */
	tempsecs = timestruct->secs;

	/*  Initialize all the values.  */
	timestruct->yrs = 0;
	timestruct->mths = timestruct->dom = 0;
	timestruct->hrs = timestruct->mins = timestruct->no_secs = 0;
	timestruct->jul_100 = timestruct->jul_dig = 0;

	/*
	 * Calculate the current year.  We guess at the year by assuming
	 * there are no leapyears.  If the number of seconds in all the
	 * years up to this point is more than the number of seconds I
	 * am converting, I must have guessed wrong, so I take off the
	 * year I shouldn't have added in.
	 */

	timestruct->yrs = tempsecs / SECPERYEAR;
	if ((i = SECSTOYEAR(timestruct->yrs)) > tempsecs) {
		timestruct->yrs--;
		i = SECSTOYEAR(timestruct->yrs);
	}
	tempsecs -= i;

	/*  
	 * Correct days per month for February if this is a leap year.  
	 * Add 2 for base of 68.  A leap year. 
	 */
	if(!((timestruct->yrs+2) % 4))  {
		tempdpm[FEB_INDEX] = 29;
	}

	/*  Calculate the current month.  */
	while(tempsecs >= (tempdpm[timestruct->mths] * SECPERDAY))  {
		tempsecs -= (tempdpm[timestruct->mths] * SECPERDAY);
		if(((int)timestruct->jul_dig + tempdpm[timestruct->mths]) > 99){
			timestruct->jul_100++;
			timestruct->jul_dig = (int)timestruct->jul_dig +
					      tempdpm[timestruct->mths] - 100;
		}
		else  {
			timestruct->jul_dig += tempdpm[timestruct->mths];
		}
		timestruct->mths++;
	}
	timestruct->mths++;

	/*  Calculate the current day.  */
	timestruct->jul_dig += timestruct->dom = tempsecs / SECPERDAY;
	tempsecs %= SECPERDAY;
	timestruct->dom++;
	timestruct->jul_dig++;
	if(timestruct->jul_dig > 99)  {
		timestruct->jul_100++;
		timestruct->jul_dig -= 100;
	}

	/*  Calculate the current hour.  */
	timestruct->hrs = tempsecs / SECPERHOUR;
	tempsecs %= SECPERHOUR;

	/*  Calculate the current minute.  */
	timestruct->mins = tempsecs / SECPERMINUTE;
	tempsecs %= SECPERMINUTE;

	/*  The remainder is the current seconds.  */
	timestruct->no_secs = tempsecs;

	/*  Convert to Binary Coded Decimal.  */
	timestruct->ms = (int)timestruct->ms / 10 * 16 +
				((int)timestruct->ms % 10);
	timestruct->mins = (int)timestruct->mins / 10 * 16 +
				((int)timestruct->mins % 10);
	timestruct->hrs = (int)timestruct->hrs / 10 * 16 +
				((int)timestruct->hrs % 10);
	timestruct->dom = (int)timestruct->dom / 10 * 16 +
				((int)timestruct->dom % 10);
	timestruct->mths = (int)timestruct->mths / 10 * 16 +
				((int)timestruct->mths % 10);
	timestruct->yrs = (int)timestruct->yrs / 10 * 16 +
				((int)timestruct->yrs % 10);
	timestruct->no_secs = (int)timestruct->no_secs / 10 * 16 + 
				((int)timestruct->no_secs % 10);
	timestruct->jul_dig = (int)timestruct->jul_dig / 10 * 16 + 
				((int)timestruct->jul_dig % 10);
}
