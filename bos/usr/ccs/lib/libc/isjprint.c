static char sccsid[] = "@(#)22	1.1  src/bos/usr/ccs/lib/libc/isjprint.c, libcnls, bos411, 9428A410j 2/26/91 17:42:14";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: isjprint
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

#ifdef isjprint
#undef isjprint
#endif

int
isjprint(int c)
{
	return( iswprint(c) );
}
