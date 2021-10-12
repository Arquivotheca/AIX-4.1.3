static char sccsid[] = "@(#)64	1.12  src/bos/usr/ccs/lib/libc/NCdecode.c, libcnls, bos411, 9428A410j 2/26/91 12:43:21";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCdecode
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
 * NAME: NCdecode
 *
 * FUNCTION: Convert 1 or 2 chars to an wchar_t
 *
 * NOTE:     Macro NCisshift tests shift char 
 *
 * RETURN VALUE DESCRIPTION: Return the number of characters converted.
 */
/*
 * Convert 1 or 2 chars to an wchar_t; return # chars converted.
 */
int NCdecode(char *c, wchar_t *nlc)
{
	int rc;
	rc = mbtowc(nlc, c, MB_CUR_MAX);
	if (rc > 1)
		return(rc);
	else
		return(1);
}
