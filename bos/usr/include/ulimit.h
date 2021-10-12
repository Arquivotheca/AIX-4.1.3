/* @(#)29	1.13  src/bos/usr/include/ulimit.h, sysproc, bos411, 9428A410j 12/7/93 19:27:45 */

/*
 * COMPONENT_NAME: SYSPROC 
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ULIMIT
#define _H_ULIMIT

#ifndef _H_STANDARDS
#include <standards.h>
#endif

#ifndef _H_TYPES
#include <sys/types.h>
#endif

#ifdef _XOPEN_SOURCE

#ifdef _NO_PROTO
extern long ulimit();

#else 

#ifndef _KERNEL
extern long ulimit(int, ...);

#endif /* _KERNEL */
#endif /* _NO_PROTO */

#define UL_GETFSIZE	1	
#define UL_SETFSIZE	2

#endif /* _XOPEN_SOURCE */

#ifdef _ALL_SOURCE

#define GET_FSIZE	1
#define SET_FSIZE	2
#define GET_DATALIM	3
#define UL_GETMAXBRK	GET_DATALIM
#define SET_DATALIM	1004
#define GET_STACKLIM	1005
#define SET_STACKLIM	1006
#define GET_REALDIR	1007
#define SET_REALDIR	1008

#endif /* _ALL_SOURCE */

#endif /* _H_ULIMIT */ 
