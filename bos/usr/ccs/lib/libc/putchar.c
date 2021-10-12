static char sccsid[] = "@(#)78	1.11  src/bos/usr/ccs/lib/libc/putchar.c, libcio, bos411, 9428A410j 10/20/93 14:30:42";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: putchar
 *		putchar_unlocked
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
/* putchar.c,v $ $Revision: 2.7.1.2 $ (OSF) */

#include <stdio.h>
#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef putchar_unlocked
#endif
#undef putchar

/*
 * FUNCTION:	A subroutine version of the macro putchar.  This function is
 *		created to meet ANSI C standards.  The putchar function writes
 *		the character specified by c (converted to an unsigned char)
 *		to stdout, at the position indicated by the assoicated file
 *		poistion indicator for the stream, and advances the indicator
 *		appropriately.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The putchar function returns the character written.  If a write
 *		error occurs, the error indicator for the stream is set and
 * 		putchar returns EOF.
 *
 */                                                                   

int 	
putchar(int c)
{
#ifdef	_THREAD_SAFE
	return(putc_locked(c, stdout));
#else
	return(putc(c, stdout));
#endif
}

#ifdef	_THREAD_SAFE
int 	
putchar_unlocked(int c)
{
	return(putc_unlocked(c, stdout));
}
#endif
