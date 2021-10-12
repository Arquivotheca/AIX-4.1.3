static char sccsid[] = "@(#)29	1.7  src/bos/usr/ccs/lib/libPW/xopen.c, libPW, bos411, 9428A410j 11/10/93 15:18:34";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xopen
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

# include "errno.h"

/*
 * FUNCTION: Interface to open(II).
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns file descriptor on success,
 *	fatal() on failure.
 */
/*
	Interface to open(II) which differentiates among the various
	open errors.
	Returns file descriptor on success,
	fatal() on failure.
*/


#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)

xopen(name,mode)
char name[];
int mode;
{
	register int fd;
	extern int errno;
	char Error[NL_TEXTMAX];

	if ((fd = open(name,mode)) < 0) {
		if(errno == EACCES) {
			catd = catopen(MF_PW, NL_CAT_LOCALE);
			if(mode == 0)
				sprintf(Error, MSGSTR(XNORD,
					"`%s' unreadable (ut5)"), name);
			else if(mode == 1)
				sprintf(Error, MSGSTR(XNOWRT,
					"`%s' unwritable (ut6)"), name);
			else
				sprintf(Error, MSGSTR(XNORDWRT,
					"`%s' unreadable or unwritable (ut7)"),
					name);
			catclose(catd);
			fd = fatal(Error);
		}
		else
			fd = xmsg(name,"xopen");
	}
	return(fd);
}
