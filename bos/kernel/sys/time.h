/* @(#)15	1.35  src/bos/kernel/sys/time.h, sysproc, bos411, 9428A410j 6/22/94 18:04:48 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: ADD_NS_TO_TIMEVAL
 *		ADD_TO_NS
 *		ntimeradd
 *		ntimerclear
 *		ntimercmp
 *		ntimerisset
 *		ntimersub
 *		timerclear
 *		timercmp
 *		timerisset
 *		
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_SYS_TIME
#define _H_SYS_TIME

#include <sys/types.h>

/*
 * The base timer services are based on the POSIX realtime extensions.
 * These timers services support one or more timers per process and
 * a very fine timer granularity.
 */

/*
 * The following are the BSD labels for the timer types.
 */
#define	ITIMER_REAL		0		/* Per-process real time */
#define	ITIMER_VIRTUAL		1		/* Per-process time */
#define	ITIMER_PROF		2		/* Per-process user time */

/* 
 * The following are the AIX labels for the timer types.
 */
#define	ITIMER_VIRT		3		/* Per-process time */
#define ITIMER_REAL1		20		/* Per-thread real time */

/* 
 * The following are the Posix labels for the timer types.
 */
#define	TIMEOFDAY		9		/* per process sys clock time */	
/*
 * Constants defining the hard-coded id's for each type of timer.
 */
#define	TIMERID_ALRM	(ITIMER_REAL)		/* alarm() */
#define	TIMERID_REAL	(ITIMER_REAL) 		/* bsd real */
#define	TIMERID_VIRTUAL	(ITIMER_VIRTUAL) 	/* bsd virtual */
#define	TIMERID_PROF	(ITIMER_PROF)		/* bsd profiling */
#define	TIMERID_VIRT	(ITIMER_VIRT)		/* aix virtual */
#define	TIMERID_TOD	(TIMERID_VIRT+1)	/* posix timers */
#define TIMERID_REAL1	(ITIMER_REAL1)		/* aix real1 */

/*
 * Constants defining the number of timers for each timer type.
 *
 * NALRM  	number of per-process alarm and real timers. 
 * NPROF  	number of per-process profiling timers. 
 * NVIRTUAL  	number of per-process virtual timers. 
 * NTIMEOFDAY	number of per-process time of day timers.
 * NTIMERS	total number of per-process timers.
 *
 * NALRMT 	number of per-thread alarm and real timers.
 * NTIMERST	total number of per-thread timers.
 */

#define NALRM           1      
#define NPROF           1    
#define NVIRTUAL        2  
#define NTIMEOFDAY      5 
#define NTIMERS         (NALRM + NPROF + NVIRTUAL + NTIMEOFDAY)

#define NALRM_THREAD	1
#define NTIMERS_THREAD	(NALRM_THREAD)

/*
 *  The Epoch is the time 0 hours, 0 minutes, 0 seconds, January 1, 1970
 *  Coordinated Universal Time. 
 */
#define	MIN_SECS_SINCE_EPOCH	0

/*
 *  System time constants
 */
#define NS_PER_TICK	(NS_PER_SEC/HZ)	/* nanoseconds per clock tick	*/
#define	uS_PER_SECOND	(1000000)	/* # of microseconds in one sec.*/
#define	NS_PER_uS	(1000)		/* # of nanoseconds in one us	*/
#define	MAX_SECS_TO_uS	4000		/* most seconds representable as*/
					/* microseconds in unsigned int	*/
#define	MAX_NS_TO_uS	294967296	/* most ns representable as us	*/
					/* in uint given MAX_SECS_TO_uS	*/
					/* is reached.			*/
#define	NS_PER_SEC	1000000000		/* # of nanosecs in 1 second*/
#define	uS_PER_SEC	(NS_PER_SEC / 1000)	/* # of microsecs in 1 sec  */
#define	NS_PER_MSEC	(NS_PER_SEC / 1000)	/* # of nanosecs in 1 msec  */

#define	MAX_DEC_SECS	2
#define	MAX_DEC_NS	147483647

/*
 * The following is used to hold a timer value.
 */
struct timestruc_t {
        unsigned long	tv_sec;         /* seconds			*/
        	 long	tv_nsec;        /* and nanoseconds		*/
};

/*
 * The following is the BSD equivalent of the timerstruc. The main
 * difference is that the maximum resolution is specificed in microseconds
 * instead of nanoseconds.
 */
struct timeval {
	int		tv_sec;		/* seconds */
	int		tv_usec;	/* microseconds */
};

/*
 * The following is used to define a timer.
 */
struct itimerstruc_t {
        struct  timestruc_t it_interval; /* timer interval		*/
        struct  timestruc_t it_value;    /* current value		*/
};

#ifdef _ALL_SOURCE
int gettimeofday(struct timeval *, void *);

typedef struct timebasestruct {
	int		flag;    /* indicats time base or real time */
	unsigned long	tb_high; /* high 32 bits, or seconds */
	unsigned long	tb_low;	 /* low 32 bits, or nanoseconds */
	} timebasestruct_t;	

int read_real_time(timebasestruct_t *, size_t);
int time_base_to_time(timebasestruct_t *, size_t);
#define TIMEBASE_SZ (sizeof (struct timebasestruct))
#endif /* _ALL_SOURCE */

#ifdef _KERNEL
/*
 * External kernel variables 
 */

extern time_t lbolt;              /* number ticks since last boot */
extern time_t time;               /* memory mapped time in secs since Epoch */
extern struct timestruc_t tod; 	  /* memory mapped timer structure */

#endif  /* _KERNEL */

/*
 * Operations on timestruc_t.
 *
 * Note that ntimercmp only works for cmp values of !=, >, and <.
 */
#define	ntimerisset(tvp)	((tvp)->tv_sec || (tvp)->tv_nsec)
#define	ntimerclear(tvp)	(tvp)->tv_sec = (tvp)->tv_nsec = 0
#define	ntimercmp(tvp, fvp, cmp)					\
	((tvp).tv_sec cmp (fvp).tv_sec || 				\
	 (tvp).tv_sec == (fvp).tv_sec && 				\
	 (tvp).tv_nsec cmp (fvp).tv_nsec)
#define	ntimeradd(tvp, svp, rvp)					\
	{								\
		(rvp).tv_sec = (tvp).tv_sec + (svp).tv_sec;		\
		(rvp).tv_nsec = (tvp).tv_nsec + (svp).tv_nsec;		\
		if((rvp).tv_nsec > (NS_PER_SEC - 1))   {		\
			(rvp).tv_nsec -= NS_PER_SEC;			\
			(rvp).tv_sec++;					\
		}							\
		assert((rvp).tv_nsec < NS_PER_SEC);			\
	}
#define	ntimersub(tvp, svp, rvp)					\
	{								\
		assert((tvp).tv_nsec <= NS_PER_SEC);			\
		assert((svp).tv_nsec <= NS_PER_SEC);			\
		assert(!((tvp).tv_sec == 0 && ((svp).tv_nsec > (tvp).tv_nsec)));\
		if((tvp).tv_sec == (svp).tv_sec)  {			\
			assert((tvp).tv_nsec >= (svp).tv_nsec);		\
		}							\
		else  {							\
			assert((tvp).tv_sec > (svp).tv_sec);		\
		}							\
									\
		if((svp).tv_nsec > (tvp).tv_nsec)  {			\
			assert((tvp).tv_sec > (svp).tv_sec);		\
			(rvp).tv_sec = ((tvp).tv_sec - 1) - (svp).tv_sec;\
			(rvp).tv_nsec = ((int)(((uint)((tvp).tv_nsec) + NS_PER_SEC) - (uint)((svp).tv_nsec)));\
		}							\
		else  {							\
			(rvp).tv_sec = (tvp).tv_sec - (svp).tv_sec;	\
			(rvp).tv_nsec = (tvp).tv_nsec - (svp).tv_nsec;	\
		}							\
									\
		assert((tvp).tv_nsec <= NS_PER_SEC);			\
		assert((svp).tv_nsec <= NS_PER_SEC);			\
		assert((rvp).tv_nsec <= NS_PER_SEC);			\
	}

/*
 * The rest of this file is the interface to the BSD timer services.
 * Most of these services are implemented as subroutines that convert
 * the interface to the corresponding POSIX timer service.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)time.h	7.1 (Berkeley) 6/4/86
 */

/*
 * The following is the BSD equivalent of the timerstruc. The main
 * difference is that the maximum resolution is specificed in microseconds
 * instead of nanoseconds.
 *
 * struct timeval {
 *       int		tv_sec;         * seconds *
 *       int		tv_usec;        * microseconds *
 * };
 */

/*
 * Operations on timevals.
 *
 * Note that timercmp only works for cmp values of !=, >, and <.
 */

#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0
#define	timercmp(tvp, fvp, cmp)						\
	((tvp)->tv_sec cmp (fvp)->tv_sec ||				\
	 (tvp)->tv_sec == (fvp)->tv_sec &&				\
	 (tvp)->tv_usec cmp (fvp)->tv_usec)

#define	ADD_TO_NS(tvp, delta)						\
{									\
	(tvp).tv_nsec += delta;						\
	if((tvp).tv_nsec >= NS_PER_SEC)  {				\
		(tvp).tv_sec++;						\
		(tvp).tv_nsec -= NS_PER_SEC;				\
	}								\
}

/*
 *  Adds a value of nanoseconds to a timeval structure (which traditionally
 *  contains microseconds).
 */
#define	ADD_NS_TO_TIMEVAL(tvp, delta)					\
{									\
	(tvp).tv_usec += delta;						\
	if((tvp).tv_usec >= NS_PER_SEC)  {				\
		(tvp).tv_sec++;						\
		(tvp).tv_usec -= NS_PER_SEC;				\
	}								\
}

/*
 * The following is the BSD equivalent of the itimerstruc_t.
 */
struct	itimerval {
	struct		timeval it_interval; /* timer interval */
	struct		timeval it_value; /* current value */
};

/*
 * The following provides a way to convert the current time in GMT
 * to a local time.
 */
struct timezone {
	int		tz_minuteswest;	/* minutes west of Greenwich */
	int		tz_dsttime;	/* type of dst correction */
};
#define	DST_NONE		0	/* not on dst */
#define	DST_USA			1	/* USA style dst */
#define	DST_AUST		2	/* Australian style dst */
#define	DST_WET			3	/* Western European dst */
#define	DST_MET			4	/* Middle European dst */
#define	DST_EET			5	/* Eastern European dst */
#define	DST_CAN			6	/* Canada */

#ifdef	_BSD_INCLUDES
#include <time.h>
#endif	/* _BSD_INCLUDES */

#endif /* _H_SYS_TIME */
