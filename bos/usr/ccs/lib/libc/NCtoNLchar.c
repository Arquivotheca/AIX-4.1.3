static char sccsid[] = "@(#)81	1.3  src/bos/usr/ccs/lib/libc/NCtoNLchar.c, libcnls, bos411, 9428A410j 2/26/91 12:44:06";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCtoNLchar
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

/*******************************
** this v3.1 routine will work**
** with all v3.2 single byte  **
** code sets and with pc932.  **
** All other code pages are   **
** not supported.             **
********************************/

#include <stdlib.h>
#include <NLctype.h>

#ifdef NCtoNLchar
#undef NCtoNLchar
#endif

int NCtoNLchar(int c)
{
	if (MB_CUR_MAX == 1)
		return ( (c) & 0xff);
	else
		return ( (c) & 0xffff);
}

