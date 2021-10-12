/* @(#)35	1.7  src/bos/kernel/sys/timeb.h, sysproc, bos411, 9428A410j 3/4/94 10:03:51 */

#ifndef _H_TIMEB
#define _H_TIMEB

/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef _H_TIME
#include <time.h>	/* for time_t */
#endif /* _H_TIME */

struct timeb {
	time_t		time;		/* Seconds */
	unsigned short	millitm;	/* milliseconds */
	short		timezone;	/* Local timezone in minutes */
	short		dstflag;	/* TRUE if DST is in effect */
};

#ifdef _NO_PROTO
extern int ftime();
#else /* _NO_PROTO */
extern int ftime(struct timeb *);
#endif /* _NO_PROTO */

#endif /* _H_TIMEB */
