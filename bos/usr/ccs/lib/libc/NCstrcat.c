static char sccsid[] = "@(#)76	1.1  src/bos/usr/ccs/lib/libc/NCstrcat.c, libcnls, bos411, 9428A410j 2/26/91 17:38:59";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrcat 
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 , 1991
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
 * NAME: NCstrcat
 *
 * FUNCTION: concatonate two strings of type wchar_t
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first character of the concatonated.
 */
wchar_t *NCstrcat(wchar_t *s1, wchar_t *s2)
{
	return( wcscat(s1,s2) );
}
