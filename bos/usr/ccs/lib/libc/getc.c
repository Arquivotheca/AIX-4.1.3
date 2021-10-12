static char sccsid[] = "@(#)46	1.11  src/bos/usr/ccs/lib/libc/getc.c, libcio, bos411, 9428A410j 4/20/94 17:47:51";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: getc
 *		getc_unlocked
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
/* getc.c,v $ $Revision: 1.6.2.2 $ (OSF) */

#include <stdio.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef getc_unlocked
#endif	/* _THREAD_SAFE */
#undef getc

/*
 * FUNCTION:	A subroutine version of the macro getc.  This function was
 *		created to meet ANSI C standards.  The getc function obtains
 *		the next character (if present) as an unsigned char
 *		converted to an int, from the file pointed to by stream, and
 *		advances the associated file position indicator for the stream.
 *		POSIX 1003.4a defines this to be locked by default so the
 *		single thread version becomes getc_unlocked() and a new
 *		thread safe version is defined with the same interface.
 *
 * RETURN VALUE DESCRIPTION:	
 *		Returns the next character from the input stream pointed to
 *		by stream.  If the stream is at end-of-file, the end-of-file
 *		indicator for the stream is set and getc returns EOF.  If a 
 *		read error occurs, the error indicator for the stream is set 
 *		and getc returns EOF.
 *
 */  

#ifdef _THREAD_SAFE
int	
getc_unlocked(FILE *stream)
#else
int	
getc(FILE *stream)
#endif	/* _THREAD_SAFE */
{
	if (--(stream)->_cnt < 0)
		return(__filbuf(stream));
	else
		return((int) *(stream)->_ptr++);
}

#ifdef _THREAD_SAFE
int	
getc(FILE *stream)
{
	register int        rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (--(stream)->_cnt < 0) {
		TS_PUSH_CLNUP(filelock);
		rc = __filbuf(stream);
		TS_POP_CLNUP(0);
		}
	else
		rc = (int) *(stream)->_ptr++;

	TS_FUNLOCK(filelock);
	return(rc);
}
#endif	/* _THREAD_SAFE */
