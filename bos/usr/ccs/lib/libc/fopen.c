static char sccsid[] = "@(#)59	1.22  src/bos/usr/ccs/lib/libc/fopen.c, libcio, bos41B, 412_41B_sync 12/10/94 16:12:46";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: FREEIOB
 *		_endopen
 *		fopen
 *		freopen
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
/* fopen.c,v $ $Revision: 2.8.2.6 $ (OSF) */

#include <stdio.h>
#include <errno.h>
#include "stdiom.h"
#include <fcntl.h>
#include "ts_supp.h"
#include "push_pop.h"
#include "glue.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern FILE *_findiop();
static FILE *_endopen();

/*
 * FUNCTION: The fopen subroutine opens the file named by the path
 *           parameter and associates a stream with it. 
 *
 * PARAMETERS: 
 *	     char *filename  - points to a character string that contains
 * 	                   the name of the file to be opened.
 *	     char *mode  - points to a character string that has one
 *	                   of the following values:
 *   			   "r", "w", "a", "r+", "w+", "a+", "rb", "wb",
 *			   "ab", "r+b", "w+b", "a+b"
 *
 *                         Note: In this implementation, there is not any 
 *                               difference between text and binary files.
 *                               Therefore, the b type will be ignored.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns a pointer to the FILE structure of this stream.
 *
 */
FILE *
fopen(const char *filename, const char *mode)
{
	return (_endopen(filename, mode, _findiop()));
}


/*
 *
 * FUNCTION: The  freopen subroutine  substitutes  the  named file  in
 *           place of the open stream.   The original stream is closed
 *           whether  or not  the  open succeeds.   
 *
 * PARAMETERS: 
 *	     char *filename  - points to a character string that contains
 * 	                   the name of the file to be opened.
 *	     char *mode  - points to a character string that has one
 *	                   of the following values:
 *   			   "r", "w", "a", "r+", "w+", "a+", "rb", "wb",
 *			   "ab", "r+b", "w+b", "a+b"
 *
 *                         Note: In this implementation, there is not any 
 *                               difference between text and binary files.
 *                               Therefore, the b type will be ignored.
 *	     FILE *stream   - points to an open stream
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns a pointer to the FILE structure associated with stream.
 *
 */
FILE *
freopen(const char *filename, const char *mode, FILE *stream)
{
	TS_FDECLARELOCK(filelock)

	if (stream == NULL) {
		errno = ENOENT;
		return (NULL);
	}

#ifdef _THREAD_SAFE
	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	(void) fclose_unlocked(stream);	/* doesn't matter if this fails */
	TS_POP_CLNUP(0);

	stream->_flag |= _IOINUSE;
	TS_FUNLOCK(filelock);
#else
	(void) fclose(stream);		/* doesn't matter if this fails */
#endif	/* _THREAD_SAFE */

	return (_endopen(filename, mode, stream));
}


		/* In the case of _THREAD_SAFE, care must be taken that
		 * the _iop_rmutex is locked first so that another thread
		 * doesn't get too far into findiop() before we release
		 * FILE pointer.  Or that we don't modify freefile until
		 * the current thread that might be in findiop() is done.
		 *
		 * In the non thread safe case, there is only one thread
		 * running, so we can just decrement.  But if freopen()
		 * calls _endopen(), it's possible that fclose() set
		 * freefile to 0, and if freopen fails, it will set it to -1
		 * which would be bad.  So instead of just decrementing,
		 * cast it to a signed int and shift the high bit down.
		 * If it is a -1, then negation will result in a 0, and
		 * freefile will be 0.  If free file was positive, then the
		 * negation will result in 0xFFFFFFFF so the result will be
		 * freefile-1.  Efectively, freefile is decremented if
		 * if it is currently > 0, otherwise, it is reset to 0.
		 */
#ifdef _THREAD_SAFE
extern struct rec_mutex _iop_rmutex;
#define	FREEIOB(fp)	\
		TS_LOCK(&_iop_rmutex); \
		TS_FLOCK(filelock, fp); \
		if (fp->_cnt < _glued.freefile)	\
			_glued.freefile = fp->_cnt; \
		fp->_flag = 0; \
		TS_FUNLOCK(filelock); \
		TS_UNLOCK(&_iop_rmutex); 
#else
#define	FREEIOB(fp)	\
		_glued.freefile = (--_glued.freefile) & ~((int)(_glued.freefile)>>31);
#endif	/* _THREAD_SAFE */


static FILE *
_endopen(char *filename, char *mode, FILE *stream)
{
	int	plus, oflag, fd, sv_errno;
	TS_FDECLARELOCK(filelock)

	if (stream == NULL) {
		errno = ENOENT;
		return (NULL);
	}
	if (filename == NULL || filename[0] == '\0') {
		errno = ENOENT;
		FREEIOB(stream);
		return (NULL);
	}

	/*
	 * Validate the file modes
	 */
	if (mode == NULL || mode[0] == '\0') {
		errno = EINVAL;
		FREEIOB(stream);
		return (NULL);
	}

	plus = (mode[1] == '+') || (mode[1] && mode[2] == '+');

	switch (mode[0]) {
	case 'w':
		oflag = O_TRUNC | O_CREAT | (plus ? O_RDWR : O_WRONLY);
		break;
	case 'a':
		oflag = O_APPEND | O_CREAT | (plus ? O_RDWR : O_WRONLY);
		break;
	case 'r':
		oflag = plus ? O_RDWR : O_RDONLY;
		break;
	default:
		errno = EINVAL;
		FREEIOB(stream);
		return (NULL);
	}

	if ((fd = open(filename, oflag, 0666)) < 0) {
		FREEIOB(stream);
		return (NULL);
	}
	if (mode[0] == 'a' && !plus)
		(void)lseek(fd, (off_t)0, SEEK_END);

	TS_FLOCK(filelock, stream);

	stream->_cnt = 0;
	stream->_file = fd;
        stream->_flag &= ~_IONONSTD;
	stream->__stdioid = __forkid;
	stream->_flag = plus ? _IORW : (mode[0] == 'r') ? _IOREAD : _IOWRT;
	/*********
	  Don't care about the errno from isatty, so save
	  and restore the current value
	**********/
	sv_errno = errno;
	stream->_flag |= (isatty(fd) ? _IOISTTY : 0);
	errno = sv_errno;
        stream->__newbase =
		stream->_bufendp = stream->_base = stream->_ptr = NULL;

	TS_FUNLOCK(filelock);
	return (stream);
}
