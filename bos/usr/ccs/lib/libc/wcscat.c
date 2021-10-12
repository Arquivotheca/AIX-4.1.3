static char sccsid[] = "@(#)14	1.1  src/bos/usr/ccs/lib/libc/wcscat.c, libcstr, bos411, 9428A410j 2/26/91 17:49:20";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcscat
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
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
 *  Concatenate s2 on the end of s1.  S1's space must be large enough.
 *  Return s1.
 */

/*
 * NAME: wcscat
 *
 * FUNCTION: concatonate two strings of type wchar_t
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the beginning of the concatonated string.
 */
wchar_t *wcscat(wchar_t *s1, wchar_t *s2)
{
	register wchar_t *os1;

	os1 = s1;
	while(*s1++)
		;
	--s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}
