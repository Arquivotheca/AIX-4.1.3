static char sccsid[] = "@(#)29	1.3  src/bos/usr/bin/uucp/uucpname.c, cmduucp, bos411, 9428A410j 6/16/90 00:02:19";
/* 
 * COMPONENT_NAME: UUCP uucpname.c
 * 
 * FUNCTIONS: uucpname 
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

/*	/sccs/src/cmd/uucp/s.uucpname.c
	uucpname.c	1.1	7/29/85 16:33:52
*/
#include "uucp.h"
/* VERSION( uucpname.c	5.2 -  -  ); */

/*
 * get the uucp name
 * return:
 *	none
 */
void
uucpname(name)
register char *name;
{
	char *s;

#ifdef BSD4_2
	int nlen;
	char	NameBuf[MAXBASENAME + 1];

	/* This code is slightly wrong, at least if you believe what the */
	/* 4.1c manual says.  It claims that gethostname's second parameter */
	/* should be a pointer to an int that has the size of the buffer. */
	/* The code in the kernel says otherwise.  The manual also says that */
	/* the string returned is null-terminated; this, too, appears to be */
	/* contrary to fact.  Finally, the variable containing the length */
	/* is supposed to be modified to have the actual length passed back; */
	/* this, too, doesn't happen.  So I'm zeroing the buffer first, and */
	/* passing an int, not a pointer to one.  *sigh*

	/*		--Steve Bellovin	*/
	bzero(NameBuf, sizeof NameBuf);
	nlen = sizeof NameBuf;
	gethostname(NameBuf, nlen);
	s = NameBuf;
	s[nlen] = '\0';
#else !BSD4_2
#ifdef UNAME
	struct utsname utsn;
	char *p;

	uname(&utsn);
	s = utsn.nodename;
#ifdef AIX
	if ((p = strchr(s,'.')) != NULL)
		*p = '\0';
#endif AIX
#else !UNAME
	char	NameBuf[MAXBASENAME + 1], *strchr();
	FILE	*NameFile;

	s = MYNAME;
	NameBuf[0] = '\0';

	if ((NameFile = fopen("/etc/whoami", "r")) != NULL) {
		/* etc/whoami wins */
		(void) fgets(NameBuf, MAXBASENAME + 1, NameFile);
		(void) fclose(NameFile);
		NameBuf[MAXBASENAME] = '\0';
		if (NameBuf[0] != '\0') {
			if ((s = strchr(NameBuf, '\n')) != NULL)
				*s = '\0';
			s = NameBuf;
		}
	}
#endif UNAME
#endif BSD4_2

	(void) strncpy(name, s, MAXBASENAME);
	name[MAXBASENAME] = '\0';
	return;
}
