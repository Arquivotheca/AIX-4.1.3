static char sccsid[] = "@(#)18	1.6  src/bos/usr/ccs/lib/libbsd/mktemp.c, libbsd, bos411, 9428A410j 8/1/91 09:19:03";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS:  mktemp
 *
 * ORIGINS: 26 27 10
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>		/* for NULL		*/
#include <stdlib.h>		/* for MB_CUR_MAX	*/
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/access.h>		/* for access(,F_OK)...	*/

extern char *__lucky();			/* see stdio/DStemp.c */

/*
 * NAME:	mktemp
 *
 * FUNCTION:	mktemp - construct a unique filename
 *
 * NOTES:	mktemp() expects a string with zero or more trailing 'X's,
 *		and overwrites no more than 6 of the X's with a unique
 *		encoding of the process' pid and a pseudo-random number.
 *
 * RETURN VALUE DESCRIPTION:
 *		If a unique filename was not generated, a "/" is returned.
 *		Else the filename is returned.
 */

char *
mktemp(template)
char *template;
{
	int mb_cur_max = MB_CUR_MAX;
	char *s = template;
	char *t = __lucky();	/* get a unique six-character string */
	int olderrno = errno;
	int tlen;		/* length of unique string in bytes */

	s += strlen(template);	/* point at the terminal null */
	t += (tlen = strlen(t));

	if (mb_cur_max == 1)
		while(*--s == 'X' && tlen--)
			*s = *--t;
	else
	{
		char *sp;		/* current data point in s */
		char *xptr;		/* pointer to first 'X' in string */
		int count;		/* # bytes in mb character */
		wchar_t wcx, wcs;

		xptr = s;

		/* search forward for the first of the trailing 'X's in s */
		mbtowc(&wcx, "X", MB_CUR_MAX);
		for(sp = template; *sp != '\0'; sp+=count) {
			count = mbtowc(&wcs, sp, MB_CUR_MAX);
			if(wcs == wcx) {
				if(xptr == s)
					xptr = sp;
			} else
				xptr = s; /* trailing 'X's must be contiguous */
		}
		while(--s >= xptr && tlen--)
			*s = *--t;
	}

	if(access(template, F_OK) == 0)
	{
		errno = olderrno;	/* access() may set errno */
		return ("/");
	}
	else
	{
		errno = olderrno;	/* access() may set errno */
		return(template);
	}
}
