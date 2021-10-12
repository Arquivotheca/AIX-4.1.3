static char sccsid[] = "@(#)20	1.2  src/bos/kernel/proc/timevalops.c, sysproc, bos411, 9428A410j 4/1/93 13:40:56";
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: timevaladd
 *		timevalfix
 *		
 *
 *   ORIGINS: 27,26
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)kern_resource.c	7.1 (Berkeley) 6/5/86
 */                                                                   

#include "sys/types.h"
#include "sys/time.h"

/*
 * Add and subtract routines for timevals.
 * N.B.: subtract routine doesn't deal with
 * results which are before the beginning,
 * it just gets very confused in this case.
 * Caveat emptor.
 */
timevalfix( struct timeval *t1)
{

	if (t1->tv_usec < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000000;
	}
	if (t1->tv_usec >= 1000000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000000;
	}
}

timevaladd( struct timeval *t1, struct timeval *t2)
{

	t1->tv_sec += t2->tv_sec;
	t1->tv_usec += t2->tv_usec;
	timevalfix(t1);
}
