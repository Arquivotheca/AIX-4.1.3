static char sccsid[] = "@(#)50	1.2  src/bos/usr/ccs/lib/libc/wstrncpy.c, libcnls, bos411, 9428A410j 6/8/91 17:18:02";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrncpy
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
 *
 *  Copy s2 to s1, truncating or null-padding to always copy n elements
 *  (bytes or wchar_ts).  Return s1.  Two versions here:  NLstrncpy (works
 *  with ASCII containing embedded NLS code points) and NCstrncpy (works
 *  with wchar_ts).
 *
 */

wchar_t * wstrncpy(wchar_t *s1, wchar_t *s2, int n)
{
	return (wcsncpy(s1, s2, n));
}
