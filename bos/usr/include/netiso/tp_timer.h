/* @(#)65	1.5  src/bos/usr/include/netiso/tp_timer.h, sockinc, bos411, 9428A410j 3/5/94 12:41:55 */

/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/

/*
 * ARGO Project, Computer Sciences Dept., University of Wisconsin - Madison
 */
/* 
 * ARGO TP
 *
 * $Header: tp_timer.h,v 5.1 88/10/12 12:21:41 root Exp $
 * $Source: /usr/argo/sys/netiso/RCS/tp_timer.h,v $
 *	(#)tp_timer.h	7.3 (Berkeley) 8/29/89 *
 *
 * ARGO TP
 * The callout structures used by the tp timers.
 */

#ifndef __TP_CALLOUT__
#define __TP_CALLOUT__

/* C timers - one per tpcb, generally cancelled */

struct	Ccallout {
	int	c_time;		/* incremental time */
	int c_active;	/* this timer is active? */
};

/* E timers - generally expire or there must be > 1 active per tpcb */
struct Ecallout {
	int	c_time;		/* incremental time */
	int c_func;		/* function to call */
	u_int c_arg1;	/* argument to routine */
	u_int c_arg2;	/* argument to routine */
	int c_arg3;		/* argument to routine */
	struct Ecallout *c_next;
};

#endif /* __TP_CALLOUT__ */
