static char sccsid[] = "@(#)34	1.10  src/bos/usr/ccs/lib/libc/rew.c, libcio, bos411, 9428A410j 4/20/94 17:52:23";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: FFLUSH
 *		rewind
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
/* rew.c,v $ $Revision: 2.8.2.2 $ (OSF) */

/*LINTLIBRARY*/
#include <stdio.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#define FFLUSH(a) (fflush_unlocked(a))
#define FILENO	fileno_unlocked
#include "stdio_lock.h"
#else	/* _THREAD_SAFE */
#define FFLUSH(a) (fflush(a))
#define FILENO	fileno
#endif	/* _THREAD_SAFE */

extern long lseek();

void	
rewind(FILE *stream)
{
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	TS_PUSH_CLNUP(filelock);
	(void) FFLUSH(stream);
	TS_POP_CLNUP(0);

	(void) lseek(FILENO(stream), 0L, SEEK_SET);
	stream->_cnt = 0;
	stream->_ptr = stream->_base;
	stream->_flag &= ~(_IOERR | _IOEOF);
	if(stream->_flag & _IORW)
		stream->_flag &= ~(_IOREAD | _IOWRT);
	TS_FUNLOCK(filelock);
}
