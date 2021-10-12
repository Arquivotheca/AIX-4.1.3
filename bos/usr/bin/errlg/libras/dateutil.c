static char sccsid[] = "@(#)84	1.4  src/bos/usr/bin/errlg/libras/dateutil.c, cmderrlg, bos411, 9428A410j 5/19/94 10:04:41";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: datetosecs, datetosecs2
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:      datetosecs
 * FUNCTION:  convert a mmddhhmmyy date string to the time() equivalent
 * INPUT:     'datestr'  mmddhhmmyy string
 * RETURNS:   time() equivalent if successful
 *            0 if 'datestr' is bad
 */

/*
 * NAME:      datetosecs2
 * FUNCTION:  convert a mmddhhmmssyy date string to the time() equivalent
 * INPUT:     'datestr'  mmddhhmmssyy string
 * RETURNS:   time() equivalent if successful
 *            0 if 'datestr' is bad
 */
#define _ILS_MACROS
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <libras.h>

extern	mktime();


#define RANGECHK(n,l,u) \
	if(rangechk(n,l,u,"n") < 0) \
		return(-1);

static daysmonth[] = {
	31,28,31,30,
	31,30,31,31,
	30,31,30,30
};

/*
 * convert datestr mmddhhmmyy to # of seconds since 1970, as in time()
 */
datetosecs(datestr)
char *datestr;
{
	int		c;
	int		rv;
	int		month;
	int		day;
	int		year;
	int		hour;
	int		minute;
	time_t	jtime;
	char	*cp;
	struct	tm	dtg;

	/* datestring must be 10 digits long */
	if(strlen(datestr) != 10) return(-1);

	cp = datestr;
	while(c = *cp++) {
		/* Bad character in datestring */
		if(!isdigit(c)) return(-1);
	}

	/* Datestring must be of the format mmddmmhhyy */
	rv = sscanf(datestr,"%2d%2d%2d%2d%2d",&month,&day,&hour,&minute,&year);
	if(rv != 5) return(-1);

	RANGECHK(month,1,12);		/* RANGECHK returns -1 on invalid range */
	RANGECHK(day,1,31);
	RANGECHK(hour,0,23);
	RANGECHK(minute,0,59);

	dtg.tm_mon	= month-1;	/* 0 based */
	dtg.tm_mday	= day;
	dtg.tm_hour	= hour;		/* 0 based */
	dtg.tm_min	= minute;	/* 0 based */
	dtg.tm_sec	= 0;		/* 0 based */
	if ( year < 38 ) /* this is account for year later than 1999 */
	   year = year + 100;
	dtg.tm_year	= year;
	dtg.tm_isdst= -1;		/* tell mktime to handle daylight savings */

	jtime = mktime(&dtg);

	return(jtime);
}

static rangechk(value,lower,upper,str)
char *str;
{
	int rv;

	if(lower <= value && value <= upper)
		rv=1;
	else {
		rv=-1;
		Debug("%s field %02d not in range [%d,%d]\n",
		str,value,lower,upper);
	}
	return(rv);
}

static prtime(jtime)
time_t jtime;
{
	struct tm *tm;

	if(!Debugflg)
		return;
	tm = localtime(&jtime);
	Debug("datetosecs: %02d%2d%02d%02d%02d\n",
		tm->tm_mon,
		tm->tm_mday,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_year);
	Debug("datetosecs: %s",asctime(tm));
}


/*
 * convert datestr mmhhddmmssyy to # of seconds since 1970, using mktime().
 * Note: same as datetosecs except for seconds included in input datestr.
 */

time_t
datetosecs2(datestr)
char *datestr;
{
	int c;
	int rv;
	int month;
	int day;
	int year;
	int hour;
	int minute;
	int	sec;
	time_t jtime;
	char *cp;
	struct	tm	dtg;

	/* datestring must be 12 digits long */
	if(strlen(datestr) != 12) return(-1);

	cp = datestr;
	while(c = *cp++) {
		/* Bad character in datestring */
		if(!isdigit(c)) return(-1);
	}

	/* Datestring must be of the format mmddmmhhssyy */
	rv = sscanf(datestr,"%2d%2d%2d%2d%2d%2d",&month,&day,&hour,&minute,&sec,&year);
	if(rv != 6) return(-1);

	RANGECHK(month,1,12);		/* RANGECHK returns -1 on invalid range */
	RANGECHK(day,1,31);
	RANGECHK(hour,0,23);
	RANGECHK(minute,0,59);
	RANGECHK(sec,0,59);

	dtg.tm_mon	= month-1;	/* 0 based */
	dtg.tm_mday	= day;
	dtg.tm_hour	= hour;		/* 0 based */
	dtg.tm_min	= minute;	/* 0 based */
	dtg.tm_sec	= sec;		/* 0 based */
	if ( year < 38 ) /* this is account for year later than 1999 */
	   year = year + 100;
	dtg.tm_year	= year;
	dtg.tm_isdst= -1;		/* tell mktime to handle daylight savings */

	jtime = mktime(&dtg);

	return(jtime);
}

