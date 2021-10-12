static char sccsid[] = "@(#)77	1.3  src/bos/usr/ccs/lib/libc/NCisalnum.c, libcnls, bos411, 9428A410j 2/26/91 12:43:38";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCisalnum
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>

#ifdef NCisalnum
#undef NCisalnum
#endif

int NCisalnum(int pc)
{

	return( iswalnum(pc) );
}
