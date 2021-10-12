static char sccsid[] = "@(#)74	1.14  src/bos/usr/ccs/lib/libc/NCdecstr.c, libcnls, bos411, 9428A410j 6/14/91 09:53:04";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCdecstr
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
 * NAME: NCdecstr
 *
 * FUNCTION: Convert a string of chars to wchar_ts.
 *
 * RETURN VALUE DESCRIPTION: Return the length of string converted.
 */

/*
 * Convert a string of chars to wchar_ts; return length of string produced.
 */

int
NCdecstr(char *c, wchar_t *nlc, int len)
{
	int rc;
	rc = mbstowcs(nlc, c, len);
	if (rc == len) {
		rc--;
		nlc[rc] = (wchar_t) '\0';
	}
	return(rc);
	
}
