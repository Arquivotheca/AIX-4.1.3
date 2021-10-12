static char sccsid[] = "@(#)93	1.10  src/bos/usr/ccs/lib/libc/fgetc.c, libcio, bos411, 9428A410j 4/20/94 17:39:25";
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fgetc
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
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* fgetc.c,v $ $Revision: 2.8.2.2 $ (OSF) */

/*LINTLIBRARY*/
#ifdef	_THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#define GETC getc_unlocked
#include "stdio_lock.h"
#else	/* _THREAD_SAFE */
#define GETC getc
#endif	/* _THREAD_SAFE */

int 	
fgetc(FILE *stream)
{
	register int c;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	TS_PUSH_CLNUP(filelock);

	c = GETC(stream);

	TS_POP_CLNUP(0);
	TS_FUNLOCK(filelock);
	return (c);
}
#ifdef _THREAD_SAFE
int 	
fgetc_unlocked(FILE *stream)
{
	register int c;
	c = getc_unlocked(stream);
	return (c);
}
#endif /* _THREAD_SAFE */
