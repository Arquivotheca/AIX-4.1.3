static char sccsid[] = "@(#)44	1.11  src/bos/usr/ccs/lib/libc/ungetwc.c, libcio, bos411, 9428A410j 10/20/93 14:32:30";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: ungetwc
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
/* ungetwc.c,v $ $Revision: 1.8.2.2 $ (OSF) */

/*LINTLIBRARY*/
#include <stdio.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef  _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


wint_t	
ungetwc(wint_t c, FILE *stream)
{
	int bytesinchar;
	unsigned char *new_base;
	char mbstr[MB_LEN_MAX];
	TS_FDECLARELOCK(filelock)

	if (c == WEOF)
		return (WEOF);
	TS_FLOCK(filelock, stream);
	if (stream->_base == NULL && _findbuf(stream)) {
		TS_FUNLOCK(filelock);
		return (WEOF);
	}

	 _FORKCMP(stream);

	if (((bytesinchar = wctomb(mbstr,c)) == -1)
	    || ((stream->_base - 2 * MB_LEN_MAX)
		> (new_base = stream->_ptr - bytesinchar))
	    || (stream->_flag & _IOWRT)) {
		TS_FUNLOCK(filelock);
		return (WEOF);
	}

	/* If the stream is really a string (possibly in read-only storage),
	   can't write to it. */
	if (stream->_flag & _IONOFD) {
		if (new_base < stream->_base) {
			TS_FUNLOCK(filelock);
			return (WEOF);
		}
		stream->_ptr = new_base;
	}
	else {
		/* mark stream as having ungetc() chars on it.  See fseek() */
		stream->_flag |= _IOUNGETC;
		memcpy(stream->_ptr = new_base, mbstr, bytesinchar);
		if (stream->_ptr < (unsigned char *)stream->__newbase)
			stream->__newbase = (char *)stream->_ptr;
	}
	stream->_flag &= ~_IOEOF ;
	stream->_cnt += bytesinchar;
	TS_FUNLOCK(filelock);
	return (c);
}
