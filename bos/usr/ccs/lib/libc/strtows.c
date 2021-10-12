static char sccsid[] = "@(#)75	1.2  src/bos/usr/ccs/lib/libc/strtows.c, libcnls, bos411, 9428A410j 6/9/91 17:14:37";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: strtows
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <ctype.h>

/*
 * NAME: strtows
 *
 * FUNCTION: Convert a string of chars to wchar_ts.
 *
 * RETURN VALUE DESCRIPTION: Return the length of string converted.
 */

/*
 * Convert a string of chars to wchar_ts; return length of string produced.
 */

wchar_t * strtows (wchar_t *nlc, char *c)
{
	mbstowcs (nlc, c, strlen (c) + 1);
	return (nlc);
}
