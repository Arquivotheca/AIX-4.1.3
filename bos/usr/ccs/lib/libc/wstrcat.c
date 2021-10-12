static char sccsid[] = "@(#)42	1.2  src/bos/usr/ccs/lib/libc/wstrcat.c, libcnls, bos411, 9428A410j 6/8/91 17:17:46";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcstrcat
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 ,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <string.h>
#include <ctype.h>

/*
 * NAME: wstrcat
 *
 * FUNCTION: concatonate two strings of type wchar_t
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the first character of the concatonated.
 */
wchar_t *wstrcat(wchar_t *s1,  wchar_t *s2)
{
	return ( wcscat(s1, s2) );
}
