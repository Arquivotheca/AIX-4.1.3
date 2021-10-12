static char sccsid[] = "@(#)95	1.13  src/bos/usr/ccs/lib/libc/NCencstr.c, libcnls, bos411, 9428A410j 6/14/91 09:53:06";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCencstr
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

#include <stdlib.h>
#include <ctype.h>

/*
 * NAME: NCencstr
 *
 * FUNCTION: Convert a string of NLchars to chars.
 *
 * NOTE:     Macro NLchrlen returns the length of NLchar.
 *           Macro NCenc does the same as NCencode().
 *
 * RETURN VALUE DESCRIPTION: Return the length of string converted.
 */

/*
 * Convert a string of NLchars to chars; return length of string produced.
 */

int
NCencstr(wchar_t *nlc, char *c, int len)
{
	int rc;

	rc = wcstombs(c, nlc, len);

	if (rc == len) {
		rc--;
		c[rc] = '\0';
	}
	return(rc);
}
