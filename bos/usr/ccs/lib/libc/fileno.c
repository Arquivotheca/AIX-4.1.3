static char sccsid[] = "@(#)47	1.3  src/bos/usr/ccs/lib/libc/fileno.c, libcio, bos411, 9428A410j 10/20/93 14:28:36";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: fileno
 *		fileno_unlocked
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
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* fileno.c,v $ $Revision: 1.5.2.2 $ (OSF) */

#include <stdio.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

#ifdef fileno
#undef fileno
#endif

/*
 * FUNCTION:
 *	fileno() returns the integer file descriptor associated with a
 *	stream.
 * 
 * RETURNS:
 *	If an error occurs, a value of -1 is returned and errno is set to
 *	indicate the error.
 *
 */

int
fileno(FILE *stream)
{
	register int rc;
	TS_FDECLARELOCK(filelock);

	TS_FLOCK(filelock, stream);
	rc = stream->_file;
	TS_FUNLOCK(filelock);
	return (rc);
}
#ifdef _THREAD_SAFE
#ifdef fileno_unlocked
#undef fileno_unlocked
#endif
int
fileno_unlocked(FILE *stream)
{
	return(stream->_file);
}
#endif /* _THREAD_SAFE */
