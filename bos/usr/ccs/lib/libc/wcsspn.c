static char sccsid[] = "@(#)27	1.2  src/bos/usr/ccs/lib/libc/wcsspn.c, libcstr, bos411, 9428A410j 1/12/93 11:20:39";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcsspn
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
 * NAME: wcsspn
 *
 * FUNCTION: Compute the number of wchar_t characters in the initial
 *  segment of the string pointed to by string1, which consists entirely of
 *  wchar_t characters from the string pointed to by string2.
 *
 * PARAMETERS:
 *	wchar_t *string1	-	the wide character string
 *	wchar_t *string2	-	the wide character string
 *
 * RETURN VALUE DESCRIPTION: the number of wchar_t in the segment.
 */
size_t 
wcsspn(const wchar_t *string1,const wchar_t *string2)
{
	register wchar_t *q;
	register wchar_t *p;

	for(q=string1; *q != '\0'; q++) {
		for (p = string2; *p != 0 && *p != *q; ++p)
			;
		if(*p == 0)
			break;
	}
	return(q - string1);
}
