/* @(#)72	1.29.1.8  src/bos/usr/include/time.h, libctime, bos411, 9428A410j 7/8/94 11:02:46 */
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

#ifndef _H_TIME
#define _H_TIME

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 *
 *      The ANSI standard requires that certain values be in time.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present.
 *
 *      This header includes all the ANSI required entries.  In addition
 *      other entries for the AIX system are included.
 *
 */
#ifdef _ANSI_C_SOURCE

/* The following definitions are required to be in time.h by ANSI */

#ifndef NULL
#define NULL	0			/* p79268 */
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long 	size_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef int		clock_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef long		time_t;
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC	1000000		/* microseconds in sec */
#endif

struct	tm {	/* see ctime(3) */
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
};

#ifdef _NO_PROTO
    extern size_t 	strftime();
    extern clock_t 	clock();
    extern double 	difftime();
    extern time_t 	mktime();
    extern time_t 	time();
    extern char 	*asctime();
    extern char 	*ctime();
    extern struct tm    *gmtime();
    extern struct tm    *localtime();

/* REENTRANT FUNCTIONS */
#ifdef _THREAD_SAFE
/* See comments in stdlib.h on _AIX32_THREADS */
#if _AIX32_THREADS
    extern int 	asctime_r();
    extern int	ctime_r();
    extern int 	gmtime_r();
    extern int 	localtime_r();
#else	/* POSIX 1003.4a Draft 7 prototype */
    extern char 	*asctime_r();
    extern char		*ctime_r();
    extern struct tm	*gmtime_r();
    extern struct tm	*localtime_r();
#endif /* _AIX32_THREADS */
#endif /* _THREAD_SAFE */

#else	/* use POSIX required prototypes */
    extern size_t 	strftime(char *, size_t, const char *, const struct tm *);
    extern clock_t 	clock(void);
    extern double 	difftime(time_t, time_t);
    extern time_t 	mktime(struct tm *);
    extern time_t 	time(time_t *);
    extern char 	*asctime(const struct tm *);
    extern char 	*ctime(const time_t *);
    extern struct tm *gmtime(const time_t *);
    extern struct tm *localtime(const time_t *);

/* REENTRANT FUNCTIONS */
#ifdef _THREAD_SAFE
#if _AIX32_THREADS
    extern int 	asctime_r(const struct tm *, char *, int);
    extern int	ctime_r(const time_t *, char *, int);
    extern int	gmtime_r(const time_t *, struct tm *);
    extern int 	localtime_r(const time_t *, struct tm *);
#else	/* POSIX 1003.4a Draft 7 prototypes */
    extern char		*asctime_r(const struct tm *, char *);
    extern char		*ctime_r(const time_t *, char *);
    extern struct tm	*gmtime_r(const time_t *, struct tm *);
    extern struct tm	*localtime_r(const time_t *, struct tm *);
#endif /* _AIX32_THREADS */

#endif	/* _THREAD_SAFE */

#endif /* _NO_PROTO */

#endif /*_ANSI_C_SOURCE */
 
/*
 *   The following are values that have historically been in time.h.
 *
 *   They are NOT part of the ANSI defined time.h and therefore are
 *   not included when _ANSI_C_SOURCE is defined.
 *
 */

#ifdef _POSIX_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#ifndef CLK_TCK
#define CLK_TCK   100       /* clock ticks/second, >= 10 */
#endif

extern char *tzname[];

#ifdef _NO_PROTO
    extern void tzset();
#else
    extern void tzset(void);
#endif /* _NO_PROTO */

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE
    extern long timezone;
    extern int daylight;
#ifdef _NO_PROTO
    extern char         *strptime();
#else /* _NO_PROTO */
    extern char         *strptime(const char *, const char *, struct tm *);
#endif /* _NO_PROTO */
#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE
#ifdef _NO_PROTO
	extern struct tm *getdate();
#else /* _NO_PROTO */
	extern struct tm *getdate(const char *);
#endif /* _NO_PROTO */
#endif /* _ALL_SOURCE */

#ifdef _ALL_SOURCE

#ifndef _H_STDDEF
#include <stddef.h>
#endif

#define	TIMELEN	26
/*  Suggested default length of time/date buffer */
#   define NLTBMAX	64
#   ifdef _NO_PROTO
	extern unsigned char *NLctime(), *NLasctime();
	extern char *NLstrtime();
    	extern size_t       wcsftime();
#   else /* ~ _NO_PROTO */
	extern unsigned char *NLctime(long *);
	extern unsigned char *NLasctime(struct tm *);
	extern char *NLstrtime(char *, size_t, const char *, const struct tm *);
    	extern size_t       wcsftime(wchar_t *, size_t, const char *, const struct tm *);
#   endif /* _NO_PROTO */
#endif /* _ALL_SOURCE */

#endif /* _H_TIME */
