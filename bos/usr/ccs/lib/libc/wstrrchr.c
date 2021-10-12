static char sccsid[] = "@(#)52	1.2  src/bos/usr/ccs/lib/libc/wstrrchr.c, libcnls, bos411, 9428A410j 6/8/91 17:18:06";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrrchr
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 ,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <string.h>
#include <ctype.h>

/*
 *  Return the ptr in sp at which the character c last appears; NULL
 *  if not found.  
 */

wchar_t * wstrrchr(wchar_t *sp, int c)
{
	return ( wcsrchr(sp, (wchar_t)c) );
}
