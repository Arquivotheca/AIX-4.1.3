static char sccsid[] = "@(#)78 1.7 src/bos/usr/bin/sccs/lib/logname.c, cmdsccs, bos411, 9428A410j 2/6/94 10:35:28";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: logname
 *
 * ORIGINS: 3, 10, 27, 71
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#include <pwd.h>
#include <sys/types.h>

extern void          endpwent();
extern uid_t           getuid();

char	*logname()
{
register struct passwd *pw;

	pw = getpwuid(getuid());
	endpwent();
	if(pw == (struct passwd *) 0)
	    return(0);
	else	
	    return (pw->pw_name);
}
