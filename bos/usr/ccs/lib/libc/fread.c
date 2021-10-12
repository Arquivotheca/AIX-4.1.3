static char sccsid[] = "@(#)91	1.20  src/bos/usr/ccs/lib/libc/fread.c, libcio, bos411, 9428A410j 4/20/94 17:46:04";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: MIN
 *		fread
 *		fread_unlocked
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
/* fread.c,v $ $Revision: 2.11.1.3 $ (OSF) */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

#define MIN(x, y)	(x < y ? x : y)

extern int __filbuf();

/*
 * FUNCTION:	The fread function reads, into the array pointed to by
 *		ptr, up to nmemb members whose size is specified by size, 
 *		from the stream pointed to be stream.
 *
 * 		This version reads directly from the buffer rather than
 *		looping on getc.  Ptr args aren't checked for NULL because
 *		the program would be a catastrophic mess anyway.  Better
 *		to abort than just to return NULL.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The fread function returns the number of members successfully
 *		read, which may be less than nmemb if a read error or end-of-
 *		file is encountered.  If size or nmemb is zero, fread returns
 *		zero and the contents of the array and the state of the
 *		stream remains unchanged.
 *
 */  
size_t 	
#ifdef _THREAD_SAFE
fread_unlocked(void *ptr, size_t size, size_t nmemb, FILE *stream)
#else
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
#endif	/* _THREAD_SAFE */
{
	unsigned nleft;
	int n;

	if (size <= 0 || nmemb <= 0)
		return (0);

        /* Check if stream was opened for reading */
        if (!(stream->_flag & _IOREAD)) {
                if (stream->_flag & _IORW) {
                        stream->_flag |= _IOREAD;
                }
                else {
                        errno = EBADF;
			stream->_flag |= _IOERR;
                        return (0);
                }
        }

	for (nleft = nmemb * size; ; ) {
		if (stream->_cnt <= 0) { /* empty buffer */
			if (__filbuf(stream) == EOF) {
				return (nmemb - (nleft + size - 1)/size);
			}
			stream->_ptr--;
			stream->_cnt++;
		}
		n = MIN(nleft, stream->_cnt);
		ptr = (char *)memcpy(ptr, (void *) stream->_ptr, (size_t)n) + n;
		stream->_cnt -= n;
		stream->_ptr += n;
		_BUFSYNC(stream);
		if ((nleft -= n) == 0) {
			return (nmemb);
		}
	}
}


#ifdef	_THREAD_SAFE
size_t 	
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	register size_t rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	rc = fread_unlocked(ptr, size, nmemb, stream);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
