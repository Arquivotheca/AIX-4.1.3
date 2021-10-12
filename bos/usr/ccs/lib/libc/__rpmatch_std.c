static char sccsid[] = "@(#)73	1.3.1.2  src/bos/usr/ccs/lib/libc/__rpmatch_std.c, libcpat, bos411, 9428A410j 1/12/93 11:10:28";
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions 
 *
 * FUNCTIONS: __rpmatch_std
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
#include <regex.h>


/************************************************************************/
/* __rpmatch_std - determine if response matches yes or no pattern	*/
/*	       - works for all locales and all code sets		*/
/*	       - enforces requirement for pattern to be at beginning	*/
/*	       -   of response						*/
/************************************************************************/
int
__rpmatch_std(_LC_resp_objhdl_t hdl, const char *response)
{
	regex_t	reg;		/* regular expression status buffer	*/
	regmatch_t pmatch;	/* start/stop match offsets		*/

/*
 * check for positive response
 */
	if (__OBJ_DATA(hdl)->yesexpr != (char *)0)
		if (regcomp(&reg, __OBJ_DATA(hdl)->yesexpr, REG_EXTENDED) == 0)
			if ((regexec(&reg, response, (size_t)1, &pmatch, 0) == 0) &&
				(pmatch.rm_so == 0))
				{
				regfree(&reg);
				return (1);
				}
			else
				regfree(&reg);
/*
 * check for negative response
 */
	if (__OBJ_DATA(hdl)->noexpr != (char *)0)
		if (regcomp(&reg, __OBJ_DATA(hdl)->noexpr, REG_EXTENDED) == 0)
			if ((regexec(&reg, response, (size_t)1, &pmatch, 0) == 0) &&
				(pmatch.rm_so == 0))
				{
				regfree(&reg);
				return (0);
				}
			else
				regfree(&reg);
/*
 * response does not match either yes or no expression
 */
	return (-1);
}
