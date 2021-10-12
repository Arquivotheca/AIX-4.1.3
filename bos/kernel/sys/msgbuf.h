/* @(#)09	1.7  src/bos/kernel/sys/msgbuf.h, sysipc, bos411, 9428A410j 6/16/90 00:32:50 */
/*
 * COMPONENT_NAME:
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 26
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_MSGBUF
#define _H_MSGBUF

/*
 * BSD to AIX compatibility <sys/msgbuf.h> include file
 * COPYRIGHT 1987 IBM CORP.
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)msgbuf.h	7.1 (Berkeley) 6/4/86
 */

#define	MSG_MAGIC	0x063061
#define	MSG_BSIZE	(4096 - 3 * sizeof (long))
struct	msgbuf {
	long	msg_magic;
	long	msg_bufx;
	long	msg_bufr;
	char	msg_bufc[MSG_BSIZE];
};
#ifdef _BSD
#ifdef _KERNEL
struct	msgbuf msgbuf;
#endif
#endif /* _BSD */
 
#endif /* _H_MSGBUF */
