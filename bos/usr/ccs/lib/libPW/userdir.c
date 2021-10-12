static char sccsid[] = "@(#)40	1.7  src/bos/usr/ccs/lib/libPW/userdir.c, libPW, bos411, 9428A410j 6/2/91 22:18:50";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: userdir
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
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

#include <stdio.h>
#include <pwd.h>

/*
 * FUNCTION: Gets user's login directory.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns pointer to login directory on success,
 *	0 on failure.
 */
/*
	Gets user's login directory.
	The argument must be an integer.
	Note the assumption about position of directory field in
	password file (no group id in password file).
	Returns pointer to login directory on success,
	0 on failure.
        Remembers user ID and login directory for subsequent calls.
 */



char *
userdir(uid_t uid)
{
#define	ODIRMAX	33
	static uid_t ouid = -1;
	static char odir[ODIRMAX];
	struct passwd *pwd;

	if (ouid != uid || ouid == -1) {
		if ((pwd = getpwuid(uid)) == NULL)
		    return(NULL);
		strncpy(odir, pwd->pw_dir, ODIRMAX);
		odir[ODIRMAX - 1] = '\0';
	}
	return(odir);
}
