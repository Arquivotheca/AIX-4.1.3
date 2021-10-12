static char sccsid[] = "@(#)44	1.4  src/bos/usr/ccs/lib/libc/utimer.c, libctime, bos411, 9428A410j 7/11/90 13:16:48";
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions 
 *
 * FUNCTIONS: ualarm, usleep 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*LINTLIBRARY*/

#include <sys/types.h>
#include <sys/timer.h>	/* for the POSIX timer structure defines.	*/
#include <sys/events.h>	/* for the POSIX delivery mechanism defines.	*/
#include <sys/errno.h>	/* for the errno variable and errno defines.	*/
#include <sys/rtc.h>	/* for machine dependent time limits.		*/

/*
 * NAME:  ualarm
 *
 * FUNCTION:  Send a SIGALRM signal to the invoking process in a specified
 *	number of microseconds.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine may only be called from a process.
 *
 * RETURN VALUE DESCRIPTION:  The number of microseconds previously remaining
 *	in the alarm clock.
 *
 * EXTERNAL PROCEDURES CALLED:  incinterval()
 *				gettimerid()
 */
unsigned int 
ualarm(value, interval)
unsigned int value;
unsigned int interval;
{
	register int rv;		/* return value from sys. calls	*/
	register unsigned int arv;	/* actual ret. val. from ualarm	*/
	register timer_t timerid;	/* timer to issue request for	*/
	struct itimerstruc_t itvalue;	/* time for alarm to occur	*/
	struct itimerstruc_t oitvalue;	/* time left for pending alarm	*/
	int saverrno=errno;		/* Errno to restore later */

	itvalue.it_value.tv_sec = value / uS_PER_SECOND;
	itvalue.it_value.tv_nsec = (value % uS_PER_SECOND) * NS_PER_uS;
	itvalue.it_interval.tv_sec = interval / uS_PER_SECOND;
	itvalue.it_interval.tv_nsec = (interval % uS_PER_SECOND) * NS_PER_uS;
	oitvalue.it_value.tv_sec = 0;
	oitvalue.it_value.tv_nsec = 0;
	oitvalue.it_interval.tv_sec = 0;
	oitvalue.it_interval.tv_nsec = 0;

	/*
	 *  If a previous alarm(), ualarm(), or setitimer(ITIMER_REAL) has
	 *  been issued, then the "alarm" timer has been initialized and
	 *  the first call to incinterval() will work.  Otherwise, it has
	 *  not been initialized, -1 is returned, and errno is set to 
	 *  EINVAL.  For the latter case, a call to gettimerid() is needed
	 *  to initialize the "alarm" timer.
	 */
	rv = incinterval(TIMERID_ALRM, &itvalue, &oitvalue);
	if(rv == 0)  {
		TS_TO_uS(oitvalue.it_value, arv);
		return(arv);
	}
	else  {
		if(errno == EINVAL)  {
			timerid = gettimerid(TIMERID_ALRM, DELIVERY_SIGNALS);
			rv = incinterval(timerid, &itvalue, &oitvalue);
			if(rv == 0) {
				TS_TO_uS(oitvalue.it_value, arv);
				errno=saverrno;
				return(arv);
			}
		}
	}
}

/*
 * NAME:  usleep
 *
 * FUNCTION:  Suspend execution for an interval of time.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine may only be called from a process.
 *
 * RETURN VALUE DESCRIPTION:  0 if the usleep() function returns because the
 *	requested time has elapsed.  If the usleep() function returns because
 *	of premature arousal due to delivery of a signal, the return value
 *	will be the "unslept" amount (the requested time minus the time
 *	actually slept).
 *
 * EXTERNAL PROCEDURES CALLED:  nsleep()
 */
unsigned int 
usleep(useconds)
unsigned int useconds;
{
	struct	timestruc_t request;	/* requested sleep time		*/
	struct	timestruc_t remain;	/* unslept sleep time		*/

	if(useconds == 0)  {
		return(0);
	}

	/*  Initialize all the timer structures for the nsleep() call.  */
	request.tv_sec = useconds / uS_PER_SECOND;
	request.tv_nsec = (useconds % uS_PER_SECOND) * NS_PER_uS;
	remain.tv_sec = 0;
	remain.tv_nsec = 0;

	if(nsleep(&request, &remain) == -1)  {
		/*
		 *  Return the unslept number of microseconds to the caller.  
		 *  Note that if the unslept time was not an exact number of
		 *  seconds (i.e. the ns portion of the unslept time is 
		 *  non-zero), then we round UP.
		 */
		if(remain.tv_nsec % NS_PER_uS)  {
			return((remain.tv_nsec / NS_PER_uS) + 1 +
				(remain.tv_sec * uS_PER_SECOND));
		}
		return((remain.tv_nsec / NS_PER_uS) +
			(remain.tv_sec * uS_PER_SECOND));
	}
	return(0);
}
