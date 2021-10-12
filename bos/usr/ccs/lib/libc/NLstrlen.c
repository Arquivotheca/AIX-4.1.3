static char sccsid[] = "@(#)96	1.1  src/bos/usr/ccs/lib/libc/NLstrlen.c, libcnls, bos411, 9428A410j 2/26/91 17:40:18";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrlen
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
 * NAME: NLstrlen
 *
 * FUNCTION: Counts the number of bytes in the string pointed to by s before
 *	the terminating null character.  Works with strings containing embedded
 *	NLS code points and is identical to strlen.
 *
 * RETURN VALUE DESCRIPTION: An integer, the number of bytes in s before the
 *	terminating null character.
 */
int NLstrlen(char *s)
{
	return ( strlen(s) );
}
