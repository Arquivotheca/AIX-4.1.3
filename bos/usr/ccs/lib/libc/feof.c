static char sccsid[] = "@(#)70	1.9  src/bos/usr/ccs/lib/libc/feof.c, libcio, bos411, 9428A410j 10/20/93 14:28:15";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: feof
 *		feof_unlocked
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

#include <stdio.h>

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#undef feof

/*
 * FUNCTION:	A subroutine version of the macro feof.  This function was
 *	        created to meet ANSI C standards. The feof function tests the
 *		end-of-file indicator for the stream pointed to by stream.
 *
 * RETURN VALUE DESCRIPTION:	
 *		non-zero if and only if the end-of-file indicator is set
 *		for stream.
 *
 */

int 	
feof(FILE *stream)
{
	register int rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = (stream)->_flag & _IOEOF;
	TS_FUNLOCK(filelock);
	return (rc);
}

#ifdef _THREAD_SAFE
#undef feof_unlocked
int feof_unlocked(FILE *stream)
{
	return((stream)->_flag & _IOEOF);
}
#endif /* _THREAD_SAFE */
