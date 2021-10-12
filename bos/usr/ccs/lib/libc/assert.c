static char sccsid[] = "@(#)77	1.6  src/bos/usr/ccs/lib/libc/assert.c, libcproc, bos411, 9428A410j 11/10/93 15:26:35";
/*
 * COMPONENT_NAME: LIBCPROC
 *
 * FUNCTIONS: __assert, _assert 
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

#include "libc_msg.h"
#include <nl_types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define WRITE(s, n)	(void) write(2, (s), (n))
#define WRITESTR(s1, n, s2)	WRITE((s1), n), \
				WRITE((s2), (unsigned) strlen(s2))
#define MBLEN(x)       ((mbleng=mblen(x,MB_CUR_MAX)) > 1 ? mbleng : 1)


/*
 * NAME:	__assert
 *
 * FUNCTION:	__assert - print out the "Assertion failed" message
 *		for the assert() macro (in assert.h)
 *
 * NOTES:	__assert prints out the
 *		Assertion failed: "expression", file "filename", line "linenum"
 *		message without using stdio.  It's called from the assert()
 *		macro in assert.h.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
__assert(char *assertion, char *filename, int line_num)
{
	static char linestr[] = ", line NNNNN\n";
	char *p = &linestr[7];	/* p points to the first 'N' in linestr */
	char tmpbuf[16];	/* must be at least as large as linestr[] */
	int ddiv, digit;
	char *msgbuf, *firstN;
	nl_catd catd = catopen(MF_LIBC,NL_CAT_LOCALE);
	int moveleft = 0;

	msgbuf = catgets(catd, MS_LIBC, M_ASSERT1, "Assertion failed: ");
	WRITESTR(msgbuf, (unsigned) strlen(msgbuf), assertion);

	msgbuf = catgets(catd, MS_LIBC, M_ASSERT2, ", file  ");
	WRITESTR(msgbuf, (unsigned) strlen(msgbuf), filename);

	/* Since the catgets string is in RO storage, copy it elsewhere
	   so we can write the line number into it later. */
	strcpy(tmpbuf, catgets(catd, MS_LIBC, M_ASSERT3, linestr));
	msgbuf = tmpbuf;

	if (*(p = strstr(msgbuf, "NNNNN")) == 0) {
		msgbuf = linestr;
		p = strstr(msgbuf, "NNNNN");
	}
	catclose(catd);

	firstN = p;

	/* convert line_num to ascii... */
	for (ddiv = 10000; ddiv != 0; line_num %= ddiv, ddiv /= 10)
		if ((digit = line_num/ddiv) != 0 || p != firstN || ddiv == 1)
			*p++ = digit + '0';
		else
			moveleft++;
	if (moveleft)
		do
			*p = *(p + moveleft);
		while (*p++);
	WRITE(msgbuf, (unsigned) strlen(msgbuf));
	(void) abort();
}

/* this function is kept to maintain binary compatibility */
void
_assert(char *assertion, char *filename, int line_num)
{
	__assert(assertion, filename, line_num);
}

