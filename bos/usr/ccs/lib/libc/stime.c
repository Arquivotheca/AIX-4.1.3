static char sccsid[] = "@(#)26	1.7  src/bos/usr/ccs/lib/libc/stime.c, libctime, bos411, 9428A410j 6/16/90 01:34:30";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: stime 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/time.h>		/* POSIX timer structure declaration	*/
#include <sys/errno.h>		/* declaration of errno variable and errnos*/

/*
 * NAME:  stime
 *                                                                    
 * FUNCTION:  Set the system time.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine may only be called under a process.
 *
 *	It does not page fault 
 *                                                                   
 * NOTES:  The settimer() system call (POSIX) is actually used to implement
 *		this function.  settimer() must check privilege and set
 *		errno as appropriate.
 *
 * DATA STRUCTURES:  None
 *
 * RETURN VALUE DESCRIPTION:  The return value from settimer():  0 upon
 *	successful completion, -1 upon error with errno set as follows:
 *	EFAULT:  An argument address referenced invalid memory;
 *	EPERM:   The requesting process does not have the appropriate 
 *		 privilege to set the system time;
 *	EINVAL:  The type of timer passed as a parameter to settimer()
 *		 does not specify a known timer;
 *	EINVAL:  The value passed as the time parameter specifies an invalid
 *		 system time.
 */
int
stime(tp)
register long	*tp;				/* address of clock value */
{
	struct timestruc_t t;

	if(*tp < MIN_SECS_SINCE_EPOCH)  {
		errno = EINVAL;
		return(-1);
	}

	t.tv_sec = *tp;
	t.tv_nsec = 0;

	return(settimer(TIMEOFDAY, &t));
}
