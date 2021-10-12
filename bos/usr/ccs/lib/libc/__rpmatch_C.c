static char sccsid[] = "@(#)72	1.2.1.1  src/bos/usr/ccs/lib/libc/__rpmatch_C.c, libcpat, bos411, 9428A410j 5/25/92 14:04:26";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions 
 *
 * FUNCTIONS: __rpmatch_C
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
 */

#include <sys/localedef.h>


/************************************************************************/
/* __rpmatch_C - determine if response matches yes or no pattern		*/
/*	     - optimized for the C locale				*/
/************************************************************************/
int
__rpmatch_C(_LC_resp_objhdl_t hdl, const char *response)
{
/*
 * check for positive response
 */
	if (*response == 'y' || *response == 'Y')
		return (1);
/*
 * check for negative response
 */
	if (*response == 'n' || *response == 'N')
		return (0);
/*
 * response does not match either yes or no expression
 */
	return (-1);
}
