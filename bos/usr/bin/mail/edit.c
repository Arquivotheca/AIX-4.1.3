static char sccsid[] = "@(#)53  1.12  src/bos/usr/bin/mail/edit.c, cmdmailx, bos41J, 9517A_all 4/21/95 16:49:56";
/* 
 * COMPONENT_NAME: CMDMAILX edit.c
 * 
 * FUNCTIONS: MSGSTR, edit1, editor, visual 
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
 * static char *sccsid = "edit.c       5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

#include "rcv.h"
#include <stdio.h>
#include <sys/stat.h>

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Perform message editing functions.
 */

/*
 * Edit a message list.
 */

editor(msgvec)
	int *msgvec;
{
	char *edname;

	if ((edname = value("EDITOR")) == NOSTR)
		edname = EDITOR;
	return(edit1(msgvec, edname));
}

/*
 * Invoke the visual editor on a message list.
 */

visual(msgvec)
	int *msgvec;
{
	char *edname;

	edname = value("VISUAL");
	if (edname == NOSTR || *edname == NULL) 
		edname = VISUAL;
	return(edit1(msgvec, edname));
}

/*
 * Edit a message by writing the message into a funnily-named file
 * (which should not exist) and forking an editor on it.
 * We get the editor from the stuff above.
 */
void (*sigint)(int), (*sigquit)(int);

edit1(msgvec, ed)
	int *msgvec;
	char *ed;
{
	register char *cp, *cp2;
	register int c;
	int *ip, pid, mesg, lines;
	long ms;
	FILE *ibuf, *obuf;
	char edname[40];
	struct message *mp;
	extern char tempEdit[];
	off_t fsize(), size;
	struct stat statb;
	long modtime;

	/*
	 * Set signals; locate editor.
	 */

	sigint = signal(SIGINT, SIG_IGN);
	sigquit = signal(SIGQUIT, SIG_IGN);

	/*
	 * Deal with each message to be edited . . .
	 */

	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		mp = &message[mesg-1];
		mp->m_flag |= MODIFY;

		/*
		 * Make up a name for the edit file of the
		 * form "/tmp/Messagen.pid" and make sure it doesn't
		 * already exist.
		 */

		sprintf(edname, "/tmp/Message%u.%x", mesg, getpid());
		if (!access(edname, 2)) {
			printf(MSGSTR(FEXISTS, "%s: file exists\n"), edname); /*MSG*/
			goto out;
		}

		/*
		 * Copy the message into the edit file.
		 */

		close(creat(edname, 0600));
		if ((obuf = Fopen(edname, "w")) == NULL) {
			perror(edname);
			goto out;
		}
		if (send(mp, obuf, 0, NOSTR) < 0) {
			perror(edname);
			Fclose(obuf);
			remove_file(edname);
			goto out;
		}
		fflush(obuf);
		if (ferror(obuf)) {
			remove_file(edname);
			Fclose(obuf);
			goto out;
		}
		Fclose(obuf);

		/*
		 * If we are in read only mode, make the
		 * temporary message file readonly as well.
		 */

		if (readonly)
			chmod(edname, 0400);

		/*
		 * Fork/execlp the editor on the edit file.
		 */

		if (stat(edname, &statb) < 0)
			modtime = 0;
		modtime = statb.st_mtime;
		pid = vfork();
		if (pid == -1) {
			perror("fork");
			remove_file(edname);
			goto out;
		}
		if (pid == 0) {
			sigchild();
			if (sigint != SIG_IGN)
				signal(SIGINT, SIG_DFL);
			if (sigquit != SIG_IGN)
				signal(SIGQUIT, SIG_DFL);
			execlp(ed, ed, edname, 0);
			perror(ed);
			_exit(1);
		}
		while (wait(&mesg) != pid)
			;

		/*
		 * If in read only mode, just remove the editor
		 * temporary and return.
		 */

		if (readonly) {
			remove_file(edname);
			continue;
		}

		/*
		 * Now copy the message to the end of the
		 * temp file if it was changed; else continue
		 * with the next message.
		 */

		if (stat(edname, &statb) < 0) {
			perror(edname);
			goto out;
		}
		if (modtime == statb.st_mtime) {  /* no changes made */
			remove_file(edname);
			continue;  /* with next message in list */
		}
		if ((ibuf = Fopen(edname, "r")) == NULL) {
			perror(edname);
			remove_file(edname);
			goto out;
		}
		remove_file(edname);
		fseek(otf, (long) 0, 2);
		size = fsize(otf);
		mp->m_block = blockof(size);
		mp->m_offset = offsetof(size);
		ms = 0L;
		lines = 0;
		while ((c = getc(ibuf)) != EOF) {
			if (c == '\n')
				lines++;
			putc(c, otf);
			if (ferror(otf))
				break;
			ms++;
		}
		mp->m_size = ms;
		mp->m_lines = lines;
		if (ferror(otf))
			perror("/tmp");
		Fclose(ibuf);
	}

	/*
	 * Restore signals and return.
	 */

out:
	signal(SIGINT, sigint);
	signal(SIGQUIT, sigquit);
}
