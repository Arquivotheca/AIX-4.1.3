/* @(#)34	1.13  src/bos/usr/include/assert.h, sysdb, bos412, 9446A412a 11/14/94 15:10:12 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 *      The ANSI standard requires that certain values be in assert.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present.  This header includes all ANSI required entries.  
 *
 */

#undef assert
#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#ifdef _NO_PROTO
extern void __assert();
#else
extern void __assert(const char *, const char *, int);
#endif /* _NO_PROTO */

#if __STDC__ == 1
#define assert(__EX) ((__EX) ? ((void)0) : __assert(# __EX, __FILE__, __LINE__))
#else
#define assert(__EX) ((__EX) ? ((void)0) : __assert("__EX", __FILE__, __LINE__))
#endif /* __STDC__*/

#endif /* NDEBUG */
