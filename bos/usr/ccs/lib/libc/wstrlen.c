static char sccsid[] = "@(#)48	1.2  src/bos/usr/ccs/lib/libc/wstrlen.c, libcnls, bos411, 9428A410j 6/8/91 17:17:58";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrlen
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 ,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <ctype.h>

/*
 * NAME: wstrlen
 *
 * FUNCTION: Counts the number of code points in the string pointed to by s
 *	before the terminating null character.  
 *
 * RETURN VALUE DESCRIPTION: An integer, the number of code points in s before
 *	the terminating null character.
 */

int wstrlen(wchar_t *s)
{
	return (wcslen(s));
}
