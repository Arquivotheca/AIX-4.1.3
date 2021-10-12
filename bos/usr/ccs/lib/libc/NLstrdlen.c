static char sccsid[] = "@(#)95	1.1  src/bos/usr/ccs/lib/libc/NLstrdlen.c, libcnls, bos411, 9428A410j 2/26/91 17:40:14";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrdlen
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1991
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

#include <ctype.h>

/*
 * NAME: NLstrdlen(s)
 *
 * FUNCTION: Counts the number of code points in the string pointed to by s
 *	before the terminating null character.  It can be used to compute the
 *	"display length" of a string.  This differs from NLstrlen in that
 *	multi-byte chars are one character long while being two bytes long.
 *
 * RETURN VALUE DESCRIPTION: An integer, the number of code points in s before
 *	the terminating null character.
 */
int NLstrdlen(char *s)
{
	register int c;

	for (c = 0; *s != '\0'; s += NLchrlen(s), ++c)
		;
	return (c);
}
