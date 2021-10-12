static char sccsid[] = "@(#)67	1.11  src/bos/usr/ccs/lib/libc/putc.c, libcio, bos411, 9428A410j 4/20/94 17:51:00";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: putc
 *		putc_unlocked
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
/* putc.c,v $ $Revision: 1.6.2.2 $ (OSF) */

#include <stdio.h>
#include "ts_supp.h"
#include "push_pop.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef putc_unlocked
#endif	/* _THREAD_SAFE */
#undef putc

/*
 * FUNCTION:	A subroutine version of the macro putc.  This function is
 *		created to meet ANSI C standards.  The putc function writes
 *		the character specified by c (converted to an unsigned char)
 *		to the output stream pointed to by stream, at the position
 *		indicated by the assoicated file poistion indicator for the
 *		stream, and advances the indicator appropriately.
 *		POSIX 1003.4a requires that this function is locked by default
 *		so an unlocked version is also provided.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The putc function returns the character written.  If a write
 *		error occurs, the error indicator for the stream is set and
 * 		putc returns EOF.
 *
 */  

#ifdef _THREAD_SAFE
int 	
putc_unlocked(int c, FILE *stream)

#else
int 	
putc(int c, FILE *stream)
#endif	/* _THREAD_SAFE */
{
	if (--(stream)->_cnt < 0)
		return (__flsbuf((unsigned char) (c), (stream)));
	else
		return ((int) (*(stream)->_ptr++ = (unsigned char) (c)));
}

#ifdef	_THREAD_SAFE
int 	
putc(int c, FILE *stream)
{
	register int        rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (--(stream)->_cnt < 0) {
		TS_PUSH_CLNUP(filelock);
		rc = __flsbuf((unsigned char) (c), (stream));
		TS_POP_CLNUP(0);
		}
	else
		rc = ((int) (*(stream)->_ptr++ = (unsigned char) (c)));

	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
