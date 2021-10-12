static char sccsid [] = "@(#)09  1.9  src/bos/usr/bin/ex/printf.c, cmdedit, bos411, 9428A410j 1/21/93 09:20:53";
/*
 * COMPONENT_NAME: (CMDEDIT) printf.c
 *
 * FUNCTIONS: printf
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#define VPRINTF
#include "ex.h"
#include <stdio.h>
#include <stdarg.h>
#ifndef VPRINTF
#include <values.h>
#endif
/*
 * ex_printf performs the equivalent of an sprintf and then calls vi's
 * putchar() to output string[]
 */
/*   VARARGS1  */

int _doprnt(char *, va_list, FILE *);

int
ex_printf(char *format, ...)
{
	static char string[2*8192];	/* for at least 8192 2-byte chars */
	register int count;
#ifndef VPRINTF
	FILE siop;
#endif
	va_list ap;
	int char_len;

#ifndef VPRINTF
	siop._cnt = MAXINT;
	siop._base = siop._ptr = (unsigned char *)string;
	siop._flag = (_IOWRT|_IONOFD);
#endif
	va_start(ap, format);
	
#ifndef VPRINTF
	count = ((int)_doprnt(format, ap, &siop));
#else
	count = vsprintf(string, format, ap);
#endif
	va_end(ap);
#ifndef VPRINTF
	*siop._ptr = '\0'; /* plant terminating null character */
#endif
	if (count > 0) {
		register char *s = string;
		wchar_t nlc;
		int count2 = count;
		while((count2 > 0) && ((char_len = mbtowc(&nlc, s, MB_CUR_MAX)) > 0)){
			s += char_len;
			ex_putchar(nlc);
			count2--;
		}
	}
	return(count);
}
