static char sccsid[] = "@(#)18	1.8  src/bos/usr/ccs/lib/libPW/xmsg.c, libPW, bos411, 9428A410j 11/10/93 15:18:26";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: xmsg
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
 * FUNCTION: Print an error message based on errno.
 *
 * RETURN VALUE DESCRIPTIONS:
 *		fatal()
 */
/*
	Call fatal with an appropriate error message
	based on errno.  If no good message can be made up, it makes
	up a simple message.
	The second argument is a pointer to the calling functions
	name (a string); it's used in the manufactured message.
*/


#include "pw_msg.h"
nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_PW, Num, Str)

# include	"errno.h"
# include	"sys/types.h"
# include	"macros.h"


xmsg(file,func)
char *file, *func;
{
	register char *str;
	char Error[NL_TEXTMAX];
	char buf[NL_TEXTMAX];
	extern int errno, sys_nerr;
	/*extern char *sys_errlist[];*/
	extern char *dname();

	catd = catopen(MF_PW, NL_CAT_LOCALE);
	switch (errno) {
	case ENFILE:
		str = MSGSTR(XTABFULL, "file table full (ut3)");
		break;
	case ENOENT:
		sprintf(str = Error, MSGSTR(XNOFILE, 
			"`%s' does not exist (ut4)"), file);
		break;
	case EACCES:
		copy(file,buf);
		sprintf(str = Error, MSGSTR(XNODWRT,
			"directory `%s' unwritable (ut2)"), dname(buf));
		break;
	case ENOSPC:
		str = MSGSTR(XNOSPC, "no space! (ut10)");
		break;
	case EFBIG:
		str = MSGSTR(XTOOBIG, "file too big (ut8)");
		break;
	default:
		if ((unsigned)errno < sys_nerr)
			perror(errno);
		else
			sprintf(str = buf, MSGSTR(XERRNO, "errno = %d"),
				errno);
		sprintf(Error, MSGSTR(XFUNC, "%s, function = `%s' (ut11)"),
			str, func);
		str = Error;
		break;
	}
	catclose(catd);
	return(fatal(str));
}
