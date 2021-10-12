/* @(#)47	1.9  src/bos/kernel/sys/ttydev.h, cmdtty, bos411, 9428A410j 6/16/90 00:39:11 */

/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * <bsd/sys/ttydev.h - a la 4.xBSD for BSD to AIX porting tools
 *	derived from BSD <sys/ttydev.h>
 * COPYRIGHT 1987 IBM CORP.
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)ttydev.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Terminal definitions related to underlying hardware.
 */
#ifndef _H_TTYDEV_
#define	_H_TTYDEV_

/*
 * Speeds
 */

#define	B0	0x00000000
#define	B50	0x00000001
#define	B75	0x00000002
#define	B110	0x00000003
#define	B134	0x00000004
#define	B150	0x00000005
#define	B200	0x00000006
#define	B300	0x00000007
#define	B600	0x00000008
#define	B1200	0x00000009
#define	B1800	0x0000000a
#define	B2400	0x0000000b
#define	B4800	0x0000000c
#define	B9600	0x0000000d
#define	B19200	0x0000000e
#define	B38400	0x0000000f
#define EXTA    B19200
#define EXTB    B38400

#ifdef _KERNEL
/*
 * Hardware bits.
 * SHOULD NOT BE HERE.
 */
#define	DONE	0200
#define	IENABLE	0100

/*
 * Modem control commands.
 */
#define	DMSET		0
#define	DMBIS		1
#define	DMBIC		2
#define	DMGET		3
#endif
#endif/* _H_TTYDEF_ */
