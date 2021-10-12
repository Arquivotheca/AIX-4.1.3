static char sccsid[] = "@(#)07	1.7  src/bos/usr/ccs/lib/libPW/xlink.c, libPW, bos411, 9428A410j 11/10/93 15:18:20";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS:  xlink
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */

# include	"errno.h"

/*
 * FUNCTION: Interface to link(II).
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns 0 on success,
 *	fatal() on failure.
 */
/*
	Interface to link(II) which handles all error conditions.
	Returns 0 on success,
	fatal() on failure.
*/

#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)


xlink(f1,f2)
char *f1, *f2;
{
	extern errno;
	char Error[NL_TEXTMAX];

	if (link(f1,f2)) {
		if (errno == EEXIST || errno == EXDEV) {
			catd = catopen(MF_PW, NL_CAT_LOCALE);
			sprintf(Error, MSGSTR(XNOLINK,
				"cannot link `%s' to `%s' (ut%d)"),
				f2, f1, errno == EEXIST ? 20 : 21);
			catclose(catd);
			return(fatal(Error));
		}
		if (errno == EACCES)
			f1 = f2;
		return(xmsg(f1,"xlink"));
	}
	return(0);
}
