static char sccsid[] = "@(#)90	1.1  src/bos/usr/ccs/lib/libc/NLstrcat.c, libcnls, bos411, 9428A410j 2/26/91 17:39:54";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrcat
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
 *  Concatenate s2 on the end of s1.  S1's space must be large enough.
 *  Return s1.  Two versions here: NLstrcat (works on ASCII + NLS code
 *  points and is identical to strcat), and NCstrcat (works on wchar_t).
 */

/*
 * NAME: NLstrcat()
 *
 * FUNCTION: concatonate two strings
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the beginning of the concatonated string.
 */
char * NLstrcat(char *s1, char *s2)
{
	return (strcat (s1, s2) );
}
