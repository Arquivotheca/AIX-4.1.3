static char sccsid[] = "@(#)99	1.7  src/bos/usr/bin/uucp/gnamef.c, cmduucp, bos411, 9428A410j 3/27/91 10:59:08";
/* 
 * COMPONENT_NAME: UUCP gnamef.c
 * 
 * FUNCTIONS: gdirf, gnamef 
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

/*	/sccs/src/cmd/uucp/s.gnamef.c
	gnamef.c	1.1	7/29/85 16:33:04
*/
#include "uucp.h"
/* VERSION( gnamef.c	5.2 -  -  ); */

/*
 * get next file name from directory
 *	p	 -> file description of directory file to read
 *	filename -> address of buffer to return filename in
 *		    must be of size DIRSIZ+1 
 *			 OR
 * 		    if PDA is defined it must be of size NAME_MAX
 * returns:
 *	FALSE	-> end of directory read
 *	TRUE	-> returned name
 */
gnamef(p, filename)
register char *filename;
DIR *p;
{
#ifdef	BSD4_2 || PDA
	struct dirent dentry;
#else
	struct dirent dentry;
	register struct dirent *dp = &dentry;
#endif
/*	register struct direct *dp = &dentry;	*/

	while (1) {
#ifdef	BSD4_2 || PDA
		if (fread((char *)dp,  sizeof(dentry), 1, p) != 1)
#else	
		if ((dp = readdir(p)) == NULL)
#endif	
			return(FALSE);
	/* osf note: "d_ino" is an aix specific defined in sys/dir.h */
		if (dp->d_ino != 0 && dp->d_name[0] != '.')
			break;
	}

#ifdef PDA
	(void) strcpy(filename, dp->d_name);
#else
	(void) strncpy(filename, dp->d_name, MAXBASENAME);
	filename[MAXBASENAME] = '\0';
#endif
	return(TRUE);
}

/*
 * get next directory name from directory
 *	p	 -> file description of directory file to read
 *	filename -> address of buffer to return filename in
 *		    must be of size DIRSIZ+1
 * returns:
 *	FALSE	-> end of directory read
 *	TRUE	-> returned dir
 */
gdirf(p, filename, dir)
register char *filename;
DIR *p;
char *dir;
{
	char statname[MAXNAMESIZE];

	while (1) {
		if(gnamef(p, filename) == FALSE)
			return(FALSE);
		(void) sprintf(statname, "%s/%s", dir, filename);
		DEBUG(4, "stat %s\n", statname);
		if (DIRECTORY(statname))
		    break;
	}

	return(TRUE);
}
