static char sccsid[] = "@(#)84	1.2.1.1  src/bos/usr/ccs/lib/libc/nice.c, libcproc, bos411, 9428A410j 10/20/93 14:30:21";
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: LIBCPROC 
 *
 * FUNCTIONS: nice
 *
 * ORIGINS: 26 27 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */


#include <sys/time.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/errno.h>
#include <limits.h>
#include "ts_supp.h"
#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _nice_rmutex;
#define RETURN(val)	return(TS_UNLOCK(&_nice_rmutex), (val))
#else
#define RETURN(val)	return((val))
#endif

/*
 *
 * FUNCTION: Nice adds an increment value to the nice value of the
 *	calling process.  Nice will fail and not change the nice value if
 *	the increment value is negative and the effective user ID of the
 *	calling process does not have SET_PROC&US1.PRIORITY system privilege.
 *	Nice is restricted to setting priorities in the range 0 to 39.
 *
 * PARAMETERS:
 *	incr	- an integer value by which the process's nice value
 *		  is changed.
 *
 * NOTES: The setpriority system call does the hard work of checking
 *	privilege, enforcing system limits, and changing the process's
 *	nice value.
 *
 *	Setpriority accepts nice values in the range of -20 to 20, while
 *	nice uses a range of 0 to 39.  However, no rescaling is necessary
 *	since nice deals in changing the priority by some increment.
 *
 * RETURN VALUE DESCRIPTION: Nice returns the new nice value minus 20.
 */                                                                   

/*
 *
 * get/setpriority are the kernel (libc) system call stubs which deal
 * in range [0..40].
 */
int
nice(int incr)
{
	int prio;
	int saverr;

	TS_LOCK(&_nice_rmutex);

	saverr = errno;
	errno = 0;
	prio = getpriority(PRIO_PROCESS, 0);
	if (prio == -1 && errno) {
		RETURN (-1);
        }

	errno = saverr;

	if (geteuid() != 0 && (incr < 0 || incr > 2*NZERO -1)) {
		errno = EPERM;		/* non priviledged process and incr */
		RETURN (-1);		/* is negative or out of range.     */ 
	}
		
	if (prio + incr > PRIO_MAX - 1)		/* enforce range [0..39] */
		incr = PRIO_MAX - 1 - prio;

	if (setpriority(PRIO_PROCESS, 0, prio + incr) < 0){
		if (errno == EACCES)
		        errno = EPERM;
		RETURN (-1);
	}
	prio = getpriority(PRIO_PROCESS, 0);
	RETURN(prio);
}
