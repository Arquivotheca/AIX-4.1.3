static char sccsid[] = "@(#)53	1.2  src/bos/usr/ccs/lib/libc/wstrspn.c, libcnls, bos411, 9428A410j 6/8/91 17:18:08";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrspn
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
 *  Return the number of elements (bytes or wchar_ts) in the longest
 *  leading segment of string that consists solely of characters
 *  from charset.  
 */

int wstrspn(wchar_t *string, wchar_t *charset)
{
	return ( wcsspn(string, charset) );
}
