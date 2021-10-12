static char sccsid[] = "@(#)76  1.5  src/bos/usr/bin/mail/v7.local.c, cmdmailx, bos41J, 9516B_all 4/19/95 15:35:17";
/* 
 * COMPONENT_NAME: CMDMAILX v7.local.c
 * 
 * FUNCTIONS: demail, findmail, username 
 *
 * ORIGINS: 10  26  27  71
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * #ifndef lint
 * static char *sccsid = "v7.local.c   5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

/*
 * Mail -- a mail program
 *
 * Version 7
 *
 * Local routines that are installation dependent.
 */

#include "rcv.h"

/*
 * Locate the user's mailbox file (ie, the place where new, unread
 * mail is queued).  In Version 7, it is in /usr/spool/mail/name.
 */

findmail()
{
	register char *cp, *p;
	extern char	*getenv();

	p = getenv ("mailbox");
	if (p == NOSTR)
		p = getenv("MAILBOX");
	if (p == NOSTR)
		p = value ("mailbox");
	if (p == NOSTR)
		p = MAILBOX;
	cp = copy(p, mailname);
#ifdef notdef
	copy(myname, cp);
#endif
	if (isdir(mailname)) {
		stradd(mailname, '/');
		strcat(mailname, myname);
	}
}

/*
 * Get rid of the queued mail.
 */

demail(char *mailboxname)
{

	if (value("keep") != NOSTR)
		close(creat(mailboxname, 0666));
	else {
		if (remove_file(mailboxname) < 0)
			close(creat(mailboxname, 0666));
	}
}

/*
 * Discover user login name.
 */

username(uid, namebuf)
	char namebuf[];
{
	register char *np;

	if (uid == getuid() && (np = getenv("USER")) != NOSTR) {
		strncpy(namebuf, np, PATHSIZE);
		return(0);
	}
	return(getname(uid, namebuf));
}
