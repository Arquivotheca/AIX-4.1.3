static char sccsid[] = "@(#)63	1.3.1.1  src/bos/usr/ccs/lib/libc/NCchrlen.c, libcnls, bos411, 9428A410j 5/25/92 13:55:21";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCchrlen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/* This function returns the length of the multi-byte character the
 * wchar_t argument would translate to.
 * There is no corresponding ANSI or JCLC function.
 */

#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#ifdef NCchrlen
#undef NCchrlen
#endif

int NCchrlen(wchar_t nlc)
{
	int	rc;
	char	s[NL_NMAX];

	rc = wctomb(s, nlc);

	if (rc > 1)
		return(rc);
	else
		return (1);
}
