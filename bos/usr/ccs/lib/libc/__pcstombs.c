static char sccsid[] = "@(#)34	1.2.1.1  src/bos/usr/ccs/lib/libc/__pcstombs.c, libccppc, bos411, 9428A410j 5/25/92 13:43:53";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __pcstombs
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

int __pcstombs(char *s, size_t s_sz, 
               char *ws, size_t ws_sz, char **endptr, int *err)
{
	return _CALLMETH(__lc_charmap,__pcstombs)(__lc_charmap, 
						 s, s_sz, ws, ws_sz, 
						 endptr, err);
}
