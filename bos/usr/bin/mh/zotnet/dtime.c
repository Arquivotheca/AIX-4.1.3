static char sccsid[] = "@(#)19	1.6  src/bos/usr/bin/mh/zotnet/dtime.c, cmdmh, bos411, 9428A410j 10/20/93 07:23:11";
/* 
 * COMPONENT_NAME: CMDMH dtime.c
 * 
 * FUNCTIONS: DOTW, LDOTW, MOTY, abs, dasctime, dctime, dgmtime, 
 *            dlocaltime, dtimenow, dtimezone, dtwstime, dysize, 
 *            set_dotw, twclock, twscopy, twsort 
 *
 * ORIGINS: 26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* dtime.c - routines to do ``ARPA-style'' time structures */

/* LINTLIBRARY */


#include "tws.h"
#ifndef	INETONLY
#include "strings.h"
#else	INETONLY
#include "strings.h"
#endif	INETONLY
#include <stdio.h>
#include <sys/types.h>
#ifndef  SYS5
#include <sys/timeb.h>
#endif   not SYS5
#ifndef	BSD42
#include <time.h>
#else	BSD42
#include <time.h>
#include <sys/time.h>
#endif	BSD42

#ifdef	SYS5
extern int  daylight;
extern long timezone;
extern char *tzname[];
#endif	SYS5

/*  */

/* #define	abs(a)	(a >= 0 ? a : -a) */

#define	dysize(y)	\
	(((y) % 4) ? 365 : (((y) % 100) ? 366 : (((y) % 400) ? 365 : 366)))

/* The following macros do a sanity check on the array index before 
   indexing.  Fixes a memory fault error for PTM P0006760.  danc */
#define MOTY(mon)  ((mon > 11 || mon < 0) ? "?" : tw_moty[mon])
#define DOTW(day)  ((day > 6  || day < 0) ? "?" : tw_dotw[day])
#define LDOTW(day) ((day > 6  || day < 0) ? "?" : tw_ldotw[day])

/*  */

char *tw_moty[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
};

char *tw_dotw[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
};

char *tw_ldotw[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday", NULL
};

/*  */

static struct zone {
    char   *std,
           *dst;
    int     shift;
}                   zones[] = {
			"GMT", "BST", 0,
                        "EST", "EDT", -5,
                        "CST", "CDT", -6,
                        "MST", NULL, -7,
                        "PST", "PDT", -8,
                        "A", NULL, -1,
                        "B", NULL, -2,
                        "C", NULL, -3,
                        "D", NULL, -4,
                        "E", NULL, -5,
                        "F", NULL, -6,
                        "G", NULL, -7,
                        "H", NULL, -8,
                        "I", NULL, -9,
                        "K", NULL, -10,
                        "L", NULL, -11,
                        "M", NULL, -12,
                        "N", NULL, 1,
#ifndef	HUJI
                        "O", NULL, 2,
#else	HUJI
			"JST", "JDT", 2,
#endif	HUJI
                        "P", NULL, 3,
                        "Q", NULL, 4,
                        "R", NULL, 5,
                        "S", NULL, 6,
                        "T", NULL, 7,
                        "U", NULL, 8,
                        "V", NULL, 9,
                        "W", NULL, 10,
                        "X", NULL, 11,
                        "Y", NULL, 12,

                        NULL
};

#define CENTURY 19


/*  */

char *dtimenow () {
    long    clock;

    (void) time (&clock);
    return dtime (&clock);
}


char   *dctime (tw)
register struct tws *tw;
{
    static char buffer[25];

    if (!tw)
	return NULL;

    (void) sprintf (buffer, "%.3s %.3s %02d %02d:%02d:%02d %.4d\n",
	    DOTW(tw -> tw_wday), MOTY(tw -> tw_mon), tw -> tw_mday,
	    tw -> tw_hour, tw -> tw_min, tw -> tw_sec,
	    tw -> tw_year >= 100 ? tw -> tw_year : 1900 + tw -> tw_year);

    return buffer;
}

/*  */

struct tws *dtwstime () {
    long    clock;

    (void) time (&clock);
    return dlocaltime (&clock);
}


struct tws *dlocaltime (clock)
register long   *clock;
{
    register struct tm *tm;
#ifndef	SYS5
    struct timeb    tb;
    struct timeval  tp;
    struct timezone tpz;
    struct tm gmt;
    register struct tm *lt;
    time_t t;
    register int off;
#endif	not SYS5
    static struct tws   tw;

    if (!clock)
	return NULL;
    tw.tw_flags = TW_NULL;

    tm = localtime (clock);
    tw.tw_sec = tm -> tm_sec;
    tw.tw_min = tm -> tm_min;
    tw.tw_hour = tm -> tm_hour;
    tw.tw_mday = tm -> tm_mday;
    tw.tw_mon = tm -> tm_mon;
    tw.tw_year = tm -> tm_year;
    tw.tw_wday = tm -> tm_wday;
    tw.tw_yday = tm -> tm_yday;
    if (tm -> tm_isdst)
	tw.tw_flags |= TW_DST;
#ifndef  SYS5
#ifndef _AIX
    ftime (&tb); 
    gettimeofday(&tp,&tpz);
    tw.tw_zone = -(tpz.tz_minuteswest);
#else
    (void) time(&t);
    gmt = *gmtime(&t);
    lt = localtime(&t);
    off = (lt->tm_hour - gmt.tm_hour) * 60 + lt->tm_min - gmt.tm_min;
    /* assume that offset isn't more than a day ... */
    if (lt->tm_year < gmt.tm_year)
		off -= 24 * 60;
    else if (lt->tm_year > gmt.tm_year)
		off += 24 * 60;
    else if (lt->tm_yday < gmt.tm_yday)
		off -= 24 * 60;
    else if (lt->tm_yday > gmt.tm_yday)
		off += 24 * 60;
    tw.tw_zone = off;
#endif /* not _AIX */
#else   SYS5
    tzset ();
    tw.tw_zone = -(timezone / 60);
#endif  SYS5
    tw.tw_flags &= ~TW_SDAY, tw.tw_flags |= TW_SEXP;
    tw.tw_clock = *clock;

    return (&tw);
}


struct tws *dgmtime (clock)
register long   *clock;
{
    register struct tm *tm;
    static struct tws   tw;

    if (!clock)
	return NULL;
    tw.tw_flags = TW_NULL;

    tm = gmtime (clock);
    tw.tw_sec = tm -> tm_sec;
    tw.tw_min = tm -> tm_min;
    tw.tw_hour = tm -> tm_hour;
    tw.tw_mday = tm -> tm_mday;
    tw.tw_mon = tm -> tm_mon;
    tw.tw_year = tm -> tm_year;
    tw.tw_wday = tm -> tm_wday;
    tw.tw_yday = tm -> tm_yday;
    if (tm -> tm_isdst)
	tw.tw_flags |= TW_DST;
    tw.tw_zone = 0;
    tw.tw_flags &= ~TW_SDAY, tw.tw_flags |= TW_SEXP;
    tw.tw_clock = *clock;

    return (&tw);
}

/*  */

char   *dasctime (tw, flags)
register struct tws *tw;
int	flags;
{
    static char buffer[80],
                result[80];

    if (!tw)
	return NULL;

    (void) sprintf (buffer, "%02d %s %02d %02d:%02d:%02d %s",
	    tw -> tw_mday, MOTY(tw -> tw_mon), tw -> tw_year,
	    tw -> tw_hour, tw -> tw_min, tw -> tw_sec,
	    dtimezone (tw -> tw_zone, tw -> tw_flags | flags));

    if ((tw -> tw_flags & TW_SDAY) == TW_SEXP)
	(void) sprintf (result, "%s, %s", DOTW(tw -> tw_wday), buffer);
    else
	if ((tw -> tw_flags & TW_SDAY) == TW_SNIL)
	    (void) strcpy (result, buffer);
	else
	    (void) sprintf (result, "%s (%s)", buffer, DOTW(tw -> tw_wday));

    return result;
}

/*  */

char   *dtimezone (offset, flags)
register int     offset,
		 flags;
{
    register int    hours,
                    mins;
    register struct zone *z;
    static char buffer[10];

    if (offset < 0) {
	mins = -((-offset) % 60);
	hours = -((-offset) / 60);
    }
    else {
	mins = offset % 60;
	hours = offset / 60;
    }

#ifndef _AIX
    if (!(flags & TW_ZONE) && mins == 0)
	for (z = zones; z -> std; z++)
	    if (z -> shift == hours)
		return (z -> dst && (flags & TW_DST) ? z -> dst : z -> std);
#endif /* not _AIX */

    (void) sprintf (buffer, "%s%02d%02d",
	    offset < 0 ? "-" : "+", abs (hours), abs (mins));
    return buffer;
}

/*  */

void twscopy (tb, tw)
register struct tws *tb,
		    *tw;
{
#ifdef	notdef
    tb -> tw_sec = tw -> tw_sec;
    tb -> tw_min = tw -> tw_min;
    tb -> tw_hour = tw -> tw_hour;
    tb -> tw_mday = tw -> tw_mday;
    tb -> tw_mon = tw -> tw_mon;
    tb -> tw_year = tw -> tw_year;
    tb -> tw_wday = tw -> tw_wday;
    tb -> tw_yday = tw -> tw_yday;
    tb -> tw_zone = tw -> tw_zone;
    tb -> tw_clock = tw -> tw_clock;
    tb -> tw_flags = tw -> tw_flags;
#else	not notdef
    *tb = *tw;
#endif	not notdef
}


int     twsort (tw1, tw2)
register struct tws *tw1,
		    *tw2;
{
    register long   c1,
                    c2;

    if (tw1 -> tw_clock == 0L)
	(void) twclock (tw1);
    if (tw2 -> tw_clock == 0L)
	(void) twclock (tw2);

    return ((c1 = tw1 -> tw_clock) > (c2 = tw2 -> tw_clock) ? 1
	    : c1 == c2 ? 0 : -1);
}

/*  */

/* This routine is based on the gtime() routine written by Steven Shafer
   (sas) at CMU.  It was forwarded to MTR by Jay Lepreau at Utah-CS.
 */

static int  dmsize[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


long	twclock (tw)
register struct	tws *tw;
{
    register int    i,
                    sec,
                    min,
                    hour,
                    mday,
                    mon,
                    year;
    register long   result;

    if (tw -> tw_clock != 0L)
	return tw -> tw_clock;

    if ((sec = tw -> tw_sec) < 0 || sec > 59
	    || (min = tw -> tw_min) < 0 || min > 59
	    || (hour = tw -> tw_hour) < 0 || hour > 23
	    || (mday = tw -> tw_mday) < 1 || mday > 31
	    || (mon = tw -> tw_mon + 1) < 1 || mon > 12)
	return (tw -> tw_clock = -1L);
    year = tw -> tw_year;

    result = 0L;
    year += 1900;
    for (i = 1970; i < year; i++)
	result += dysize (i);
    if (dysize (year) == 366 && mon >= 3)
	result++;
    while (--mon)
	result += dmsize[mon - 1];
    result += mday - 1;
    result = 24 * result + hour;
    result = 60 * result + min;
    result = 60 * result + sec;
    result -= 60 * tw -> tw_zone;
    if (tw -> tw_flags & TW_DST)
	result -= 60 * 60;

    return (tw -> tw_clock = result);
}

/*  */

/*
 *    Simple calculation of day of the week.  Algorithm used is Zeller's
 *    congruence.  Currently, we assume if tw -> tw_year < 100
 *    then the century is CENTURY.
 */

set_dotw (tw)
register struct tws *tw;
{
    register int    month,
                    day,
                    year,
                    century;

    month = tw -> tw_mon - 1;
    day = tw -> tw_mday;
    year = tw -> tw_year % 100;
    century = tw -> tw_year >= 100 ? tw -> tw_year / 100 : CENTURY;

    if (month <= 0) {
	month += 12;
	if (--year < 0) {
	    year += 100;
	    century--;
	}
    }

    tw -> tw_wday =
	((26 * month - 2) / 10 + day + year + year / 4
	    - 3 * century / 4 + 1) % 7;

    tw -> tw_flags &= ~TW_SDAY, tw -> tw_flags |= TW_SIMP;
}
