static char sccsid[] = "@(#)82	1.1  src/bos/usr/ccs/lib/libc/NCstrlen.c, libcnls, bos411, 9428A410j 2/26/91 17:39:23";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NCstrlen
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
 * NAME: NCstrlen
 *
 * FUNCTION: Counts the number of wchar_ts in the string pointed to by s before
 *	the terminating null character.  The string must be of type wchar_t.
 *
 * RETURN VALUE DESCRIPTION: An integer, the number of wchar_ts in s before the
 *	terminating null character.
 */
int NCstrlen(wchar_t *s)
{
	return( wcslen(s) );
}
