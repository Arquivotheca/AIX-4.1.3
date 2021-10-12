static char sccsid[] = "@(#)86	1.4.1.1  src/bos/usr/ccs/lib/libc/perror.c, libcproc, bos411, 9428A410j 7/9/93 13:33:16";
/*
 * COMPONENT_NAME: LIBCPROC
 *
 * FUNCTIONS: perror
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

#include <string.h>
#include <errno.h>
#include <limits.h>

extern int write();
extern char *__strerror();

/*
 * NAME:	perror
 *
 * FUNCTION:	perror writes a message to the standard error output
 *		describing the last error encountered by a system
 *		call or the last error encountered by a subroutine call
 *		that set 'errno'.
 *
 * NOTES:	Perror is very useful in describing a system error after
 *		a system call fails.
 *
 *		errno is used
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void perror(const char *s)
{
	int n;
	char *c;		/* pointer to string to be printed	*/
	char strbuf[NL_TEXTMAX]; /* string to be printed.	*/
	int errno_sv;

	errno_sv = errno;	/* save the errno before calling catopen() */
	c = __strerror( errno_sv, strbuf );
	n = strlen(s);
	if(n) {
		(void) write(2, s, (unsigned) n);
		(void) write(2, ": ", 2);
	}
	(void) write(2, c, (unsigned) strlen(c));
	(void) write(2, "\n", 1);
	errno = errno_sv;
}
