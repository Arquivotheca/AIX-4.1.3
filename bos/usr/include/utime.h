/* @(#)37	1.7  src/bos/usr/include/utime.h, syslfs, bos411, 9428A410j 1/12/93 17:03:14 */

/*
 * COMPONENT_NAME: SYSLFS - Logical File System
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_UTIME
#define _H_UTIME

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

/*
 * POSIX requires that certain values be included in utime.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */

struct utimbuf {
	time_t	actime;			/* access time */
	time_t	modtime;		/* modification time */
	};

#ifdef _NO_PROTO
extern int utime();
#else
extern int utime(const char *,const struct utimbuf *); 
#endif /* _NO_PROTO */

#endif /* _H_UTIME */
