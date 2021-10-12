static char sccsid[] = "@(#)49	1.2  src/bos/usr/ccs/lib/libc/wstrncat.c, libcnls, bos411, 9428A410j 6/8/91 17:18:00";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrncat
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
 *
 */

#include <string.h>
#include <ctype.h>

/*
 * NAME: wstrncat
 *
 * FUNCTION: concatonate two strings of NCchars.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first NCchar in the resulting string.
 */
wchar_t * wstrncat(wchar_t *s1, wchar_t *s2, int n)
{
	return (wcsncat(s1, s2, n));
}
