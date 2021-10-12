static char sccsid[] = "@(#)46	1.2  src/bos/usr/ccs/lib/libc/wstrcspn.c, libcnls, bos411, 9428A410j 6/8/91 17:17:54";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wstrcspn
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
 * NAME: wstrcspn
 *
 * FUNCTION: Return the number of array elements (bytes or wchar_ts) in the
 *  longest leading segment of string that consists solely of characters
 *  NOT from charset.  Two versions here:  NLstrcspn (operates on ASCII
 *  with embedded NLS code points) and NCstrcspn (operates on wchar_ts).
 *  NLstrcspn returns length of prefix matched in bytes, NCstrcspn in
 *  wchar_ts.
 *
 * RETURN VALUE DESCRIPTION: see FUNCTION
 */
int wstrcspn(wchar_t *string, wchar_t *charset)
{
	return ( wcscspn(string, charset) );
}
