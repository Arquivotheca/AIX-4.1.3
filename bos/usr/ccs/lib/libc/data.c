static char sccsid[] = "@(#)37	1.8  src/bos/usr/ccs/lib/libc/data.c, libcio, bos411, 9428A410j 10/20/93 14:27:49";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/*LINTLIBRARY*/
#include <stdio.h>

/* some slop is allowed at the end of the buffers in case an upset in
 * the synchronization of _cnt and _ptr (caused by an interrupt or other
 * signal) is not immediately detected.
 */
unsigned char _sibuf[BUFSIZ+8+2 * MB_LEN_MAX];
unsigned char _sobuf[BUFSIZ+8+2 * MB_LEN_MAX];

/*
 * Ptrs to start of preallocated buffers for stdin, stdout.
 */
unsigned char *_stdbuf[] = { _sibuf, _sobuf };

static unsigned char _smbuf[3+1][_SBFSIZ + 2 * MB_LEN_MAX];

#include "glue.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _stdio_buf_rmutex[3];

FILE _iob[_NIOBRW] = {
	{ NULL, 0, NULL, NULL, _IOREAD, 0,0,0,&_stdio_buf_rmutex[0]},
	{ NULL, 0, NULL, NULL,  _IOWRT, 1,0,0,&_stdio_buf_rmutex[1]},
	{ _smbuf[2]+2*MB_LEN_MAX, 0, _smbuf[2]+2*MB_LEN_MAX, _smbuf[2]+2*MB_LEN_MAX+_SBFSIZ, _IOWRT+_IONBF,2,0,0,&_stdio_buf_rmutex[2]}
};
#else /* _THREAD_SAFE */
FILE _iob[_NIOBRW] = {
	{ NULL, 0, NULL, NULL, _IOREAD, 0,0,0,0},
	{ NULL, 0, NULL, NULL,  _IOWRT, 1,0,0,0},
	{ _smbuf[2]+2*MB_LEN_MAX, 0, _smbuf[2]+2*MB_LEN_MAX, _smbuf[2]+2*MB_LEN_MAX+_SBFSIZ, _IOWRT+_IONBF, 2,0,0,0}
};
#endif /* _THREAD_SAFE */

FILE _iob1[_NIOBRW], _iob2[_NIOBRW], _iob3[_NIOBRW];
static FILE *_iobptr[_NROWSTART] = { _iob,_iob1,_iob2,_iob3 };

struct glued _glued = { 2,	/* lastfile	*/
			3,	/* freefile	*/
			64,	/* nfiles	*/
			4,	/* nrows	*/
			3,	/* crow		*/
			(FILE **)_iobptr
			};
