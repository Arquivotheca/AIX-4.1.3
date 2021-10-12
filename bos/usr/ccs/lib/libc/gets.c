static char sccsid[] = "@(#)90	1.11  src/bos/usr/ccs/lib/libc/gets.c, libcio, bos411, 9428A410j 4/20/94 17:48:26";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: gets
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
/* gets.c,v $ $Revision: 2.5.2.2 $ (OSF) */

/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <stdio.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#define POP_N_LEAVE(val)	{ s0 = val; goto pop_n_leave; }
#else
#define POP_N_LEAVE(val)	return(val)
#endif	/* _THREAD_SAFE */

extern int __filbuf();
extern char *memccpy();

char *
gets(char *s)
{
	char	*p, *s0 = s;
	int	n;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdin);
	TS_PUSH_CLNUP(filelock);

	for ( ; ; ) {
		if (stdin->_cnt <= 0) { /* empty buffer */
			if (__filbuf(stdin) == EOF) {
				if (s0 == s) {
					POP_N_LEAVE(NULL);
				}
				break; /* no more data */
			}
			stdin->_ptr--;
			stdin->_cnt++;
		}
		n = stdin->_cnt;
		if ((p = memccpy(s, (char *) stdin->_ptr, '\n', n)) != NULL)
			n = p - s;
		s += n;
		stdin->_cnt -= n;
		stdin->_ptr += n;
		_BUFSYNC(stdin);
		if (p != NULL) { /* found '\n' in buffer */
			s--; /* step back over '\n' */
			break;
		}
	}
	*s = '\0';

pop_n_leave:
	TS_POP_CLNUP(0);
	TS_FUNLOCK(filelock);
	return (s0);		/* s0 is set by POP_N_LEAVE(x) to the value x */
}
