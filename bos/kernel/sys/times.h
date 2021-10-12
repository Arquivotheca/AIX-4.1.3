/* @(#)44	1.9  src/bos/kernel/sys/times.h, sysproc, bos411, 9428A410j 1/12/93 18:23:09 */

/*
 * COMPONENT_NAME: SYSPROC 
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_TIMES
#define _H_TIMES

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>			/* for clock_t */
#endif

#ifdef _POSIX_SOURCE

/*
 * POSIX requires that certain values be included in times.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

/*
 * Structure returned by times()
 */
struct tms {
	clock_t	tms_utime;		/* user time */
	clock_t	tms_stime;		/* system time */
	clock_t	tms_cutime;		/* user time, children */
	clock_t	tms_cstime;		/* system time, children */
};

#ifndef	_NONSTD_TYPES
#ifdef _NO_PROTO
extern clock_t times();
#else
extern clock_t times(struct tms *);
#endif /* _NO_PROTO */
#endif	/* _NONSTD_TYPES */

#endif /* _POSIX_SOURCE */
#endif	/* _H_TIMES */
