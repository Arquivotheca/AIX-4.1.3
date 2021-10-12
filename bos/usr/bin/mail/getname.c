static char sccsid[] = "@(#)56	1.4.1.1  src/bos/usr/bin/mail/getname.c, cmdmailx, bos41B, 9506A 1/25/95 16:41:23";
/* 
 * COMPONENT_NAME: CMDMAILX getname.c
 * 
 * FUNCTIONS: getname, getuserid 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * #ifndef lint
 * static char *sccsid = "getname.c    5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

#include <pwd.h>
#ifdef AIX
	/* function types not defined in AIX include file */
extern struct passwd *_getpwnam_shadow(char *, int);
#endif

/*
 * Getname / getuserid for those with
 * hashed passwd data base).
 *
 */

#include "rcv.h"

/*
 * Search the passwd file for a uid.  Return name through ref parameter
 * if found, indicating success with 0 return.  Return -1 on error.
 * If -1 is passed as the user id, close the passwd file.
 */

getname(uid, namebuf)
	char namebuf[];
{
	struct passwd *pw;

	if (uid == -1) {
		return(0);
	}
	if ((pw = getpwuid(uid)) == NULL)
		return(-1);
	strcpy(namebuf, pw->pw_name);
	return 0;
}

/*
 * Convert the passed name to a user id and return it.  Return -1
 * on error.  Iff the name passed is -1 (yech) close the pwfile.
 */

getuserid(name)
	char name[];
{
	struct passwd *pw;

	if (name == (char *) -1) {
		return(0);
	}
	if ((pw = _getpwnam_shadow(name,0)) == NULL)
		return 0;
	return pw->pw_uid;
}
