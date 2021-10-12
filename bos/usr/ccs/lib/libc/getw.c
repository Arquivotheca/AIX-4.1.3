static char sccsid[] = "@(#)24	1.10  src/bos/usr/ccs/lib/libc/getw.c, libcio, bos411, 9428A410j 4/20/94 17:49:30";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: RETURN
 *		getw
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
/* getw.c,v $ $Revision: 2.9.2.3 $ (OSF) */

/*LINTLIBRARY*/
/*
 * The intent here is to provide a means to make the order of
 * bytes in an io-stream correspond to the order of the bytes
 * in the memory while doing the io a `word' at a time.
 */

#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#define GETC	getc_unlocked
#define FERROR	ferror_unlocked
#define FEOF	feof_unlocked
#else	/* _THREAD_SAFE */
#define GETC	getc
#define FERROR	ferror
#define FEOF	feof
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)		return(TS_FUNLOCK(filelock), (val))
#define POP_N_LEAVE(val)	{ w = val; goto pop_n_leave; }

#else
#define	RETURN(val)		return(val)
#define	POP_N_LEAVE(val)	return(val)
#endif	/* _THREAD_SAFE */

int
getw(register FILE *stream)
{
	int w;
	register char *s = (char *)&w;
	register int i = sizeof(int);
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);

	while (--i >= 0) {
		*s++ = GETC(stream);
		if(FEOF(stream) || FERROR(stream)) {
			POP_N_LEAVE(EOF);
		}
	}

pop_n_leave:
	TS_POP_CLNUP(0);
	RETURN(w);		/* w is set by POP_N_LEAVE(x) to the value x */
}
