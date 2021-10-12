static char sccsid[] = "@(#)81	1.9  src/bos/usr/ccs/lib/libc/ferror.c, libcio, bos411, 9428A410j 10/20/93 14:28:19";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: ferror
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
/* ferror.c,v $ $Revision: 1.4.2.2 $ (OSF) */

#include <stdio.h>

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#undef ferror

/*                                                                    
 * FUNCTION:	A subroutine version of the macro ferror.  This function was
 *	        created to meet ANSI C standards.  The ferror function tests
 *		for the error indicator for the stream pointed to by stream.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The ferror function returns non-zero if and only if the error
 *		indicator is set for stream.
 *
 */  

int 	
ferror(FILE *stream)
{
	register int err;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	err = (stream)->_flag & _IOERR;
	TS_FUNLOCK(filelock);

	return err;
}
