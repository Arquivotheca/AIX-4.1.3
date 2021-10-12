static char sccsid[] = "@(#)28	1.2  src/bos/usr/ccs/lib/libc/nanotimers.c, libctime, bos411, 9428A410j 10/20/93 14:30:16";
#ifdef _POWER_PROLOG_
/*
 * COMPONENT_NAME: (LIBCTIME) Standard C Library Time Management Functions
 *
 * FUNCTIONS: ualarm, usleep
 *
 * ORIGINS: 27,71
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#endif /* _POWER_PROLOG_ */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*LINTLIBRARY*/

#define _TIMESPEC_T_

#include <errno.h>
#include <sys/timers.h>
#include "ts_supp.h"

#define NULL 0
#define NSEC_PER_USEC (1000)
#define NSEC_PER_SEC (1000000000)

#define	SETERR(err)	errno = err

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mtuex _nanotimer_rmutex;
#define RETURN(val)	return(TS_UNLOCK(&_nanotimer_rmutex), (val))

#else /* _THREAD_SAFE */
#define	RETURN(val)	return(val)
#endif /* _THREAD_SAFE */

static pid_t timer_allocated = (pid_t)0;

getclock(int clock_type, struct timespec *tp)
{
	struct timeval berktv;

	if (clock_type != TIMEOFDAY) {
		SETERR(EINVAL);
		return(-1);
	}
	if (gettimeofday(&berktv, NULL) < 0)
		return(-1);

	tp->tv_sec = berktv.tv_sec;
	tp->tv_nsec = berktv.tv_usec * NSEC_PER_USEC;
	return(0);
}

setclock(int clock_type, struct timespec *tp)
{
	struct timeval berktv;

	if (clock_type != TIMEOFDAY) {
		SETERR(EINVAL);
		return(-1);
	}
	if (tp->tv_nsec < 0 || tp->tv_nsec >= NSEC_PER_SEC) {
		SETERR(EINVAL);
		return(-1);
	}

	berktv.tv_sec = tp->tv_sec;
	berktv.tv_usec = tp->tv_nsec / NSEC_PER_USEC;
	return(settimeofday(&berktv, NULL));
}


timer_t
mktimer(int clock_type, int notify_type, struct itimercb *itimercbp)
{
	if (clock_type != TIMEOFDAY) {
		SETERR(EINVAL);
		return((timer_t)-1);
	}
	if (notify_type != DELIVERY_SIGNALS) {
		SETERR(EINVAL);
		return((timer_t)-1);
	}
	
	TS_LOCK(&_nanotimer_rmutex);

	if (timer_allocated == getpid()) {
		SETERR(EAGAIN);
		RETURN((timer_t)-1);
	}
	timer_allocated = getpid();
	RETURN((timer_t)0);
}

rmtimer(timer_t timerid)
{
	struct itimerval btimer;

	TS_LOCK(&_nanotimer_rmutex);

	if (timer_allocated != getpid() || timerid != (timer_t)0) {
		SETERR(EINVAL);
		RETURN(-1);
	}
	timer_allocated = (pid_t)0;

	/* cancel pending timer, if any */

	btimer.it_value.tv_sec = 0;
	btimer.it_value.tv_usec = 0;
	btimer.it_interval.tv_sec = 0;
	btimer.it_interval.tv_usec = 0;
	if (setitimer(ITIMER_REAL, &btimer, NULL) < 0)
		RETURN(-1);

	RETURN(0);
}


reltimer(timer_t timerid, struct itimerspec *value, struct itimerspec *ovalue)
{
	struct itimerval btimer;
	struct itimerval obtimer;
	struct itimerval *obp;

	TS_LOCK(&_nanotimer_rmutex);

	if (timer_allocated != getpid() || timerid != (timer_t)0) {
		SETERR(EINVAL);
		RETURN(-1);
	}
	if (value->it_value.tv_nsec >= NSEC_PER_SEC ||
	    value->it_interval.tv_nsec >= NSEC_PER_SEC ||
	    value->it_value.tv_nsec < 0 ||
	    value->it_interval.tv_nsec < 0) {
		SETERR(EINVAL);
		RETURN(-1);
	}
	btimer.it_value.tv_sec = value->it_value.tv_sec;
	btimer.it_value.tv_usec = value->it_value.tv_nsec / NSEC_PER_USEC;
	btimer.it_interval.tv_sec = value->it_interval.tv_sec;
	btimer.it_interval.tv_usec = value->it_interval.tv_nsec / NSEC_PER_USEC;
	if (setitimer(ITIMER_REAL, &btimer, ovalue ? &obtimer : NULL) < 0)
		RETURN(-1);
	if (ovalue != NULL) {
		ovalue->it_value.tv_sec = obtimer.it_value.tv_sec;
		ovalue->it_value.tv_nsec = obtimer.it_value.tv_usec * NSEC_PER_USEC;
		ovalue->it_interval.tv_sec = obtimer.it_interval.tv_sec;
		ovalue->it_interval.tv_nsec = obtimer.it_interval.tv_usec * NSEC_PER_USEC;
	}
	RETURN(0);
}

