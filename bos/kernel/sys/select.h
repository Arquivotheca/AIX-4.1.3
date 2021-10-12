/* @(#)90	1.10  src/bos/kernel/sys/select.h, sysios, bos411, 9431A411a 8/4/94 15:28:56 */
#ifndef _H_SELECT
#define _H_SELECT
/*
 * COMPONENT_NAME: (SYSIOS) Select system call header file
 *
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
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

#ifdef _NO_PROTO
extern	int	select();

#else	/* _NO_PROTO */
#include <sys/time.h>

extern	int select(
	unsigned long nfdsmsgs,	/* #file descriptors/message queues */
	void *readlist,		/* file descriptors to check for read */
	void *writelist,	/* file descriptors to check for write */
	void *exceptlist,	/* file descriptors to check for exceptions */
	struct timeval *timeout);	/* length of time to wait for events */
#endif /* _NO_PROTO */

/*
 * Timeout values - used for poll() system call (not for select() system call)
 */
#ifndef NO_TIMEOUT
#define NO_TIMEOUT      0       /* don't wait for a response            */
#endif

#ifndef INF_TIMEOUT
#define INF_TIMEOUT     -1      /* wait until a response is received    */
#endif


/*
 * Number of file descriptors (bits).
 * Apply to nfdsmsgs parameter of SELECT system
 * call &/or return value from SELECT.
 */
#define NFDS(x)		((x) & 0x0000FFFF)

/*
 * Lower half of word.
 * Used in hashing algorithm for devsel_hash chain.
 */
#define	LOW_HALF(x)	NFDS(x)

/*
 * Number of message queues.
 * Apply to nfdsmsgs parameter of SELECT system
 * call &/or return value from SELECT.
 */
#define NMSGS(x)	(((x) >> 16) & 0x0000FFFF)

/*
 * Upper half of word.
 * Used in hashing algorithm for devsel_hash chain.
 */
#define	HIGH_HALF(x)	NMSGS(x)

/*
 * Macro used to define a sellist structure
 * for the SELECT system call.
 */
#define	SELLIST(F,M)		\
struct				\
{				\
	unsigned int   fdsmask[F];	\
	int   msgids[M];	\
}

/*
 * Select uses bit masks of file descriptors.
 * These macros manipulate such bit fields.
 * FD_SETSIZE may be defined by the user to the maximum valued file
 * descriptor to be selected; the default here should be >= OPEN_MAX
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	2048
#endif

typedef unsigned int	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#endif  /* _H_SELECT */
