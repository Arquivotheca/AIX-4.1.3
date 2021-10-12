static char sccsid[] = "@(#)91	1.1  src/bos/usr/ccs/lib/libc/NLstrchr.c, libcnls, bos411, 9428A410j 2/26/91 17:39:58";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrchr
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
 *  Return the ptr in sp at which the character c appears, NULL if not found.
 *  Two versions: NLstrchr() for char strings and NCstrchr() for wchar_t strings.
 *  In both cases, the character being looked for must be an wchar_t.
 */

/*
 * NAME: NLstrchr
 *
 * FUNCTION: look for the occurrence of an wchar_t in a multibyte string.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first occurrence
 * of the wchar_t or a NULL on failure.
 *
 */
char * NLstrchr(char *sp, char c)
{
	return ( strchr(sp, c) );
}
