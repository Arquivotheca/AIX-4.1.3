static char sccsid[] = "@(#)96	1.8  src/bos/usr/ccs/lib/libPW/xcreat.c, libPW, bos411, 9428A410j 11/10/93 15:18:12";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xcreat
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


/*
 * FUNCTION: Create a file.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns file descriptor on success,
 *	fatal() on failure.
 */

#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)
# include	"sys/types.h"
# include	"macros.h"

/*
	"Sensible" creat: write permission in directory is required in
	all cases, and created file is guaranteed to have specified mode
	and be owned by effective user.
	(It does this by first unlinking the file to be created.)
	Returns file descriptor on success,
	fatal() on failure.
*/


xcreat(name,mode)
char *name;
int mode;
{
	register int fd;
	char d[NL_TEXTMAX];
	char Error[NL_TEXTMAX];
	extern char *dname();

	copy(name,d);
	if (!exists(dname(d))) {
		catd = catopen(MF_PW, NL_CAT_LOCALE);
		sprintf(Error, MSGSTR(XNODIR, 
			"directory `%s' does not exist (ut1)"), d);
		catclose(catd);
		return(fatal(Error));
	}
	unlink(name);
	if ((fd = creat(name,mode)) >= 0)
		return(fd);
	return(xmsg(name,"xcreat"));
}
