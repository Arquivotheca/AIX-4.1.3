/* @(#)89	1.7  src/bos/kernel/sys/lock.h, sysproc, bos411, 9428A410j 12/7/93 18:34:37 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_LOCK
#define _H_LOCK

/*
 * flags for locking procs and texts
 */
#define	UNLOCK	 0
#define	PROCLOCK 1
#define	TXTLOCK	 2
#define	DATLOCK	 4

#ifdef _NO_PROTO
extern int plock();
#else
extern int plock(int);
#endif
#endif	/* _H_LOCK */
