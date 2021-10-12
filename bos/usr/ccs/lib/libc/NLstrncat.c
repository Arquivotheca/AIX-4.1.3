static char sccsid[] = "@(#)97	1.1  src/bos/usr/ccs/lib/libc/NLstrncat.c, libcnls, bos411, 9428A410j 2/26/91 17:40:22";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrncat
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
 * NAME: NLstrncat
 *
 * FUNCTION: concatonate two strings, with a maximum length of n in the output string.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the result string.
 */
char * NLstrncat(char *s1, char *s2, int n)
{
	return ( strncat(s1, s2, n) );
}
