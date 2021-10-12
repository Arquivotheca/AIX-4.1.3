static char sccsid[] = "@(#)82	1.3  src/bos/usr/ccs/lib/libc/NLchrlen.c, libcnls, bos411, 9428A410j 2/26/91 12:44:08";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLchrlen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This function returns the length of the multibyte character pointed
 * to by the argument. The ANSI counterpart is mblen.
 * It depends on the definition of multi-byte characters...
 */

#include <stdlib.h>
#include <ctype.h>

#ifdef NLchrlen
#undef NLchrlen
#endif

int 
NLchrlen(char *c)
{
	int rc;

	rc = mblen(c, MB_CUR_MAX);

	if (rc > 1)
		return (rc);
	else
		return (1);
}
