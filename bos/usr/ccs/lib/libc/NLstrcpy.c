static char sccsid[] = "@(#)93	1.1  src/bos/usr/ccs/lib/libc/NLstrcpy.c, libcnls, bos411, 9428A410j 2/26/91 17:40:06";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrcpy
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
 *  Copy string s2 to s1.  s1 must be large enough.
 *  return s1.  Two versions here:  NLstrcpy (operates on ASCII with
 *  embedded NLS code points and is identical to strcpy), and NCstrcpy
 *  (operates on wchar_t strings).
 */

/*
 * NAME: NLstrcpy
 *
 * FUNCTION: identical to strcpy
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the target string.
 */
char * NLstrcpy(char *s1, char *s2)
{
	return ( strcpy (s1, s2) );
}
