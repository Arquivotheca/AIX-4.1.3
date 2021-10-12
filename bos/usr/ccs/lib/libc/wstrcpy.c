static char sccsid[] = "@(#)45	1.2  src/bos/usr/ccs/lib/libc/wstrcpy.c, libcnls, bos411, 9428A410j 6/8/91 17:17:53";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrcpy
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
 *
 */

/*
 * NAME: wstrcpy
 *
 * FUNCTION: like NLstrcpy except it copies double byte characters instead of mixed byte chars.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the target string.
 */
#include <string.h>
#include <ctype.h>

wchar_t * wstrcpy(wchar_t *s1, wchar_t *s2)
{
	return ( wcscpy(s1, s2) );
}
