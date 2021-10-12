static char sccsid[] = "@(#)77	1.9.2.2  src/bos/usr/ccs/lib/libc/tmpnam.c, libcio, bos411, 9428A410j 11/22/93 14:26:01";
/*
 *   COMPONENT_NAME: libcio
 *
 *   FUNCTIONS: defined
 *		tmpnam
 *
 *   ORIGINS: 3,27,85
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
/*
 * OSF/1 1.2
 */
/*
$RCSfile: tmpnam.c,v $ $Revision: 2.8.2.2 $ (OSF) $Date: 1991/12/27 20:46:49 $";
*/

/*LINTLIBRARY*/
#include <stdio.h>
#include <errno.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_tmpnam_rmutex;
#endif	/* _THREAD_SAFE */

extern char *mktemp(), *strcpy(), *strcat();
static char str[L_tmpnam], seed[] = { 'a', 'a', 'a', '\0' };

char *
tmpnam(char *s)
{
	register char *p, *q;

#ifdef _THREAD_SAFE
	if (s == NULL) {
		errno = EINVAL;
		return(NULL);
	}

	p = s;
#else
	p = (s == NULL)? str: s;
#endif	/* _THREAD_SAFE */
	(void) strcpy(p, P_tmpdir);
	TS_LOCK(&_tmpnam_rmutex);
	(void) strcat(p, seed);
	(void) strcat(p, "XXXXXX");

	q = seed;
	while(*q == 'z')
		*q++ = 'a';
	if(*q != '\0')  {
		++*q;
	}
	TS_UNLOCK(&_tmpnam_rmutex);

	(void) mktemp(p);
	return(p);
}
