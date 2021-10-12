static char sccsid[] = "@(#)54	1.2  src/bos/usr/ccs/lib/libc/wstrtok.c, libcnls, bos411, 9428A410j 6/8/91 17:18:10";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrtok
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
 */

#include <string.h>
#include <ctype.h>

/*
 *  Uses strpbrk and strspn to break string into tokens on sequentially
 *  subsequent calls.  Returns NULL when no non-separator characters
 *  remain.  `Subsequent' calls are calls with first argument NULL.
 */

wchar_t *wstrtok(wchar_t *string, wchar_t *sepset)
{
	return ( wcstok(string, sepset) );
}
