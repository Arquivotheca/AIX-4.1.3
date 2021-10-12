static char sccsid[] = "@(#)20	1.1  src/bos/usr/ccs/lib/libc/wcscspn.c, libcstr, bos411, 9428A410j 2/26/91 17:49:45";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcscspn
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
 * NAME: wcscspn
 *
 * FUNCTION: Find the length of the initial portion of wide-character 
 *  string1 of characters not in wide-character string2.
 *
 * PARAMETERS:
 *	wchar_t *string1	-	the wide character string
 *	wchar_t *string2	-	the wide character string
 *
 * RETURN VALUE DESCRIPTION: the number of wchar_t in the segment.
 */
size_t 
#ifdef _NO_PROTO
wcscspn(string1, string2)
wchar_t *string1, *string2;
#else
wcscspn(wchar_t *string1,wchar_t *string2)
#endif
{
	register wchar_t *q;
	register wchar_t *p;

	for(q=string1; *q != '\0'; q++) {
		for (p = string2; *p != 0 && *p != *q; ++p)
			;
		if(*p != 0)
			break;
	}
	return(q - string1);
}
