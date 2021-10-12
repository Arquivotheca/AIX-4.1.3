static char sccsid[] = "@(#)79	1.3  src/bos/usr/ccs/lib/libc/putws.c, libcio, bos411, 9428A410j 4/20/94 17:52:05";
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: putws
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
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* putws.c,v $ $Revision: 1.3.2.4 $ (OSF) */

/*LINTLIBRARY*/
#ifdef _THREAD_SAFE
/* Assume streams in this module are already locked.
 */
#define _STDIO_UNLOCK_CHAR_IO
#define PUTC    putc_unlocked
#else 
#define PUTC    putc
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <errno.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)		return(TS_FUNLOCK(filelock), (val))
#define POP_N_LEAVE(val)	{ rc = val; goto pop_n_leave; }

#else
#define	RETURN(val)		return(val)
#define POP_N_LEAVE(val)	return(val)
#endif	/* _THREAD_SAFE */

extern char *memccpy();

/*
 * FUNCTION: The  putws subroutine  writes the  null-terminated string
 *           pointed to by the s parameter to the standard output .
 *	     It appends a newline character.
 *
 *           This version writes directly to the buffer rather than looping
 *           on putwc.  Ptr args aren't checked for NULL because the program
 *           would be a catastrophic mess anyway.  Better to abort than just
 *           to return NULL.
 *
 * PARAMETERS: wchar_t *s      - NULL-terminated string to be written to
 *				 stdout
 * RETURN VALUE DESCRIPTIONS:
 *           Upon successful completion, the putws subroutine returns
 *	     the number of characters written.  putws returns EOF on an
 *	     error.  This happens if the routines try to write on a file
 *	     that has not been opened for writing.
 */
int 	
putws(const wchar_t *s)
{
	int	err = 0, ndone = 0; 
	int	n, rc, wccnt, bufsize, i;
	unsigned char *bufend, *retptr, mbstr[MB_LEN_MAX], *mb;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdout);

	if (_WRTCHK(stdout)) {
		RETURN(-1);
	}

	bufend = _bufend(stdout);
	bufsize = _bufsiz(stdout);

	TS_PUSH_CLNUP(filelock);

	for (wccnt = wcslen(s); wccnt > 0 ; ) {

		if (err > bufsize) {	/* the buffer does have space */
			rc = wctomb(mb = mbstr, s[ndone]);

			if (rc == -1) {			/* Bad conversion */
				stdout->_flag |= _IOERR;
				errno = EILSEQ;
				POP_N_LEAVE(WEOF);
			}

			for(i = 0; i < rc; i++) {
				if (PUTC(*mb++, stdout) == EOF) {
					POP_N_LEAVE(WEOF);
				}
			}
			err = 0; wccnt--; ndone++;
			continue;
		}

		/* check for full buf */
		while ((n = bufend - stdout->_ptr) < err || n == 0) {
			if (_xflsbuf(stdout) == EOF) {
				POP_N_LEAVE(WEOF);
			}
		}

		err = 0;	/* this is not reqd if __pcstombs sets it */

		/* Try to convert the process code string into byte string
		 * and place the string into the stdio buffer.
		 * __pcstombs(
		 *    wchar_t *  - start point of process code string.
		 *    size_t     - stop converting after these many wchars.
		 *    char *     - start placing the converted bytes here.
		 *    size_t     - amount of bytes available in the byte string.
		 *    char **    - returns the pointer to a position in byte
		 *   		   string where the next converted character
		 *  		   would have been placed.
		 *    int  *     - The return error indicator.
		 *                 0 => success.
		 *		  -1 => illegal wchar in wchar string.
		 *		  >0 => There was not enough space in the byte
		 *                      string after the last character to 
		 *			place the next character.The return
		 *			value indicates the number of bytes
		 *			reqd. to store the next character.
		 *           )
		 *
		 *   __pcstombs() returns number of wide characters converted
		 *   or -1 if it is not supported for this code set.
		 */
		if ((rc = __pcstombs(s + ndone, wccnt, stdout->_ptr, n,
				     &retptr, &err)) > 0) {
			if (err == -1) {
				if (ndone > 0
				    && stdout->_flag & (_IONBF | _IOLBF))
					_xflsbuf(stdout);
				stdout->_flag |= _IOERR;
				errno = EILSEQ;
				POP_N_LEAVE(-1);
			}
			stdout->_cnt -= (retptr - stdout->_ptr);
			stdout->_ptr = retptr;
			wccnt -= rc;
			ndone += rc;
			_BUFSYNC(stdout);
			continue;	
		}
		else {
			if (rc == 0)  {
				/* err>0 We failed as there was not 
				 * enough space in the buffer
				 * to accomodate even one character.
				 */
				if (err > 0) 
					continue;

				/* err == 0, shouldn' happen ? */
				if (err == 0)
					break;

				/* else err == -1, illegal sequnce 
				 * flush buffer if reqd and exit *
			 	 */
				if (ndone > 0
				    && stdout->_flag & (_IONBF | _IOLBF))
					_xflsbuf(stdout);
				stdout->_flag |= _IOERR;
				errno = EILSEQ;
				POP_N_LEAVE(-1);
			}
			else {
				/* rc == -1 => __pcstombs not supported *
				 * For now we return -1 later we can substitute
			  	 * code for implementing putws() using putwc()
				 * in here.
				 */
				wchar_t c, *start = (wchar_t *)s;

				while (c = *s++)
					if (putwc(c, stdout) == WEOF) {
						POP_N_LEAVE(WEOF);
					}
				putwc('\n', stdout);
				POP_N_LEAVE(s - start);
			}
		}
			
	}
	_BUFSYNC(stdout);
	PUTC('\n', stdout);
	if (stdout->_flag & (_IONBF | _IOLBF))
		if (_xflsbuf(stdout) == EOF) {
			POP_N_LEAVE(WEOF);
		}
	rc = ndone + 1; 	/* account for the newline character */

pop_n_leave:
	TS_POP_CLNUP(0);
	RETURN(rc);		/* rc is set by POP_N_LEAVE(x) to the value x */
}
