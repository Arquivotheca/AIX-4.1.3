static char sccsid[] = "@(#)99	1.8  src/bos/usr/ccs/lib/libc/putwc.c, libcio, bos411, 9428A410j 4/20/94 17:51:46";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: putwc
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
/* putwc.c,v $ $Revision: 1.7.2.2 $ (OSF) */

#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#define PUTC	putc_unlocked
#define POP_N_LEAVE(val)	{ c = val; goto pop_n_leave; }
#else	/* _THREAD_SAFE */
#define PUTC	putc
#define POP_N_LEAVE(val)	return(val)
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "stdiom.h"
#include <errno.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#undef putwc


/*
 * FUNCTION:    A subroutine version of the macro putc.  This function is
 *              created to meet ANSI C standards.  The putc function writes
 *              the character specified by c (converted to an unsigned char)
 *              to the output stream pointed to by stream, at the position
 *              indicated by the assoicated file poistion indicator for the
 *              stream, and advances the indicator appropriately.
 *
 * RETURN VALUE DESCRIPTION:	
 *              The putwc function returns the character written.  If a write
 *              error occurs, the error indicator for the stream is set and
 *              putc returns WEOF.
 *
 */
wint_t
putwc(wint_t c, FILE *stream)
{
	int	i, rc, ch;
        char	mbstr[MB_LEN_MAX], *mb;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

        if(MB_CUR_MAX == 1) {

		TS_PUSH_CLNUP(filelock);
		ch = PUTC(c, stream);
		TS_POP_CLNUP(0);

		TS_FUNLOCK(filelock);
		return (ch == EOF ? WEOF : (wint_t) ch);
	}

        if ((rc = wctomb(mbstr, c)) <= 0) {
                stream->_flag |= _IOERR;
                errno = EILSEQ;
                TS_FUNLOCK(filelock);
                return (WEOF);
        }

        if (stream->_cnt - rc < 0) {
                mb = mbstr;

		TS_PUSH_CLNUP(filelock);

                for(i = 0; i < rc; i++) {
                        if (PUTC(*mb++, stream) == EOF) {
				POP_N_LEAVE(WEOF);
			}
		}
pop_n_leave:
		TS_POP_CLNUP(0);

		TS_FUNLOCK(filelock);
                return (c);	/* c is set by POP_N_LEAVE(x) to the value x */
        }
	memcpy(stream->_ptr,mbstr,rc);
	stream->_ptr += rc;
	stream->_cnt -= rc;
	_BUFSYNC(stream);
	TS_FUNLOCK(filelock);
	return (c);
}
