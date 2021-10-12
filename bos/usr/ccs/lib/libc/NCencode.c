static char sccsid[] = "@(#)84	1.11  src/bos/usr/ccs/lib/libc/NCencode.c, libcnls, bos411, 9428A410j 2/26/91 12:43:28";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCencode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*LINTLIBRARY*/

#include <stdlib.h>
#include <ctype.h>

/*
 * NAME: NCencode
 *
 * FUNCTION: Converts NLchar to  1 or 2 chars.
 *
 * RETURN VALUE DESCRIPTION: The number of chars converted.
 */
/*
 * Convert NLchar to 1 or 2 chars; return # chars produced.
 */

int NCencode(wchar_t *nlc, char *c)
{
	int rc;

	rc = wctomb(c, *nlc);

	if (rc > 1)
		return (rc);
	else
		return (1);
}
