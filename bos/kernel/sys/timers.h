/* @(#)29	1.3  src/bos/kernel/sys/timers.h, incstd, bos411, 9428A410j 8/8/91 14:52:49 */
/*
 * COMPONENT_NAME: (INCSTD) Standard Include Files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 65
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _H_TIMERS
#define _H_TIMERS
#include <sys/time.h>

#define timespec timestruc_t		/* same struct, diff name */
#define itimerspec itimerstruc_t	/* same struct, diff name */

#define gettimer(a,b) (getinterval(a,b))

struct itimercb {
	int		itcb_count;	/* timer "overrun" count */
};


#define CLOCK_REALTIME TIMEOFDAY	/* 1003.4 */

/*
 * Notification types
 */
#define DELIVERY_SIGNALS	0	/* same as in sys/events.h */

/*
 * Functions
 */
#ifdef _NO_PROTO
int getclock();
int setclock();
timer_t mktimer();
int rmtimer();
int getinterval();
int reltimer();
#else
int getclock(int, struct timespec *);
int setclock(int, struct timespec *);
timer_t mktimer(int, int, struct itimercb *);
int rmtimer(timer_t);
int getinterval(timer_t, struct itimerspec *);
int reltimer(timer_t, struct itimerspec *, struct itimerspec *);
#endif

#endif	/* _H_TIMERS */
