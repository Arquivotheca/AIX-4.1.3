static char sccsid[] = "@(#)38	1.12  src/bos/usr/ccs/lib/libc/strrchr.c, libcstr, bos411, 9428A410j 8/2/91 16:05:20";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strrchr
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

#ifdef strrchr
#   undef strrchr
#endif /* strrchr */

/*
 * FUNCTION: Returns a pointer to the last occurrence of c, converted
 *	     to a char, in the string pointed to by s.  A NULL pointer
 *	     is returned if the character does not occur in the string. 
 *	     The terminating null character is considered to be part of
 *	     the string. 
 *
 * PARAMETERS: 
 *	     char *s - string to be searched
 *	     int  c  - character to be found
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the last occurrence of
 *	     character c in string s; NULL if c is not found in s.
 */
/*LINTLIBRARY*/


char	*
strrchr(const char *s, int c)
{
	char *r;

	r = NULL;
	do {
		if(*s == c)
			r = (char *)s;
	} while(*s++);
	return(r);
}
