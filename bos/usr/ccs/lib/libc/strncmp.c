static char sccsid[] = "@(#)04	1.10  src/bos/usr/ccs/lib/libc/strncmp.c, libcstr, bos411, 9428A410j 6/16/90 01:32:14";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strncmp
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
 * FUNCTION: Compares at most n pairs of characters from the strings
 *	     pointed to by s1 and s2, returning an integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *                                                                    
 * NOTES:    Handles the pathological case where the value of n equals
 *	     the maximum value of an unsigned long integer.
 *
 * PARAMETERS: 
 *	     char *s1 - first string
 *	     char *s2 - second string
 *	     size_t n - number of characters to compare
 *
 * RETURN VALUE DESCRIPTION: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/


int	
strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;

	if(s1 == s2)
		return(0);
	for(i = 0; i < n && *s1 == *s2++; i++)
		if(*s1++ == '\0')
			return(0);
	return((i == n)? 0: (*s1 - *--s2));
}
