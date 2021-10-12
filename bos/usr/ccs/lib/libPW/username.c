static char sccsid[] = "@(#)63	1.7  src/bos/usr/ccs/lib/libPW/username.c, libPW, bos411, 9428A410j 6/2/91 22:14:18";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: username
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
 * FUNCTION: Gets user's login name.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	Returns pointer to login name on success,
 *	pointer to string representation of used ID on failure.
 */
/*
	Gets user's login name.
	Note that the argument must be an integer.
	Returns pointer to login name on success,
	pointer to string representation of used ID on failure.

	Remembers user ID and login name for subsequent calls.
 */

char *
username(uid_t uid)
{
#define	ONAMMAX		9
	static uid_t ouid = -1;
	static char onam[ONAMMAX];
	struct passwd *pwd;

	if (ouid != uid || ouid == -1) {
		if ((pwd = getpwuid(uid)) == NULL)
			sprintf(onam, "%d", uid);
		else {
			strncpy(onam, pwd->pw_name, ONAMMAX);
			onam[ONAMMAX - 1] = '\0';
		}
		ouid = uid;
	}
	return(onam);
}

