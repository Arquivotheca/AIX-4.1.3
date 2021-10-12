static char sccsid[] = "@(#)24	1.2  src/bos/usr/ccs/lib/libc/wcsncpy.c, libcstr, bos411, 9428A410j 1/12/93 11:20:34";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library National Language Support
 *
 * FUNCTIONS: wcsncpy
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
 *
 *  Copy s2 to s1, truncating or null-padding to always copy n elements
 *  (bytes or wchar_t).  Return s1.
 *
 */

/*
 * NAME: wcsncpy
 *
 * FUNCTION: copy a specific number of characters from one wide-characters 
 *    string to another wide-character string.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the target string.
 */
wchar_t * wcsncpy(wchar_t *s1, const wchar_t *s2, size_t n)
{
	register wchar_t *os1 = s1;
	register len=n;

	while (--len >= 0)
		if ((*s1++ = *s2++) == 0)
			while (--len >= 0)
				*s1++ = 0;
	return (os1);
}
