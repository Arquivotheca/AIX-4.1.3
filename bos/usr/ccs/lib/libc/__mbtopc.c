static char sccsid[] = "@(#)30	1.2.1.1  src/bos/usr/ccs/lib/libc/__mbtopc.c, libccppc, bos411, 9428A410j 5/25/92 13:43:28";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __mbtopc
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
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <stdlib.h>

int __mbtopc(wchar_t *ws, char *s, int count, int *err)
{
	return _CALLMETH(__lc_charmap,__mbtopc)(__lc_charmap, 
					       ws, s, count, err);
}
