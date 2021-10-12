static char sccsid[] = "@(#)47        1.10  src/bos/usr/bin/mail/cmd2.c, cmdmailx, bos41J, 9515A_all 4/10/95 14:39:31";
/* 
 * COMPONENT_NAME: CMDMAILX cmd2.c
 * 
 * FUNCTIONS: MSGSTR, clob1, clobber, copycmd, Copycmd, core, delete, delm, 
 *            deltype, igcomp, igfield, igshow, next, retfield, 
 *            retshow, save, Savecmd, save1, snarf, swrite, undelete 
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
 * static char *sccsid = "cmd2.c       5.3 (Berkeley) 9/10/85";
 * #endif not lint
 */

#include "rcv.h"
#include <sys/stat.h>
#include <sys/errno.h>

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * More user commands.
 */

/*
 * If any arguments were given, go to the next applicable argument
 * following dot, otherwise, go to the next applicable message.
 * If given as first command with no arguments, print first message.
 */

next(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register int *ip, *ip2;
	int list[2], mdot;

	if (*msgvec != (int) NULL) {

		/*
		 * If some messages were supplied, find the 
		 * first applicable one following dot using
		 * wrap around.
		 */

		mdot = dot - &message[0] + 1;

		/*
		 * Find the first message in the supplied
		 * message list which follows dot.
		 */

		for (ip = msgvec; *ip != (int) NULL; ip++)
			if (*ip > mdot)
				break;
		if (*ip == (int)NULL)
			ip = msgvec;
		ip2 = ip;
		do {
			mp = &message[*ip2 - 1];
			if ((mp->m_flag & MDELETED) == 0) {
				dot = mp;
				goto hitit;
			}
			if (*ip2 != (int)NULL)
				ip2++;
			if (*ip2 == (int)NULL)
				ip2 = msgvec;
		} while (ip2 != ip);
		printf(MSGSTR(NOMSGS, "No messages applicable\n")); /*MSG*/
		return(1);
	}

	/*
	 * If this is the first command, select message 1.
	 * Note that this must exist for us to get here at all.
	 */

	if (!sawcom)
		goto hitit;

	/*
	 * Just find the next good message after dot, no
	 * wraparound.
	 */

	if (sawdel && (value("autoprint") == NOSTR)) 
		dot--; 
	sawdel = 0;
	for (mp = dot+1; mp < &message[msgCount]; mp++)
		if ((mp->m_flag & (MDELETED|MSAVED)) == 0)
			break;
	if (mp >= &message[msgCount]) {
		printf(MSGSTR(ATEOF, "At EOF\n")); /*MSG*/
		return(0);
	}
	dot = mp;
hitit:
	/*
	 * Print dot.
	 */

	list[0] = dot - &message[0] + 1;
	list[1] = (int)NULL;
	return(type(list));
}

/*
 * Save a message in a file.  Mark the message as saved
 * so we can discard when the user quits.
 */
save(str)
	char str[];
{

	return(save1(str, 1, 0));
}

/*
 * Copy a message to a file without affected its saved-ness
 */
copycmd(str)
	char str[];
{

	return(save1(str, 0, 0));
}

/*
 * Save a message in a file named after the author of the message to be
 * saved. Mark the message as saved so we can discard when the user quits.
 */
Savecmd(str)
	char str[];
{
	return(save1(str, 1, 1));
}

/*
 * Copy a message to a file named after the author of the message to be
 * saved without affecting its saved-ness.
 */
Copycmd(str)
	char str[];
{
	return(save1(str, 0, 1));
}

/*
 * Save/copy the indicated messages at the end of the passed file name.
 * If mark is true, mark the message "saved."
 */
save1(str, mark, derive_file)
	char str[];
	int mark, derive_file;
{
	register int *ip, mesg;
	register struct message *mp;
	char *file, *disp, *filename;
	int f, *msgvec, lc, t;
	long cc;
	FILE *obuf;
	struct stat statb;
	int filestat;

	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if (derive_file) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == (int)NULL) {
			if (mark)
			    printf(MSGSTR(NOSAVE, "No messages to Save.\n")); /*MSG*/
			else
			    printf(MSGSTR(NOCOPY, "No messages to Copy.\n")); /*MSG*/
			return(1);
		}
		msgvec[1] = (int)NULL;

		if (*str != NOSTR)
			if (ordgetmsglist(str, msgvec, 0) < 0)
				return(1);

		mp = &message[*msgvec - 1];
		filename = skin(hfield("from", mp));
		if ((file = (char *)malloc(strlen(filename) + strlen(homedir) + 1)) == NULL)
			panic(MSGSTR(MEMERR, "Internal error, malloc() failed")); /*MSG*/
		sprintf(file, "%s/%s", homedir, filename);
	}
	else {
		if ((file = snarf(str, &f)) == NOSTR)
			return(1);
		if (!f) {
			*msgvec = first(0, MMNORM);
			if (*msgvec == (int)NULL) {
				if (mark)
				    printf(MSGSTR(NOSAVE, "No messages to save.\n")); /*MSG*/
				else
				    printf(MSGSTR(NOCOPY, "No messages to copy.\n")); /*MSG*/
				return(1);
			}
			msgvec[1] = (int)NULL;
		}
		if (f && getmsglist(str, msgvec, 0) < 0)
			return(1);
	}

	if ((file = expand(file)) == NOSTR)
		return(1);
	printf("\"%s\" ", file);
	fflush(stdout);
	filestat = stat(file, &statb);
	if ((obuf = Fopen(file, "a")) == NULL) {
		perror(NOSTR);
		return(1);
	}
	cc = 0L;
	lc = 0;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		if ((t = send(mp, obuf, 0, NOSTR)) < 0) {
			perror(file);
			Fclose(obuf);
			return(1);
		}
		lc += t;
		cc += mp->m_size;
		if (mark)
			mp->m_flag |= MSAVED;
	}
	fflush(obuf);
	if (ferror(obuf))
		perror(file);
	errno = 0;
	Fclose(obuf);
	if(errno){
		perror(" ");
		return(1);
	}
	if (filestat >= 0)
		disp = MSGSTR(APP, "[Appended]"); /*MSG*/
	else
		disp = MSGSTR(NEW, "[New file]"); /*MSG*/
	printf("%s %d/%ld\n", disp, lc, cc);
	return(0);
}

/*
 * Write the indicated messages at the end of the passed
 * file name, minus header and trailing blank line.
 */

swrite(str)
	char str[];
{
	register int *ip, mesg;
	register struct message *mp;
	register char *file, *disp;
	char linebuf[BUFSIZ];
	int f, *msgvec, lc, cc, t;
	FILE *obuf, *mesf;
	struct stat statb;

	msgvec = (int *) salloc((msgCount + 2) * sizeof *msgvec);
	if ((file = snarf(str, &f)) == NOSTR)
		return(1);
	if ((file = expand(file)) == NOSTR)
		return(1);
	if (!f) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == (int)NULL) {
			printf(MSGSTR(NOWRT, "No messages to write.\n")); /*MSG*/
			return(1);
		}
		msgvec[1] = (int)NULL;
	}
	if (f && getmsglist(str, msgvec, 0) < 0)
		return(1);
	printf("\"%s\" ", file);
	fflush(stdout);
	if (stat(file, &statb) >= 0)
		disp = MSGSTR(APP, "[Appended]"); /*MSG*/
	else
		disp = MSGSTR(NEW, "[New file]"); /*MSG*/
	if ((obuf = Fopen(file, "a")) == NULL) {
		perror(NOSTR);
		return(1);
	}
	cc = lc = 0;
	for (ip = msgvec; *ip && ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		mesf = setinput(mp);
		t = mp->m_lines - 1;
		while (t-- > 0) {
			readline(mesf, linebuf);
			if (blankline(linebuf))
				break;
		}
		while (t-- > 0) {
			fgets(linebuf, BUFSIZ, mesf);
			fputs(linebuf, obuf);
			cc += strlen(linebuf);
		}
		lc += mp->m_lines - 2;
		mp->m_flag |= MSAVED;
	}
	fflush(obuf);
	if (ferror(obuf))
		perror(file);
	errno = 0;
	Fclose(obuf);
	if(errno){
		perror(" ");
		return(1);
	}
	printf("%s %d/%d\n", disp, lc, cc);
	return(0);
}

/*
 * Snarf the file from the end of the command line and
 * return a pointer to it.  If there is no file attached,
 * assume the message should be saved in the mbox.  (XPG4)
 * If mbox is null, print a message and return NOSTR.
 *
 * Put a null in front of the file name so that
 * the message list processing won't see it,
 * unless the file name is the only thing on the line, in
 * which case, return 0 in the reference flag variable.
 */

char *
snarf(linebuf, flag)
	char linebuf[];
	int *flag;
{
	register char *cp;

	*flag = 1;
	cp = strlen(linebuf) + linebuf - 1;

	/*
	 * Strip away trailing blanks.
	 */

	while (*cp == ' ' && cp > linebuf)
		cp--;
	*++cp = 0;

	/*
	 * Now search for the beginning of the file name.
	 */

	while (cp > linebuf && !any(*cp, "\t "))
		cp--;
	if (*cp == '\0') {
		if (mbox == '\0') {
			printf(MSGSTR(NOFILESP, "No file specified.\n")); /*MSG*/
			return(NOSTR);
		} else 
			cp = mbox;
	}
	if (any(*cp, " \t"))
		*cp++ = 0;
	else
		*flag = 0;
	return(cp);
}

/*
 * Delete messages.
 */

delete(msgvec)
	int msgvec[];
{
	sawdel = 1;
	return(delm(msgvec));
}

/*
 * Delete messages, then type the new dot.
 */

deltype(msgvec)
	int msgvec[];
{
	int list[2];
	int lastdot;

	lastdot = dot - &message[0] + 1;
	if (delm(msgvec) >= 0) {
		list[0] = dot - &message[0];
		list[0]++;
		if (list[0] > lastdot) {
			touch(list[0]);
			list[1] = (int)NULL;
			return(type(list));
		}
		printf(MSGSTR(ATEOF, "At EOF\n")); /*MSG*/
		return(0);
	}
	else {
		printf(MSGSTR(NOMORE, "No more messages\n")); /*MSG*/
		return(0);
	}
}

/*
 * Delete the indicated messages.
 * Set dot to some nice place afterwards.
 * Internal interface.
 */

delm(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register *ip, mesg;
	int last;

	last = (int)NULL;
	for (ip = msgvec; *ip != (int)NULL; ip++) {
		mesg = *ip;
		touch(mesg);
		mp = &message[mesg-1];
		mp->m_flag |= MDELETED|MTOUCH;
		mp->m_flag &= ~(MPRESERVE|MSAVED|MBOX);
		last = mesg;
	}
	if (last != (int)NULL) {
		dot = &message[last-1];
		last = first(0, MDELETED);
		if (last != (int)NULL) {
			dot = &message[last-1];
			return(0);
		}
		else {
			dot = &message[0];
			return(-1);
		}
	}

	/*
	 * Following can't happen -- it keeps lint happy
	 */

	return(-1);
}

/*
 * Undelete the indicated messages.
 */

undelete(msgvec)
	int *msgvec;
{
	register struct message *mp;
	register *ip, mesg;

	for (ip = msgvec; ip-msgvec < msgCount; ip++) {
		mesg = *ip;
		if (mesg == 0)
			return;
		touch(mesg);
		mp = &message[mesg-1];
		dot = mp;
		mp->m_flag &= ~MDELETED;
	}
}

/*
 * Interactively dump core on "core"
 */

core()
{
	register int pid;
	int status;

	if ((pid = vfork()) == -1) {
		perror("fork");
		return(1);
	}
	if (pid == 0) {
		sigchild();
		abort();
		_exit(1);
	}
	printf(MSGSTR(OK, "Okie dokie")); /*MSG*/
	fflush(stdout);
	while (wait(&status) != pid)
		;
	if (status & 0200)
		printf(MSGSTR(CORE, " -- Core dumped\n")); /*MSG*/
	else
		printf("\n");
}

/*
 * Clobber as many bytes of stack as the user requests.
 */
clobber(argv)
	char **argv;
{
	register int times;

	if (argv[0] == 0)
		times = 1;
	else
		times = (atoi(argv[0]) + 511) / 512;
	clob1(times);
}

/*
 * Clobber the stack.
 */
clob1(n)
{
	char buf[512];
	register char *cp;

	if (n <= 0)
		return;
	for (cp = buf; cp < &buf[512]; *cp++ = 0xFF)
		;
	clob1(n - 1);
}

/*
 * Add the given header fields to the retained list.
 * If no arguments, print the current list of retained fields.
 */
retfield(list)
	char *list[];
{
	char field[BUFSIZ];
	register int h;
	register struct ignore *igp;
	char **ap;

	if (argcount(list) == 0)
		return(retshow());
	for (ap = list; *ap != 0; ap++) {
		istrcpy(field, *ap);

		if (member(field, retain))
			continue;

		h = hash(field);
		if ((igp = (struct ignore *) calloc(1, sizeof (struct ignore))) == NULL)
		     panic(MSGSTR(MEMERR2, "Internal error, calloc() failed"));
		if ((igp->i_field = calloc(strlen(field) + 1, sizeof (char))) == NULL)
		     panic(MSGSTR(MEMERR2, "Internal error, calloc() failed"));
		strcpy(igp->i_field, field);
		igp->i_link = retain[h];
		retain[h] = igp;
		nretained++;
	}
	return(0);
}

/*
 * Print out all currently retained fields.
 */
retshow()
{
	register int h, count;
	struct ignore *igp;
	char **ap, **ring;
	int igcomp();

	count = 0;
	for (h = 0; h < HSHSIZE; h++)
		for (igp = retain[h]; igp != 0; igp = igp->i_link)
			count++;
	if (count == 0) {
		printf(MSGSTR(NOFIELDS, "No fields currently being retained.\n")); /*MSG*/
		return(0);
	}
	ring = (char **) salloc((count + 1) * sizeof (char *));
	ap = ring;
	for (h = 0; h < HSHSIZE; h++)
		for (igp = retain[h]; igp != 0; igp = igp->i_link)
			*ap++ = igp->i_field;
	*ap = 0;
	qsort(ring, count, sizeof (char *), igcomp);
	for (ap = ring; *ap != 0; ap++)
		printf("%s\n", *ap);
	return(0);
}

/*
 * Add the given header fields to the ignored list.
 * If no arguments, print the current list of ignored fields.
 */
igfield(list)
	char *list[];
{
	char field[BUFSIZ];
	register int h;
	register struct ignore *igp;
	char **ap;

	if (argcount(list) == 0)
		return(igshow());
	for (ap = list; *ap != 0; ap++) {
		if (isign(*ap))
			continue;
		istrcpy(field, *ap);
		h = hash(field);
		if ((igp = (struct ignore *) calloc(1, sizeof (struct ignore))) == NULL)
		     panic(MSGSTR(MEMERR2, "Internal error, calloc() failed"));
		if ((igp->i_field = calloc(strlen(field) + 1, sizeof (char))) == NULL)
		     panic(MSGSTR(MEMERR2, "Internal error, calloc() failed"));
		strcpy(igp->i_field, field);
		igp->i_link = ignore[h];
		ignore[h] = igp;
	}
	return(0);
}

/*
 * Print out all currently ignored fields.
 */
igshow()
{
	register int h, count;
	struct ignore *igp;
	char **ap, **ring;
	int igcomp();

	count = 0;
	for (h = 0; h < HSHSIZE; h++)
		for (igp = ignore[h]; igp != 0; igp = igp->i_link)
			count++;
	if (count == 0) {
		printf(MSGSTR(NOIGNORE, "No fields currently being ignored.\n")); /*MSG*/
		return(0);
	}
	ring = (char **) salloc((count + 1) * sizeof (char *));
	ap = ring;
	for (h = 0; h < HSHSIZE; h++)
		for (igp = ignore[h]; igp != 0; igp = igp->i_link)
			*ap++ = igp->i_field;
	*ap = 0;
	qsort(ring, count, sizeof (char *), igcomp);
	for (ap = ring; *ap != 0; ap++)
		printf("%s\n", *ap);
	return(0);
}

/*
 * Compare two names for sorting ignored field list.
 */
igcomp(l, r)
	char **l, **r;
{

	return(strcmp(*l, *r));
}
