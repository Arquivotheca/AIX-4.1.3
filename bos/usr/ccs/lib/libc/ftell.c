static char sccsid[] = "@(#)24	1.14  src/bos/usr/ccs/lib/libc/ftell.c, libcio, bos411, 9428A410j 4/20/94 17:47:08";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: ftell
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
/* ftell.c,v $ $Revision: 2.10.2.3 $ (OSF) */

#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#define FFLUSH	fflush_unlocked
#define FILENO	fileno_unlocked
#else	/* _THREAD_SAFE */
#define FFLUSH	fflush
#define FILENO	fileno
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <errno.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern long lseek();

/*                                                                    
 * FUNCTION: Returns the number of bytes from the beginning of the file
 *           for the current position in the stream.
 *
 * PARAMETERS: FILE *stream      - stream to be searched
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      zero if successful
 *	      -1 if not successful
 *	      errno is set on error to indicate the error
 */
long int
ftell(FILE *stream)
{
	long	tres;
	int	adjust;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (stream->_cnt < 0)
		stream->_cnt = 0;
	if (stream->_flag & _IOREAD)
		adjust = - stream->_cnt;
	else if (stream->_flag & (_IOWRT | _IORW)) {
		int rc;
		adjust = 0;

		TS_PUSH_CLNUP(filelock);
		rc = FFLUSH(stream);
		TS_POP_CLNUP(0);

		if (rc == -1) {
		        TS_FUNLOCK(filelock);
			/* p70023 - do not ignore fflush failures */
			return(-1);
		}
		if (stream->_flag & _IORW) {
			stream->_cnt = 0;
			stream->_flag &= ~_IOWRT;
			stream->_ptr = stream->_base;
		}
	} else {
		TS_FUNLOCK(filelock);
		errno = EBADF;
		return (-1);
	}
	tres = lseek(FILENO(stream), 0L, SEEK_CUR);
	if (tres >= 0)
		tres += (long)adjust;

	TS_FUNLOCK(filelock);
	return (tres);
}
