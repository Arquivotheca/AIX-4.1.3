static char sccsid[] = "@(#)72	1.1  src/bos/usr/ccs/lib/libc/NLcplen.c, libcnls, bos411, 9428A410j 2/26/91 17:53:39";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLcplen
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

#include <ctype.h>

/*
 * NAME: NLcplen(s)
 *
 * FUNCTION: Counts the number of code points in the string pointed to by s
 *	before the terminating null character.  
 *
 * NOTES: Identical to NLstrdlen, included only for compatibility.
 *
 * RETURN VALUE DESCRIPTION: An integer, the number of code points in s before
 *	the terminating null character.
 */
int NLcplen(s)
char *s;
{
	return (NLstrdlen(s));
}
