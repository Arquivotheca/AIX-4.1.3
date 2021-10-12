/* @(#)62	1.1  src/bos/usr/include/pse/perftest.h, sysxpse, bos411, 9428A410j 5/7/91 14:30:58 */
/*
 *   COMPONENT_NAME: LIBCPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * perftest.h - #defines for performance tests
 */

/* implementation maxs & constants */
#define	USECPERSEC	1000000
#define	MAX_PKTSZ	4096			/* MPS max packet size */
#define	HIMSGS		50
#define	LOMSGS		2
#define	HIWATER		(DFLT_SIZE * HIMSGS)
#define	LOWATER		(DFLT_SIZE * LOMSGS)

/* defaults */
#define	DFLT_SIZE	4096
#define	DFLT_OPS	20000

/* ioctls */
#define	PERF_SIZE	('p' << 8 | 0x01)	/* change size of msgs */
#define	PERF_FLOW	('p' << 8 | 0x02)	/* start/stop flow ctl */
#define	PERF_GEN	('p' << 8 | 0x04)	/* start generating msgs */

typedef int (*pfi_t)();
extern char *strchr();

