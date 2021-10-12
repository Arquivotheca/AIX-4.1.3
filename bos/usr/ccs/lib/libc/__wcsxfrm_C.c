static char sccsid[] = "@(#)30	1.4.1.1  src/bos/usr/ccs/lib/libc/__wcsxfrm_C.c, libcstr, bos411, 9428A410j 5/25/92 14:07:44";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: __wcsxfrm_C
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
#include <string.h>

/*
 * FUNCTION: __wcsxfrm_C - Method implementing wcsxfrm() for C locale.
 *
 * PARAMETERS:
 *           _LC_collate_objhdl_t hdl - unused
 *
 *	     wchar_t *ws1 - output string of collation weights.
 *	     wchar_t *ws2 - input string of characters.
 *           size_t n     - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to ws1.
 */

size_t __wcsxfrm_C(_LC_collate_objhdl_t hdl, wchar_t *ws1, const wchar_t *ws2, 
		   size_t n)
{
    size_t i;
    int len;

    len = wcslen(ws2);

    if (ws1==NULL || n==0)
	return len;

    n--;
    wcsncpy(ws1, ws2, n);
    ws1[n] = 0x00;
    return len;
}
