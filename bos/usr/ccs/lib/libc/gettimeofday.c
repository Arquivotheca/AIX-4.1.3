static char sccsid[] = "@(#)36	1.9  src/bos/usr/ccs/lib/libc/gettimeofday.c, libctime, bos411, 9428A410j 3/4/94 10:30:01";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: gettimeofday 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include	<sys/time.h>

/*
 * NAME:  gettimeofday
 *                                                                    
 * FUNCTION: Get the system date and time.
 *                                                                    
 * EXECUTION ENVIRONMENT:  
 *	This routine imay only be called from a process.
 * 
 * NOTES:  
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  The return value from the gettimer() system
 *	call (0 upon success, -1 upon failure) with errno set as follows:
 *
 *	EFAULT:	An argument address referenced invalid memory.
 *
 * EXTERNAL PROCEDURES CALLED:  gettimer
 */  
int 
gettimeofday(struct timeval *tp, void *tzp_arg)
{
	register int	rv;		/* gettimer() return value	*/
	struct timestruc_t t;		/* POSIX time structure		*/
	extern long timezone;
	extern int daylight;
	extern void tzset();
	struct timezone *tzp = (struct timezone *)tzp_arg;

	if((rv = gettimer(TIMEOFDAY, &t)) != 0)  {
		return(rv);
	}
	tp->tv_sec = t.tv_sec;
	tp->tv_usec = t.tv_nsec / 1000;
	if(tzp)  {
		tzset();
		tzp->tz_minuteswest = timezone / 60;
		tzp->tz_dsttime = daylight;
	}
	return(rv);
}
