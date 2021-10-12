static char sccsid[] = "@(#)16	1.1  src/bos/usr/ccs/lib/libc/wcscmp.c, libcstr, bos411, 9428A410j 2/26/91 17:49:28";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcscmp
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
 *
 * FUNCTION: Compares the wchar strings pointed to by ws1 and ws2, 
 *           returning an integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 * PARAMETERS: 
 *	     wchar_t *ws1 - first string
 *	     wchar_t *ws2 - second string
 *                                                                    
 * RETURN VALUE DESCRIPTION: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/

int wcscmp(const wchar_t *ws1, const wchar_t *ws2)
{

	if(ws1 == ws2)
		return(0);
	while(*ws1 == *ws2++)
		if(*ws1++ == '\0')
			return(0);
	return(*ws1 - *--ws2);
}
