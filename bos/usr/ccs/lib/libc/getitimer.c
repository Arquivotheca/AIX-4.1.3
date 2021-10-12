static char sccsid[] = "@(#)24	1.12  src/bos/usr/ccs/lib/libc/getitimer.c, libctime, bos411, 9428A410j 5/23/94 17:43:47";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: getitimer 
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

#include <sys/time.h>
#include <sys/timer.h>
#include <sys/errno.h>

/*
 * NAME:  getitimer
 *                                                                    
 * FUNCTION:  Get the value of an interval timer.
 *                                                                    
 * EXECUTION ENVIRONMENT:  
 *	
 *	This routine may only be called from a process.
 * 
 * NOTES:  
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  0 upon successful completion, -1 upon error
 *	with errno set as follows:
 *	EFAULT:	The value parameter specified a bad address.
 *	EINVAL:	The which parameter specified an illegal value.
 *
 * EXTERNAL PROCEDURES CALLED:
 */  
int
getitimer(int which, struct itimerval *value)
{
	register int rv;	/* return value from getinterval()	*/
	struct itimerstruc_t it;/* timer value parameter for POSIX	*/
	int oerrno = errno;	/* value of errno on entry		*/

	if((!(TIMERID_ISBSD(which))) && 
		(which != ITIMER_REAL) && (which != ITIMER_REAL1))  {
		errno = EINVAL;
		return(-1);
	}

	if (value == NULL) {
		errno=EFAULT;
		return(-1);
	}

	rv = getinterval((timer_t)which, &it);

	if(rv == 0)  {
		value->it_value.tv_sec = it.it_value.tv_sec;
		value->it_value.tv_usec = it.it_value.tv_nsec / 1000;
		value->it_interval.tv_sec = it.it_interval.tv_sec;
		value->it_interval.tv_usec = it.it_interval.tv_nsec / 1000;
	}
	else  {
		if(errno == EINVAL)  {
			/*
			 *  The timer is not defined according to the 
			 *  underlying POSIX service, which, for BSD, means
			 *  the timer is not set.
			 */
			value->it_value.tv_sec = 0;
			value->it_value.tv_usec = 0;
			value->it_interval.tv_sec = 0;
			value->it_interval.tv_usec = 0;
			errno = oerrno;	/* restore value of errno at entry */
			return(0);
		}
	}
	return(rv);
}
