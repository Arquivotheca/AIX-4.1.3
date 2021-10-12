static char sccsid[] = "@(#)78	1.4  src/bos/usr/ccs/lib/libc/getws.c, libcio, bos411, 9428A410j 4/20/94 17:50:02";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: RETURN
 *		getws
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
/* getws.c,v $ $Revision: 1.3.2.5 $ (OSF) */

/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)		return(TS_FUNLOCK(filelock), (wchar_t *)(val))
#define POP_N_LEAVE(val)	{ s = val; goto pop_n_leave; }

#else
#define	RETURN(val)		return(val)
#define POP_N_LEAVE(val)	return(val)
#endif	/* _THREAD_SAFE */


extern int __filbuf(), _wcfilbuf();

wchar_t *
getws(wchar_t *s)
{
	wchar_t *nextstart;
	char *retptr;
	int converted, err;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdin);

	if ((stdin->_flag & (_IOREAD|_IORW)) == 0) {  /* not open for reading */
		stdin->_flag |= _IOERR;	/* How could this happen? */
		errno = EBADF;
		RETURN(NULL);
	}

	TS_PUSH_CLNUP(filelock);

	for (nextstart = s;;) {
		/* If the buffer was empty try reading some more from the file.
		 */
		if (stdin->_cnt <= 0) {
			if (__filbuf(stdin) == EOF)  {
				if (nextstart == s) {
					POP_N_LEAVE(NULL);
				}
				break; 
			}
			stdin->_ptr--;
			stdin->_cnt++;
		}

		/* Try converting the bytes in the buffer to process code.
		 * __mbstopcs (
		 *	wchar_t *  -  start storing the converted wchar here.
		 *	size_t     -  upto a maximum of these many wchars.
		 *	char *     -  start reading the bytestring from here.
		 *	size_t     -  upto a maximum of these many bytes.
		 *	uchar *    -  stop converting if this byte encountered.
		 *	char **    -  pointer in bytestring after byte where 
		 *		      conversion stopped.
		 *	int *      -  Has the return error indicator
		 *		      0 => success.
		 *		      -1 => illegal character.
		 *		      >0 => not enough characters in the buffer
		 *			    to form a valid character.Need at
		 *			    least these many.
		 *	)
		 *
		 * __mbstopcs() returns number of wide characters converted 
		 * before stopping or -1 if it is not supported for this code
		 * set.
		 */

		if ((converted = __mbstopcs(nextstart, stdin->_cnt, stdin->_ptr, stdin->_cnt, '\n',
					    &retptr, &err)) > 0) {
			stdin->_cnt-=retptr - (char *)stdin->_ptr;
			stdin->_ptr = (unsigned char *)retptr;
			if (retptr[-1] == '\n') {
				/* getws() return when a newline character is
				 * found.
				 */
				nextstart[converted-1] = '\0';
				POP_N_LEAVE(s);
			}
			nextstart += converted;
			if (!err)  /* Get more bytes to convert */
				continue;
		} else {
			if (converted == -1) {
				/* __mbstopcs not supported for this 
				 * code set.  Implement a version 
				 * using mbstowcs() or getwc() here.
				 * For now this is a stub.
				 */
				stdin->_flag |= _IOERR;
				errno = EILSEQ;
				POP_N_LEAVE(NULL);
			}
		}

		if (err == -1 || _wcfilbuf(stdin, err) == EOF) {  
			/* Invalid character found .
			 * We are here because either __mbstowcs returned a -1
			 * in err or we got a character straddling the buffer
			 * boundary and _wcfilbuf() was not able to read 
			 * sufficient number of bytes.
			 */
			*nextstart = '\0';
			stdin->_flag |= _IOERR;
			errno = EILSEQ;
			POP_N_LEAVE(NULL);
		}
	}
	*nextstart = '\0';

pop_n_leave:
	TS_POP_CLNUP(0);
	RETURN(s);
}
