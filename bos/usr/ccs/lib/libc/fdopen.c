static char sccsid[] = "@(#)59	1.18  src/bos/usr/ccs/lib/libc/fdopen.c, libcio, bos411, 9428A410j 2/23/94 09:32:04";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fdopen
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

/*LINTLIBRARY*/
/*
 * Unix routine to do an "fopen" on file descriptor
 * The mode has to be repeated because you can't query its
 * status
 */
/* The fdopen() function includes all the POSIX requirements */

#include <stdio.h>
#include "stdiom.h"
#include <fcntl.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern long lseek();
extern FILE *_findiop();


/*
 * FUNCTION: routine to do an "fopen" on file descriptor
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	     - FILE * on success
 *	     - NULL on failure
 */
FILE *
fdopen(int fd, const char *mode)
{
	register FILE	*iop;
	int		status_file;
	int		sv_errno;
	TS_FDECLARELOCK(filelock)

	if ((status_file = fcntl(fd, F_GETFL)) == -1)
		return (NULL);
	status_file &= O_ACCMODE;

	if ((iop = _findiop()) == NULL)
		return (NULL);

	TS_FLOCK(filelock, iop);
	iop->_cnt = 0;
	iop->_flag = 0;
	iop->_file = fd;
	iop->__newbase = iop->_base = iop->_ptr = NULL;
	iop->__stdioid = __forkid;

	if (mode[1] == '+' || (mode[1] && mode[2] == '+')) {
		if (status_file == O_RDWR) {
			iop->_flag &= ~(_IOREAD | _IOWRT);
			iop->_flag |= _IORW;
		} 
		else {
			errno = EINVAL;
			TS_FUNLOCK(filelock);
			return (NULL);
		}

	}

	switch (*mode) {

		case 'r':
			if (!(status_file == O_RDONLY
			      || status_file == O_RDWR)) {
				iop->_flag = 0;
				errno = EINVAL;
				TS_FUNLOCK(filelock);
				return (NULL);
			}
			if (!(iop->_flag & _IORW))
				iop->_flag |= _IOREAD;
			break;
		case 'a':
			if (!(status_file == O_WRONLY
			      || status_file == O_RDWR)) {
				iop->_flag = 0;
				errno = EINVAL;
				TS_FUNLOCK(filelock);
				return (NULL);
			}
			(void) lseek(fd, 0L, SEEK_END);
			if (!(iop->_flag & _IORW))
				iop->_flag |= _IOWRT;
			break;
		case 'w':
			if (!(status_file == O_WRONLY
			      || status_file == O_RDWR)){
				iop->_flag = 0;
				errno = EINVAL;
				TS_FUNLOCK(filelock);
				return (NULL);
			}
			if (!(iop->_flag & _IORW))
				iop->_flag |= _IOWRT;
			break;
		default:
			iop->_flag = 0;
			errno = EINVAL;
			TS_FUNLOCK(filelock);
			return (NULL);
	}

	sv_errno = errno;
	iop->_flag |= (isatty(fd) ? _IOISTTY : 0);
	errno = sv_errno;
	TS_FUNLOCK(filelock);
	return (iop);
}
