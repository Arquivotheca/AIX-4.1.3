/* @(#)15	1.5  src/bos/usr/include/NLxio.h, libcnls, bos411, 9428A410j 2/10/93 16:08:01 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Library Support
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *
 * FUNCTION:	This file is designed to be included by NLxio.c
 * and any programs using _NLxin/_NLxout
 */

#ifndef _H_NLXIO
#define _H_NLXIO

struct NLxtbl {
	char source[16];
	char target[16];
	char chrs[256];
};

#ifdef   _NO_PROTO
int NLxout();
int NLxin();
#else  /*_NO_PROTO */
int NLxin(char *t,char *s,int n);
int NLxout(char *t,char *s,int n);
#endif /*_NO_PROTO */

extern struct NLxtbl *_NLxitbl;
extern struct NLxtbl *_NLxotbl;

#define _NLxin(c)	(_NLxitbl->chrs[c])
#define _NLxout(c)	(_NLxotbl->chrs[c])
#endif /* _H_NLXIO */
