static char sccsid[] = "@(#)57	1.9  src/bos/usr/ccs/lib/libc/getchar.c, libcio, bos411, 9428A410j 10/20/93 14:29:28";
/*
 *   COMPONENT_NAME: LIBCIO
 *
 *   FUNCTIONS: getchar
 *		getchar_unlocked
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
/* getchar.c,v $ $Revision: 2.8 $ (OSF) */

#include <stdio.h>
#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef getchar_unlocked
#endif
#undef getchar

/*
 * Subroutine version of the macros getchar and getchar_unlocked.
 */

int
getchar(void)
{
#ifdef	_THREAD_SAFE
	return(getc_locked(stdin));
#else
	return(getc(stdin));
#endif
}

#ifdef	_THREAD_SAFE
int
getchar_unlocked(void)
{
	return(getc_unlocked(stdin));
}
#endif
