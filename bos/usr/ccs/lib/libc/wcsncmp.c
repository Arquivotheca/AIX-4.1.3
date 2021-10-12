static char sccsid[] = "@(#)23	1.1  src/bos/usr/ccs/lib/libc/wcsncmp.c, libcstr, bos411, 9428A410j 2/26/91 17:49:56";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcsncmp
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
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

/*
 * FUNCTION: Compares at most n pairs of wchar_t from the strings
 *	     pointed to by ws1 and ws2, returning an integer as follows:
 *
 *		Less than 0	If s1 is less than ws2
 *		Equal to 0	If s1 is equal to ws2
 *		Greater than 0	If s1 is greater than ws2.
 *                                                                    
 * NOTES:    Handles the pathological case where the value of n equals
 *	     the maximum value of an unsigned long integer.
 *
 * PARAMETERS: 
 *	     wchar_t *ws1 - first string
 *	     wchar_t *ws2 - second string
 *	     size_t n - number of wchar_tacters to compare
 *
 * RETURN VALUE DESCRIPTION: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/


int wcsncmp(const wchar_t *ws1, const wchar_t *ws2, size_t n)
{
	size_t i;

	if(ws1 == ws2)
		return(0);
	for(i = 0; i < n && *ws1 == *ws2++; i++)
		if(*ws1++ == '\0')
			return(0);
	return((i == n)? 0: (*ws1 - *--ws2));
}

