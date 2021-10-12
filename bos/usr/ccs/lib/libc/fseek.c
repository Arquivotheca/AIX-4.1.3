static char sccsid[] = "@(#)02	1.23  src/bos/usr/ccs/lib/libc/fseek.c, libcio, bos411, 9428A410j 6/13/94 15:02:09";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fseek
 *		fseek_unlocked
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
/* fseek.c,v $ $Revision: 2.10.2.3 $ (OSF) */

/*LINTLIBRARY*/
/*
 * Seek for standard library.  Coordinates with buffering.
 */
#include <stdio.h>
#include <sys/errno.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#define FILENO fileno_unlocked
#define FFLUSH fflush_unlocked
#else	/* _THREAD_SAFE */
#define FILENO fileno
#define FFLUSH fflush
#endif	/* _THREAD_SAFE */

extern long lseek();

int 	
#ifdef _THREAD_SAFE
fseek_unlocked(FILE *stream, long int offset, int whence)
#else
fseek(FILE *stream, long int offset, int whence)
#endif	/* _THREAD_SAFE */
{
	int c;
	long	p;

	stream->_flag &= ~_IOEOF;
	if(stream->_flag & _IOREAD) {
                _FORKCMP(stream);
		if(whence < 2 && stream->_base && !(stream->_flag&_IONBF)) {
			c = stream->_cnt;
			p = offset;
			if(whence == 0)
				p += (long)c-lseek(FILENO(stream), 0L, SEEK_CUR);
			else
				offset -= (long)c;
			if(!(stream->_flag&_IORW) && c > 0 && p <= c &&
                                        p >= (unsigned char *)stream->__newbase - stream->_ptr) {

				/* fseek() must erase any characters put on stream by ungetc() */
				if (stream->_flag & _IOUNGETC) {
					int fd = FILENO(stream);

					stream->_flag &= ~_IOUNGETC;
                                        (void)lseek(fd, -c, SEEK_CUR);
                                        read(fd, stream->_ptr, c);
				}
				stream->_ptr += (int)p;
                                stream->_cnt -= (int)p;
				return(0);
			}
		}
		if(stream->_flag & _IORW) {
                        stream->__newbase = stream->_ptr = stream->_base;
			stream->_flag &= ~_IOREAD;
		}
		if ((p = lseek(FILENO(stream), offset, whence)) != -1)
		    stream->_cnt = 0;
	} else if(stream->_flag & (_IOWRT | _IORW)) {
	    	p = FFLUSH(stream);
		if(stream->_flag & _IORW) {
			stream->_cnt = 0;
			stream->_flag &= ~_IOWRT;
                        stream->__newbase =  stream->_ptr = stream->_base;
		}
		/* p84136 - for XPG4 */
		if (!p)
		    p = lseek(fileno(stream), offset, whence);
/*********
  p144784:  XPG4 requires EPIPE here instead of ESPIPE, but they have an
	    open interpretation request on the issue so use ESPIPE until
	    this is resolved
		    if (((p = lseek(fileno(stream), offset, whence)) == -1) &&
			(errno == ESPIPE))
			errno = EPIPE;
**********/

	}
	else {
		errno = EBADF;
		p = -1;			/* invalid or closed file block ptr */
	}
	return((p == -1)? -1: 0);
}


#ifdef	_THREAD_SAFE
int 	
fseek(FILE *stream, long int offset, int whence)
{
	register int rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	rc = fseek_unlocked(stream, offset, whence);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
