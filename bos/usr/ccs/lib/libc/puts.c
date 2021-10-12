static char sccsid[] = "@(#)89	1.13  src/bos/usr/ccs/lib/libc/puts.c, libcio, bos411, 9428A410j 4/20/94 17:51:13";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: puts
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
/* puts.c,v $ $Revision: 2.5.2.2 $ (OSF) */

#include <stdio.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#define POP_N_LEAVE(val)	{ rc = val; goto pop_n_leave; }
#else
#define POP_N_LEAVE(val)	return(val)
#endif	/* _THREAD_SAFE */

extern char *memccpy();

/*
 * FUNCTION:	The puts function writes the string pointed to by s to the
 *		stream pointed to by stdout, and appends a new-line character
 *		to the output.  The terminating character is not written.
 *
 * 		This version reads directly from the buffer rather than
 *		looping on putc.  Ptr args aren't checked for NULL because
 *		the program would be a catastrophic mess anyway.  Better
 *		to abort than just to return NULL.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The puts function returns EOF if a write error occurs;
 *		otherwise it returns a nonnegative value
 *
 */  

int 	
puts(const char *s)
{
	char	*p;
	int	ndone = 0, n;
	unsigned char	*cs, *bufend;
	int 	rc;

	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdout);

	if (_WRTCHK(stdout)) {
		TS_FUNLOCK(filelock);
		return (EOF);
	}

	bufend = _bufend(stdout);

	TS_PUSH_CLNUP(filelock);

	for ( ; ; s += n) {
		while ((n = bufend - (cs = stdout->_ptr)) <= 0) /* full buf */
			if (_xflsbuf(stdout) == EOF) {
				POP_N_LEAVE(EOF);
			}
		if ((p = memccpy((char *) cs, s, '\0', n)) != NULL)
			n = p - (char *) cs;
		stdout->_cnt -= n;
		stdout->_ptr += n;
		_BUFSYNC(stdout);
		ndone += n;
		if (p != NULL) {
			stdout->_ptr[-1] = '\n'; /* overwrite '\0' with '\n' */
			if (stdout->_flag & (_IONBF | _IOLBF)) /* flush line */
				if (_xflsbuf(stdout) == EOF) {
					POP_N_LEAVE(EOF);
				}
			POP_N_LEAVE(ndone);
		}
	}

pop_n_leave:
	TS_POP_CLNUP(0);
	TS_FUNLOCK(filelock);
	return(rc);		/* rc is set by POP_N_LEAVE(x) to the value x */
}
