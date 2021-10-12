/* @(#)97	1.2  src/bos/kernext/aio/aio_interface.h, sysxaio, bos411, 9428A410j 10/14/93 15:48:30 */
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: aio_interface.h
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define POSIX_AIO	1
#define SUN_AIO		2

#define QREADREQ	1
#define QWRITEREQ	2

struct aio_cfg {
	int	maxservers;
	int	minservers;
	int	maxreqs;
	int	kprocprio;
#ifdef DEBUG
	int	bug_level;
	int	spare[3];
#else /* DEBUG */
	int	spare[4];
#endif /* DEBUG */
} aio_cfg;	

int
kaio_rdwr(int reqtype,		/* QREADREQ or QWRITEREQ */
	  int fildes,
	  struct aiocb *ucbp,	/* pointer to aiocb in user space */
	  int aio_type);	/* POSIX_AIO or SUN_AIO */

int
iosuspend(int cnt, struct aiocb *aiocbpa[]);

int
acancel(int fildes, struct aiocb *aiocbp);

int
listio(int cmd,		         /* LIO_WAIT, LIO_NOWAIT, LIO_ASYNC, LIO_ASIG */
       struct liocb *list[],	 /* Array of aio control blocks */
       int nent,	         /* number of elements in preceeding array */
       struct sigevent *eventp); /* ignored for now */
