/* @(#)59	1.10  src/bos/kernel/sys/resource.h, sysproc, bos411, 9428A410j 12/7/93 18:41:13 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)resource.h	7.1 (Berkeley) 6/4/86
 *
 * Function:
 *		The rusage structure is included into both the proc
 *	structure and the user structure.  The rlimit structure is
 *	included into the user structure only.  The rusage and rlimit
 * 	structures exist to keep an accounting of resources on a per
 *	process basis.
 *
 */

#ifndef _H_RESOURCE
#define _H_RESOURCE

#include <sys/time.h>

/*
 * Process priority specifications to get/setpriority.
 */
#define	PRIO_MIN	-20
#define	PRIO_MAX	 20

#define	PRIO_PROCESS	0
#define	PRIO_PGRP	1
#define	PRIO_USER	2

/*
 * Resource utilization information.
 */

#define	RUSAGE_SELF	0
#define	RUSAGE_CHILDREN	-1

struct	rusage {
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
	long	ru_maxrss;
#define	ru_first	ru_ixrss
	long	ru_ixrss;		/* integral shared memory size */
	long	ru_idrss;		/* integral unshared data " */
	long	ru_isrss;		/* integral unshared stack " */
	long	ru_minflt;		/* page reclaims */
	long	ru_majflt;		/* page faults */
	long	ru_nswap;		/* swaps */
	long	ru_inblock;		/* block input operations */
	long	ru_oublock;		/* block output operations */
	long	ru_msgsnd;		/* messages sent */
	long	ru_msgrcv;		/* messages received */
	long	ru_nsignals;		/* signals received */
	long	ru_nvcsw;		/* voluntary context switches */
	long	ru_nivcsw;		/* involuntary " */
#define	ru_last		ru_nivcsw
};

/*
 * Resource limits
 */
#define	RLIMIT_CPU	0		/* cpu time in milliseconds */
#define	RLIMIT_FSIZE	1		/* maximum file size */
#define	RLIMIT_DATA	2		/* data size */
#define	RLIMIT_STACK	3		/* stack size */
#define	RLIMIT_CORE	4		/* core file size */
#define	RLIMIT_RSS	5		/* resident set size */

#define	RLIM_NLIMITS	6		/* number of resource limits */

#define	RLIM_INFINITY	0x7fffffff

struct rlimit {
	int	rlim_cur;		/* current (soft) limit */
	int	rlim_max;		/* maximum value for rlim_cur */
};

#ifndef _KERNEL
#ifdef _NO_PROTO
extern int getpriority();
extern int getrlimit();
extern int getrusage();
extern int setpriority();
extern int setrlimit();
#else
extern int getpriority(int, int);
extern int getrlimit(int, struct rlimit *);
extern int getrusage(int, struct rusage *);
extern int setpriority(int, int, int);
extern int setrlimit(int, const struct rlimit *);
#endif /* _NO_PROTO */
#endif /* _KERNEL */
#endif /* _H_RESOURCE */
