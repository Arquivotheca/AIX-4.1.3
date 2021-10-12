static char sccsid[] = "@(#)49	1.2.1.2  src/bos/usr/ccs/lib/libc/mbstowcs.c, libccppc, bos411, 9428A410j 1/12/93 11:18:00";
/*
 * COMPONENT_NAME: LIBCCPPC
 *
 * FUNCTIONS: mbstowcs
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

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
	return _CALLMETH(__lc_charmap,__mbstowcs)(__lc_charmap, pwcs, s, n);
}
