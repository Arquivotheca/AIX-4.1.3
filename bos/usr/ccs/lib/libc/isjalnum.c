static char sccsid[] = "@(#)12	1.1  src/bos/usr/ccs/lib/libc/isjalnum.c, libcnls, bos411, 9428A410j 2/26/91 17:41:34";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: isjalnum
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

#ifdef isjalnum
#undef isjalnum
#endif

isjalnum(int c)
{
	return ( iswalnum(c) );
}
