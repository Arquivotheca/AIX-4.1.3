static char sccsid[] = "@(#)24	1.1  src/bos/usr/ccs/lib/libc/isjspace.c, libcnls, bos411, 9428A410j 2/26/91 17:42:23";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: isjspace
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

#include <ctype.h>
#include <NLctype.h>

#ifdef isjspace
#undef isjspace
#endif

int
isjspace(int c)
{
	return( iswspace(c) );
}
