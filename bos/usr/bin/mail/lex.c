static char sccsid[] = "@(#)59  1.17  src/bos/usr/bin/mail/lex.c, cmdmailx, bos41J, 9523C_all 6/8/95 17:12:41";
/* 
 * COMPONENT_NAME: CMDMAILX lex.c
 * 
 * FUNCTIONS: MSGSTR, announce, commands, contin, execute, 
 *            hangup, isprefix, lex, mail_load, newfileinfo, pversion, 
 *            setfile, setmsize, stop, strace
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
 * static char *sccsid = "lex.c        5.4 (Berkeley) 11/2/85";
 * #endif not lint
 */

#include "rcv.h"
#include <sys/stat.h>
#include <errno.h>

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Lexical processing of commands.
 */

#define MAXPROMPT	80
char	prompt[MAXPROMPT+1];
char	*def_prompt = "? ";

/*
 * Set up editing on the given file name.
 * If isedit is true, we are considered to be editing the file,
 * otherwise we are reading our mail which has signficance for
 * mbox and so forth.
 */

setfile(name, isedit)
	char *name;
{
	FILE *ibuf;
	int i;
	struct stat stb;
	static int shudclob;
	static char efile[128];
	extern char tempMesg[];
	extern int errno;

	if ((ibuf = Fopen(name, "r")) == NULL) {
		if (!isedit && errno == ENOENT)
			goto nomail;
		perror(name);
		return(-1);
	}

	if (fstat(fileno(ibuf), &stb) < 0) {
		perror("fstat");
		Fclose(ibuf);
		return (-1);
	}

	switch (stb.st_mode & S_IFMT) {
	case S_IFDIR:
		Fclose(ibuf);
		errno = EISDIR;
		perror(name);
		return (-1);

	case S_IFREG:
		break;

	default:
		Fclose(ibuf);
		errno = EINVAL;
		perror(name);
		return (-1);
	}

	if (!isedit && stb.st_size == 0) {
		Fclose(ibuf);
nomail:
		if (mail_test == FALSE) {
			fprintf(stderr, MSGSTR(NOFORYOU, "No mail for %s\n"), myname);
		}
		return(-1);
	}

	/*
	 * Looks like all will be well.  We must now relinquish our
	 * hold on the current set of stuff.  Must hold signals
	 * while we are reading the new file, else we will ruin
	 * the message[] data structure.
	 */

	holdsigs();
	if (shudclob) {
		if (isedit)
			edstop();
		else
			quit();
	}

	/*
	 * Copy the messages into /tmp
	 * and set pointers.
	 */

	readonly = 0;
	if ((i = open(name, 1)) < 0)
		readonly++;
	else
		close(i);
	if (shudclob) {
		fclose(itf);
		fclose(otf);
	}
	shudclob = 1;
	edit = isedit;
	strncpy(efile, name, 128);
	editfile = efile;
	if (name != mailname)
		strcpy(mailname, name);

	/*
	 * Need to lock here, to ensure the mail delivery agent doesn't
	 * write between fsize() and setptr()
	 */
	lockf(fileno(ibuf), F_LOCK, 0);
	mailsize = fsize(ibuf);
	if ((otf = fopen(tempMesg, "w")) == NULL) {
		perror(tempMesg);
		exit(1);
	}
	if ((itf = fopen(tempMesg, "r")) == NULL) {
		perror(tempMesg);
		exit(1);
	}
	remove(tempMesg);
	setptr(ibuf);
	setmsize(msgCount);
	Fclose(ibuf);
	relsesigs();
	sawcom = 0;
	return(0);
}

/*
 * Interpret user commands one by one.  If standard input is not a tty,
 * print no prompt.
 */

int	*msgvec;

commands()
{
	int eofloop, shudprompt;
	void stop(int);
	register int n;
	char *pptr;
	char linebuf[LINESIZE];
	void hangup(int), contin(int);

	signal(SIGCONT, SIG_DFL);
	if (rcvmode && !sourcing) {
		if (signal(SIGINT, SIG_IGN) != SIG_IGN)
			signal(SIGINT, (void(*)(int))stop);
		if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
			signal(SIGHUP, (void(*)(int))hangup);
	}
	for (;;) {
		shudprompt = intty && !sourcing;
		if ((pptr = value("prompt")) != NOSTR)
			strncpy(prompt, pptr, MAXPROMPT);
		else
                {
                        if (value("noprompt") == NOSTR)
                                strcpy(prompt, def_prompt);
                        else
                                strcpy(prompt, "");
                }

		setexit();

		/*
		 * Print the prompt, if needed.  Clear out
		 * string space, and flush the output.
		 */

		if (!rcvmode && !sourcing)
			return;
		eofloop = 0;
top:
		if (shudprompt) {
			printf(prompt);
			fflush(stdout);
			signal(SIGCONT, (void(*)(int))contin);
		} else
			fflush(stdout);
		sreset();

		/*
		 * Read a line of commands from the current input
		 * and handle end of file specially.
		 */

		n = 0;
		for (;;) {
			if (readline(input, &linebuf[n]) <= 0) {
				if (n != 0)
					break;
				if (loading)
					return;
				if (sourcing) {
					unstack();
					goto more;
				}
				if (value("ignoreeof") != NOSTR && shudprompt) {
					if (++eofloop < 25) {
						printf(MSGSTR(QUIT, "Use \"quit\" to quit.\n")); /*MSG*/
						goto top;
					}
				}
				if (edit)
					edstop();
				return;
			}
			if ((n = strlen(linebuf)) == 0)
				break;
			n--;
			if (linebuf[n] != '\\')
				break;
			linebuf[n++] = ' ';
		}
		signal(SIGCONT, SIG_DFL);
		if (execute(linebuf, 0))
			return;
more:		;
	}
}

/*
 * Execute a single command.  If the command executed
 * is "quit," then return non-zero so that the caller
 * will know to return back to main, if he cares.
 * Contxt is non-zero if called while composing mail.
 */

execute(linebuf, contxt)
	char linebuf[];
{
	char word[LINESIZE];
	char *arglist[MAXARGC];
	struct cmd *com;
	register char *cp, *cp2;
	register int c;
	int muvec[2];
	int edstop(), e;

	/*
	 * Strip the white space away from the beginning
	 * of the command, then scan out a word, which
	 * consists of anything except digits and white space.
	 *
	 * Handle ! escapes differently to get the correct
	 * lexical conventions.
	 */

	cp = linebuf;
	while (any(*cp, " \t"))
		cp++;
	if (*cp == '!') {
		if (sourcing) {
			fprintf(stderr, MSGSTR(NOTNOW, "Can't \"!\" while sourcing\n")); /*MSG*/
			unstack();
			return(0);
		}
		shell(cp+1);
		return(0);
	}
	cp2 = word;
	while (*cp && !any(*cp, " \t0123456789$^.:/-+*'\""))
		*cp2++ = *cp++;
	*cp2 = '\0';

	/*
	 * Look up the command; if not found, print a msg.
	 * If sourcing, ignore blank lines.          
	 * If command is null, set the command to 'print'
	 * as specified by XPG4.                       
	 */

	if (sourcing && equal(word, ""))
		return(0);
	if (equal(word, ""))
	   strcpy(word,"print");

	com = lex(word);
	if (com == NONE) {
		printf(MSGSTR(UNKNOWN, "Unknown command: \"%s\"\n"), word); /*MSG*/
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}

	/*
	 * See if we should execute the command -- if a conditional
	 * we always execute it, otherwise, check the state of cond.
	 */

	if ((com->c_argtype & F) == 0)
		if (cond == CRCV && !rcvmode || cond == CSEND && rcvmode)
			return(0);

	/*
	 * Special case so that quit causes a return to
	 * main, who will call the quit code directly.
	 * If we are in a source file, just unstack.
	 */

	if (com->c_func == edstop && sourcing) {
		if (loading)
			return(1);
		unstack();
		return(0);
	}
	if (!edit && com->c_func == edstop) {
		signal(SIGINT, SIG_IGN);
		return(1);
	}

	/*
	 * Process the arguments to the command, depending
	 * on the type he expects.  Default to an error.
	 * If we are sourcing an interactive command, it's
	 * an error.
	 */

	if (!rcvmode && (com->c_argtype & M) == 0) {
		fprintf(stderr, MSGSTR(NOEXEC, "May not execute \"%s\" while sending\n"), com->c_name); /*MSG*/
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}
	if (sourcing && com->c_argtype & I) {
		fprintf(stderr, MSGSTR(NOEXECS, "May not execute \"%s\" while sourcing\n"), com->c_name); /*MSG*/
		if (loading)
			return(1);
		unstack();
		return(0);
	}
	if (readonly && com->c_argtype & W) {
		printf(MSGSTR(READONLY, "May not execute \"%s\" -- message file is read only\n"), com->c_name); /*MSG*/
		if (loading)
			return(1);
		if (sourcing)
			unstack();
		return(0);
	}
	if (contxt && com->c_argtype & R) {
		printf(MSGSTR(NORECRSV, "Cannot recursively invoke \"%s\"\n"), com->c_name); /*MSG*/
		return(0);
	}
	e = 1;
	switch (com->c_argtype & ~(F|P|I|M|T|W|R)) {
	case MSGLIST:
		/*
		 * A message list defaulting to nearest forward
		 * legal message.
		 */
		if (msgvec == 0) {
			printf(MSGSTR(ILLLIST, "Illegal use of \"message list\"\n")); /*MSG*/
			return(-1);
		}
		if (strcmp(com->c_name, "Followup") == 0) {
			if (*cp != NOSTR)
				if ((c = ordgetmsglist(cp, msgvec, 
							com->c_msgflag)) < 0)
					break;
		} else
			if ((c = getmsglist(cp, msgvec, com->c_msgflag)) < 0)
				break;
		if (c  == 0) {
			*msgvec = first(com->c_msgflag, com->c_msgmask);
			msgvec[1] = (int)NULL;
		}
		if (*msgvec == (int)NULL) {
			printf(MSGSTR(NOAPPLY, "No applicable messages\n")); /*MSG*/
			break;
		}
		e = (*com->c_func)(msgvec);
		break;

	case NDMLIST:
		/*
		 * A message list with no defaults, but no error
		 * if none exist.
		 */
		if (msgvec == 0) {
			printf(MSGSTR(ILLLIST, "Illegal use of \"message list\"\n")); /*MSG*/
			return(-1);
		}
		if (getmsglist(cp, msgvec, com->c_msgflag) < 0)
			break;
		e = (*com->c_func)(msgvec);
		break;

	case STRLIST:
		/*
		 * Just the straight string, with
		 * leading blanks removed.
		 */
		while (any(*cp, " \t"))
			cp++;
		e = (*com->c_func)(cp);
		break;

	case RAWLIST:
		/*
		 * A vector of strings, in shell style.
		 */
		if ((c = getrawlist(cp, arglist,
				sizeof arglist / sizeof *arglist)) < 0)
			break;
		if (c < com->c_minargs) {
			printf(MSGSTR(NEEDARG, "%s requires at least %d arg(s)\n"), com->c_name, com->c_minargs); /*MSG*/
			break;
		}
		if (c > com->c_maxargs) {
			printf(MSGSTR(TOOMANY, "%s takes no more than %d arg(s)\n"), com->c_name, com->c_maxargs); /*MSG*/
			break;
		}
		e = (*com->c_func)(arglist);
		break;

	case NOLIST:
		/*
		 * Just the constant zero, for exiting,
		 * eg.
		 */
		e = (*com->c_func)(0);
		break;

	default:
		panic(MSGSTR(NOARGTYP, "Unknown argtype")); /*MSG*/
	}

	/*
	 * Exit the current source file on
	 * error.
	 */

	if (e && loading)
		return(1);
	if (e && sourcing)
		unstack();
	if (com->c_func == edstop)
		return(1);
	if (value("autoprint") != NOSTR && com->c_argtype & P)
		if ((dot->m_flag & MDELETED) == 0) {
			muvec[0] = dot - &message[0] + 1;
			muvec[1] = 0;
			type(muvec);
		}
	if (!sourcing && (com->c_argtype & T) == 0)
		sawcom = 1;
	return(0);
}

/*
 * When we wake up after ^Z, reprint the prompt.
 */
void
contin(int s)
{

	printf(prompt);
	fflush(stdout);
}

/*
 * Branch here on hangup signal and simulate quit.
 */
void
hangup(int s)
{

	holdsigs();
	if (edit) {
		if (setexit())
			exit(0);
		edstop();
	}
	else
		quit();
	exit(0);
}

/*
 * Set the size of the message vector used to construct argument
 * lists to message list functions.
 */
 
setmsize(sz)
{

	if (msgvec != (int *) 0)
		cfree(msgvec);
	if ((msgvec = (int *) calloc((unsigned) (sz + 1), sizeof *msgvec)) == NULL)
		panic(MSGSTR(MEMERR2, "Internal error, calloc() failed"));
}

/*
 * Find the correct command in the command table corresponding
 * to the passed command "word"
 */

struct cmd *
lex(word)
	char word[];
{
	register struct cmd *cp;
	extern struct cmd cmdtab[];

	for (cp = &cmdtab[0]; cp->c_name != NOSTR; cp++)
		/*
		 * mail's comment character, "#", is not understood
		 * unless you have a space between the character and 
		 * the comment. Since it is common to not leave a space
		 * between the comment and the comment character, we 
		 * need to do a little hacking.
		 */
		if ((*cp->c_name == '#' && *word == '#') 
			|| (isprefix(word, cp->c_name)))
			return(cp);
	return(NONE);
}

/*
 * Determine if as1 is a valid prefix of as2.
 * Return true if yep.
 */

isprefix(as1, as2)
	char *as1, *as2;
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ == *s2)
		if (*s2++ == '\0')
			return(1);
	return(*--s1 == '\0');
}

/*
 * The following get called on receipt of a rubout. This is
 * to abort printout of a command, mainly.
 * Dispatching here when command() is inactive crashes rcv.
 * Close all open files except 0, 1, 2, the message catalog file,
 * and the temporary files.
 * The special call to getuserid() is needed so it won't get
 * annoyed about losing its open file.
 * Also, unstack all source files.
 */

int	inithdr;			/* am printing startup headers */

void
stop(int s)
{
	register FILE *fp;
	void stop(int);

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, (void(*)(int))stop);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, (void(*)(int))hangup);

	noreset = 0;
	if (!inithdr)
		sawcom++;
	inithdr = 0;
	while (sourcing)
		unstack();
	getuserid((char *) -1);

	close_all_files();

	if (image >= 0) {
		close(image);
		image = -1;
	}
	fprintf(stderr, MSGSTR(INTRPT, "Interrupt\n")); /*MSG*/
	reset(0);
}

/*
 * Announce the presence of the current Mail version,
 * give the message count, and print a header listing.
 */

char	*greeting	= "Mail %s  Type ? for help.\n";

announce(pr)
{
	int vec[2], mdot;
	extern char *version;

	if (pr && value("quiet") == NOSTR)
		printf(MSGSTR(GREET, greeting), version); /*MSG*/
	mdot = newfileinfo();
	vec[0] = mdot;
	vec[1] = 0;
	dot = &message[mdot - 1];
	if (msgCount > 0 && !noheader) {
		inithdr++;
		headers(vec);
		inithdr = 0;
	}
}

/*
 * Announce information about the file we are editing.
 * Return a likely place to set dot.
 */
newfileinfo()
{
	register struct message *mp;
	register int u, n, mdot, d, s;
	char fname[BUFSIZ], zname[BUFSIZ], *ename;

	for (mp = &message[0]; mp < &message[msgCount]; mp++)
		if (mp->m_flag & MNEW)
			break;
	if (mp >= &message[msgCount])
		for (mp = &message[0]; mp < &message[msgCount]; mp++)
			if ((mp->m_flag & MREAD) == 0)
				break;
	if (mp < &message[msgCount])
		mdot = mp - &message[0] + 1;
	else
		mdot = 1;
	s = d = 0;
	for (mp = &message[0], n = 0, u = 0; mp < &message[msgCount]; mp++) {
		if (mp->m_flag & MNEW)
			n++;
		if ((mp->m_flag & MREAD) == 0)
			u++;
		if (mp->m_flag & MDELETED)
			d++;
		if (mp->m_flag & MSAVED)
			s++;
	}
	ename = mailname;
	if (getfold(fname) >= 0) {
		strcat(fname, "/");
		if (strncmp(fname, mailname, strlen(fname)) == 0) {
			sprintf(zname, "+%s", mailname + strlen(fname));
			ename = zname;
		}
	}
	printf("\"%s\": ", ename);
	if (msgCount == 1)
		printf(MSGSTR(ONEMSG, "1 message")); /*MSG*/
	else
		printf(MSGSTR(MSGS, "%d messages"), msgCount); /*MSG*/
	if (n > 0)
		printf(MSGSTR(NUMNEW, " %d new"), n); /*MSG*/
	if (u-n > 0)
		printf(MSGSTR(UNREAD, " %d unread"), u); /*MSG*/
	if (d > 0)
		printf(MSGSTR(DELETED, " %d deleted"), d); /*MSG*/
	if (s > 0)
		printf(MSGSTR(SAVED, " %d saved"), s); /*MSG*/
	if (readonly)
		printf(MSGSTR(ONLYREAD, " [Read only]")); /*MSG*/
	printf("\n");
	return(mdot);
}

strace() {}

/*
 * Print the current version number.
 */

pversion(e)
{
	extern char *version;

	printf(MSGSTR(VERS, "Version %s\n"), version); /*MSG*/
	return(0);
}

/*
 * Load a file of user definitions.
 */
mail_load(name)
	char *name;
{
	register FILE *in, *oldin;

	if ((in = Fopen(name, "r")) == NULL)
		return;
	oldin = input;
	input = in;
	loading = 1;
	sourcing = 1;
	commands();
	loading = 0;
	sourcing = 0;
	input = oldin;
	Fclose(in);
}
