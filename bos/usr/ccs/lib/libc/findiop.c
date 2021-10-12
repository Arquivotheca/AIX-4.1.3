static char sccsid[] = "@(#)37	1.12  src/bos/usr/ccs/lib/libc/findiop.c, libcio, bos41B, 412_41B_sync 12/9/94 21:42:02";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: _findiop
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
/* findiop.c,v $ $Revision: 2.8.2.5 $ (OSF) */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#include "ts_supp.h"
#include "glue.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#include "rec_mutex.h"

extern struct rec_mutex	_iop_rmutex;

#endif /* _THREAD_SAFE */

FILE *
_findiop(void)
{
	register FILE *fp, **fpp;
	register int i, size;
	int	cur_row, num_rows;
	TS_FDECLARELOCK(filelock)

	TS_LOCK(&_iop_rmutex);

	for(;;) {

		/* Look through the iobptr list for a free iob.
		 * Start at the hint supplied by freefile.
		 */
		for (i = _glued.freefile; i < _glued.nfiles; i++) {
			if (!inuse(fp = (FILE *)&_glued.iobptr[i>>4][i&0xf])) 
			{
#ifdef _THREAD_SAFE
				/* Make sure next thread through doesn't use
				 * this iob. Check the file lock. If null,
				 * allocate it. If it's non-null, testlock().  
				 * If we get it, it's ours.
				 */
				if (fp->_lock == NULL)
					(void)_rec_mutex_alloc((rec_mutex_t *)
								&fp->_lock);
				if ((TS_FTRYLOCK(filelock, fp))) {	
					if (!inuse(fp)) { /* found one */
						fp->_flag = _IOINUSE;
						TS_FUNLOCK(filelock);
						_glued.freefile = i + 1;
						if (i > _glued.lastfile)
							_glued.lastfile = i;
						fp->_cnt = i;
						TS_UNLOCK(&_iop_rmutex);
						return (fp);
					}
					/* not free, give it back */
					TS_FUNLOCK(filelock);
				}	/* lock failed, must be in use */
#else	/* _THREAD_SAFE */
				_glued.freefile = i + 1;
				if (i > _glued.lastfile)
					_glued.lastfile = i;
				return (fp);
#endif	/* _THREAD_SAFE */
			}  /* end of if statement */
		}

		/* There are no free iobs in the current list.
		 * Grow the list to the next size up and repopulate.
		 */
		if (_glued.nrows < _NROWEXTEND)
			num_rows = _NROWEXTEND;
		else
			num_rows =  _glued.nrows << 2;
		if (!(fpp = (FILE **)malloc(size =
					    sizeof(FILE **) * num_rows))) {
				TS_UNLOCK(&_iop_rmutex);
				return (NULL);
		}

		/* Copy the old list into the new and free the old if it
		 * is not the initial one (which is static storage).
		 */
		memset(fpp, 0, size);
		memcpy((void *)fpp, (void *)_glued.iobptr,
		       sizeof(FILE **) * _glued.nrows);
		if (_glued.nrows > _NROWSTART)
			free(&_glued.iobptr);

		/* Reset counters.
		 */
		_glued.nfiles =  num_rows * _NIOBRW;
		_glued.iobptr = fpp;
		_glued.nrows = num_rows;
		cur_row = _glued.crow + 1;
		_glued.crow = num_rows - 1;

		/* Set up new iob rows.
		 */
		fpp = (FILE **)&_glued.iobptr[cur_row];
		while (cur_row++ <  num_rows) {
			if (!(*fpp = (FILE *)malloc(_NROWSIZE))) {
				TS_UNLOCK(&_iop_rmutex);
				return (NULL);
			}
			memset(*fpp++, 0, _NROWSIZE);
		}

		/* Now retry the list.
		 */
	}
}
