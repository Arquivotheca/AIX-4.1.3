static char sccsid[] = "@(#)06	1.1  src/bos/usr/bin/uucp/strsave.c, cmduucp, bos411, 9428A410j 6/17/93 13:26:14";
/* 
 * COMPONENT_NAME: CMDUUCP strsave.c
 * 
 * FUNCTIONS: strsave, eaccess
 *            
 *
 * ORIGINS: 10  27  3 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
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

/* #ident	"@(#)uucp:strsave.c	1.2" */

/* copy str into data space -- caller should report errors. */

/* export */
extern char *strsave();

/* import */
extern char *malloc();

char *
strsave(str)
register char *str;
{
	register char *rval;
	extern char *malloc();

	rval = malloc(strlen(str) + 1);
	if (rval != 0)
		strcpy(rval, str);
	return(rval);
}

/*	Determine if the effective user id has the appropriate permission
	on a file.  Modeled after access(2).
	amode:
		00	just checks for file existence.
		04	checks read permission.
		02	checks write permission.
		01	checks execute/search permission.
		other bits are ignored quietly.
*/

#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern int	errno;


int
eaccess( path, amode )
char		*path;
register int	amode;
{
int x;
	struct stat	s;

	if( stat( path, &s ) == -1 )
		return  -1;
	amode &= 07;
	if( !amode )
		return  0;		/* file exists */

	if( (amode & s.st_mode) == amode )
		return  0;		/* access permitted by "other" mode */

	amode <<= 3;
	if( getegid() == s.st_gid  &&  (amode & s.st_mode) == amode )
		return  0;		/* access permitted by group mode */

	amode <<= 3;
	if( geteuid() == s.st_uid  &&  (amode & s.st_mode) == amode )
		return  0;		/* access permitted by owner mode */

	errno = EACCES;
	return  -1;
}
