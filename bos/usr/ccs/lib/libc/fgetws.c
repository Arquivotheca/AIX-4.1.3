static char sccsid[] = "@(#)76	1.4  src/bos/usr/ccs/lib/libc/fgetws.c, libcio, bos411, 9428A410j 4/20/94 17:43:36";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: RETURN
 *		fgetws
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* fgetws.c,v $ $Revision: 1.3.2.7 $ (OSF) */

/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getwc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "stdiom.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return(TS_FUNLOCK(filelock), (wchar_t *)(val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */


extern int __filbuf(),_wcfilbuf();

wchar_t 	*
fgetws(wchar_t *s, int n, FILE *stream)
{
	wchar_t *nextstart;
	char  *retptr;
	int converted,err;
	int failed = 0;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if ((stream->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
		errno = EBADF;
		stream->_flag |= _IOERR;
		RETURN(NULL);
	}
	nextstart=s;

	TS_PUSH_CLNUP(filelock);

	for(n--;;) {
		/* If the buffer was empty try reading some more from the file.
		 */
		if(stream->_cnt <= 0 ) {
			if(__filbuf(stream) == EOF)  {
				if(nextstart == s )  {
					failed = 1;
					goto pop_n_leave;
				}
				break; 
			}
			stream->_ptr--;
			stream->_cnt++;
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
		if((converted=__mbstopcs(nextstart,n,stream->_ptr,stream->_cnt,
					'\n',&retptr,&err)) > 0 ) {
			stream->_cnt-=retptr - (char *)stream->_ptr;
			stream->_ptr=(unsigned char *)retptr;
			nextstart += converted;
			if(((n-=converted) <= 0) || retptr[-1] == '\n')
				break;
			if(! err )
				continue;
		} else {
			if(converted == -1) {
				/* __mbstopcs not supported for this 
				 * code set.  Implement a version 
				 * using mbstowcs() or getwc() here.
				 * For now this is a stub.
				 */
				stream->_flag |= _IOERR;
				errno = EILSEQ;
				failed = 1;
				goto pop_n_leave;
			}
		}


		if(err == -1 || _wcfilbuf(stream,err) == EOF) {

			/* Invalid character found .
			 * We are here because either __mbstowcs returned a -1
			 * in err or we got a character straddling the buffer
			 * boundary and _wcfilbuf() was not able to read 
			 * sufficient number of bytes.
			 */

			*nextstart = (wchar_t)'\0';
			stream->_flag |= _IOERR;
			errno = EILSEQ;
			failed = 1;
			goto pop_n_leave;
		}

	}

pop_n_leave:

	TS_POP_CLNUP(0);

	if (failed)
		RETURN(NULL);

	*nextstart= (wchar_t)'\0';
	RETURN(s);
}
