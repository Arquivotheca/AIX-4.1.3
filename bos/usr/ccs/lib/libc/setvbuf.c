static char sccsid[] = "@(#)00	1.13  src/bos/usr/ccs/lib/libc/setvbuf.c, libcio, bos411, 9428A410j 10/20/93 14:31:39";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: setvbuf
 *		setvbuf_unlocked
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
/* setvbuf.c,v $ $Revision: 1.7.2.3 $ (OSF) */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern void free();
extern int isatty();

/*
 * FUNCTION: Assigns buffering to a stream.
 */

int 	
#ifdef	_THREAD_SAFE
setvbuf_unlocked(FILE *stream, char *buf, int mode, size_t size)
#else
setvbuf(FILE *stream, char *buf, int mode, size_t size)
#endif	/* _THREAD_SAFE */
{
	if(stream->_base != NULL && stream->_flag & _IOMYBUF) {
		free((char*)(stream->_base-(2 * MB_LEN_MAX)));
		stream->_base = NULL;
	}
	stream->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);

	switch (mode) {
	/*note that the flags are the same as the possible values for mode*/
	case _IONBF:
		/* file is unbuffered */
		stream->_flag |= _IONBF;
		stream->_base = malloc((unsigned)(_SBFSIZ + 8 + 2*MB_LEN_MAX));
		if(stream->_base == NULL)
			return (-1);
		stream->_base += 2 * MB_LEN_MAX;
		stream->_flag |= _IOMYBUF;
		_bufend(stream) = stream->_base + _SBFSIZ;
		break;
	case _IOLBF:
	case _IOFBF:
		stream->_flag |= mode;
		size = (size == 0) ? BUFSIZ : size;
		stream->_base = malloc((unsigned)(size + 8 + 2 * MB_LEN_MAX));
		if(stream->_base == NULL)
			return (-1);
		stream->_base += 2 * MB_LEN_MAX;
		stream->_flag |= _IOMYBUF;
		_bufend(stream) = stream->_base + size;
		break;
	default:
		return (-1);
	}
	stream->__newbase = stream->_ptr = stream->_base;
	stream->_cnt = 0;
	return (0);
}


#ifdef	_THREAD_SAFE
int 	
setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
	register int rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = setvbuf_unlocked(stream, buf, mode, size);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
