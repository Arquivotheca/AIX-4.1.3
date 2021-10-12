static char sccsid[] = "@(#)27	1.11  src/bos/usr/ccs/lib/libc/strpbrk.c, libcstr, bos411, 9428A410j 6/16/90 01:32:23";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strpbrk
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
 * FUNCTION: Returns a pointer to the first occurrence in the string
 *	     pointed to by s1 of any character from the string pointed
 *	     to by s2.  A NULL pointer is returned if no character
 *	     matches are found.
 *
 * PARAMETERS:
 *	     char *s1 - string to be searched
 *	     char *s2 - string containing characters to be found
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the first occurrence of
 *	     any character from s2 in s1; NULL if no matches are found.
 */
/*LINTLIBRARY*/


char *
strpbrk(const char *s1, const char *s2)
{
	char *p;

	do {
		for(p=(char *)s2; *p != '\0' && *p != *s1; ++p)
			;
		if(*p != '\0')
			return((char *)s1);
	}
	while(*s1++);
	return(NULL);
}
