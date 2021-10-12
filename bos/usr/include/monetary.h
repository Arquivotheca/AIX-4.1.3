/* @(#)98	1.4  src/bos/usr/include/monetary.h, libcfmt, bos411, 9428A410j 5/27/94 13:27:50 */

/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_MONETARY
#define _H_MONETARY

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifdef _XOPEN_SOURCE

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#ifdef _NO_PROTO
extern ssize_t strfmon();
#else
extern ssize_t strfmon(char *, size_t, const char *, ...);
#endif

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#ifndef _H_STDDEF
#include <stddef.h>
#endif

#endif /* _ALL_SOURCE */

#endif /* _H_MONETARY */
