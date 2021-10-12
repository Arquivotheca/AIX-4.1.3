static char sccsid[] = "@(#)26	1.3.1.2  src/bos/usr/ccs/lib/libc/__mbstopcs.c, libccppc, bos411, 9428A410j 2/10/93 15:55:39";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __mbstopcs
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/localedef.h>
#include <sys/lc_sys.h>
#include <stdlib.h>

int __mbstopcs(wchar_t *ws, size_t ws_sz, 
               char *s, size_t s_sz, int stopchr, char **endptr, int *err)
{
	return _CALLMETH(__lc_charmap,__mbstopcs)(__lc_charmap, 
						 ws, ws_sz, s, s_sz, 
						 stopchr, endptr, err);
}
