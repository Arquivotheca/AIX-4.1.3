static char sccsid[] = "@(#)68	1.8  src/bos/usr/ccs/lib/libc/settimeofday.c, libctime, bos411, 9428A410j 6/16/90 01:34:25";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: settimeofday 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
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
 * NAME:  settimeofday
 *                                                                    
 * FUNCTION: Set the system date and time.
 *
 * EXECUTION ENVIRONMENT:  
 *	This routine may only be called by a process.
 * 
 * NOTES:  An assumption is made that the settimer() system call does 
 *	appropriate authority checking.  That is, if the caller does not
 *	have the appropriate authority to set the system time, settimer()
 *	will fail and this routine will return before setting the 
 *	"timezone" and "daylight" variables.
 *
 * DATA STRUCTURES:  
 *
 * RETURN VALUE DESCRIPTION:  The return value from the settimer() system
 *	call (0 upon success, -1 upon failure) with errno set as follows:
 *
 *	EFAULT:	An argument referenced invalid memory.
 *	EPERM:	A user other than superuser attempted to set the time.
 *
 * EXTERNAL PROCEDURES CALLED:  settimer
 */  
int 
settimeofday(tp, tzp)
struct timeval *tp;
struct timezone *tzp;
{
	register int	rv;		/* return value from settimer()*/
	struct timestruc_t t;		/* POSIX timer structure	*/
	extern long timezone;
	extern int daylight;

	t.tv_sec = tp->tv_sec;
	t.tv_nsec = tp->tv_usec * 1000;
	if((rv = settimer(TIMEOFDAY, &t)) != 0)  {
		return(rv);
	}
	if(tzp)  {
		timezone = tzp->tz_minuteswest * 60;
		daylight = tzp->tz_dsttime;
	}
	return(rv);
}
