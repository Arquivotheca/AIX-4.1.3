static char sccsid[] = "@(#)67	1.3.1.2  src/bos/usr/ccs/lib/libc/regexec.c, libcpat, bos411, 9428A410j 1/12/93 11:18:50";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: regexec
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

/************************************************************************/
/* Global data shared by all regcomp() and regexec() methods            */
/************************************************************************/

int __reg_bits[8] = {      		/* bitmask for [] bitmap        */
                0x00000001, 0x00000002, 0x00000004, 0x00000008,
                0x00000010, 0x00000020, 0x00000040, 0x00000080};


/*
 * FUNCTION: regexec()
 *
 * DESCRIPTION: determine if Regular Expression pattern matches string
 *	        invoke appropriate method for this locale
*/

int regexec(const regex_t *preg, const char *string, 
	    size_t nmatch, regmatch_t pmatch[], int eflags)
{
    return _CALLMETH(__lc_collate, __regexec)(__lc_collate, preg, 
					   string, nmatch, pmatch, eflags);
}
