/* @(#)45	1.39  src/bos/kernel/sys/param.h, sysproc, bos411, 9428A410j 10/27/93 08:00:29 */
#ifndef _H_PARAM
#define _H_PARAM

/*
 * COMPONENT_NAME: (SYSPROC) process management
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 26, 9, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <sys/m_param.h>
#include <sys/limits.h>
#include <jfs/fsparam.h>

#define MAXUPRC	CHILD_MAX
#define NOFILE	OPEN_MAX

/*
 * fundamental variables
 * don't change too often
 */

#define	MAXPID	PID_MAX		/* max process id			*/
#define	MAXUID	UID_MAX		/* max user id				*/
#define	MAXLINK	LINK_MAX	/* max links				*/

#define MAXBLK	500		/* max blocks possible for phys IO	*/
#define	CANBSIZ	MAX_CANON	/* max size of typewriter line		*/
#define	NCARGS	ARG_MAX		/* # characters in exec arglist		*/
#define NGROUPS NGROUPS_MAX     /* max number of concurrent groups for user */

/*
 * fundamental constants of the implementation--
 * cannot be changed easily
 */
#define PAGESIZE	4096	/* page size in bytes			*/
#define PGSHIFT		12	/* log2 of PAGESIZE			*/
#define	NBPW	sizeof(int)	/* number of bytes in an integer	*/
#define NBPB		(8)	/* number of bits in a byte		*/
#define BPBSHIFT	(3)	/* log2(NBPB)				*/
#define UWSHIFT		(5)	/* log2(number of bits in an integer)	*/


/*
 * UBSIZE: Units for communication with user in ulimit, ustat.
 */
#define UBSIZE	512		/* bytes in a "user block" (<= BSIZE)	*/
#define UBSHIFT 9		/* LOG2(UBSIZE)				*/

#define NCPS	1		/* Number of clicks per segment		*/
#define NBPC	PAGESIZE	/* Number of bytes per click		*/
#define NCPD	1		/* Number of clicks per disk block	*/
#define BPCSHIFT       12	/* LOG2(NBPC) if exact			*/

#ifndef NULL
#define	NULL	0		/* p79626 */
#endif

#define GOOD	0
#define BAD	(-1)

/*
 * MAXPATHLEN defines the longest permissable path length
 * after expanding symbolic links. It is used to allocate
 * a temporary buffer in which to do the name expansion.
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly.
 */
#define MAXPATHLEN	(PATH_MAX+1)
#define MAXSYMLINKS	20
#ifndef MAXBSIZE
#ifdef	BSIZE
#define	MAXBSIZE	BSIZE
#else
#define	MAXBSIZE	4096
#endif
#endif

#define	MAXPATH	512		/* maximum pathlength for mount args	*/

#define	CMASK	0		/* default mask for file creation	*/
#define	CDLIMIT	(1L<<13)	/* default max write address		*/
#define	NODEVICE	(dev_t)(-1)

#define BASEPRI()	(PS == 7)	/** really ??????? **/

#define BUSOPEN(ics)	   (ics & 0x18)
    /* true if user has /dev/bus open */

#define	lobyte(X)	(((unsigned char *)&(X))[1])
#define	hibyte(X)	(((unsigned char *)&(X))[0])
#define	loword(X)	(((ushort *)&(X))[1])
#define	hiword(X)	(((ushort *)&(X))[0])

/*
 * BSD style param constants for use in AIX
 */

/*
 * bit map related macros
 */
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/*
 * Macros for fast min/max.
 */
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif /* MAX */

/*
 * Macros for counting and rounding.
 */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

/*
 * Maximum size of hostname recognized and stored in the kernel.
 */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN	256
#endif	/* MAXHOSTNAMELEN */

/* for AIX sockets */
/* MBUF CLUSTER defines, totally unlike the vax */
#define NBPG		PAGESIZE
/* This used to be CLSIZE, changed to avoid name collision, see sys/tty.h */
#define CLUSTERSIZE	1			/* one cluster per page	*/
#define	CLBYTES		(CLUSTERSIZE*PAGESIZE)	/* PAGESIZE from AIX	*/
#define	CLOFSET		(CLUSTERSIZE*PAGESIZE-1)/* for clusters, like PGOFSET */
#define	claligned(x)	((((int)(x))&CLOFSET)==0)
#define	CLOFF		CLOFSET
#define	CLSIZELOG2	0			/* CLUSTERSIZE == PAGESIZE */
#define	CLSHIFT		(BPCSHIFT+CLSIZELOG2)	/* Bytes per Click shift AIX */

#if CLUSTERSIZE==1
#define	clbase(i)	(i)
#define	clrnd(i)	(i)
#else
/* give the base virtual address (first of CLUSTERSIZE) */
#define	clbase(i)	((i) &~ (CLUSTERSIZE-1))
/* round a number of clicks up to a whole cluster */
#define	clrnd(i)	(((i) + (CLUSTERSIZE-1)) &~ (CLUSTERSIZE-1))
#endif

/* BSD param.h also includes signal.h */
#ifdef	_BSD
#ifndef _KERNEL
#include <sys/signal.h>
#endif /* _KERNEL */
#endif	/* _BSD */

#include "sys/sysmacros.h"

#endif /* _H_PARAM */

