static char sccsid[] = "@(#)15	1.16  src/bos/usr/ccs/lib/libc/fgets.c, libcio, bos411, 9428A410j 4/29/94 13:20:23";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: MIN
 *		RETURN
 *		fgets
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
/* fgets.c,v $ $Revision: 2.7.2.5 $ (OSF) */

/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "stdiom.h"

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return(TS_FUNLOCK(filelock), (char *)(val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */


#define MIN(x, y)	(x < y ? x : y)

extern int __filbuf();

char 	*
fgets(char *s, int n, FILE *stream)
{
	char *p, *save = s;
	int i, leave=0;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if ((stream->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
		errno = EBADF;
		RETURN(NULL);
	}

	TS_PUSH_CLNUP(filelock);

	for (n--; n > 0; n -= i) {
		if (stream->_cnt <= 0) { /* empty buffer */
			if (__filbuf(stream) == EOF) {
				if (save == s) {
					leave = 1;
					goto pop_n_leave;
				}
				break; /* no more data */
			}
			stream->_ptr--;
			stream->_cnt++;
		}
		i = MIN(n, stream->_cnt);
		if ((p = memccpy((void *)s,(void *)stream->_ptr,(int)'\n',(size_t)i)) !=NULL)
			i = p - s;
		s += i;
		stream->_cnt -= i;
		stream->_ptr += i;
		_BUFSYNC(stream);
		if (p != NULL)
			break; /* found '\n' in buffer */
	}

pop_n_leave:

	TS_POP_CLNUP(0);

	if (leave)
		RETURN(NULL);

	*s = '\0';
	RETURN(save);
}
