static char sccsid[] = "@(#)50	1.2.1.2  src/bos/usr/ccs/lib/libc/wcswidth.c, libccppc, bos411, 9428A410j 1/12/93 11:20:50";
/*
 * COMPONENT_NAME: LIBCCPPC
 *
 * FUNCTIONS: wcswidth
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/lc_sys.h>
#include <stdlib.h>
#include <ctype.h>

/*
  returns the number of characters for a SINGLE-BYTE codeset
*/
int wcswidth(wchar_t *wcs, size_t n)
{
    return _CALLMETH(__lc_charmap,__wcswidth)(__lc_charmap, wcs, n);
}
