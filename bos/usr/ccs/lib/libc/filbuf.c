static char sccsid[] = "@(#)26	1.15  src/bos/usr/ccs/lib/libc/filbuf.c, libcio, bos411, 9428A410j 4/20/94 17:43:55";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: __filbuf
 *		_filbuf
 *		_wcfilbuf
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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/* filbuf.c,v $ $Revision: 2.8.5.4 $ (OSF) */

/*LINTLIBRARY*/

#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include "stdiom.h"
#include <unistd.h>
#include <errno.h>

#include "glue.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

extern struct rec_mutex _iop_rmutex;
#endif /* _THREAD_SAFE */

extern _findbuf();
extern struct glued _glued;

#ifdef __filbuf
#undef __filbuf
#endif /* __filbuf */
int
__filbuf(iop)
register FILE *iop;
{
	register FILE *diop;
	register int i;
	TS_FDECLARELOCK(filelock)

	/* get buffer if we don't have one */
	if (iop->_base == NULL && _findbuf(iop))
		return (EOF);

	if ( !(iop->_flag & _IOREAD) )
		if (iop->_flag & _IORW)
			iop->_flag |= _IOREAD;
		else {
			iop->_flag |= _IOERR;
			errno = EBADF;
			return (EOF);
		}

	_FORKCMP(iop);

	/* if this device is a terminal (line-buffered) or unbuffered, then
	 * flush buffers of all line-buffered devices currently writing
	 */
	if (iop->_flag & (_IOLBF | _IONBF)) {

		/* TS: Grab iop_rmutex for the duration of the flush to save
		 * repeated relocking to find the next iob (tradeoff).
		 */
		TS_LOCK(&_iop_rmutex);
		TS_PUSH_CLNUP(&_iop_rmutex);
		for (i = 0; i < _glued.lastfile; i++) {
			diop = &_glued.iobptr[ i >> 4][i & ~0xfff0];
			/* do not flush the current stream */
			if (diop != iop && diop->_flag & _IOLBF) {
#ifdef  _THREAD_SAFE
				/* Avoid deadlock by only flushing file
				 * streams iff we can lock them, otherwise
				 * assume they're busy and leave alone.
				 */
				if (!TS_FTRYLOCK(filelock, diop))
					continue;
				TS_PUSH_CLNUP(filelock);
				(void) fflush_unlocked(diop);
				TS_POP_CLNUP(0);
				TS_FUNLOCK(filelock);
#else
				(void) fflush(diop);
#endif /* _THREAD_SAFE */
			}
		}
		TS_POP_CLNUP(0);
		TS_UNLOCK(&_iop_rmutex);
	}

	iop->_flag &= ~_IOUNGETC;
	iop->_ptr = iop->_base;
	iop->_cnt = read((int)fileno(iop), (char *)iop->_base,
			(unsigned)((iop->_flag & _IONBF) ? 1 : _bufsiz(iop) ));
	if (--iop->_cnt >= 0)		/* success */
		return (*iop->_ptr++);
	if (iop->_cnt != -1)		/* error */
		iop->_flag |= _IOERR;
	else {				/* end-of-file */
		iop->_flag |= _IOEOF;
		if (iop->_flag & _IORW)
			iop->_flag &= ~_IOREAD;
	}
	iop->_cnt = 0;
	return (EOF);
}

#ifdef _filbuf
#undef _filbuf
#endif /* _filbuf */
int
_filbuf(iop)
register FILE *iop;
{
	return (__filbuf(iop));
}

/*
 *	_wcfilbuf() is essentially the same as __filbuf except that it is
 *	only called by wide character read routines when they encounter a
 *	wide character straddling a buffer boundary.
 *	_wcfilbuf() places the existing contents of the buffer in the area
 *	before _b ase and then tries to populate the buffer from the underlying
 *	file._wcfilbuf() loops around the read till it has atleast charbytes
 *	number of charcters in the buffer or it encounters end-of-file or read
 *	error.
 *	Returns EOF on failure.
 */

int
_wcfilbuf(iop,charbytes)
register FILE *iop;	/* stream to read from */
int charbytes;		/* need to have these many chars in buffer to form a
			 * valid character.
			 */
{
	register FILE *diop;
	register int i,savecount,rc;
	register char *nextstart;
	TS_FDECLARELOCK(filelock)

	/* get buffer if we don't have one */
	if (iop->_base == NULL && _findbuf(iop))
		return (EOF);

	if ( !(iop->_flag & _IOREAD) )
		if (iop->_flag & _IORW)
			iop->_flag |= _IOREAD;
		else {
			iop->_flag |= _IOERR;
			errno = EBADF;
			return(EOF);
		}

	nextstart= (char *)iop->_base;

	_FORKCMP(iop);

	/* if this device is a terminal (line-buffered) or unbuffered, then
	 * flush buffers of all line-buffered devices currently writing
	 */
	if (iop->_flag & (_IOLBF | _IONBF)) {

		/* TS: Grab iop_rmutex for the duration of the flush to save
		 * repeated relocking to find the next iob (tradeoff).
		 */
		TS_LOCK(&_iop_rmutex);
		TS_PUSH_CLNUP(&_iop_rmutex);
		for (i = 0; i < _glued.lastfile; i++) {
			diop = &_glued.iobptr[ i >> 4][i & ~0xfff0];
			/* do not flush the current stream */
			if (diop != iop && diop->_flag & _IOLBF) {
#ifdef  _THREAD_SAFE
				/* Avoid deadlock by only flushing file
				 * streams iff we can lock them, otherwise
				 * assume they're busy and leave alone.
				 */
				if (!TS_FTRYLOCK(filelock, diop))
					continue;
				TS_PUSH_CLNUP(filelock);
				(void) fflush_unlocked(diop);
				TS_POP_CLNUP(0);
				TS_FUNLOCK(filelock);
#else
				(void) fflush(diop);
#endif /* _THREAD_SAFE */
			}
		}
		TS_POP_CLNUP(0);
		TS_UNLOCK(&_iop_rmutex);
	}

	iop->_flag &= ~_IOUNGETC;
	if (iop->_cnt > 0) {
		savecount=iop->_cnt;
		memcpy(iop->__newbase = (char *)iop->_base - savecount,
			iop->_ptr,savecount);
		iop->_ptr= (unsigned char *)iop->__newbase;

	} else {
		iop->__newbase= (char *)(iop->_ptr=iop->_base);
		iop->_cnt=0;
	}
	for(;;) {
		if ((rc = read((int)fileno(iop), (char *) nextstart,
			       (unsigned)((iop->_flag & _IONBF) ? 1 :
			       _bufend(iop)-(unsigned char *)nextstart))) > 0) {

			/* success */
			if ((iop->_cnt += rc) >= charbytes)
				return (0);
			nextstart += rc;
			continue;

		}
		if (rc == -1) {		/* error */
			iop->_flag |= _IOERR;
		}
		else {			/* end-of-file */
			if (!iop->_cnt) {
				iop->_flag |= _IOEOF;
				if (iop->_flag & _IORW)
					iop->_flag &= ~_IOREAD;
			}
			iop->_flag |= _IOERR;
			errno = EILSEQ;
		}
		return (EOF);
	}
}

