static char sccsid[] = "@(#)81	1.5  src/bos/usr/bin/uucp/chremdir.c, cmduucp, bos411, 9428A410j 6/17/93 13:54:07";
/* 
 * COMPONENT_NAME: CMDUUCP chremdir.c
 * 
 * FUNCTIONS: chremdir, mkremdir 
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	/sccs/src/cmd/uucp/s.chremdir.c
	chremdir.c	1.1	7/29/85 16:32:42
*/
#include "uucp.h"
/* VERSION( chremdir.c	5.2 -  -  ); */

extern nl_catd catd;
/*
 * chremdir(sys)
 * char	*sys;
 *
 * create SPOOL/sys directory and chdir to it
 * side effect: set RemSpool
 */
void
chremdir(sys)
char	*sys;
{
	int	ret;

	mkremdir(sys);	/* set RemSpool, makes sure it exists */
	DEBUG(6, "chdir(%s)\n", RemSpool);
	ret = chdir(RemSpool);
	ASSERT(ret == 0, MSGSTR(MSG_UDEFS_11, "CANNOT CHANGE DIRECTORY"),
 		RemSpool, errno);
	(void) strcpy(Wrkdir, RemSpool);
	return;
}

/*
 * mkremdir(sys)
 * char	*sys;
 *
 * create SPOOL/sys directory
 */

void
mkremdir(sys)
char	*sys;
{
	(void) sprintf(RemSpool, "%s/%s", SPOOL, sys);
	(void) mkdirs2(RemSpool, DIRMASK);
	return;
}
