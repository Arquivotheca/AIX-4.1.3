static char sccsid[] = "@(#)45	1.3  src/bos/usr/ccs/lib/libc/strdup.c, libcstr, bos411, 9428A410j 7/6/90 10:44:07";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strdup
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

/*
 * FUNCTION: Copy string argument to a new malloc'ed string.
 *
 * PARAMETERS: 
 *	     char *s1 - string to copy
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to new string
 */
/*LINTLIBRARY*/

#include	<sys/types.h>	/* for NULL */
#include	<string.h>      /* for strcpy */

char  *
strdup(char *s1)
{
	char *s2;

	if (s1 == NULL || (s2 = (char *)malloc(strlen(s1)+1)) == NULL)
		return(NULL);
	return((char *)strcpy(s2, s1));
}
