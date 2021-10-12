static char sccsid[] = "@(#)78	1.2  src/bos/usr/ccs/lib/libc/clock.c, libcproc, bos411, 9428A410j 10/20/93 14:26:59";
/*
 *   COMPONENT_NAME: LIBCPROC
 *
 *   FUNCTIONS: TIMES
 *		clock
 *
 *   ORIGINS: 3,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* clock.c,v $ $Revision: 1.8.2.2 $ (OSF) */

/*
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 */

#include <sys/times.h>
#include <time.h>		/* for CLK_TCK (clock ticks per second) */

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_clock_rmutex;
#endif	/* _THREAD_SAFE */

/*
 * TIMES computes full amount of cpu time used including user, sys, child user,
 * and child sys times...
 */
#define TIMES(B)	(B.tms_utime+B.tms_stime+B.tms_cutime+B.tms_cstime)

static clock_t first = (clock_t) -1; /* cpu time used after first call	*/

/*
 * NAME:	clock
 *
 * FUNCTION:	clock - return CPU time used
 *
 * NOTES:	Clock returns the amount of CPU time used (in
 *		microseconds) since the first call to clock.
 *
 * RETURN VALUE DESCRIPTION:	mount of CPU time used (in
 *		microseconds) since the first call to clock
 */
clock_t
clock(void)
{
	struct tms buffer;

	TS_LOCK(&_clock_rmutex);

	/* set first if first time in */
	if((times(&buffer) != (clock_t)(-1)) && (first == (clock_t)(-1)))
		first = TIMES(buffer);

	TS_UNLOCK(&_clock_rmutex);

	/*
	 * compute difference, convert to microseconds.
	 * struct tms elements are in CLK_TCK's...
	 */
	return((TIMES(buffer) - first) * (clock_t)(1000000L/CLK_TCK));
}
