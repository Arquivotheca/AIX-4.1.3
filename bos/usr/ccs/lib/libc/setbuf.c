static char sccsid[] = "@(#)78	1.12  src/bos/usr/ccs/lib/libc/setbuf.c, libcio, bos411, 9428A410j 10/20/93 14:31:32";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: setbuf
 *
 *   ORIGINS: 3,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* setbuf.c,v $ $Revision: 2.6.2.3 $ (OSF) */

/*LINTLIBRARY*/
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#define FILENO	fileno_unlocked
#else	/* _THREAD_SAFE */
#define FILENO	fileno
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern int isatty();
extern unsigned char _smbuf[][_SBFSIZ];
extern unsigned char *_stdbuf[];


void 	
setbuf(FILE *stream, char *buf)
{
	int fno;  /* file number */
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	fno = FILENO(stream);

	if (stream->_base != NULL && stream->_flag & _IOMYBUF) {
		free((void *)(stream->_base - 2 * MB_LEN_MAX));
		stream->_base = NULL;
	}
	stream->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);

	if ((stream->_base = (unsigned char*)buf) == NULL) {
		stream->_flag |= _IONBF; /* file unbuffered except in fastio */

		if (fno < 2)  /* for stdin, stdout, use the existing bufs */
			_bufend(stream) = (stream->_base =
					  (_stdbuf[fno] + 2 * MB_LEN_MAX))
					  + BUFSIZ;

		else {	/* otherwise, use small buffer */
			stream->_base = (unsigned char *)
					malloc((size_t)_SBFSIZ + 8
						+ 2 * MB_LEN_MAX);

			if (stream->_base != NULL)
				stream->_base += 2 * MB_LEN_MAX;
			stream->_flag |= _IOMYBUF;
			_bufend(stream) = stream->_base + _SBFSIZ;
		}
	}
	else {  /* regular buffered I/O, standard buffer size */
		int current_errno = errno;
		_bufend(stream) = stream->_base
				  + (BUFSIZ - (8 + 2 * MB_LEN_MAX));
		if (isatty(fno))
			stream->_flag |= _IOLBF;
		else
			errno = current_errno;
	}
	stream->__newbase = stream->_ptr = stream->_base;
	stream->_cnt = 0;
	TS_FUNLOCK(filelock);
}
