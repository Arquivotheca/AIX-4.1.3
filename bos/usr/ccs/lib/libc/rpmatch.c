static char sccsid[] = "@(#)71	1.2.1.2  src/bos/usr/ccs/lib/libc/rpmatch.c, libcpat, bos411, 9428A410j 1/12/93 11:19:05";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: rpmatch
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/lc_sys.h>
#include <sys/localedef.h>

int rpmatch(const char *s)
{
	return _CALLMETH(__lc_resp, __rpmatch)(__lc_resp,s);
}
