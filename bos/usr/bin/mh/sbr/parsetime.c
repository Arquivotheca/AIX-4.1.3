static char sccsid[] = "@(#)46	1.7  src/bos/usr/bin/mh/sbr/parsetime.c, cmdmh, bos411, 9428A410j 2/1/93 16:53:25";
/*
 * COMPONENT_NAME: CMDMH parsetime.c
 *
 * FUNCTIONS: breakargs, getval, skipval, shorttime, parsetime,
 *            timearg, lower, numbers, reperror, error, words,
 *            timedmod, noonmid, daymod, weekday, smonth, checktime,
 *            leapyear, construct, adjust
 *
 * ORIGINS: 10 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * char *
 * parsetime(char *buf, time_t *tp);
 *
 * A standard routine to convert a future time (in English) to seconds.
 * Arguments are order-independent (except for suffixes), and words
 * may be shortened to a non-ambiguous abbreviation.  As the time
 * must be in the future, unspecified years, months and days default
 * to the "next" year, month or day if necessary; otherwise the
 * current month, day and hour are used.
 * 
 * type is either TIMES in which days, times are recognised, or just DAYS.
 *
 * Tries hard to give meaningful messages, and make sure the user
 * gets the time she/he wanted!
 *
 * If no error, returns NULL, *tp filled in with time.
 * If error, returns error string, *tp = 0;
 *
 * Syntax:
 *
 *	timespec ::= { time | day | month | year } .
 *	
 *	time ::= [ hour [ ":" min [ ":" second ] ] ] [ timemodifier ] .
 *	
 *	timemodifier ::= "am" | "pm" | "noon" | "midday" | "midnight" | "now" .
 *	
 *	day ::= ( dayofweek [ "week" ] ) | number .
 *	
 *	dayofweek ::= "sunday" | "monday" | "tuesday" | "wednesday" |
 *		      "thursday" | "friday" | "saturday" | "tomorrow" |
 *		      "today" .
 *	
 *	month ::= "january" | "february" | "march" | "april" | "may" | "june" |
 *		  "july" | "august" | "september" | "october" | "november" |
 *		  "december" .
 *	
 *	year ::= "19" number .
 * (really, year must be greater than 1900)
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#ifdef USG
#include <fcntl.h>
#endif
#include <signal.h>
#include <sgtty.h>

#include "mh_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_MH,n,s)

/* libc routines of interest */
char *strchr();
char *malloc();
static timearg(), lower(), numbers(), reperror(), words(), checktime(),
       leapyear(), adjust();
static long construct();

#define CMPN(a, b, n)	(*(a) != *(b) ? *(a) - *(b) : strncmp(a, b, n))

#define SECINWEEK	604800L
#define SECINDAY	 86400L
#define SECINHOUR	  3600L
#define SECINMIN	    60L
#define	DAYSTO1983	(10*365 + 3*366)

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE		/* Always compare to FALSE */
#define TRUE (1)	/* should be !FALSE, but some cpp's won't hack it */
#endif

/* buf is modified - substrings are null terminated */
static void
breakargs(buf, argc, argv)
char *buf;
int *argc;
char *argv[];
{
    register i = 0;

    *argc = 0;
    while (1) {
	while (*buf == '\040' || *buf == '\t' || *buf == '\n')
	    buf++;		/* eat white space */
	if (*buf == '\0')
	    break;
	argv[i++] = buf;
	(*argc)++;
	while ((*buf != '\040') && (*buf != '\t') &&
	    (*buf != '\0') && (*buf != '\n'))
	    buf++;		/* eat nonwhite space */
	if (*buf == '\0')
	    break;
	*buf++ = '\0';
    }
}

#define	NOW 	-1

static timemod(), noonmid(), daymod(), weekday(), smonth();

static struct slist {
    char *s_name;
    int (*s_action)();
    char s_val;
} slist[] = {
    { "am",		timemod, 	0},
    { "pm",		timemod, 	12},
    { "noon",		noonmid, 	12},
    { "midday", 	noonmid, 	12},
    { "midnight", 	noonmid, 	0},
    { "now", 		noonmid, 	NOW},
    { "week",		daymod, 	0},
    { "sunday", 	weekday, 	0},
    { "monday", 	weekday, 	1},
    { "tuesday", 	weekday, 	2},
    { "wednesday", 	weekday, 	3},
    { "thursday", 	weekday, 	4},
    { "friday", 	weekday, 	5},
    { "saturday", 	weekday, 	6},
    { "tomorrow", 	weekday, 	7},
    { "today", 		weekday, 	8},
    { "january", 	smonth,		0},
    { "february", 	smonth,		1},
    { "march", 		smonth,		2},
    { "april", 		smonth,		3},
    { "may", 		smonth,		4},
    { "june", 		smonth,		5},
    { "july", 		smonth,		6},
    { "august", 	smonth,		7},
    { "september", 	smonth,		8},
    { "october", 	smonth,		9},
    { "november", 	smonth,		10},
    { "december", 	smonth,		11},
    { "", 		0, 		0}
};

static char daysinmonth[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int hour, minute, second, day, year, dayofweek, month;
static int settime, setday, setyear, setdayofweek, setmonth;
static int setweek, err, setmod;
static char *errstr;
static char *curarg;
static struct tm *tim;


/* getval is used to get numeric values in short form dates:
 * mm/dd/yy[yy] or dd.mm.yy[yy] or mm dd,yy[yy]
 */
static int
getval(str)
register char *str;
{
    register ans = 0;
    register cnt = 0;

    while (isdigit(*str)) {
	ans *= 10;
	ans += (*str - '0');
	cnt++;
	str++;
    }
    if (cnt == 0 || cnt > 4)
	error(MSGSTR(SHDATE,"Bad value in short form date."));
    return ans;
}

/* skip pass a digit string */
static char *
skipval(str)
register char *str;
{
    while (isdigit(*str))
	str++;
    return str;
}

/* shorttime - parse short forms:
 * mm/dd/yy[yy] or dd.mm.yy[yy] or mm dd,yy[yy]
 * return TRUE if conversion complete (even if error).
 * return FALSE if it didn't look like a short form time.
 */
static int
shorttime(str)
register char *str;
{
    register char *p,
	          *q;
    register tday, tmonth, tyear;

    q = str;
    tday = tmonth = tyear = 0;
    if (strchr(str, ' ') != NULL && strchr(str, ',') != NULL
	    && isdigit((char)*((char *)index(str, ',')-1)) 
    	    && isdigit((char)*((char *)index(str, ',')+1)) ) {
	tmonth = getval(str);
	p = skipval(str);
	if (*p++ != ' ') {
	    error(MSGSTR(BADDATE3,"Invalid mm dd,yy[yy] format."));
	    return TRUE;
	}
	tday = getval(p);
	p = skipval(p);
	if (*p++ != ',') {
	    error(MSGSTR(BADDATE3,"Invalid mm dd,yy[yy] format."));
	    return TRUE;
	}
	tyear  = getval(p);
    } else if (strchr(str, '/') != NULL) {
	tmonth = getval(str);
	p = skipval(str);
	if (*p++ != '/') {
	    error(MSGSTR(BADDATE1,"Invalid mm/dd/yy[yy] format."));
	    return TRUE;
	}
	tday = getval(p);
	p = skipval(p);
	if (*p++ != '/') {
	    error(MSGSTR(BADDATE1,"Invalid mm/dd/yy[yy] format."));
	    return TRUE;
	}
	tyear = getval(p);
    } else if (strchr(str, '.') != NULL) {
	tday = getval(str);
	p = skipval(str);
	if (*p++ != '.') {
	    error(MSGSTR(BADDATE2,"Invalid dd.mm.yy[yy] format."));
	    return TRUE;
	}
	tmonth = getval(p);
	p = skipval(p);
	if (*p++ != '.') {
	    error(MSGSTR(BADDATE2,"Invalid dd.mm.yy[yy] format."));
	    return TRUE;
	}
	tyear = getval(p);
    } else {
	return FALSE;
    }
    if (!err) {		/* if success */
	if (setday++) {
	    reperror(MSGSTR(DAY,"day"));
	} else {
	    day = tday;
	}
	if (setmonth++) {
	    reperror(MSGSTR(MONTH,"month"));
	} else {
	    month = tmonth - 1;		/* month is 0 based */
	}
	if (setyear++) {
	    reperror(MSGSTR(YEAR,"year"));
	} else {
	    if (tyear < 100)
		tyear += (year / 100) * 100;		/* get right century */
	    year = tyear;
	}
    }
    return TRUE;
}

/* return error string, if any */
char *
parsetime(buf, tp)
char *buf;
time_t *tp;
{
    long now, then;
    int argc;
    char *av[40];	/* should be enough - not checked */
    char **argv;


    char *p;

    if ((p = malloc(strlen(buf) + 1)) == NULL) {
	*tp = 0;
	return ((char *)(MSGSTR(MEMORY,"Out of memory.")));
    }
    strcpy(p, buf);

    now = time((long *) 0);
    tim = localtime(&now);

    /*
     * set defaults 
     */
    hour =((now-(SECINDAY*(now/SECINDAY)))/SECINHOUR); /*adjust for timezone*/
    minute = tim->tm_min;
    second = tim->tm_sec;
    day = tim->tm_mday;
    year = tim->tm_year + 1900;
    dayofweek = tim->tm_wday;
    month = tim->tm_mon;

    settime = setday = setyear = setdayofweek = setmonth = 0;
    setweek = err = setmod = 0;
    errstr = NULL;

    if(!shorttime(p)) {		/* try short forms first */
	    breakargs(p, &argc, av);	/* if not, do longer forms */
	    argv = av;
#if 0
    curarg = p;
#endif
	    while (argc--) {
		curarg = *argv++;
		    timearg(curarg);
		if (err) {
		    *tp = 0;
		    return errstr;
		}
	    }
    }
    checktime();
    if (err) {
	*tp = 0;
	return errstr;
    }
    then = construct();
    if (settime == 0 && setday == 0 && setyear == 0 &&
	setdayofweek == 0 && setmonth == 0 && setweek == 0) {
	*tp = 0;
	return ((char *)(MSGSTR(NOPARTS,"No recognizable date parts found.")));
    }
    if (then < now) {
	*tp = 0;
	return ((char *)(MSGSTR(TIMEPAST,"Time is in the past.")));
    }
    *tp = then;
    return NULL;
}

static
timearg(s)
char *s;
{
    lower(s);
    if (isdigit(*s))
	numbers(s);
    else
	words(s);
}

static
lower(s)
register char *s;
{
    while (*s) {
	*s = isupper(*s) ? tolower(*s) : *s;
	s++;
    }
}


static
numbers(s)
register char *s;
{
    register int val;

    val = 0;
    while (isdigit(*s))
	val = val * 10 + *s++ - '0';
    if (val > 1900) {
	if (setyear++) {
	    reperror(MSGSTR(YEAR,"year"));
	} else {
	    year = val;
	}
    } else if (*s == '\0') {
	if (setday++) {
	    reperror(MSGSTR(DAY,"day"));
	} else {
	    day = val;
	}
    } else if (settime++) {
	reperror(MSGSTR(TIME,"time"));
    } else {
	hour = val;
	if (*s == ':') {
	    s++;
	    val = 0;
	    while (isdigit(*s))
		val = val * 10 + *s++ - '0';
	    minute = val;
	    if (*s == ':') {
		s++;
		val = 0;
		while (isdigit(*s))
		    val = val * 10 + *s++ - '0';
		second = val;
	    } else {
		second = 0;
	    }
	} else {
	    minute = second = 0;
	}
    }
    if (*s)
	words(curarg = s);
}


static
reperror(s)
char *s;
{
    error(MSGSTR(REPERROR,"Repeated %s argument: \"%s\""), s, curarg);
}


/* VARARGS1 */

error(s, a1, a2, a3, a4)
char *s;
int a1, a2, a3, a4;
{
    static char buf[80];

    err++;
    (void) sprintf(buf, s, a1, a2, a3, a4);
    strcat(buf, "\n");
    errstr = buf;
}


static
words(s)
char *s;
{
    register struct slist *sp, *found;
    register int size;
    register char *wstart;

    sp = slist;
    wstart = s;
    size = 0;
    while (*s && !isdigit(*s))
	size++, s++;
    found = (struct slist *) 0;
    while (*(sp->s_name)) {
	if (CMPN(sp->s_name, wstart, size) == 0) {
	    if (!found) {
		found = sp;
		if (strlen(sp->s_name) == size)
		    break;	/* otherwise an abbreviation */
	    } else {
		error(MSGSTR(AMBABBR,"Ambiguous abbreviation: \"%.*s\""), size, wstart);
		return;
	    }
	}
	sp++;
    }
    if (found)
	(*(found->s_action))(found->s_val);
    if (*s)
	numbers(curarg = s);
}


static
timemod(val)
int val;
{
    if (!settime)
	error(MSGSTR(AMPMTIME,"Can only use \"am\" or \"pm\" after a time."));
    else if (setmod++)
	reperror(MSGSTR(TIMEMOD,"time modifier"));
    else if (hour < 12)
	hour += val;
    else if (hour > 12)
	error(MSGSTR(AMPMCLOCK,"Can't use \"am\" or \"pm\" with 24 hour clock."));
    else if (val == 0) /* am && hour == 12 */
	hour = 0;	/* 12am correction */
}


static
noonmid(val)
int val;
{
    if (val < 0) {		/* NOW */
	if (settime++)
	    reperror(MSGSTR(TIME,"time"));
    /* let defaults work */
    } else if (setmod++) {	 /* noon, midnight */
	reperror(MSGSTR(TIMEMOD,"time modifier"));
    } else {
	if (!settime)
	    settime++;
	else if (hour != 12 || minute != 0 || second != 0)
	    error(MSGSTR(BADTIME2,"Illegal time: %02d:%02d:%02d %s"), hour, minute, second, curarg);
	hour = val;
	minute = second = 0;
    }
}


static
daymod()
{
    if (setweek++)
	reperror("\b");
    else if (!setdayofweek)
	error(MSGSTR(WEEKDAY,"Can only use \"week\" after a weekday name."));
    else
	dayofweek += 7;
}


static
weekday(val)
int val;
{
    setdayofweek++;
    if (val < 7) {
	dayofweek = val - dayofweek;	/* now a displacement */
	if (dayofweek <= 0)
	    dayofweek += 7;
    } else if (val == 7) {		/* tomorrow */
	dayofweek = 1;
    } else {			/* today */
	dayofweek = 0;
    }
}


static
smonth(val)
int val;
{
    if (setmonth++)
	reperror(MSGSTR(DOM,"day of month"));
    else
	month = val;
}


static
checktime()
{
    register int dim;

    if (year < 1983 || year > 2038)
	error(MSGSTR(BADYEAR,"Year out of range."));
    if (hour > 23 || minute > 59 || second > 59)
	error(MSGSTR(BADTIME1,"Illegal time: %02d:%02d:%02d"), hour, minute, second);
    if (!setdayofweek) {
	dim = daysinmonth[month] + (month == 1 ? leapyear(year) : 0);
	if (day > dim)
	    error(MSGSTR(BADMONTHD,"Month day out of range. (> %d)"), dim);
    }
}


static
leapyear(y)
int y;
{
    return ((y % 4) == 0 && (y % 100) != 0) || (y % 400 == 0);
}


static long 
construct()
{
    register int i, days;

    adjust();
    days = DAYSTO1983;
    for (i = 1983; i < year; i++)
	days += 365 + leapyear(i);
    for (i = 0; i < month; i++)
	days += daysinmonth[i] + (i == 1 ? leapyear(year) : 0);
    days += day - 1;	/* days since 1 Jan 1970 */
    if (setdayofweek && !(setmonth || setyear))
	days += dayofweek;
    return days * SECINDAY + hour * SECINHOUR + minute * SECINMIN + second;
}


static
adjust()
{
    register int dim;

    /*
     * make sure time defaults to the future
     */
    if (setdayofweek || setyear || month > tim->tm_mon)
	return;
    if (month < tim->tm_mon) {
	year++;
	return;
    }
    /*
     * month == tim->tm_mon
     */
    if (day > tim->tm_mday)
	return;
    if (day < tim->tm_mday) {
	if (setmonth || ++month / 12)
	    year++, month %= 12;
	return;
    }
    /*
     * month == tim->tm_mon && day == tim->tm_mday
     */
    if ((long)(hour*SECINHOUR + minute*SECINMIN + second) <
	(long)(tim->tm_hour*SECINHOUR + tim->tm_min*SECINMIN + tim->tm_sec)) {
	dim = daysinmonth[month] + (month == 1? leapyear(month): 0);
	if (setday || ++day / dim) {
	    if (setmonth || ++month / 12)
		year++, month %= 12;
	    day %= dim;
	}
	return;
    }
}
