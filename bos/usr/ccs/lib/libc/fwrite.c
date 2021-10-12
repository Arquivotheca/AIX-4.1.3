static char sccsid[] = "@(#)35	1.23  src/bos/usr/ccs/lib/libc/fwrite.c, libcio, bos411, 9428A410j 4/20/94 17:47:29";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fwrite
 *		fwrite_unlocked
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
/* fwrite.c,v $ $Revision: 2.13.1.3 $ (OSF) */

#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <string.h>
#include "stdiom.h"
#include <sys/errno.h>
#include "ts_supp.h"
#include "push_pop.h"

extern int _xwrite(int f, char *b, int n, int tt);

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#define FILENO fileno_unlocked
#else	/* _THREAD_SAFE */
#define FILENO fileno
#endif	/* _THREAD_SAFE */


/*
 * FUNCTION:	The fwrite function writes, from the array pointed to by ptr,
 *		up to nmemb mebers whose size is specified by size, to the
 *		stream pointed to by stream.
 *
 * 		This version reads directly from the buffer rather than
 *		looping on putc.  Ptr args aren't checked for NULL because
 *		the program would be a catastrophic mess anyway.  Better
 *		to abort than just to return NULL.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The fwrite function returns the number of members successfully
 *		written, which will be less than nmemb only if a write error
 *	 	is encountered.
 *
 */                                                                   
size_t	
#ifdef	_THREAD_SAFE
fwrite_unlocked(const void *ptr, size_t size, size_t nmemb, FILE *stream)
#else
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
#endif	/* _THREAD_SAFE */
{
	unsigned nleft;
	int n;
	unsigned char *cptr, *bufend;
	int nwritten;

	if (size <= 0 || nmemb <= 0)
		return (0);

	if (_WRTCHK(stream)) {
		errno = EBADF;
		stream->_flag |= _IOERR;
		return(0);
	}

	bufend = _bufend(stream);
	nleft = nmemb*size;

	/* if the file is unbuffered, or if the buffer is empty and we are
	   writing more than a buffer full, do a direct write */
	if (stream->_base >= stream->_ptr)  {
		/* this covers the unbuffered case, too */
		if (stream->_flag & _IONBF
		    || nleft >= bufend - stream->_base)  {
			n = 0;
			cptr = (unsigned char *)ptr;
			while (nleft != 0) {
			  	nwritten = _xwrite(FILENO(stream),
				    (cptr+n), nleft, _IOISTTY & stream->_flag);
				if (nwritten > 0) {
					nleft -= nwritten;
					n += nwritten;
				} else {		/* write FAILED */
					stream->_flag |= _IOERR;
					break;		/* exit the WHILE */
				}
			}
			return (n/size);
		}
	}
	for (; ; ptr = (void *)((char *)ptr + n)) {
		while ((n = bufend - (cptr = stream->_ptr)) <= 0)  /* full buf */
			if (_xflsbuf(stream) == EOF) {
				return (nmemb - (nleft + size - 1)/size);
			}
		if (n > nleft) n = nleft;
		(void) memcpy((void *)cptr, (void *)ptr, (size_t)n);
		stream->_cnt -= n;
		stream->_ptr += n;
		_BUFSYNC(stream);
		if ((nleft -= n) == 0)
			break;
	}
	/* flush if linebuffered with a newline */
	if (stream->_flag & _IOLBF
	    && memchr((void *)cptr, (int)'\n', (size_t)n))
		(void) _xflsbuf(stream);
	return (nmemb);
}


#ifdef	_THREAD_SAFE
size_t	
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	register size_t rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	rc = fwrite_unlocked(ptr, size, nmemb, stream);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
