static char sccsid[] = "@(#)68	1.3  src/bos/usr/ccs/lib/libc/NCeqvmap.c, libcnls, bos411, 9428A410j 2/26/91 12:43:33";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCeqvmap
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

#include <NLchar.h>

#ifdef NCeqvmap
#undef NCeqvmap
#endif

int
NCeqvmap(int ucval)
{
	return( 1 );
}
