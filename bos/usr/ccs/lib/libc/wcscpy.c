static char sccsid[] = "@(#)19	1.1  src/bos/usr/ccs/lib/libc/wcscpy.c, libcstr, bos411, 9428A410j 2/26/91 17:49:41";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcscpy
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
 * FUNCTION: Copy the string pointed to by ws2, including the terminating
 *	     null wchar, into the wchar_t array pointed to by ws1.
 *	     No check is made for overflow of the array pointed to by ws1.
 *	     Overlapping copies toward the left work as expected, but
 *	     overlapping copies to the right may give unexpected results.
 *
 * PARAMETERS: 
 *	     wchar_t *ws1 - overlaid string
 *	     wchar_t *ws2 - copied string
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer equal to ws1.
 */
/*LINTLIBRARY*/

wchar_t  * wcscpy(wchar_t *ws1, const wchar_t *ws2)
{
	wchar_t *ows1;

	ows1 = ws1;
	while(*ws1++ = *ws2++)
		;
	return(ows1);
}
