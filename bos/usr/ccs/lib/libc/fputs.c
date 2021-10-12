static char sccsid[] = "@(#)81	1.11  src/bos/usr/ccs/lib/libc/fputs.c, libcio, bos411, 9428A410j 4/20/94 17:45:23";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fputs
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
/* fputs.c,v $ $Revision: 2.5.2.2 $ (OSF) */

/*LINTLIBRARY*/
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
 * FUNCTION: The  fputs subroutine  writes the  null-terminated string
 *           pointed to by the s parameter to the output stream speci-
 *           fied by the stream  parameter.  The fputs subroutine does
 *           not append a new-line character.
 *
 *           This version writes directly to the buffer rather than looping
 *           on putc.  Ptr args aren't checked for NULL because the program
 *           would be a catastrophic mess anyway.  Better to abort than just
 *           to return NULL.
 *
 * PARAMETERS: char *s      - NULL-terminated string to be written to
 *             FILE *stream      - File to be written to
 *
 * RETURN VALUE DESCRIPTIONS:
 *           Upon successful completion, the fputs subroutine returns
 *	     the number of characters written.  fputs returns EOF on an
 *	     error.  This happens if the routines try to write on a file
 *	     that has not been opened for writing.
 */
int 	
fputs(const char *s, FILE *stream)
{
	int ndone = 0, n;
	unsigned char *cs, *bufend;
	char *p;
	int rc = 0;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (_WRTCHK(stream)) {
		TS_FUNLOCK(filelock);
		return (EOF);
	}
	bufend = _bufend(stream);

	TS_PUSH_CLNUP(filelock);

	for ( ; ; s += n) {
		while ((n = bufend - (cs = stream->_ptr)) <= 0)  /* full buf */
			if (_xflsbuf(stream) == EOF) {
				POP_N_LEAVE(EOF);
			}
		if ((p = memccpy((char *) cs, s, '\0', n)) != NULL)
			n = (p - (char *) cs) - 1;
		stream->_cnt -= n;
		stream->_ptr += n;
		_BUFSYNC(stream);
		ndone += n;
		if (p != NULL)  { /* done; flush buffer if "unbuffered" or if
				     line-buffered */
			if (stream->_flag & (_IONBF | _IOLBF))
				if (_xflsbuf(stream) == EOF)
					ndone = EOF;
			POP_N_LEAVE(ndone);
		}
	}

pop_n_leave:
	TS_POP_CLNUP(0);
	TS_FUNLOCK(filelock);
	return(rc);
}
