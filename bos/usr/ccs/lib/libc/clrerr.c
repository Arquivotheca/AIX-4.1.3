static char sccsid[] = "@(#)04	1.9  src/bos/usr/ccs/lib/libc/clrerr.c, libcio, bos411, 9428A410j 10/20/93 14:27:24";
#ifdef _POWER_PROLOG_
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: clearerr
 *
 *   ORIGINS: 3,27,71
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
#endif /* _POWER_PROLOG_ */

/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */

/*LINTLIBRARY*/
#include <stdio.h>

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#undef clearerr

void 	
clearerr(FILE *stream)
{
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	stream->_flag &= ~(_IOERR | _IOEOF);
	TS_FUNLOCK(filelock);
}
