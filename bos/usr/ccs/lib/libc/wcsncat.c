static char sccsid[] = "@(#)22	1.2  src/bos/usr/ccs/lib/libc/wcsncat.c, libcstr, bos411, 9428A410j 1/12/93 11:20:31";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library National Language Support
 *
 * FUNCTIONS: wcsncat
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
 *  Concatenate s2 on the end of s1.  S1's space must be large enough.
 *  At most n elements (bytes or wchar_t) are moved.  Return s1.
 *
 */

/*
 * NAME: wcsncat
 *
 * FUNCTION: concatonate two strings of wchar_t.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first wchar_t in 
 *     the resulting string.
 */
wchar_t * wcsncat(wchar_t *s1, const wchar_t *s2, size_t n)
{
	register wchar_t *os1;
	register len=n;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		if(--len < 0) {
			*--s1 = 0;
			break;
		}
	return(os1);
}
