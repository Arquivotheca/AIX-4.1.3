static char sccsid[] = "@(#)74  1.9.1.3  src/bos/usr/bin/mail/temp.c, cmdmailx, bos411, 9428A410j 11/17/93 10:41:23";
/* 
 * COMPONENT_NAME: CMDMAILX temp.c
 * 
 * FUNCTIONS: MSGSTR, tinit 
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
 * static char *sccsid = "temp.c       5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

#include "rcv.h"

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Give names to all the temporary files that we will need.
 */

#define FILE_LEN (20)  /* long enough to hold "/tmp/Rs" + pid */
char	tempMail[FILE_LEN];
char	tempQuit[FILE_LEN];
char	tempEdit[FILE_LEN];
char	tempSet[FILE_LEN];
char	tempResid[FILE_LEN];
char	tempMesg[FILE_LEN];

tinit()
{
	register char *cp, *cp2;
	char uname[PATHSIZE];
	register int err = 0;
	register pid_t pid;

	pid = getpid();
	sprintf(tempMail, "/tmp/Rs%05d", pid);
	sprintf(tempResid, "/tmp/Rq%05d", pid);
	sprintf(tempQuit, "/tmp/Rm%05d", pid);
	sprintf(tempEdit, "/tmp/Re%05d", pid);
	sprintf(tempSet, "/tmp/Rx%05d", pid);
	sprintf(tempMesg, "/tmp/Rx%05d", pid);

	if (strlen(myname) != 0) {
		uid = getuserid(myname);
		if (uid == -1) {
			printf(MSGSTR(NOUSER, "\"%s\" is not a user of this system\n"), myname); /*MSG*/
			exit(1);
		}
	}
	else {
		uid = getuid();
		if (username(uid, uname) < 0) {
			(void)copy(MSGSTR(UNKNOWN_USER,"unknown_user"),myname);
			err++;
			if (rcvmode) {
				printf(MSGSTR(WHOAREYOU, "Who are you!?\n")); /*MSG*/
				exit(1);
			}
		}
		else
			(void)copy(uname, myname);
	}
	cp = value("HOME");
	if (cp == NOSTR)
		cp = ".";
	(void)copy(cp, homedir);
	
	if ((cp = value("MBOX")) == NOSTR) {
		cp = copy(homedir, mbox);
		(void)copy("/mbox", cp);
	} else {
		(void)copy(cp, mbox);
	}
	
	if ((cp = value("MAILRC")) == NOSTR) {
		cp = copy(homedir, mailrc);
		(void)copy("/.mailrc", cp);
	} else {
		(void)copy(cp, mailrc);
	}

	if ((cp = value("DEAD")) == NOSTR) {
		cp = copy(homedir, deadletter);
		(void)copy("/dead.letter", cp);
	} else {
		(void)copy(cp, deadletter);
	}
}
