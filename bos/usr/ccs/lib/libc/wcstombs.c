static char sccsid[] = "@(#)51	1.2.1.2  src/bos/usr/ccs/lib/libc/wcstombs.c, libccppc, bos411, 9428A410j 1/12/93 11:20:42";
/*
 * COMPONENT_NAME: LIBCCPPC
 *
 * FUNCTIONS: wcstombs
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

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
	return _CALLMETH(__lc_charmap,__wcstombs)(__lc_charmap, s, pwcs, n);
}
