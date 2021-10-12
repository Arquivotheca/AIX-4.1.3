/* @(#)47	1.14  src/bos/kernel/sys/conf.h, sysios, bos411, 9428A410j 6/16/90 00:25:11 */
#ifndef _H_CONF
#define _H_CONF
/*
 * COMPONENT_NAME: (SYSIOS) Device Configuration header file
 *
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/types.h>    /* needed for caddr_t definition */

/* device switch structure is defined in device.h */

/* Number of static device drivers and first major number not used by
 * device drivers statically bound with the kernel
 */
#define	STATIC_DEVCNT	10

extern	int	devcnt;		/* maximum number of entries in devsw table */

/*
 * Line discipline switch.
 */
struct linesw {
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_input)();
	int	(*l_output)();
	int	(*l_mdmint)();
};
extern struct linesw linesw[];

extern int	linecnt;
/*
 * Terminal switch
 */
struct termsw {
	int	(*t_input)();
	int	(*t_output)();
	int	(*t_ioctl)();
};
extern struct termsw termsw[];

extern int	termcnt;
#endif	/* _H_CONF */









