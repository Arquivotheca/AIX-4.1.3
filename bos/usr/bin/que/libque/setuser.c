static char sccsid[] = "@(#)68	1.14.1.1  src/bos/usr/bin/que/libque/setuser.c, cmdque, bos411, 9428A410j 1/29/93 12:03:04";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "common.h"
#include <IN/standard.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
/* struct passwd *getpwuid(); */
/* AIX security enhancement	*/
#include <sys/id.h>
#include <ctype.h>

#include "libq_msg.h"

static int 	uid,gid, 
		euid,egid;
boolean		setyet=FALSE;
/* AIX security enhancement	*/
uid_t	getuidx(int);

/* AIX security enhancement	*/
/* removed setuser()		*/
/* removed mygetuid()		*/
/* removed mygeteuid()		*/
/* removed mygetgid()		*/
/* removed mygetegid()		*/
/* removed besuperuser()	*/
/* removed beuser()		*/

/* get user login directory of user who exec'd this program*/
char * getlgdir()
{
	struct passwd passwdd,*pw= &passwdd;
	char * retval;
	uid_t	ruid;

	/* AIX security enhancement	*/
	if ((ruid=getuidx(ID_REAL)) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGULGD,"Cannot get user's login directory.  Errno (getuidx) = %d"),errno);

	pw = getpwuid(ruid);
	retval = Qalloc(strlen(pw->pw_dir)+1);
	strcpy(retval,pw->pw_dir);
	return(retval);
}

/* get user name of user who exec'd this program*/
char * getlgnam()
{
	struct passwd passwdd,*pw= &passwdd;
	char * retval;
	uid_t	ruid;

	/* AIX security enhancement	*/
	if ((ruid=getuidx(ID_REAL)) == -1)
		syserr((int)EXITFATAL,GETMESG(MSGULGN,"Cannot get user's log name. Errno (getuidx) = %d"),errno);

	pw = getpwuid(ruid);
	retval = Qalloc(strlen(pw->pw_name)+1);
	strcpy(retval,pw->pw_name);
	return(retval);
}
