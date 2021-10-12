static char sccsid[] = "@(#)75  1.14  src/bos/usr/bin/mail/tty.c, cmdmailx, bos411, 9428A410j 11/17/93 10:41:34";
/* 
 * COMPONENT_NAME: CMDMAILX tty.c
 * 
 * FUNCTIONS: MSGSTR, grabh, readtty, signull, ttycont, ttystop
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
 * static char *sccsid = "tty.c        5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

/*
 * Mail -- a mail program
 *
 * Generally useful tty stuff.
 */

#include "rcv.h"
#include <sys/syspest.h>

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

static	int	c_erase;		/* Current erase char */
static	int	c_kill;			/* Current kill char */
static  jmp_buf	rewrite;			/* Place to go when continued */
static  int	hadcont;		/* Saw continue signal */
#ifndef TIOCSTI
static	int	ttyset;			/* We must now do erase/kill */
static	int	c_lflag;		/* current line modes (SysV style) */
static	int	c_line;			/* Current line value */
static	int	eof, eol;		/* used as MIN and TIME when !ICANON */
#endif

void ttystop(int);
void ttycont(int);
void signull(int);

BUGXDEF(mailbug)			/* define debug var */

/*
 * Read all relevant header fields.
 */

grabh(hp, gflags)
	struct header *hp;
{
	struct sgttyb ttybuf;
	void (*saveint)(int);
#ifndef TIOCSTI
	void (*savequit)(int);
#endif
	void (*savecont)(int);
	void (*savetstp)(int);
	void (*savettou)(int);
	void (*savettin)(int);
	int errs;

	savecont = signal(SIGCONT, (void(*)(int))signull);
	savetstp = signal(SIGTSTP, SIG_DFL);
	savettou = signal(SIGTTOU, SIG_DFL);
	savettin = signal(SIGTTIN, SIG_DFL);
	errs = 0;
#ifndef TIOCSTI
	ttyset = 0;
#endif
	if (gtty(fileno(stdin), &ttybuf) < 0) {
		perror("gtty");
		return(-1);
	}
	c_erase = ttybuf.sg_erase;
	c_kill = ttybuf.sg_kill;
#ifndef TIOCSTI
	c_lflag = ttybuf.c_lflag;
	c_line = ttybuf.c_line;
	eof = ttybuf.c_cc[4];
	eol = ttybuf.c_cc[5];
	ttybuf.sg_erase = ttybuf.sg_kill = 0;
	ttybuf.c_lflag &= ~ICANON;
	ttybuf.c_line = 0;
	ttybuf.c_cc[4] = ttybuf.c_cc[5] = 1;
	if ((saveint = signal(SIGINT, SIG_IGN)) == SIG_DFL)
		signal(SIGINT, SIG_DFL);
	if ((savequit = signal(SIGQUIT, SIG_IGN)) == SIG_DFL)
		signal(SIGQUIT, SIG_DFL);
#endif

	if (gflags & GTO) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_to != NOSTR */ )
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_to = readtty("To: ", hp->h_to);
		if (hp->h_to != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GSUBJECT) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_subject != NOSTR */ )
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_subject = readtty("Subject: ", hp->h_subject);
		if (hp->h_subject != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GCC) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_cc != NOSTR */)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_cc = readtty("Cc: ", hp->h_cc);
		if (hp->h_cc != NOSTR)
			hp->h_seq++;
	}
	if (gflags & GBCC) {
#ifndef TIOCSTI
		if (!ttyset /* && hp->h_bcc != NOSTR */)
			ttyset++, stty(fileno(stdin), &ttybuf);
#endif
		hp->h_bcc = readtty("Bcc: ", hp->h_bcc);
		if (hp->h_bcc != NOSTR)
			hp->h_seq++;
	}
out:
	signal(SIGTSTP, (void(*)(int))savetstp);
	signal(SIGTTOU, (void(*)(int))savettou);
	signal(SIGTTIN, (void(*)(int))savettin);
	signal(SIGCONT, (void(*)(int))savecont);
#ifndef TIOCSTI
	ttybuf.sg_erase = c_erase;
	ttybuf.sg_kill = c_kill;
	ttybuf.c_lflag = c_lflag;
	ttybuf.c_line = c_line;
	ttybuf.c_cc[4] = eof;
	ttybuf.c_cc[5] = eol;
	if (ttyset)
		stty(fileno(stdin), &ttybuf);
	signal(SIGQUIT, (void(*)(int))savequit);
	signal(SIGINT, (void(*)int))saveint);
#endif
	return(errs);
}

/*
 * Read up a header from standard input.
 * The source string has the preliminary contents to
 * be read.
 *
 */

char *
readtty(pr, src)
	char *pr, *src;
{
	char ch, canonb[BUFSIZ];
	int c;
	register char *cp, *cp2;

	BUGLPR(mailbug, BUGACT, ("readtty: pr='%s', src='%s'\n", pr, src));
	fputs(pr, stdout);
	fflush(stdout);
	if (src != NOSTR && strlen(src) > BUFSIZ - 2) {
		printf(MSGSTR(TOOLONG, "too long to edit\n")); /*MSG*/
		return(src);
	}
#ifndef TIOCSTI
	if (src != NOSTR)
		cp = copy(src, canonb);
	else
		cp = copy("", canonb);
	fputs(canonb, stdout);
	fflush(stdout);
#else
	cp = src == NOSTR ? "" : src;
	while (c = *cp++) {
		if (c == c_erase || c == c_kill) {
			ch = '\\';
			ioctl(0, TIOCSTI, &ch);
		}
		ch = c;
		ioctl(0, TIOCSTI, &ch);
	}
	cp = canonb;
	*cp = 0;
#endif
	cp2 = cp;
	while (cp2 < canonb + BUFSIZ)
		*cp2++ = 0;
	cp2 = cp;

	/* set up the context to restart on signals */
	if (setjmp(rewrite)) {
		if (src)  /* save the original string for the retry */
		    strcpy(canonb, src);
		goto redo;
	}
	signal(SIGTSTP, (void(*)(int))ttystop);
	signal(SIGTTOU, (void(*)(int))ttystop);
	signal(SIGTTIN, (void(*)(int))ttystop);
	signal(SIGCONT, (void(*)(int))ttycont);
	clearerr(stdin);
	while (cp2 < canonb + BUFSIZ) {
		c = getc(stdin);
		if (c == EOF || c == '\n')
			break;
		*cp2++ = c;
	}
	*cp2 = 0;
	signal(SIGCONT, (void(*)(int))signull);
	if (c == EOF && ferror(stdin) && hadcont) {
redo:
		hadcont = 0;
		cp = strlen(canonb) > 0 ? canonb : NOSTR;
		BUGLPR(mailbug, BUGACT,
		    ("readtty before recurse: canonb='%s', cp='%s'\n",
		    canonb, cp ? cp : "<NULL>"));
		clearerr(stdin);
		return(readtty(pr, cp));
	}
#ifndef TIOCSTI
	if (cp == NOSTR || *cp == '\0')
		return(src);
	cp2 = cp;
	if (!ttyset)
		return(strlen(canonb) > 0 ? savestr(canonb) : NOSTR);
	while (*cp != '\0') {
		c = *cp++;
		if (c == c_erase) {
			if (cp2 == canonb) 
				continue;
			if (cp2[-1] == '\\') {
				cp2[-1] = c;
				continue;
			}
			cp2--;
			continue;
		}
		if (c == c_kill) {
			if (cp2 == canonb)
				continue; 
			if (cp2[-1] == '\\') {
				cp2[-1] = c;
				continue;
			}
			cp2 = canonb;
			continue;
		}
		*cp2++ = c;

	}
	*cp2 = '\0';
#endif TIOCSTI
	if (equal("", canonb))
		return(NOSTR);
	return(savestr(canonb));
}

/*
 * Receipt continuation.
 */
void
ttycont(int s)
{

	hadcont++;
	longjmp(rewrite, 1);
}

/*
 * Null routine to satisfy
 * silly system bug that denies us holding SIGCONT
 */
void
signull(int s)
{}

void
ttystop(int s)
{
	void (*old_action)(int);

	old_action = signal(s, SIG_DFL);
	
	sigsetmask(sigblock(0) & ~sigmask(s));
	kill(0, s);
	sigblock(sigmask(s));
	signal(s, old_action);
	longjmp(rewrite, 1);
}

	  
