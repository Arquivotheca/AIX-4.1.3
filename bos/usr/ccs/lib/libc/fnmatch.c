static char sccsid[] = "@(#)59	1.2.1.2  src/bos/usr/ccs/lib/libc/fnmatch.c, libcpat, bos411, 9428A410j 1/12/93 11:14:25";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: fnmatch
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
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/lc_sys.h>
#include <sys/localedef.h>

/*
 * FUNCTION: fnmatch
 *
 * DESCRIPTION: Filename pattern matching stub function.  Invokes
 * method appropriate for this locale.
*/
int fnmatch(const char *ppat, const char *string, int flags)
{
	return _CALLMETH(__lc_collate, __fnmatch)(__lc_collate, ppat, 
					       string, string, flags);
}
