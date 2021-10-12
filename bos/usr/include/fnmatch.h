/* @(#)96	1.2  src/bos/usr/include/fnmatch.h, libcpat, bos411, 9428A410j 1/12/93 16:59:05 */

/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Matching
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_FNMATCH
#define _H_FNMATCH

#ifndef _H_STANDARDS
#include <standards.h>
#endif

/*
 * values for the fnmatch() flags variable
 */

#ifdef _XOPEN_SOURCE

#define FNM_PATHNAME    1
#define FNM_PERIOD      2
#define FNM_NOESCAPE    8


/* Pattern Matching error codes */

#define	FNM_NOMATCH	1	/* pattern does not match string	*/
#define FNM_NOSYS	4	/* this function is not supported	*/

/* Pattern Matching function prototypes */

#ifdef _NO_PROTO
extern	int	fnmatch();
#else
extern	int	fnmatch(const char *, const char *, int);
#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#define FNM_QUOTE       4
#define FNM_ESLASH	2	/* * ? [] can't match /			*/
#define FNM_EPERIOD	3	/* * ? [] can't match .			*/

#endif /* _ALL_SOURCE */

#endif /* _H_FNMATCH */
