static char sccsid[] = "@(#)65	1.3.1.2  src/bos/usr/ccs/lib/libc/regerror.c, libcpat, bos411, 9428A410j 1/12/93 11:18:47";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: regerror
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
 * FUNCTION: regerror()
 *
 * DESCRIPTION: fetch message text of regcomp() or regexec() error
 *	        invoke appropriate method for this locale.
 */

size_t regerror(int errcode, const regex_t *preg, 
		char *errbuf, size_t errbuf_size)
{
    return _CALLMETH(__lc_collate,__regerror)(__lc_collate, errcode, preg, 
					   errbuf, errbuf_size);
}
