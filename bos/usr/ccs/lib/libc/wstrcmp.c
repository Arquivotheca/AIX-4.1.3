static char sccsid[] = "@(#)44	1.2  src/bos/usr/ccs/lib/libc/wstrcmp.c, libcnls, bos411, 9428A410j 6/8/91 17:17:50";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrcmp
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

#include <string.h>
#include <ctype.h>

/*
 *
 *  Compare strings.  Returns:  s1>s2; >0  s1==s2; 0  s1<s2; <0.
 *  Two versions here:  NLstrcmp (operates on ASCII with embedded NLS
 *  code points) and NCstrcmp (operates on wchar_ts).
 *
 */

int wstrcmp(wchar_t *s1, wchar_t *s2)
{
	return (wcscoll(s1, s2));
}
