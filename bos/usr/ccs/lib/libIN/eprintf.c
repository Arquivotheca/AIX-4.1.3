static char sccsid[] = "@(#)92	1.7  src/bos/usr/ccs/lib/libIN/eprintf.c, libIN, bos411, 9428A410j 6/10/91 10:23:13";
/*
 * LIBIN: eprintf
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
 * FUNCTION: Flush stdout and then print arguments on stderr.
 *
 * RETURN VALUE DESCRIPTION: Return exit code from vfprintf() of print
 *	     arguments.
 */

#include <stdio.h>
#include <stdarg.h>
#include <IN/standard.h>

eprintf(char *fmt, ...) 
{
	va_list arglist;
	register int ret;

	va_start(arglist, fmt);
	fflush(stdout);
	ret = vfprintf(stderr, fmt, arglist);
	fflush(stderr);
	va_end(arglist);
	return ret;
}
