static char sccsid[] = "@(#)15	1.1  src/bos/usr/ccs/lib/libc/wcschr.c, libcstr, bos411, 9428A410j 2/26/91 17:49:24";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcschr
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
 *  Return the ptr in sp at which the character c appears;
 *  NULL if not found. 
 */

/*
 * NAME: wcschr
 *
 * FUNCTION: look for the first occurrence of a wchar_t
 *
 * RETURN VALUE DESCRIPTION: returns either a NULL (failure) or a 
 * pointer to the occurrence of the char.
 *
 */
wchar_t * wcschr(wchar_t *sp, wchar_t c)
{
	do {
		if(*sp == c)
			return(sp);
	} while(*sp++);
	return((wchar_t *)NULL);
}
