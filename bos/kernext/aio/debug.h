/* @(#)00	1.3  src/bos/kernext/aio/debug.h, sysxaio, bos411, 9428A410j 10/14/93 15:49:35 */
/*
 * COMPONENT_NAME: (SYSXAIO) Asynchronous I/O
 *
 * FUNCTIONS: debug.h
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
#ifdef DEBUG

extern int bug_level;

#define BUGLEV(l)	bug_level |= l
#define PANIC(s)	panic(s)
#define DPRINTF(p)	do { if (bug_level) printf p; } while (0)
#define DBGPRINTF(t, p)	do { if (bug_level & t) printf p; } while (0)
#define DSPRINTF(p)
#define DEBUG_DISP_TOP(n)
#define DEBUG_ENV(n,s)

#define DBG_KNOTS		0x0001
#define DBG_FIND_QUEUE		0x0002
#define DBG_CANCEL		0x0004
#define DBG_CANCEL_VERBOSE	0x0008
#define DBG_SERVER		0x0010
#define DBG_IO			0x0020
#define DBG_QUEUE		0x0040

#else /* DEBUG */

#ifndef NDEBUG
#define NDEBUG
#endif

#define BUGLEV(l)
#define PANIC(s)
#define DPRINTF(p)
#define DBGPRINTF(t, p)
#define DSPRINTF(p)
#define DEBUG_DISP_TOP(n)
#define DEBUG_ENV(n,s)

#endif /* DEBUG */
