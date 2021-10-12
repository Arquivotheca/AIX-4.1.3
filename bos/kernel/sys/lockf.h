/* @(#)91	1.7  src/bos/kernel/sys/lockf.h, syslfs, bos411, 9428A410j 12/9/92 08:14:08 */
#ifndef _H_LOCKF
#define _H_LOCKF

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef S_ISGID
#include <sys/stat.h>
#endif

#define S_ENFMT S_ISGID

#define F_ULOCK 0       /* Unlock a previously locked region */
#define F_LOCK  1       /* Lock a region for exclusive use */
#define F_TLOCK 2       /* Test and lock a region for exclusive use */
#define F_TEST  3       /* Test region for other processes locks */

#endif /* _H_LOCKF */

#ifdef _NO_PROTO
extern int lockf();
#else
extern int lockf (int fdes, int function, long size);
#endif /* _NO_PROTO */
