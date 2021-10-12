static char sccsid[] = "@(#)87	1.1  src/bos/usr/ccs/lib/libc/sleep.c, libcproc, bos411, 9428A410j 2/26/91 17:47:03";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sleep
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/timer.h>	/* for the POSIX timer structure defines.	*/

/*
 * NAME:  sleep
 *
 * FUNCTION:  Suspend execution for an interval of time.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine may only be called from a process.
 *
 *
 * NOTES:
 *
 *	1)  sleep() is no longer based upon the alarm signal.  Instead,
 *	    it is now based upon the nsleep() routine.  Using nsleep()
 *	    provides for a more efficient and thus more accurate sleep 
 *	    mechanism.  When sleep() used alarm(), it would do an alarm(0)
 *	    to cancel any pending alarm requests.  This alarm() call would
 *	    return the amount of time left until the SIGALRM would have 
 *	    been generated.  Because the granularity of alarm()'s return
 *	    value was seconds, accuracy was lost resulting in SIGALRM being
 *	    generated sooner than the alarm() call requested.  This accuracy
 *	    loss increased as more sleep()'s were done between the time
 *	    the alarm() was issued and the time the alarm request expired.
 *	    This implementation fixes that.
 *	2)  Because this implementation is not based upon signals, it does
 *	    not alter the signal state of the calling process.  This means
 *	    that signals which are ignored or blocked upon entry to sleep()
 *	    do not cause a early return and remain ignored or blocked after
 *	    sleep() returns.
 *
 * RETURN VALUE DESCRIPTION:  0 if the sleep() function returns because the
 *	requested time has elapsed.  If the sleep() function returns because
 *	of premature arousal due to delivery of a signal, the return value
 *	will be the "unslept" amount (the requested time minus the time
 *	actually slept).
 *
 * EXTERNAL PROCEDURES CALLED:  nsleep()
 */
unsigned int 
sleep(unsigned int sleepsec)
{
	struct	timestruc_t request;	/* requested sleep time		*/
	struct	timestruc_t remain;	/* unslept sleep time		*/

	if(sleepsec == 0)  {
		return(0);
	}

	/*  Initialize all the timer structures for the nsleep() call.  */
	request.tv_sec = sleepsec;
	request.tv_nsec = 0;
	remain.tv_sec = 0;
	remain.tv_nsec = 0;

	if(nsleep(&request, &remain) == -1)  {
		/*
		 *  Return the unslept time to the caller.  Note that if the
		 *  unslept time was not an exact number of seconds (i.e.
		 *  the ns portion of the unslept time is non-zero), then
		 *  we round to nearest second.
		 */
		if(remain.tv_nsec >= 500000000)  {
			return(++remain.tv_sec);
		}
		return(remain.tv_sec);
	}
	return(0);

}
