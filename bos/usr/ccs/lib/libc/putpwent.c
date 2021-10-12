static char sccsid[] = "@(#)99	1.7  src/bos/usr/ccs/lib/libc/putpwent.c, libcs, bos411, 9428A410j 3/24/93 11:05:30";
/*
 * COMPONENT_NAME: (LIBCS) Standard C Library System Security Functions 
 *
 * FUNCTIONS: putpwent 
 *
 * ORIGINS: 3, 27 
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

/*LINTLIBRARY*/
#include <stdio.h>
#include <pwd.h>

/*                                                                    
 * FUNCTION: Update a user description in the files /etc/passwd
 *		and /etc/security/passwd.
 *
 * PARAMETERS: pw - struct passwd *pw;  the description of the users
 *					password entry.
 *
 * RETURN VALUE DESCRIPTIONS: Upon successful conpletion, PUTPWENT returns
 *				a value of 0. If PUTPWENT fails, a nonzero
 *				value is returned.
 *
 */
/*
 *
 * format a password file entry
 */

int
putpwent(p, f)
register struct passwd *p;
register FILE *f;
{
	if ( p == NULL || f == NULL )
		return(-1);
	/*
	 * AIX Version 2.2.1 security enhancement 
	 *	If yellow pages escape ('+') entry then don't
	 *	change the uid or gid fields
	 */
	(void) fprintf(f, "%s:", p->pw_name);

	if (strcmp (p->pw_name, "+") == 0)
			(void) fprintf(f, ":");
	else
			(void) fprintf(f, "!:");

	if (*p->pw_name == '+' && strlen (p->pw_name) > 1)
	{
			(void) fprintf(f, "::%s:%s:%s",
			p->pw_gecos,
			p->pw_dir,
			p->pw_shell);
	}
	else
	{
			(void) fprintf(f, "%u:%u:%s:%s:%s",
			p->pw_uid,
			p->pw_gid,
			p->pw_gecos,
			p->pw_dir,
			p->pw_shell);
	}
	(void) putc('\n', f);
	return(ferror(f));
}
/* 
 * TCSEC Division C Class C2 
 */
