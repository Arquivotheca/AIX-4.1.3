static char sccsid[] = "@(#)21	1.2  src/bos/usr/ccs/lib/libc/wcslen.c, libcstr, bos411, 9428A410j 1/12/93 11:20:28";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library National Language Support
 *
 * FUNCTIONS: wcslen
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
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

#include <sys/types.h>

/*
 * NAME: wcslen
 *
 * FUNCTION: Determine the number of characters in the wide-character string.
 *
 * RETURN VALUE DESCRIPTION: An integer, the number of wchar_t characters 
 *     that precede the terminating wchar_t null character.
 */
size_t wcslen(const wchar_t *s)
{
	register wchar_t *s0 = s + 1;

	while (*s++ != 0)
		;
	return (s - s0);
}
