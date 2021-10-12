static char sccsid[] = "@(#)84	1.3  src/bos/usr/ccs/lib/libc/NCisxdigit.c, libcnls, bos411, 9428A410j 2/26/91 12:44:03";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCisxdigit
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

#ifdef NCisxdigit
#undef NCisxdigit
#endif

int NCisxdigit(int pc)
{

	return( iswxdigit(pc) );
}
