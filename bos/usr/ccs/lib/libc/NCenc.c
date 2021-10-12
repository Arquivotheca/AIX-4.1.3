static char sccsid[] = "@(#)67	1.3  src/bos/usr/ccs/lib/libc/NCenc.c, libcnls, bos411, 9428A410j 2/26/91 12:43:26";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCenc
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

/* Note that this function is NLS/JLS independent */

#include <stdlib.h>
#include <ctype.h>

#ifdef NCenc
#undef NCenc
#endif

int NCenc(wchar_t *x, char *c)
{
	int rc;

	rc = wctomb(c, *x);

	if (rc > 1)
		return (rc);
	else
		return (1);
}
