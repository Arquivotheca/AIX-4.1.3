static char sccsid[] = "@(#)83	1.1  src/bos/usr/ccs/lib/libc/NCstrncat.c, libcnls, bos411, 9428A410j 2/26/91 17:39:26";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrncat
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

#include <string.h>
#include <ctype.h>

/*
 * NAME: NCstrncat
 *
 * FUNCTION: concatonate two strings of NCchars.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first NCchar in the resulting string.
 */
wchar_t * NCstrncat(wchar_t *s1, wchar_t *s2, int n)
{
	return ( wcsncat(s1, s2, n) );
}
