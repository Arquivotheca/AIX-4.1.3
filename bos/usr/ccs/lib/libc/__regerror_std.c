static char sccsid[] = "@(#)43	1.2.1.2  src/bos/usr/ccs/lib/libc/__regerror_std.c, libcpat, bos411, 9428A410j 11/10/93 15:26:07";
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
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/localedef.h>
#include <stdlib.h>
#include <regex.h>
#include "libc_msg.h"

/************************************************************************/
/* __regerror_std() - Get Text for RE Error Message			*/
/************************************************************************/

size_t
__regerror_std(_LC_collate_objhdl_t hdl, int errcode, const regex_t *preg, 
		char *errbuf, size_t errbuf_size)
{
	int	erroff;		/* index into local error text table	*/
	nl_catd	catd;		/* MRI catalog descriptor		*/
	int	msglen;		/* error message length			*/
	char	*pmsg;		/* ptr to MRI message			*/
	char	*etable[] =
		{
		"pattern not found",			/* REG_NOMATCH	*/
		"invalid pattern",			/* REG_BADPAT	*/
		"invalid collating element",		/* REG_ECOLLATE	*/
		"invalid character class",		/* REG_ECTYPE	*/
		"last character is \\",			/* REG_EESCAPE	*/
		"invalid backreference number",		/* REG_ESUBREG	*/
		"[] imbalance",				/* REG_EBRACK	*/
		"\\( \\) or ( ) imbalance",		/* REG_EPAREN	*/
		"\\{ \\} imbalence",			/* REG_EBRACE	*/
		"invalid \\{ \\} repetition",		/* REG_BADBR	*/
		"invalid range expression",		/* REG_ERANGE	*/
		"out of memory",			/* REG_ESPACE	*/
		"*?+ not preceded by valid expression",	/* REG_BADRPT	*/
		"invalid multibyte character",		/* REG_ECHAR	*/
		"^ anchor not at beginning of pattern",	/* REG_EBOL	*/
		"$ anchor not at end of pattern",	/* REG_EEOL	*/
		};
#define MAXERROR (sizeof etable / sizeof (char *)) /* max error index	*/

/*
 * verify error code is a valid RE error
 * return error status if invalid error code
 */
	erroff = errcode - REG_NOMATCH;
	if (erroff < 0 || erroff >= MAXERROR)
		return (0);
/*
 * open message catalog
 * get error message text pointer
 * move up to 'errbuf_size' bytes of error message text into errbuf
 * close catalog
 * return true length of error message text
 */
	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
	pmsg = catgets(catd, MS_REG, errcode, etable[erroff]);
	msglen = strlen(pmsg) + 1;
	if (errbuf_size > 0)
		if (msglen <= errbuf_size)
			strcpy(errbuf, pmsg);
		else
			{
			strncpy(errbuf, pmsg, errbuf_size-1);
			errbuf[errbuf_size-1] = '\0';
			}
	catclose(catd);
	return (msglen);
}
