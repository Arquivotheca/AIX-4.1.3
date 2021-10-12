static char sccsid[] = "@(#)69	1.1  src/bos/usr/ccs/lib/libc/NCflatchr.c, libcnls, bos411, 9428A410j 9/22/89 13:19:22";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCflatchr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <NLctype.h>

static int
__NCflatchr(int c)
{
	return(NCflatchr(c));
}

#ifdef NCflatchr
#undef NCflatchr
#endif

NCflatchr(int c)
{
	return(__NCflatchr(c));
}

