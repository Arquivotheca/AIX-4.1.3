static char sccsid[] = "@(#)02	1.46  src/bos/usr/ccs/lib/libc/ctime.c, libctime, bos41J, 9520A_a 5/12/95 17:27:38";

/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: ctime, localtime, gmtime, asctime, tzset, mktime 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This module converts time as follows:
 * The epoch is 0000 Jan 1 1970 UTC (Coordinated Universal Time).
 * The argument time is in seconds since then.
 * The localtime(time) entry returns a pointer to an array
 * containing
 *  seconds (0-59)
 *  minutes (0-59)
 *  hours (0-23)
 *  day of month (1-31)
 *  month (0-11)
 *  year-1970
 *  weekday (0-6, Sun is 0)
 *  day of the year
 *  daylight savings flag
 *
 * The routine corrects for daylight saving
 * time and will work in any time zone provided
 * "timezone" is adjusted to the difference between
 * Universal and local standard time (measured in seconds).
 * In places like Michigan "daylight" must
 * be initialized to 0 to prevent the conversion
 * to daylight time.
 * There is a table which accounts for the peculiarities
 * undergone by daylight time in 1974-1975.
 *
 * The routine does not work
 * in Saudi Arabia which runs on Solar time.
 *
 * asctime(tvec)
 * where tvec is produced by localtime
 * returns a ptr to a character string
 * that has the ascii time in the form
 *	Thu Jan 01 00:00:00 1970
 *	01234567890123456789012345
 *	0	  1	    2
 *
 * ctime(timer) just calls localtime, then asctime.
 *
 * tzset() looks for an environment variable named
 * TZ. It should be in the form "[:]ESTn" or "[:]ESTnEDT",
 * where "n" represents a string of digits with an optional
 * negative sign (for locations east of Greenwich, England), or
 * "ESThh[:mm[:ss]][DST[hh[:mm[:ss]]][,start[/time],end[/time]]],
 * where "start" and "end" specify the day to start and end daylight
 * time.  The format is either "Jn" (1-based Julian), "n" (0-based Julian),
 * or "Mm.n.d" (month, week of month, day of week).
 * If the variable is present, it will set the external
 * variables "timezone", "daylight", and "tzname"
 * appropriately. It is called by localtime, and
 * may also be called explicitly by the user.
 */

/*
 * dysize(A) -- calculates the number of days in a year.  The year must be
 * 	the current year minus 1900 (i.e. 1990 - 1900 = 90, so 'A' should
 *	be 90).
 */
#define dysize(A) (((1900+(A)) % 4 == 0 && (1900+(A)) % 100 != 0 || (1900+(A)) % 400 == 0) ? 366:365)
/*
 * YR(A) -- compute the number of days since 0 A.D.  'A' must be the current
 * 		year minus 1900 (i.e. 1990 - 1900 = 90, so 'A' should be 90).
 */
#define YR(A) ((1900+A)*365 +(1900+A)/4 - (1900+A)/100 + ((1900+A) - 1600)/400)

#include <sys/errno.h>
#include <time.h>
#include <stdlib.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _ctime_rmutex;
#endif /* _THREAD_SAFE */

/* Define the latest possible julian date of the last Sunday in April (LSA),
 * the last Sunday in October (LSO), and the first Sunday in April (FSA).
 */
#define LSA  119
#define LSO  303
#define FSA   96

/* Define the number of seconds in a day and in an hour */

#define SECS_IN_DAY	86400L
#define SECS_IN_HOUR	 3600L

/*  
 *  Initialize tzname fields allowing for timezones up to length NLTZSIZE
 *  This prevents strcpy() in tzset from overflowing.
 */

#define TZNAME_MAX         255
#define	NLTZSIZE           TZNAME_MAX

#ifdef _THREAD_SAFE

/**********
  In libc_r.a, The following are imported from libc.a 
  so there is only 1 copy of each.
**********/
extern long	timezone;		/* Default timezone is UTC (old GMT) */
extern int	daylight;		/* no default daylight saving time.  */
extern char     tznamex[2][NLTZSIZE +1];
extern char	*tzname[2];

#else /* _THREAD_SAFE */

long	    timezone = 0;		/* Default timezone is UTC (old GMT) */
int	    daylight = 0;		/* no default daylight saving time.  */
static char tznamex[2][NLTZSIZE +1]={"UTC",""};
char	    *tzname[2]={tznamex[0],tznamex[1]};

#endif /* _THREAD_SAFE */

#ifndef _THREAD_SAFE
static char cbuf[26];
#endif /* _THREAD_SAFE */

static int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int dmsize_ly[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
 * The following table is used for 1974 and 1975 and
 * gives the day number of the first day after the Sunday of the
 * change.
 */
static struct { int	daylb; int	dayle; }
daytab[] = {
	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	58,	303,	/* 1975: Last Sun. in Feb - last Sun in Oct */
};

/*
 * Jjulian indicates a Julian date (either 0- or 1-based)
 * Mtimezone indicates that the TZ variable is of the form:
 *	TZA0TZB,M3.2.0,M10.5.0
 * Ltimezone indicates whether tzset() has been called already
 */

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * WARNING:  DO NOT SET THE VARIABLE internal_call ANYWHERE BEFORE
 *           TALKING TO A LIBRARY DEVELOPER.
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
static int internal_call;
static int Jjulian,Mtimezone,Ltimezone;
static int chngdy;
static long daylbegin = -1, daylend = -1;
static long dstbegsec, dstendsec;
static long dstchg=3600L;
static int weekday(struct tm *t,int d);
static char *atosec(char *p,long *result);
static char *ct_numb(char *cp,int n);
static char *gettzname(char *p, char *dst);
static char *getdlight(char *p,long *day,long *chtime);
static int ds_in_effect(long copyt);

static time_t globaltime;

#ifdef _THREAD_SAFE
char *
ctime_r(const time_t *timer, char *cbuf)
{
	struct tm	tm;

	if (cbuf == NULL) {
		errno = EINVAL;
		return(NULL);
	}
	if (localtime_r(timer, &tm) == NULL)
		return(NULL);

	if (asctime_r(&tm, cbuf) == NULL)
		return(NULL);

	return(cbuf);
}
#else /* _THREAD_SAFE */
char *
ctime(const time_t *timer)
{
	return(asctime(localtime(timer)));
}
#endif /* _THREAD_SAFE */


#ifdef _THREAD_SAFE
struct tm *
localtime_r(const time_t *timer, struct tm *ct)
#else /* _THREAD_SAFE */
struct tm *
localtime(const time_t *timer)
#endif /* _THREAD_SAFE */
{
	int dayno;
	long tm_sec, tm_diff;
	int p1,p2;
#ifndef _THREAD_SAFE
	struct tm *ct;
	static struct tm lcloc;	/* the static structure returned to the user */
	static struct tm lcloc_2;/* the static structure returned to the user */
#endif /* _THREAD_SAFE */
	long copyt;
	int lc_daylbegin, lc_daylend;

#ifdef _THREAD_SAFE
	if (ct == NULL) {
		errno = EINVAL;
		return(NULL);
	}
#endif /* _THREAD_SAFE */

	TS_LOCK(&_ctime_rmutex);

	if(!Ltimezone) {
		globaltime = *timer ;
		Jjulian=0;
	  	tzset();
	}
	lc_daylbegin=daylbegin;
	lc_daylend=daylend;
	copyt = *timer - timezone;
#ifdef _THREAD_SAFE
	/* 
	 * Even though gmtime_r() now returns "struct tm*" like gmtime(),
	 *  there's no need to assign the return value to *lcloc like the
	 *  non-thread safe code.  It's just more work that isn't needed.
	 */
	(void)gmtime_r(&copyt, ct);
#else /* _THREAD_SAFE */
	if (internal_call) {
		lcloc_2 = *gmtime(&copyt);
		ct = &lcloc_2;
	} else {
		lcloc = *gmtime(&copyt);
		ct = &lcloc;
	}
#endif /* _THREAD_SAFE */

	if( daylight ) {
		dayno = ct->tm_yday;
		if (lc_daylbegin==-1 || lc_daylend==-1)
		{
			if (ct->tm_year < 87)
			   lc_daylbegin = LSA;     /* last Sun in Apr */
			else
			   lc_daylbegin = FSA;     /* first Sun in Apr */
			lc_daylend = LSO;          /* Last Sun in Oct */
			if(ct->tm_year == 74 || ct->tm_year == 75) {
				lc_daylbegin = daytab[ct->tm_year-74].daylb;
				lc_daylend = daytab[ct->tm_year-74].dayle;
			}
		}
		lc_daylbegin = weekday(ct, lc_daylbegin);
		lc_daylend = weekday(ct, lc_daylend);
		tm_sec = (ct->tm_hour*60+ct->tm_min)*60+ct->tm_sec;
                tm_diff = dstendsec - dstchg;
                if ((tm_diff < 0) && ((tm_sec - SECS_IN_DAY) >= tm_diff))
                        dayno++;
		p1 = dayno>lc_daylbegin || (dayno==lc_daylbegin && tm_sec>=dstbegsec);
		p2 = dayno<lc_daylend || (dayno==lc_daylend && tm_sec<dstendsec); 
		if ((lc_daylend>lc_daylbegin && p1 && p2) ||
				 (lc_daylend<lc_daylbegin && (p1||p2)))
		{
		/* Daylend is smaller than lc_daylbegin in the southern
		 * hemisphere.
		 */
 			if (dayno == lc_daylend && tm_sec < dstendsec &&
 				tm_sec >= dstendsec - dstchg) {
#ifdef _THREAD_SAFE
				(void)gmtime_r(&copyt, ct);
#else /* _THREAD_SAFE */
				ct = gmtime(&copyt);
#endif /* _THREAD_SAFE */
			} else {
				copyt += dstchg;
#ifdef _THREAD_SAFE
				(void)gmtime_r(&copyt, ct);
#else /* _THREAD_SAFE */
				ct = gmtime(&copyt);
#endif /* _THREAD_SAFE */
				ct->tm_isdst++;
			}
		}
	}
	TS_UNLOCK(&_ctime_rmutex);
	return(ct);
}

/*
 * The argument is a 0-origin julian day number.
 * The value returned is the day number of the first
 * Sunday on or before the day given in the t structure.
 * If chngdy is set, it is consulted to see which day-of-week daylight
 * savings time should be moved up to.  If zero, day number is not changed
 * for weekday, allowing DST to start on a specific calendar date.
 * However, day number is adjusted by one to preserve calendar date
 * on a leap year (provided Jjulian and Mtimezone are not set).
 */

static int
weekday(struct tm *t,int d)
{
	if(!Jjulian && !Mtimezone)
	 if(d > 58) /* day 58 ([0-365] is feb 28 */
	   d += dysize(t->tm_year) - 365;
	if(chngdy == 0)
		/* don't move d to nearest day-of-week */
		return d;
	return(d - (d - t->tm_yday + t->tm_wday - chngdy + 700) % 7);
}

#ifdef _THREAD_SAFE
struct tm *
gmtime_r(const time_t *timer, struct tm *xtime_ptr)
#else /* _THREAD_SAFE */
struct tm *
gmtime(const time_t *timer)
#endif /* _THREAD_SAFE */
{
	int d0, d1;
	long hms, day;
#ifndef _THREAD_SAFE
	static struct tm xtime;
	static struct tm xtime_2;
	struct tm *xtime_ptr;

	if (internal_call)
		xtime_ptr = &xtime_2;
	else
		xtime_ptr = &xtime;
#else /* _THREAD_SAFE */
	if (xtime_ptr == NULL) {
		errno = EINVAL;
		return(NULL);
	}
#endif /* _THREAD_SAFE */

	/*
	 * break initial number into days
	 */
	hms = *timer % SECS_IN_DAY;
	day = *timer / SECS_IN_DAY;
	if(hms < 0) {
		hms += SECS_IN_DAY;
		day -= 1;
	}
	/*
	 * generate hours:minutes:seconds
	 */
	xtime_ptr->tm_sec = hms % 60;
	d1 = hms / 60;
	xtime_ptr->tm_min = d1 % 60;
	d1 /= 60;
	xtime_ptr->tm_hour = d1;

	/*
	 * day is the day number.
	 * generate day of the week.
	 * The addend is 4 mod 7 (1/1/1970 was Thursday)
	 */

	xtime_ptr->tm_wday = (day + 7340036L) % 7;

	/*
	 * year number
	 */
	if(day >= 0)
		for(d1=70; day >= dysize(d1); d1++)
			day -= dysize(d1);
	else
		for(d1=70; day < 0; d1--)
			day += dysize(d1-1);
	xtime_ptr->tm_year = d1;
	xtime_ptr->tm_yday = d0 = day;

	/*
	 * generate month
	 */

        /* Notice that dmsize_ly[] is used below to avoid modifying
         * the dmsize[] array which can cause problem when multiple
	 * threads are accessing the array.
         */

	if(dysize(d1) == 366)
		for(d1=0; d0 >= dmsize_ly[d1]; d1++)
			d0 -= dmsize_ly[d1];
	else
		for(d1=0; d0 >= dmsize[d1]; d1++)
			d0 -= dmsize[d1];

	xtime_ptr->tm_mday = d0+1;
	xtime_ptr->tm_mon = d1;
	xtime_ptr->tm_isdst = 0;
	return(xtime_ptr);

}

#ifdef _THREAD_SAFE
char *
asctime_r(const struct tm *timeptr,char *outbuf)
#else /* _THREAD_SAFE */
char *
asctime(const struct tm *timeptr)
#endif /* _THREAD_SAFE */
{
	char *cp, *ncp;
	int *tp;

#ifdef _THREAD_SAFE
	char cbuf[26];
	if (outbuf == NULL) {
		errno = EINVAL;
		return((char *)NULL);
	}
#endif /* _THREAD_SAFE */

	/**********
	  for AES
	**********/
	if (timeptr == (struct tm *)NULL) {
		errno = EFAULT;
		return((char *)NULL);
	}

	cp = cbuf;
	for(ncp = "Day Mon 00 00:00:00 1900\n"; *cp++ = *ncp++; );
	ncp = &"SunMonTueWedThuFriSat"[3*timeptr->tm_wday];
	cp = cbuf;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp++;
	tp = (int *)&timeptr->tm_mon;
	ncp = &"JanFebMarAprMayJunJulAugSepOctNovDec"[(*tp)*3];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp = ct_numb(cp, *--tp);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	if(timeptr->tm_year >= 100) {
		cp[1] = '2';
		cp[2] = '0';
	}
	cp += 2;
	cp = ct_numb(cp, timeptr->tm_year+100);
#ifdef _THREAD_SAFE
	strcpy(outbuf, cbuf);	/* outbuf[] must be at least 26 bytes long */
#endif /* _THREAD_SAFE */	/*  as supplied by the user.		   */
	return(cbuf);
}

static char *
ct_numb(char *cp,int n)
{
	cp++;
	if(n >= 10)
		*cp++ = (n/10)%10 + '0';
	else
		*cp++ = ' ';
	*cp++ = n%10 + '0';
	return(cp);
}

/* The tzset() function includes all the POSIX requirements */

void
tzset(void)
{
	char 	*p, *q;
	int 	n;
	int 	sign;

	TS_LOCK(&_ctime_rmutex);
	
	chngdy = 0;

	if((p = getenv ("TZ")) && *p) {

		while (*p == ':')
			p++;

		p = gettzname(p,tzname[0]);
		p = atosec(p, &timezone);

		/*
		 * if anything else left, get daylight TZ name
		 */
		if (*p != ',' && *p != '\0') {
			p = gettzname(p,tzname[1]);
			daylight = 1;
		} else {
			strcpy(tzname[1],"   ");
			daylight = 0;
		}

		if (*p) {
			/* this is an extended TZ variable */
			if (*p == ',') {
				/* no dst offset */
				dstchg = 1*60*60;
				p++;
			}
			else {
				/* dst offset specified */
				p = atosec(p, &dstchg);
				dstchg = timezone - dstchg;
				if (*p) 
					p++; 
			} 
			/* get daylight start/end */
			p = getdlight(p, &daylbegin, &dstbegsec);
			getdlight(p, &daylend, &dstendsec);
		}
		else {
			dstchg = 1*60*60;
			chngdy = 7;
			daylbegin = daylend = -1;
			dstbegsec = dstendsec = 2*60*60;
		}
	}
	TS_UNLOCK(&_ctime_rmutex);
}

static
char *atosec(char *p,long *result)
{
	int n, sign = 0;
	long v;
	/*
	 *  Convert string of form [-+]hh:mm to seconds
	 */
	if ((sign = (*p == '-')) || (*p == '+')) {
		p++;
	}

	n = 0;
	while(*p >= '0' && *p <= '9')
		n = (n * 10) + *p++ - '0';
	v = ((long)(n * 60)) * 60;
	if(*p == ':') {
		p++;
		n = 0;
		while(*p >= '0' && *p <= '9')
			n = (n * 10) + *p++ - '0';
		v += n * 60;
	}
	if (*p == ':') {
		p++;
		n = 0;
		while (*p >= '0' && *p <= '9')
			n = (n * 10) + *p++ - '0';
		v += n;
	}
	if(sign)
		v = -v;
	*result = v;
	return p;
}

/*
 * POSIX 1003.1-1990, 8.1.1 Extensions to Time Functions
 *   std and dst  Indicates no less than three, nor more than
 *                TZNAME_MAX bytes that are the designation
 *                for the standard (std) or summer (dst) time
 *                zone. ... Any characters except a leading
 *                colon (:),or: digits, the comma (,), the
 *                minus (-), the plus (+), and the null
 *                character are permitted to appear in these
 *                fields, ...
 */

static
char *
gettzname(char *p, char *dst)
{
	char *q;
	int i, n;

	/*
	 * copy name into destination buffer
	 */
	n = NLTZSIZE;
	q = dst;
	if ( *p && *p != ':')
	{
		while (*p && n-- > 0 && *p != ',' && *p != '-' && *p != '+' &&
				(*p < '0' || *p > '9') )
		{
			*q++ = *p++;
		}
	}
	*q = '\0';

	/*
	 * if the name is too short, pad it to 3 characters
	 */
	if ( (n = strlen(dst)) < 3 )
	{
		while (n < 3)
		{
			dst[n++] = ' ';
		}
		dst[n] = '\0';
	}
	return p;
}

static char *
getdlight(char *p,long *day,long *chtime)
{
	char	buf[32];
	char	*s;
	int	Jflag = 0;

	Mtimezone = 0;

	/* Set default time */
	sprintf(buf, "%s", "02:00:00");
	atosec(buf, chtime);

	switch(*p) {
	case 'J': /* Julian day (1-based) */
		Jflag++;
		p++;
	default:  /* Julian day (0-based) */
		for (s = buf; (*s = *p) && *p != ',' && *p != '/'; 
			p++, s++);
		*s = '\0';

		/* convert to 0-based Julian */
		*day = atoi(buf) - Jflag;
		if(!Jflag)
			Jjulian++;
		break;
	case 'M':
	{
		time_t	tim;
#ifdef _THREAD_SAFE
		struct tm temp;
#endif /* _THREAD_SAFE */
		struct	tm *tmp;
		int	i, j, k, n;
		int 	fday, mon;
	
		internal_call=1;
		Mtimezone++;

		/* Get details on Jan 1 of this year */
#ifdef _THREAD_SAFE
		tmp = &temp;
		(void)gmtime_r(&globaltime, tmp);
#else /* _THREAD_SAFE */
		tmp = gmtime(&globaltime);
#endif /* _THREAD_SAFE */
		tmp->tm_mon = 0;
		tmp->tm_mday = 1;

		/* after mktime, adjust for timezone since mktmp() */
		/* works on local time, not gmtime */
		tim = mktime(tmp) - timezone;

#ifdef _THREAD_SAFE
		(void)gmtime_r(&tim, tmp);
#else /* _THREAD_SAFE */
		tmp = gmtime(&tim);
#endif /* _THREAD_SAFE */
		fday = tmp->tm_wday;

		internal_call=0;

		/* Figure out day number */
		for (p++, s = buf; (*s = *p) && *p != '.' && *p != '/'; 
			p++, s++);
		if (*p == '\0')
			return(p);
		*s = '\0';
		j = atoi(buf) - 1;
		for (mon = 0, *day = 0; mon < j; mon++) {
			*day += dmsize[mon];

			/* Keep track of first day of week of month */
			if (mon == 1 && dysize(tmp->tm_year) == 366) {
				/* If Feb, add leap day. */
				(*day)++;
				fday = (fday + dmsize[mon] + 1) % 7;
			}
			else
				fday = (fday + dmsize[mon]) % 7;
		}

		/* Get week number */
		for (p++, s = buf; (*s = *p) && *p != '.' && *p != '/'; 
			p++, s++);
		if (*p == '\0')
			return(p);
		*s = '\0';
		j = atoi(buf);

		/* Get day of week */
		for (p++, s = buf; (*s = *p) && *p != '.' && *p != '/' && *p != ','; 
			p++, s++);
		*s = '\0';
		i = atoi(buf);

		/* Add days in partial month */
		/* The first i-day is the nth day of the month */
		if (fday <= i)
			n = i - fday;
		else
			n = ( 7 - (fday - i));
		*day += n;

		/* k = current week number */
		for (k = 1; k < j && (n + 7*k < dmsize[mon]); k++)
			*day += 7;
	}
		break;
	case '\0':
		return(p);
	}
	if (*p == '\0')
		return(p);
	if (*p == ',')
		return(++p);

	/* Get daylight time of day.  Leading + or - illegal. */
	switch(*++p) {
	case '+':
	case '-':
		p++;
	}
	for (s = buf; (*s = *p) && *p != ','; p++, s++);
	*s = '\0';
	atosec(buf, chtime);
	/* skip over trailing ',' */
	if (*p == ',')
		p++;
	return(p);
}

/*
 * FUNCTION: 
 *   The mktime() function converts the broken-down time, expressed as
 *   local time, in the structure pointed to by timeptr, into a time
 *   since the Epoch ( 00:00:00 GMT January 1, 1970) with the same encoding
 *   as that of the values returned by the time() function.  The original 
 *   values of the tm_wday and tm_yday components of the structure are 
 *   ignored, and the original values of the other components are not 
 *   restricted to the ranges described in the <time.h> entry.  The range 
 *   [0,61] for tm_sec allows for the occasional leap second or double leap
 *   second. -- X/Open definition of mktime().
 *
 *   The tm  structure is defined  in the time.h  header file,
 *   and it contains the following members:
 *
 *        int tm_sec;   (* Seconds (0 - 61) *)
 *        int tm_min;   (* Minutes (0 - 61) *)
 *        int tm_hour;  (* Hours (0 - 23) *)
 *        int tm_mday;  (* Day of month (1 - 31) *)
 *        int tm_mon;   (* Month of year (0 - 11) *)
 *        int tm_year;  (* Year - 1900 *)
 *        int tm_wday;  (* Day of week (0 - 6) (Sunday = 0) *)
 *        int tm_yday;  (* Day of year (0 - 365) *)
 *        int tm_isdst; (* Nonzero = Daylight saving time flag *)
 *
 *
 * PARAMETERS: 
 *	struct tm *timeptr - pointer to be converted
 *
 * NOTES: mktime() is included in this module for access to the dstchg
 *        (DST change) variable.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	- returns the time in seconds
 *
 */

time_t 	
mktime(struct tm *timeptr)
{
	struct tm gmtptr;
	time_t secs = 0L,savsecs;
	time_t seconds, minutes, hours;
	register int i, days = 0;
	int tm_sec;
	static int mdays[]={0,31,59,90,120,151,181,212,243,273,304,334,-1,-1};

	/*
	 * Since we aren't allowed to look at tm_yday by X/Open, we have
	 * to compute the number of days into the year ourselves.  Note that
	 * tm_mon is 0-based, so months go from 0 - 11.  We use that as an 
	 * index into mdays[], which gives us the number of days without 
	 * accounting for leap years.  If dysize() returns 366, we simply add
	 * 1 to days (if we're in March or later).  The numbers assigned
	 * into days are the number of days that MUST have passed already if 
	 * we're in a given month (i.e. if tm_mon == 3 (April), then AT LEAST 
	 * 90 days have already passed (we add 1 later if it's a leap year, 
	 * which gives us the total number of days)).  
	 */
	/*
	 * This gross kludge ensures that our month and year are within
	 * acceptable limits.
	 */
	if (timeptr->tm_mon < 0) {
lt_repeat:
		timeptr->tm_year--;
		timeptr->tm_mon += 12;
		if (timeptr->tm_mon < 0) goto lt_repeat;  /* Yuck! */
		days = mdays[timeptr->tm_mon];
	} else if (timeptr->tm_mon > 11) {
gt_repeat:
		timeptr->tm_year++;
		timeptr->tm_mon -= 12;
		if (timeptr->tm_mon > 11) goto gt_repeat; /* Yuck * 2 */
		days = mdays[timeptr->tm_mon];
	} else
		days = mdays[timeptr->tm_mon];

	days += (timeptr->tm_mday - 1);	/* convert to 0-origin */
	/*
	 * Compute the number of days since (or until) Jan. 1, 1970, using the
	 * YR macro.  Number of days until the beginning of the year - number
	 * of days until Jan. 1, 1970 = # of days between Jan. 1, 1970 and 
	 * today.
	 */
	days += (YR(timeptr->tm_year)) - (YR(70));
	if (dysize(timeptr->tm_year) == 366)
		days--;
	if ((dysize(timeptr->tm_year) == 366) && (timeptr->tm_mon > 1))
		days++;

	/*
	 * quickly adjust seconds minutes and hours into rough bounds
	 */
	seconds = timeptr->tm_sec;
	minutes = timeptr->tm_min + (seconds / 60);
	seconds = seconds % 60;
	if (seconds < 0) {
		seconds += 60;
		minutes -= 1;
	}
	hours = timeptr->tm_hour + (minutes / 60);
	minutes = minutes % 60;
	if (minutes < 0) {
		minutes += 60;
		hours -= 1;
	}
	days += hours / 24;
	hours = hours % 24;
	if (hours < 0) {
		hours += 24;
		days -= 1;
	}

	/*
	 * now accumulate seconds of day into one value
	 */
	seconds += hours * SECS_IN_HOUR + minutes * 60;

	/*
	 * now check for calendar overflow or underflow (UNIX claims any
	 * date in before the epoch is invalid but be careful though,
	 * some dates in day -1 are valid when adjusted by timezone
	 *
	 * The acutual checks are if the day is less than day -1, if there
	 * are more days than can fit in LONG_MAX or if there are exactly
	 * the number of days that fit in LONG_MAX and the seconds left over
	 * are greater than the seconds in that day.
	 */
	if ((days < -1) || 
	    (days > (LONG_MAX / SECS_IN_DAY)) ||
	    ((days == (LONG_MAX / SECS_IN_DAY)) &&
	     (seconds > (LONG_MAX % SECS_IN_DAY)))) {
		/**********
		  we still need to call tzset even though we failed
		**********/
		if (!internal_call)  /* prevent infinite recursion */
			tzset();   
		return(-1);
	}
	secs = days * SECS_IN_DAY;/* seconds since 1970 (until beg. of today) */
	secs += seconds;

	/*
	 * X/Open says that mktime() should perform as though tzset() were
	 * called.  The best way to assure that is to call tzset().  It also
	 * gives us the value of timezone which we use to offset time before
	 * calling localtime() (makes localtime() give us back the same thing
	 * we gave it, instead of factoring in the difference between local
	 * time and GMT.)
	 */
	if (!internal_call)	{ /* prevent infinite recursion */
	  	Jjulian=0;
		globaltime=secs;
	    	tzset();	/* get timezone set up for use */
	}

	/*
	 * ANSI C says if tm_isdst is 0 the daylight saving is not in  
	 * effect.
	 * If it is > 0 mktime should assume it is in effect.
	 * If it is == -1 mktime should figure out if it is in effect.
	 */

/****
and we save the 'true seconds' due to subtracting
*****/

	savsecs = secs;
	if(timeptr->tm_isdst)
	{
		if(timeptr->tm_isdst > 0 )
		  if (daylight)
			secs -= dstchg;
		  else
			timeptr->tm_isdst = 0;
		else
		{
	 	  if(ds_in_effect(savsecs))
			secs -= dstchg;
		}
	}

	/*
	 * Rebuild the timeptr structure by calling localtime() with the
	 * newly calculated number of seconds since the Epoch as the
	 * argument.  Store the results back into timeptr.  Note that 
	 * timezone is added to secs to produce the UTC time so that the
	 * return from localtime() is correct.
	 */
	if (timezone > 0 && secs > (LONG_MAX - timezone)) {
		return(-1);
	}
	secs += timezone;
	if (secs < 0) {
		return(-1);
	}

	Ltimezone=1;
#ifdef _THREAD_SAFE
	/* 
	 * Even though localtime_r() now returns "struct tm*" like localtime(),
	 *  there's no need to assign the return value to *timeptr like the
	 *  non-thread safe code.  It's just more work that isn't needed.
	 */
	(void)localtime_r(&secs, timeptr);
#else /* _THREAD_SAFE */
	*timeptr = *localtime(&secs); 
#endif /* _THREAD_SAFE */
	Ltimezone=0;

	return(secs);
}

/*
 * ds_in_effect() -- this routine is basically a copy of localtime(), but
 *	changed to return only TRUE or FALSE, indicating whether Daylight
 *	Savings Time is currently in effect.
 */
int ds_in_effect(long copyt)
{
	int dayno;
	long tm_sec;
	int p1,p2;
	struct tm *ct,dsloc;
	int ds_daylbegin=daylbegin,ds_daylend=daylend;

#ifdef _THREAD_SAFE
	ct = &dsloc;
	(void)gmtime_r(&copyt, ct);
#else /* _THREAD_SAFE */
	ct = gmtime(&copyt);
#endif /* _THREAD_SAFE */

	if( daylight ) {
		dayno = ct->tm_yday;
		if (ds_daylbegin == -1 || ds_daylend == -1)
		{
			if (ct->tm_year < 87)
			   ds_daylbegin = LSA;     /* last Sun in Apr */
			else
			   ds_daylbegin = FSA;     /* first Sun in Apr */
			ds_daylend = LSO;          /* Last Sun in Oct */
			if(ct->tm_year == 74 || ct->tm_year == 75) {
				ds_daylbegin = daytab[ct->tm_year-74].daylb;
				ds_daylend = daytab[ct->tm_year-74].dayle;
			}
		}
		ds_daylbegin = weekday(ct, ds_daylbegin);
		ds_daylend = weekday(ct, ds_daylend);
		tm_sec = (ct->tm_hour*60+ct->tm_min)*60+ct->tm_sec;
		p1 = dayno>ds_daylbegin || (dayno==ds_daylbegin && tm_sec>=dstbegsec);
		p2 = dayno<ds_daylend || (dayno==ds_daylend && tm_sec<dstendsec); 
		if ((ds_daylend>ds_daylbegin && p1 && p2) ||
				 (ds_daylend<ds_daylbegin && (p1||p2)))
		{
		/* Daylend is smaller than ds_daylbegin in the southern
		 * hemisphere.
		 */
 			if (dayno == ds_daylbegin && tm_sec >= dstbegsec &&
 				tm_sec < dstbegsec + dstchg) {
				return(0);
			} else {
				return(1);
			}
		}
	}
return(0);
}
