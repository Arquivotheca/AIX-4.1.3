static char sccsid[] = "@(#)70	1.10  src/bos/usr/ccs/lib/libc/fputc.c, libcio, bos411, 9428A410j 4/20/94 17:44:58";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fputc
 *		fputc_unlocked
 *
 *   ORIGINS: 3,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* fputc.c,v $ $Revision: 2.7 $ (OSF) */

/*LINTLIBRARY*/
#include <stdio.h>
#include "ts_supp.h"
#include "push_pop.h"
#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#define PUTC putc_unlocked
#else /* _THREAD_SAFE */
#define PUTC putc
#endif /* _THREAD_SAFE */

int 	
fputc(int c, FILE *stream)
{
	register int rc;
	TS_FDECLARELOCK(filelock);

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	rc = PUTC(c, stream);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return rc;
}
#ifdef _THREAD_SAFE
int 	
fputc_unlocked(int c, FILE *stream)
{
	return (putc_unlocked(c, stream));
}
#endif /* _THREAD_SAFE */
