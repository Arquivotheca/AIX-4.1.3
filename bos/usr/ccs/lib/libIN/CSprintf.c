static char sccsid[] = "@(#)38	1.7  src/bos/usr/ccs/lib/libIN/CSprintf.c, libIN, bos411, 9428A410j 6/10/91 10:15:01";
/*
 * LIBIN: CSprintf
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: 
 *
 * RETURN VALUE DESCRIPTION: 
 */

#include <stdio.h>
#include <stdarg.h>

char *
CSprintf(char *str, char *fmt, ...) 
{       va_list ap;
	register n;

	va_start(ap, fmt);
	n = vsprintf(str, fmt, ap);
	va_end(ap);
	return str + n;
}
