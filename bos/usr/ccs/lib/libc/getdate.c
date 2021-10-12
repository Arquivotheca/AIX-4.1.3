static char sccsid[] = "@(#)84	1.2  src/bos/usr/ccs/lib/libc/getdate.c, libcfmt, bos411, 9428A410j 3/30/94 15:00:40";
/*
 *   COMPONENT_NAME: libcfmt
 *
 *   FUNCTIONS: calc_day
 *		check_tm
 *		clr_tm
 *		expect_char
 *		expect_str
 *		fill_tm
 *		getdate_r
 *		getdigits
 *		interpret
 *		jan1
 *		parse
 *
 *   ORIGINS: 85
 *
 *                    SOURCE MATERIALS
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#define _ILS_MACROS
#include <sys/stat.h>
#include <sys/mode.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <langinfo.h>
#include <stdlib.h>

#include "ts_supp.h"

#define NONE	0x00000000
#define SECONDS	0x00000001
#define MINUTES 0x00000002
#define HOURS	0x00000004
#define MDAY	0x00000008
#define MONTH	0x00000010
#define YEAR	0x00000020
#define WDAY	0x00000040

#ifdef _THREAD_SAFE

#include "rec_mutex.h"
extern struct rec_mutex _ctime_rmutex;	/* guard tzname[] access */

#else
int getdate_err;
#endif	/* _THREAD_SAFE */

#define BUFSIZE 100

extern char *getenv();

static int getdate_day[7] = {DAY_1, DAY_2, DAY_3, DAY_4, DAY_5, DAY_6, DAY_7};
static int getdate_aday[7] = {ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4, ABDAY_5, 
				ABDAY_6, ABDAY_7};
static int getdate_mon[12] = {MON_1, MON_2, MON_3, MON_4, MON_5, MON_6, MON_7,
			 	MON_8, MON_9, MON_10, MON_11, MON_12};
static int getdate_amon[12] = {ABMON_1, ABMON_2, ABMON_3, ABMON_4, ABMON_5, 
				ABMON_6, ABMON_7, ABMON_8, ABMON_9, ABMON_10, 
				ABMON_11, ABMON_12};

static char * interpret( char *, char *, struct tm* );
static char * expect_char( char *, char *);
static int calc_day(struct tm *);
static int check_tm(struct tm*, int[]);
static int jan1(int);


static int getdigits(char *str, int *num, int width)
/*
 * Try to read a field of exactly 'width' decimal digits from 'str' into 'num'
 * Returns TRUE if successful, FALSE otherwise.
 */
{
	char buf[5];		/* Space for 4 digit year and NUL */

	strncpy(buf,str,width);	/* Get width digits */
	buf[width] = '\0';	/*  and append a NUL */

	*num = strtoul(buf, &str, 10); /* Attempt a conversion */

	if ((str-buf) == 0)		/* Were any digits converted? */
		return 0;

	return 1;		/* SUCCESS ! */
}
    

static char *
parse(char *template, char *string, struct tm *timeptr)
{
	while((*template != '\0') && (*template != '\n')) {
		if(*template == '%') 
			string = interpret(++template, string, timeptr);
		else 
			string = expect_char(template, string);
		if(string == NULL) break;
		template++;
	}
	return(string);
}
/*
 *  expect_char :  
 *		returns a pointer to next character in string if 
 *		expected character (template) is found, otherwise
 *		returns NULL
 *		
 *		White space is ignored.
 */
static char *
expect_char(char *template, char *string)
{
	while( isspace(*string) )
		string++;

	if(isspace(*template)) return(string);

	if(tolower(*template) == tolower(*string))
		return(++string);
	else
		return(NULL);
}

/*
 *  expect_str :
 *		returns pointer to next character in string following
 *		expected string (template) if expected string is found,
 * 		otherwise returns NULL
 */
static char *
expect_str(char *template, char *string)
{
	while(*template != '\0') {
		string = expect_char(template, string);
		if(string == NULL) break;
		template++;
	}
	return(string);
}

static char *
interpret(char *descriptor, char *string, struct tm *timeptr)
{
	int i;
	char *tmp_str = NULL;

	switch(*descriptor) {
	case '%':		/* Literal '%' embedded in string */ 
		if (*string != '%')
			string = NULL;
		break;

	case 'a':		/* Abbreviated day of week */
		for(i=0; i <= 6; i++) {
			tmp_str = expect_str(nl_langinfo(getdate_aday[i]),
					     string);
			if(tmp_str) {
				timeptr->tm_wday = i;
				break;
			}
		}
		string = tmp_str;
		break;

	case 'A':		/* Full day of week */
		for(i=0; i <= 6; i++) {
			tmp_str = expect_str(nl_langinfo(getdate_day[i]),
					     string);
			if(tmp_str) {
				timeptr->tm_wday = i;
				break;
			}
		}
		string = tmp_str;
		break;

	case 'b':		/* Abbreviated month */
	case 'h':
		for(i=0; i <= 11; i++) {
			tmp_str = expect_str(nl_langinfo(getdate_amon[i]),
					     string);
			if(tmp_str) {
				timeptr->tm_mon = i ;
				break;
			}
		}
		string = tmp_str;
		break;

	case 'B':		/* Full month */ 
		for(i=0; i <= 11; i++) {
			tmp_str = expect_str(nl_langinfo(getdate_mon[i]),
					     string);
			if(tmp_str) {
				timeptr->tm_mon = i ;
				break;
			}
		}
		string = tmp_str;
		break;

	case 'c':		/* date and time format */
		string = parse(nl_langinfo(D_T_FMT), string, timeptr);
		break;

	case 'd':		/* day of month (01-31, leading zero opt. */
	case 'e':
		if (getdigits(string,&i,2))
			string +=2;
		else if (getdigits(string,&i,1))
			string++;
		else {
			string = NULL;
			break;
		}
		if (i >= 1 && i <= 31)
			timeptr->tm_mday = i;
		else
			string = NULL;
		break;

	case 'D':		/* Date formatted as %m/%d/%y  */
		string = parse("%m/%d/%y", string, timeptr);
		break;

	case 'H':		/* hour 00-23 */
		if (getdigits(string, &i, 2) && i >= 0 && i <= 23) {
			string += 2;
			timeptr->tm_hour += i;
		} else
			string = NULL;
		break;

	case 'I':		/* hour 01-12 */
		if (getdigits(string, &i, 2) && i>=1 && i<=12) {
			string += 2;
			timeptr->tm_hour = i;
		} else
			string = NULL;
		break;

	case 'm':		/* month number (01-12) */
		if (getdigits(string, &i, 2))
			string += 2;
		else if (getdigits(string, &i, 1))
			string += 1;
		else {
			string = NULL;
			break;
		}

		if (i>=1 && i<=12)
			timeptr->tm_mon = i-1;
		else
			string = NULL;
		break;

	case 'M':		/* Minute (00-59) */
		if (getdigits(string, &i, 2) && i>=0 && i<=59 ) {
			string += 2;
			timeptr->tm_min = i;
		} else
			string = NULL;
		break;

	case 'S':		/* Seconds (00-59) */
		if (getdigits(string, &i, 2) && i<=59 ) {
			string += 2;
			timeptr->tm_sec = i;
		} else
			string = NULL;
		break;
	case 'n':		/* Match a newline with white space */
		while (isspace(*string) && *string != '\n')
			string++;
		if (*string++ != '\n')
			string = NULL;
		break;

	case 'p':		/* Locale's AM or PM string */
		tmp_str = expect_str(nl_langinfo(PM_STR), string);
		if(tmp_str) {
			if((timeptr->tm_hour + 12) < 24) {
				timeptr->tm_hour += 12;
			}
			string = tmp_str;
		} else
			string = expect_str(nl_langinfo(AM_STR), string);
		break;
	case 'R':		/* Time as %H:%M */
		string = parse("%H:%M", string, timeptr);
		break;

	case 'r':		/* time as %I:%M:%S %p */
		string = parse("%I:%M:%S %p", string, timeptr);
		break;

	case 't':		/* Match white-space up to a tab */
		while (isspace(*string) && *string != '\t')
			string++;
		if (*string++ != '\t')
			string = NULL;
		break;

	case 'T':		/* time as %H:%M:%S */
		string = parse("%H:%M:%S", string, timeptr);
		break;

	case 'w':		/* weekday number (0=Sun - 6) */
		if (getdigits(string, &i, 1) && i<= 6 && i>=0) {
			string++;
			timeptr->tm_wday = i;
		} else
			string = NULL;
		break;

	case 'x':		/* locale's appropriate date */
		string = parse(nl_langinfo(D_FMT), string, timeptr);
		break;

	case 'X':		/* locale's appropriate time format */
		string = parse(nl_langinfo(T_FMT), string, timeptr);
		break;

	case 'Y':		/* Year with century */
		if (getdigits(string, &i, 4) && i >= 1900) {
			timeptr->tm_year = i - 1900;
			string += 4;
		} else
			string = NULL;
		break;

	case 'y':		/* Year without century */
		if (getdigits(string, &i, 2)) {
			string += 2;
			timeptr->tm_year = i;
		} else
			string = NULL;
		break;

	case 'Z':		/* time zone name */
		TS_LOCK(&_ctime_rmutex);
		tmp_str = expect_str(tzname[0], string);
		if(tmp_str != NULL) {
			timeptr->tm_isdst = 0;
			string = tmp_str;
		} else {
			string = expect_str(tzname[1], string);
			if(string != NULL) timeptr->tm_isdst = 1;
		}
		TS_UNLOCK(&_ctime_rmutex);
		break;

	default:
		string = NULL;
		break;
	}

	return(string);
}

static void 
clr_tm(struct tm *timeptr)
{ 
	timeptr->tm_sec = 0;
	timeptr->tm_min = 0;
	timeptr->tm_hour = 0;
	timeptr->tm_mday = 0;
	timeptr->tm_mon = -1;
	timeptr->tm_year = 0;
	timeptr->tm_wday = -1;
	timeptr->tm_yday = 0;
	timeptr->tm_isdst = -1;
}

static int
fill_tm(struct tm *timeptr)
{
#ifdef  _THREAD_SAFE
	struct tm local_timebuf;
	struct tm *local_timeptr = &local_timebuf;
#else
	struct tm *local_timeptr;
#endif	/* _THREAD_SAFE */
	struct tm	tmp_timebuf;
	time_t epoch;
	long givenmask = 0;

	epoch = time(NULL);
#ifdef  _THREAD_SAFE
	(void) localtime_r(&epoch, local_timeptr);
#else
	local_timeptr = localtime(&epoch);
#endif	/* _THREAD_SAFE */

	if(timeptr->tm_sec != 0) givenmask |= SECONDS;
	if(timeptr->tm_min != 0) givenmask |= MINUTES;
	if(timeptr->tm_hour != 0) givenmask |= HOURS;

	/* If no hour, minute and second are given
	 * assume the localtime values.
	 */
	if (((SECONDS | MINUTES | HOURS) & givenmask) == 0) {
		timeptr->tm_sec = local_timeptr->tm_sec;
		timeptr->tm_min = local_timeptr->tm_min;
		timeptr->tm_hour = local_timeptr->tm_hour;
	}

	/* Set defaults to localtime values.
	 */
	if (timeptr->tm_mday == 0)
		timeptr->tm_mday = local_timeptr->tm_mday;
	else
		givenmask |= MDAY;
	if (timeptr->tm_mon < 0)
		timeptr->tm_mon = local_timeptr->tm_mon;
	else
		givenmask |= MONTH;
	if (timeptr->tm_year == 0)
		timeptr->tm_year = local_timeptr->tm_year;
	else
		givenmask |= YEAR;

	/* tm_wday is not used except to work out tm_mday
	 */
	if (timeptr->tm_wday >= 0)
		givenmask |= WDAY;

	/*
	 * We ignore boundaries and assume mktime() will fix them.
	 */
	switch (givenmask & (WDAY | MDAY | MONTH | YEAR)) {
	case WDAY	:
		/* Only the weekday is specified
		 *	if requested day has passed make it next week
		 */
		timeptr->tm_mday +=
			(timeptr->tm_wday - local_timeptr->tm_wday + 7) % 7;
		break;

	case MDAY | WDAY	:
		/* FALLTHROUGH */

	case MDAY	:
		/* Only the monthday is specified
		 *	if requested day has passed make it next month
		 */
		if (timeptr->tm_mday < local_timeptr->tm_mday)
			timeptr->tm_mon++;
		break;

	case MONTH	:
		/* Only the month is specified
		 *	set day to 1st of month
		 */
		timeptr->tm_mday = 1;
		/* FALLTHROUGH */

	case MONTH | MDAY	:
	case MONTH | WDAY	:
	case MONTH | MDAY | WDAY	:
		/* Month is specified
		 *	if requested month has passed make it next year
		 */
		if (timeptr->tm_mon < local_timeptr->tm_mon)
			timeptr->tm_year++;
		break;

	case YEAR	:
		/* Only the year is specified
		 *	assume 1st day of 1st month
		 */
		timeptr->tm_mday = 1;
		timeptr->tm_mon = 0;
		break;

	case NONE	:
		/* No date is specified
		 *	if requested time has passed make it tomorrow
		 */
		if (timeptr->tm_hour * 3600
		    + timeptr->tm_min * 60 + timeptr->tm_sec
		    < local_timeptr->tm_hour * 3600
		      + local_timeptr->tm_min * 60 + local_timeptr->tm_sec)
			timeptr->tm_mday++;

	default	:
		/* Some combination of values
		 */
		break;
	}

	/* This is ugly.
	 * We need to calculate tm_wday the hard way iff tm_mday was not
	 * already set and we were given a month or year.
	 * In this case tm_mday will be first tm_wday
	 * in the (timeptr) month.
	 * We have to do this _after_ any tm_year adjustment.
	 */
	if (!(givenmask & MDAY)
	    && (givenmask & WDAY) && (givenmask & (MONTH | YEAR)))
		timeptr->tm_mday = calc_day(timeptr);

	/* We now have an timeptr with the essential values for our
	 * target date. Use mktime() to fix it up correctly.
	 */
	tmp_timebuf = *timeptr;
	(void)mktime(timeptr);


	/* Finally we check that the converted time hasn't had to
	 * adjust any of the values the user specified.
	 */
	if ((givenmask & WDAY) && timeptr->tm_wday != tmp_timebuf.tm_wday
	    || (givenmask & MDAY) && timeptr->tm_mday != tmp_timebuf.tm_mday
	    || (givenmask & MONTH) && timeptr->tm_mon != tmp_timebuf.tm_mon
	    || (givenmask & YEAR) && timeptr->tm_year != tmp_timebuf.tm_year)
		return (8);

	return (0);
} 

static int
check_tm(struct tm *timeptr, int getdate_mlen[])
{
	int ret_val = 0;

	if(timeptr->tm_mday > getdate_mlen[timeptr->tm_mon]) ret_val = 8;

	return(ret_val);
}

static int 
calc_day(struct tm *timeptr)
{
	int i,d, jan1_day;
	int getdate_mlen[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        d = jan1(1900 + timeptr->tm_year);
        switch((jan1(1900 + timeptr->tm_year + 1) + 7 - d) % 7) {
        /*
         *      non-leap year
         */
        case 1:
                getdate_mlen[1] = 28;
                break;
        /*
         *      leap year
         */
	case 2:
		break;
        /*
         *      1752
         */
        default:
                getdate_mlen[9] = 19;
                break;
        }

	jan1_day = d;
        for(i=0; i < timeptr->tm_mon; i++) {
                d += getdate_mlen[i];
        }

	d %= 7;
	return ((timeptr->tm_wday - d + 7) % 7 + 1);
}

/*
 * NAME: jan1
 *
 * FUNCTION: returns the day of the week of Jan 1 of the given year
 *
 * RETURN VALUE DESCRIPTION:  0 through 6 correspond to Sunday through Saturday
 *
 */

static int jan1(int yr)
{
        register int y, d;

/*
 *      normal gregorian calendar has
 *      one extra day per four years
 */

        y = yr;
        d = 4+y+(y+3)/4;

/*
 *      julian calendar is the
 *      regular gregorian
 *      less three days per 400
 */

        if(y > 1800) {
                d -= (y-1701)/100;
                d += (y-1601)/400;
        }

/*
 *      great calendar changeover instant
 */

        if(y > 1752)
                d += 3;

        return(d%7);
}

/*
 * 	
 *	Routine: getdate
 *
 * 	Function: converts user supplied string into date and time structure
 *
 *	Arguments:
 * 		start_string	character pointer to start of user supplied
 *				string.
 *	Returns:
 *		Pointer to "struct tm" if successful, otherwise NULL.
 * 
 *	Errors:
 *		1	DATEMSK environment variable is null or undefined
 *		2	the template file cannot be opened for reading
 *		3	failed to get file status information
 *		4	the template file is not a regular file
 *		5	an error encountered while reading template file
 *		6	memory allocation failed
 *		7	there is no line in the template that matches the input
 *		8	invalid input specification
 */

#ifdef  _THREAD_SAFE
struct tm *
getdate_r(const  char *start_string, struct tm *time_structp, int *getdate_errp)
{
	char *result;
#else
struct tm *
getdate(const  char *start_string)
{
	static struct tm time_struct, *time_structp = &time_struct;
	int *getdate_errp = &getdate_err;
#endif	/* _THREAD_SAFE */
	struct stat stat_buf; 
	FILE *fd;
	int ret;
	char *datemsk, *string = NULL, *template;
	char buffer[BUFSIZE];

	(void) tzset();

	datemsk = getenv("DATEMSK");
	if((datemsk == NULL) || (*datemsk == '\0')) {
		*getdate_errp = 1;
		return(NULL);
	}

	fd = fopen(datemsk,"r");
	if(fd == NULL) {
		*getdate_errp = 2;
		return(NULL);
	} 
	if(fstat(fileno(fd), &stat_buf) < 0) {
		*getdate_errp = 3;
		return(NULL);
	}
	if(!(S_ISREG(stat_buf.st_mode))) { 
		*getdate_errp = 4;
		return(NULL);
	}

	while (template = fgets(buffer, BUFSIZE, fd)) {
		string = start_string;
    		clr_tm(time_structp); 
		string = parse(template, string, time_structp);
		if(string == NULL)
			continue; /* Not a match, try next template */
		while (isspace(*string))
			string++; /* Strip trailing white-space */
		if (*string == '\0') break; /* Match! */
	}
	if (ferror(fd)) {
	    *getdate_errp = 5;
	    fclose(fd);
	    return (NULL);
	}

	fclose(fd);
	if(string == NULL) {
		*getdate_errp = 7;
		return(NULL);
	} 
	ret = fill_tm(time_structp);
	if(ret != 0) {
		*getdate_errp = ret;
		return(NULL);
	}
	return(time_structp);
}
