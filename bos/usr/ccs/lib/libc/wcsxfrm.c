static char sccsid[] = "@(#)96	1.2.1.2  src/bos/usr/ccs/lib/libc/wcsxfrm.c, libcstr, bos411, 9428A410j 1/12/93 11:20:53";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcsxfrm
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <string.h>

/*
 * FUNCTION: wcsxfrm - transform wchar string to string of 
 *                     collation weights.
 *
 *           Calls wcsxfrm() method for C locale.
 *
 * PARAMETERS:
 *	     wchar_t *ws1 - output string of collation weights.
 *	     wchar_t *ws2 - input string of characters.
 *           size_t n     - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to ws1.
 */

size_t wcsxfrm(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
	return _CALLMETH(__lc_collate,__wcsxfrm)(__lc_collate,ws1, ws2, n);
}
