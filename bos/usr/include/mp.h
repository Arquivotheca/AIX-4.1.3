/* @(#)67	1.4  src/bos/usr/include/mp.h, libbsd, bos411, 9428A410j 8/25/93 16:33:57 */
/*
 * COMPONENT_NAME: (LIBBSD) System header files
 *
 * FUNCTIONS:
 *                                                                    
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
#ifndef _H_MP
#define _H_MP

#define MINT struct mint
MINT
{	int len;
	short *val;
};

#define FREE(x) {if(x.len!=0) {free((void *)x.val); x.len=0;}}
#define shfree(u) free((void *)u)

struct half
{	short high;
	short low;
};

#ifdef _NO_PROTO
extern MINT *itom();
extern short *xalloc();
#else
extern MINT *itom(int);
extern short *xalloc(int, char*);
#endif

#ifdef lint
extern xv_oid;
#define VOID xv_oid =
#else
#define VOID
#endif
#endif
