static char sccsid[] = "@(#)04	1.8  src/bos/usr/ccs/lib/libc/getwc.c, libcio, bos411, 9428A410j 4/20/94 17:49:47";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: getwc
 *		getwc_unlocked
 *
 *   ORIGINS: 27,71
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
/* getwc.c,v $ $Revision: 1.7.2.5 $ (OSF) */

#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#define GETC getc_unlocked
#else
#define GETC getc
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


/*
 * FUNCTION:	
 * Get a multi-byte character and return it as a wide char.
 *
 * RETURN VALUE DESCRIPTION:	
 * Returns WEOF if getc returns EOF.
 *
 */  
wint_t
#ifdef	_THREAD_SAFE
getwc_unlocked(FILE *stream)
#else
getwc(FILE *stream)
#endif	/* _THREAD_SAFE */
{
	int	mbcurmax,charlen,ch;
	wchar_t	wc;
	int	err,err1;

	/* * * * * * * * * * * * *
	 * single byte code set	 *
 	* * * * * * * * * * * * */

	if (MB_CUR_MAX == 1)
		return(((ch = (GETC(stream))) == EOF) ? WEOF : ch);

	/* * * * * * * * * * * * * * * * * *
 	 * multi byte code set		   *
	 * * * * * * * * * * * * * * * * * */

	if (stream->_cnt <= 0) {	/* If buffer empty get data from file */
		if (__filbuf(stream) == EOF)
			return (WEOF);
		else {
			stream->_ptr--;
			stream->_cnt++;
		}
	}

	if ((charlen = __mbtopc(&wc, stream->_ptr, stream->_cnt, &err)) > 0 ) {
		stream->_ptr += charlen;
		stream->_cnt -= charlen;
		return (wc);
	}
	else {
		if (charlen == -1) {
			stream->_flag |= _IOERR;
			errno = EILSEQ;
			return (WEOF);
		}

		/* Either we got an illegal sequence or the character straddles
		 * the buffer boundary */
		if (err > 0) {
			/* There were not enough bytes in the buffer to form
			 * the character.
			 * Could be a split char or an illegal sequence
			 * Try to read some more from the underlying file
			 * so that we have atleast err bytes in the buffer.
			 */

			if (_wcfilbuf(stream, err) == EOF) {
				/*
				 * Couldn't get enough bytes to complete the
				 * character. Could be due to a physical read
				 * error or we hit the end-of-file.
				 * errno and _IOERR flag are set in wcfilbuf().
				 */
				return (WEOF);
			}

			/* Try converting again */

			if ((charlen = __mbtopc(&wc, stream->_ptr,
						err, &err1)) > 0) {
				/* Got a valid character straddling the buffer
				 * boundary
				 */
				stream->_cnt -= charlen;
				stream->_ptr += charlen;
				return (wc);
			}

			/* Has to be an illegal sequence */

			stream->_flag |= _IOERR;
			errno = EILSEQ;
			return (WEOF);
		}
		else {	/* err == -1, an illegal sequence */
			stream->_flag |= _IOERR;
			errno = EILSEQ;
			return (WEOF);
		}
	}
}


#ifdef	_THREAD_SAFE
wint_t
getwc(FILE *stream)
{
	register wint_t rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	rc = getwc_unlocked(stream);
	TS_POP_CLNUP(0);

	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
