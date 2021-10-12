static char sccsid[] = "@(#)63	1.2.1.2  src/bos/usr/ccs/lib/libc/regcomp.c, libcpat, bos411, 9428A410j 1/12/93 11:18:43";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: regcomp
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
#include <regex.h>

/*
 * FUNCTION: regcomp()
 * 
 * DESCRIPTION: compile Regular Expression for use by regexec()
 *	        invoke appropriate method for this locale.
 */

int regcomp(regex_t *preg, const char *pattern, int cflags)
{
    return _CALLMETH(__lc_collate,__regcomp)(__lc_collate, preg, 
					  pattern, cflags);
}
