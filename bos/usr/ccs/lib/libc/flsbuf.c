static char sccsid[] = "@(#)48	1.34  src/bos/usr/ccs/lib/libc/flsbuf.c, libcio, bos41J, 9511A_all 2/10/95 17:26:13";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: FERROR
 *		FFLUSH
 *		FILENO
 *		PUTC
 *		__flsbuf
 *		_bufsync
 *		_cleanup
 *		_findbuf
 *		_flsbuf
 *		_wrtchk
 *		_xflsbuf
 *		_xwrite
 *		fclose
 *		fclose_unlocked
 *		fflush
 *		fflush_unlocked
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* flsbuf.c,v $ $Revision: 2.13.3.3 $ (OSF) */

/*LINTLIBRARY*/
#include "stdiom.h"
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "glue.h"
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif /* _THREAD_SAFE */

extern struct glued _glued;
extern unsigned char *_stdbuf[];
extern int _xwrite(int, char *, int, int);

#ifdef _THREAD_SAFE
extern struct rec_mutex _iop_rmutex;

#define	FFLUSH(fp)	fflush_unlocked(fp)
#define	FERROR(fp)	ferror_unlocked(fp)
#define	FILENO(fp)	fileno_unlocked(fp)
#define	PUTC(c, fp)	putc_unlocked(c, fp)

#else
#define	FFLUSH(fp)	fflush(fp)
#define	FERROR(fp)	ferror(fp)
#define	FILENO(fp)	fileno(fp)
#define	PUTC(c, fp)	putc(c, fp)
#endif /* _THREAD_SAFE */


/*
 * Flush buffers on exit
 */
void
_cleanup()
{
	register FILE *iop;
	register int i;
	int row, last_file;
	TS_FDECLARELOCK(filelock)

	/*
	 * For multiple threads this is ugly. We try to flush and
	 * close all streams which are open for writing when _cleanup()
	 * is called. Since holding the _iop_rmutex and trying to lock
	 * a stream may deadlock we take an iob row at a time from the
	 * iobptr list (iob rows are never freed). When a locked stream
	 * is encountered we only block if it is writing.
	 * Since we are on the exit path we do not free malloc'd storage.
	 */
	row = 0;
	TS_LOCK(&_iop_rmutex);
	last_file = _glued.lastfile;
	iop = _glued.iobptr[row];
	TS_UNLOCK(&_iop_rmutex);

	for (;;) {
		for (i = 0; i < _NIOBRW; i++, iop++) {
			if (!(iop->_flag))
				continue;
#ifdef _THREAD_SAFE
			if (!TS_FTRYLOCK(filelock, iop)) {
				if (!(iop->_flag & _IOWRT))
					continue;
				TS_FLOCK(filelock, iop);
			}
			TS_PUSH_CLNUP(filelock);
			(void) fflush_unlocked(iop);
			TS_POP_CLNUP(0);
			TS_FUNLOCK(filelock);
#else
			(void) fflush(iop);
#endif	/* _THREAD_SAFE */
		}
		if ((last_file -= _NIOBRW) <= 0)
			break;

		TS_LOCK(&_iop_rmutex);
		iop = _glued.iobptr[++row];
		TS_UNLOCK(&_iop_rmutex);
	}
}

/*
 *	fclose() will flush (output) buffers for a buffered open
 *	FILE and then issue a system close on the _fileno.  The
 *	_base field will be reset to NULL for any but stdin and
 *	stdout, the _ptr field will be set the same as the _base
 *	field. The  _cnt field will be zeroed.  If buffers had been
 *	obtained via malloc(), the space will be free()'d.  In case
 *	the FILE was not open, or fflush() or close() failed, an EOF
 *	will be returned, otherwise the return value is 0.
 *
 *	If the stream is not open for read, read write, or write, then
 *	we should not be trying to close it. Set return equal to EOF and
 *	set errno to EBADF.
 *
 *	The stream flag should be set to IOCLOSE - but because IOCLOSE
 *	does not equal zero and some routines depend on a close'd stream
 *	having the flags zero'd out, then we must set it to zero instead.
 */
int 	
#ifdef _THREAD_SAFE
fclose_unlocked(FILE *stream)
#else
fclose(FILE *stream)
#endif	/* _THREAD_SAFE */
{
	int	rtn = EOF;
	int	fd;

	if (stream == NULL) {
		errno = EBADF;
		return(rtn);
	}
	if (stream->_flag & (_IOREAD | _IOWRT | _IORW)) {
		rtn = (stream->_flag & _IONBF)? 0: FFLUSH(stream);
		if (close((int)FILENO(stream)) < 0) {
			rtn = EOF;
		}
	}
	else {
		rtn = EOF;
		errno = EBADF;
	}
	if (stream->_flag & _IOMYBUF) {
		free((void *)(stream->_base - 2 * MB_LEN_MAX));
		stream->_base = NULL;
	}

	TS_LOCK(&_iop_rmutex);

	_glued.freefile = 0;

	TS_UNLOCK(&_iop_rmutex);

	stream->_flag = 0;
	stream->_cnt = 0;
	stream->__newbase = (char *)(stream->_ptr = stream->_base);
	stream->__stdioid=0;

	return (rtn);
}


#ifdef _THREAD_SAFE
int
fclose(FILE *stream)
{
	int rc;
	TS_FDECLARELOCK(filelock)

	if (stream == NULL) {
		errno = EBADF;
		return (EOF);
	}
	TS_FLOCK(filelock, stream);
	TS_PUSH_CLNUP(filelock);
	rc = fclose_unlocked(stream);
	TS_POP_CLNUP(0);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */


/*
 *	The fflush() routine must take care because of the
 *	possibility for recursion. The calling program might
 *	do IO in an interupt catching routine that is likely
 *	to interupt the write() call within fflush()
 */
int 	
#ifdef _THREAD_SAFE
fflush_unlocked(FILE *stream)
#else
fflush(FILE *stream)
#endif	/* _THREAD_SAFE */
{
	register FILE *iop;
	register int i;
	int rval = 0;
	TS_FDECLARELOCK(filelock)

	/* fflush(NULL) implements all requirements of ANSI C  plus
	 * the additional requirements from POSIX 1003b/D4 which deal
	 * mainly with flushing of streams where the last operation
	 * was input.
	 */
	/*
	 * Flush all buffers recusively.
	 * This should do the right thing for thread safe also.
	 */
	if (stream == NULL) {
		TS_LOCK(&_iop_rmutex);
		TS_PUSH_CLNUP(&_iop_rmutex);
		for (i = 0; i <= _glued.lastfile; i++) {
			if ((iop = &_glued.iobptr[ i >> 4][i & ~0xfff0])
				&& inuse(iop)) {

				/* Avoid deadlock by only flushing file
				 * streams iff we can lock them, otherwise
				 * assume they're busy and leave alone.
				 */
				if (!TS_FTRYLOCK(filelock, iop))
					continue;
				TS_PUSH_CLNUP(filelock);
				if (FFLUSH(iop) != 0)
					rval = EOF;
				TS_POP_CLNUP(0);
				TS_FUNLOCK(filelock);
			}
		}
		TS_POP_CLNUP(0);
		TS_UNLOCK(&_iop_rmutex);
		return (rval);
	}

	/* If the stream is closed, then it shouldn't be fflushed.
	 * Can't check the IOCLOSE flag because fclose has to zero
	 * out the flag's not set it to IOCLOSE.
	 */

	if (stream->_flag == 0) {
		errno = EBADF;
		return (EOF);
	}

	/* If the underlying file descriptor has been closed, and  */
	/* fclose() wasn't called on this file pointer, XPG4 says  */
	/* we should set errno to EBADF and return EOF.	           */
	/* Since fflush usually ends up calling write() which does */
	/* check for a closed file descriptor, this isn't normally */
	/* a problem.  However, if fflush should be called before  */
	/* a buffer is allocated (i.e. _ptr is NULL) or while the  */
	/* buffer is empty (i.e. sometime before the file descript */
	/* was closed, the buffer had already been flushed), the   */
	/* write() routine doesn't get called, and fflush returns  */
	/* normally without setting EBADF.  So, for the case where */
	/* the buffer is empty or isn't allocated yet, call fcntl  */
	/* to check the status of the the file descriptor.	   */

	if ((stream->_base == NULL || stream->_ptr == stream->_base) &&
		(fcntl(stream->_file, F_GETFL) == -1)) {
			if(stream->_flag & _IOMYBUF) {
				free((void *)(stream->_base - 2 * MB_LEN_MAX));
				stream->_base = NULL;
			}
			stream->_flag = 0;
			return(EOF);	/* fcntl() will set errno=EBADF */
		}

	/* Perform the check to test whether the application is     *
	 * following the POSIX.1 streamio rules or not.		    *
	 */

	_FORKCMP(stream) ;

	/*
	 * If the last operation on the stream was a read invalidate
	 * the buffer and adjust the underlying open file description
	 * so that the next operation deals with the byte after the
	 * last one read from the stream.
	 * Adjust the underlying filedescription only if
	 *  i) the last operation was a read.
	 * ii) The application is following POSIX.1 streamio rules.
	 *iii) Application is not following POSIX rules but called
	 *     fflush() directly and not from fclose().
	 * iv) There is something in the buffer and the stream is
	 *     not at EOF. (_cnt > 0 ).
	 *  v) This should not be done for non buffered streams as
	 *     they are always flushed except in case of where
	 *     characters were put back using ungetc,which going by
	 *     ANSI fseek definition ought to be discarded.
	 *
	 * If the underlying file description points to a nonseekable
	 * device do not invalidate the buffer as data would be lost
	 * if we do so.
	 */
	if (!(stream->_flag & _IOWRT)) {
		if (!(stream->_flag & _IONONSTD)) {
			if (!(stream->_flag & _IONBF)
			    && (stream->_flag & _IOREAD)
			    && (stream->_cnt > 0)) {
				if (lseek((int)FILENO(stream),
					  (off_t)(-stream->_cnt),
					  SEEK_CUR) == -1
				    && errno == ESPIPE)
					return (0);
			}
		}

		stream->_flag &= ~_IOUNGETC;
		stream->_cnt = 0;
		stream->__newbase = (char *)(stream->_ptr= stream->_base);
		return (0);
	}
	while (!(stream->_flag & _IONBF) && (stream->_flag & _IOWRT)
	       && (stream->_base != NULL) && (stream->_ptr > stream->_base))
		(void)_xflsbuf(stream);
	/*
	 * ANSI C allows output to be followed by input after an intervening
	 * call to fflush().For this to work properly the stream count has
	 * be reset to 0 to force a call to __filbuf on next read.
	 */
	if (stream->_base == stream->_ptr && stream->_flag & _IORW) {
		stream->_cnt = 0;
		stream->_flag &= ~_IOWRT;
	}

	stream->_flag &= ~_IOUNGETC;
	stream->__newbase = (char *)stream->_base;
	return (FERROR(stream) ? EOF : 0);
}


#ifdef _THREAD_SAFE
/*
 * 	In the null case, we call fflush_unlocked, which
 * 	will call fflush again, so locking should be ok.
 */
int
fflush(FILE *stream)
{
	int	rc;
	TS_FDECLARELOCK(filelock);

	if (stream == NULL)
		return (fflush_unlocked(NULL));
	TS_FLOCK(filelock, stream);
	TS_PUSH_CLNUP(filelock);
	rc = fflush_unlocked(stream);
	TS_POP_CLNUP(0);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */


/* The routine __flsbuf may or may not actually flush the output buffer.  If
 * the file is line-buffered, the fact that iop->_cnt has run below zero
 * is meaningless: it is always kept below zero so that invocations of putc
 * will consistently give control to __flsbuf, even if the buffer is far from
 * full.  __flsbuf, on seeing the "line-buffered" flag, determines whether the
 * buffer is actually full by comparing iop->_ptr to the end-of-buffer pointer
 * _bufend(iop).  If it is full, or if an output line is completed (with a
 * newline), the buffer is flushed.  (Note: the character argument to __flsbuf
 * is not flushed with the current buffer if the buffer is actually full--
 * it goes into the buffer after flushing.)
 */
#ifdef __flsbuf
#undef __flsbuf
#endif /* __flsbuf */
int
__flsbuf(unsigned char c, register FILE *iop)
{
	unsigned char c1;

    do {
	if ((iop->_flag & (_IOLBF | _IOWRT | _IOEOF)) == (_IOLBF | _IOWRT)) {

		_FORKCMP(iop);

		if (iop->_ptr >= _bufend(iop))  /* if buffer full, */
			break;		/* exit do-while, and flush buf. */
		if ((*iop->_ptr++ = c) != '\n')
			return (c);
		return (_xflsbuf(iop) == EOF ? EOF : c);
	}
	if ((iop->_flag & (_IONBF | _IOWRT | _IOEOF)) == (_IONBF | _IOWRT)) {
		iop->_cnt = 0;
		c1 = c;
		if (1 == _xwrite((int)FILENO(iop), (char *)&c1, (unsigned)1, _IOISTTY & iop->_flag))
			return (c);
		iop->_flag |= _IOERR;
		return (EOF);
	}
	/*
	 * The _wrtchk call is here rather than at the top of __flsbuf to
	 * reduce overhead for line-buffered I/O under normal circumstances.
	 */

	if (_WRTCHK(iop)) { /* is writing legitimate? */
		iop->_flag |= _IOERR;
		errno = EBADF;
		return (EOF);
	}
    } while ((iop->_flag & (_IONBF | _IOLBF)));


    (void) _xflsbuf(iop);   /* full buffer:  flush buffer */
    (void) PUTC((char) c, iop);  /* then put "c" in newly emptied buf */
			/* (which, because of signals, may NOT be empty) */
    return (FERROR(iop) ? EOF : c);
}

#ifdef _flsbuf
#undef _flsbuf
#endif /* _flsbuf */
int
_flsbuf(unsigned char c, register FILE *iop)
{
	return (__flsbuf(c, iop));
}


/* The function _xflsbuf writes out the current contents of the output
 * buffer delimited by iop->_base and iop->_ptr.
 * iop->_cnt is reset appropriately, but its value on entry to _xflsbuf
 * is ignored.
 *
 * The following code is not strictly correct.  If a signal is raised,
 * invoking a signal-handler which generates output into the same buffer
 * being flushed, a peculiar output sequence may result (for example,
 * the output generated by the signal-handler may appear twice).  At
 * present no means has been found to guarantee correct behavior without
 * resorting to the disabling of signals, a means considered too expensive.
 * For now the code has been written with the intent of reducing the
 * probability of strange effects and, when they do occur, of confining
 * the damage.  Except under extremely pathological circumstances, this
 * code should be expected to respect buffer boundaries even in the face
 * of interrupts and other signals.
 */
int
_xflsbuf(iop)
register FILE *iop;
{
	register unsigned char *base;
	register int n;
	register int nwritten;
	register int offset;

	n = iop->_ptr - (base = iop->_base);
	iop->__newbase = (char *)(iop->_ptr = base);
	iop->_cnt = (iop->_flag &(_IONBF | _IOLBF)) ? 0 : _bufsiz(iop);
	iop->_flag &= ~_IOUNGETC;
	_BUFSYNC(iop);

	offset = 0;
	while (n > 0) {
		nwritten = _xwrite((int)FILENO(iop),(char*)(base+offset), 
		    (unsigned)n, _IOISTTY & iop->_flag);
		if (nwritten > 0) {
			n -= nwritten;
			offset += nwritten;
		} else {
			iop->_flag |= _IOERR;
			return (EOF);
		}
	}

	return (0);
}


/*
 * The function _wrtchk checks to see whether it is legitimate to write
 * to the specified device.  If it is, _wrtchk sets flags in iop->_flag for
 * writing, assures presence of a buffer, and returns 0.  If writing is not
 * legitimate, EOF is returned.
 */
int
_wrtchk(register FILE *iop)
{
	if ((iop->_flag & (_IOWRT | _IOEOF)) != _IOWRT) {
		if (!(iop->_flag & (_IOWRT | _IORW))) {
			iop->_flag |= _IOERR;
			errno = EBADF;
			return (EOF);  /* bogus call--read-only file */
		}
		iop->_flag = iop->_flag & ~_IOEOF | _IOWRT; /* fix flags */
	}
	/* if this is first I/O to file--get buffer */
	if (iop->_base == NULL && _findbuf(iop))
		return (EOF);

	_FORKCMP(iop);

	if (iop->_ptr == iop->_base && !(iop->_flag & (_IONBF | _IOLBF)))  {
		iop->_cnt = _bufsiz(iop); /* first write since seek--set cnt */
		_BUFSYNC(iop);
	}
	return (0);
}


/*
 * _findbuf, called only when iop->_base == NULL, locates a predefined buffer
 * or allocates a buffer using malloc.  If a buffer is obtained from malloc,
 * the _IOMYBUF flag is set in iop->_flag.
 */
int
_findbuf(iop)
register FILE *iop;
{
	register int fno = FILENO(iop); /* file number */
	int size = BUFSIZ;
	struct stat stbuf;
	int saved_errno;

	saved_errno = errno;
	if (fno < 2)  /* use existing bufs for stdin, stdout */
		_bufend(iop) = (unsigned char *)(iop->__newbase =
			       (char *)(iop->_base =
			       _stdbuf[fno] + 2 * MB_LEN_MAX)) + BUFSIZ;
	else
	{
		if (iop->_base = malloc((size_t)(size+8+2*MB_LEN_MAX))) {
			/* use a normal sized buffer
			 */
			iop->_base += 2 * MB_LEN_MAX ;
			iop->_flag |= _IOMYBUF;
			_bufend(iop) = iop->_base + size;
		} else if (iop->_base =
			   malloc((size_t)(_SBFSIZ+8 + 2 * MB_LEN_MAX)))  {
			/* make do with small buffer
			 */
			iop->_base += 2 * MB_LEN_MAX ;
			iop->_flag |= _IOMYBUF;
			_bufend(iop) = iop->_base + _SBFSIZ;
		} else {
			/* no space left
			 */
			errno = ENOMEM;
			iop->_flag |= _IOERR;
			return (-1);
		}
	}

	iop->__newbase = (char *)(iop->_ptr = iop->_base);
	if (isatty(fno) && !(iop->_flag & _IONBF))
		iop->_flag |= _IOLBF;
	errno = saved_errno;
	return (0);
}


/*
 * The function _bufsync is called because interrupts and other signals
 * which occur in between the decrementing of iop->_cnt and the incrementing
 * of iop->_ptr, or in other contexts as well, may upset the synchronization
 * of iop->_cnt and iop->ptr.  If this happens, calling _bufsync should
 * resynchronize the two quantities (this is not always possible).  Resyn-
 * chronization guarantees that putc invocations will not write beyond
 * the end of the buffer.  Note that signals during _bufsync can cause
 * _bufsync to do the wrong thing, but usually with benign effects.
 */
_bufsync(iop)
register FILE *iop;
{
	register int spaceleft;

	if ((spaceleft = _bufend(iop) - iop->_ptr) < 0)
		iop->_ptr = _bufend(iop);
	else if (spaceleft < iop->_cnt)
		iop->_cnt = spaceleft;
}

int 
_xwrite(int f, char *b, int n, int tt)
{
    int old_n = n;		/* original count */
    int rc;			/* return from write */

    while (n) {
      if ((rc = write(f, b, n)) < 0) {

	if (tt && errno == EAGAIN) {	/* NDELAY set and device is full */
	  usleep(100000);	/* sleep for .1 seconds */
	  continue;		/* and then retry */
	}

	if (old_n == n)		/* other error -- nothing written */
	  return -1;		/* just return -1 */
	else			/* partial write and then error */
	  break;		/* return what we wrote */
      }
      
      n -= rc;			/* decrement counter */
      b += rc;			/* move on to next batch */
    }
    return old_n - n;
}
