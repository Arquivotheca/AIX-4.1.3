static char sccsid[] = "@(#)50        1.23  src/bos/usr/bin/mail/collect.c, cmdmailx, bos41J, 9523A_all 6/5/95 11:30:02";
/* 
 * COMPONENT_NAME: CMDMAILX collect.c
 * 
 * FUNCTIONS: MSGSTR, addto, collect, collhup, collint, collstop,
 *            exwrite, find_nlortab, forward, mesedit, mespipe,  
 *           readshell, savedeadletter  
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
 * static char *sccsid = "collect.c    5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

/*
 * Mail -- a mail program
 *
 * Collect input from standard input, handling
 * ~ escapes.
 */

#include "rcv.h"
#include <sys/stat.h>
#include <paths.h>

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Read a message from standard output and return a read file to it
 * or NULL on error.
 */

/*
 * The following hokiness with global variables is so that on
 * receipt of an interrupt signal, the partial message can be salted
 * away on dead.letter.  The output file must be available to flush,
 * and the input to read.  Several open files could be saved all through
 * Mail if stdio allowed simultaneous read/write access.
 */

char *tilde[] = {
"Control Commands:",
"   <EOT>           Send message (Ctrl-D on many terminals).",
"   ~q              Quit editor without saving or sending message.",
"   ~p              Display the contents of the message buffer.",
"   ~x		    Exit editor without saving or sending message.",
"   ~: <mcmd>       Run a mailbox command, <mcmd>.",
"Add to Heading:",
"   ~h              Add to lists for To:, Subject:, Cc: and Bcc:.",
"   ~t <addrlist>   Add user addresses in <addrlist> to the To: list.",
"   ~s <subject>    Set the Subject: line to the string specified by <subject>.",
"   ~c <addrlist>   Add user addresses in <addrlist> to Cc: (copy to) list.",
"   ~b <addrlist>   Add user addresses in <addrlist> to Bcc: (blind copy) list.",
"Add to Message:",
"   ~{a,A}	    Insert the Autograph string, denoted by environment ",
"		    variable sign (or Sign for A), into the message.",
"   ~d              Append the contents of dead.letter to the message.",
"   ~i <string>	    Insert the value of <string> into the text of the message."
"   ~r <filename>   Append the contents of <filename> to the message.",
"   ~f <numlist>    Append the contents of message numbers <numlist>.",
"   ~m <numlist>    Append/indent the contents of message numbers <numlist>.",
"   ~<! <command>   Append the standard output of <command> to the message.",
"Change Message:",
"   ~e              Edit the message using an external editor (default is e).",
"   ~v              Edit the message using an external editor (default is vi).",
"   ~w <filename>   Write the message to <filename>.",
"   ~! <command>    Start a shell, run <command>, and return to the editor.",
"   ~| <command>    Pipe the message to standard input of <command>; REPLACE",
"                   the message with the standard output from that command.",
"================ Mail Editor Commands  (continue on next line) ================",
NULL
};

static	void	(*saveint)(int);	/* Previous SIGINT value */
static	void	(*savehup)(int);	/* Previous SIGHUP value */
static 	void	(*savetstp)(int);	/* Previous SIGTSTP value */
static	void	(*savettou)(int);	/* Previous SIGTTOU value */
static	void	(*savettin)(int);	/* Previous SIGTTIN value */
static	FILE	*collf;			/* File for saving away */
int		hadintr;		/* Have seen one SIGINT so far */

static	jmp_buf	colljmp;		/* To get back to work	*/
static	volatile int colljmp_p;		/* whether to long jump */
static	jmp_buf	collabort;		/* To end collection with error */

extern	char	tempMail[];

void	collint(int);
void	collhup(int);
void	collstop(int);

static	jmp_buf pipestop;

FILE *
collect(hp)
	struct header *hp;
{
	FILE *fbuf, *obuf;
	int lc, cc, escape, eofcount;
	register int c, t;
	char linebuf[LINESIZE], *cp;
	char getsub;
	int hcount;
	int omask;
	int plines = 0;
	void brokpipe(int);

	collf = NULL;
	tab_seq = FALSE;
	/*
	 * Start catching signals from here, but we still die on
	 * interrupts until we are in the main loop.
	 */
	omask = sigblock(sigmask(SIGINT) | sigmask(SIGHUP));
	if ((saveint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
		signal(SIGINT, (void(*)(int))collint);
	if ((savehup = signal(SIGHUP, SIG_IGN)) != SIG_IGN)
		signal(SIGHUP, (void(*)(int))collhup);
	savetstp = signal(SIGTSTP, (void(*)(int))collstop);
	savettou = signal(SIGTTOU, (void(*)(int))collstop);
	savettin = signal(SIGTTIN, (void(*)(int))collstop);

	/*
	 * ANSI says we must keep setjmp signals 
	 */
	if (setjmp(collabort)) {
		remove_file(tempMail);
		goto err;
	}
	
	if (setjmp(colljmp)) {
		remove_file(tempMail);
		goto err;
	}
	
	sigsetmask(omask & ~(sigmask(SIGINT) | sigmask(SIGHUP)));

	noreset++;
	if ((collf = Fopen(tempMail, "w+")) == NULL) {
		perror(tempMail);
		goto err;
	}

	unlink(tempMail);

	/*
	 * If we are going to prompt for a subject,
	 * refrain from printing a newline after
	 * the headers (since some people mind).
	 */

	t = GTO|GSUBJECT|GCC|GNL;
	getsub = 0;
	if (sflag != NOSTR && hp->h_subject == NOSTR)
		hp->h_subject=sflag;
	if (intty && sflag == NOSTR && hp->h_subject == NOSTR && (value("noasksub") == NOSTR))
		t &= ~GNL, getsub++;
	if (hp->h_seq != 0) {
		puthead(hp, stdout, t);
		fflush(stdout);
	}

	/* 
	 * Set the ccflag here so if user does a ~h, the cc field 
	 * will have some value to display.
	*/
	if (ccflag != NOSTR && hp->h_cc == NOSTR)
		hp->h_cc = ccflag;

	escape = ESCAPE;
	if ((cp = value("escape")) != NOSTR)
		escape = *cp;
	eofcount = 0;
	hadintr = 0;

	/*
	 * If we are interactive, line buffer the temp file
	 * so we will get errors fairly quickly, not after
	 * typing in BUFSIZ characters.
	 */
	if (intty)
		setlinebuf(collf);
	if (!setjmp(colljmp)) {
		if (getsub)
			grabh(hp, GSUBJECT);
	} else {
		/*
		 * Come here for printing the after-signal message.
		 * Duplicate messages won't be printed because
		 * the write is aborted if we get a SIGTTOU.
		 */

cont:
		if (hadintr) {
			fflush(stdout);
			fprintf(stderr, MSGSTR(INTR, 
			"\n(Interrupt -- one more to kill letter)\n"));
		} else {
			printf(MSGSTR(CONT, "(continue)\n"));
			fflush(stdout);
		}
	}
	for(;;) {
		colljmp_p = 1;
		c = readline(stdin, linebuf);
		colljmp_p = 0;
		if (c <= 0) {
			if (intty && value("ignoreeof") != NOSTR && 
				++eofcount < 25) {
				printf(MSGSTR(TERM,
				"Use \".\" to terminate letter\n"), escape);
				continue;		
			}
			break;
		}
		eofcount = 0;
		hadintr = 0;
	
		if (intty && linebuf[0] == '.' && linebuf[1] == '\0' &&
		   (value("dot") != NOSTR || value("ignoreeof") != NOSTR))
			break;
		if (linebuf[0] != escape || rflag != NOSTR) {
			if ((t = putline(collf, linebuf)) < 0)
				goto err;
			continue;
		}
		c = linebuf[1];
		switch (c) {
		default:
			/*
			 * On double escape, just send the single one.
			 * Otherwise, it's an error.
			 */

			if (c == escape) {
				if (putline(collf, &linebuf[1]) < 0)
					goto err;
				else
					break;
			}
			printf(MSGSTR(NOESC, "Unknown tilde escape.\n")); /*MSG*/
			break;

		case 'C':
			/*
			 * Dump core.
			 */

			core();
			break;

		case '!':
			/*
			 * Shell escape, send the balance of the
			 * line to sh -c.
			 */

			shell(&linebuf[2]);
			break;

		case ':':
		case '_':
			/*
			 * Escape to command mode, but be nice!
			 */

			execute(&linebuf[2], 1);
			goto cont;

		case '.':
			/*
			 * Simulate end of file on input.
			 */
			goto out;

		case 'q':
		case 'Q':
			/*
			 * Force a quit of sending mail.
			 * Act like an interrupt happened.
			 */

			hadintr++;
			collint(SIGINT);
			exit(1);

		case 'h':
			/*
			 * Grab a bunch of headers.
			 */
			if (!intty || !outtty) {
				printf(MSGSTR(NOCANDO, "~h: no can do!?\n")); /*MSG*/
				break;
			}
			grabh(hp, GTO|GSUBJECT|GCC|GBCC);
			goto cont;

		case 't':
			/*
			 * Add to the To list.
			 */

			hp->h_to = addto(hp->h_to, &linebuf[2]);
			hp->h_seq++;
			break;

		case 's':
			/*
			 * Set the Subject list.
			 */

			cp = &linebuf[2];
			while (isspace(*cp))
				cp++;
			hp->h_subject = savestr(cp);
			hp->h_seq++;
			break;

		case 'c':
			/*
			 * Add to the CC list.
			 */

			hp->h_cc = addto(hp->h_cc, &linebuf[2]);
			hp->h_seq++;
			break;

		case 'b':
			/*
			 * Add stuff to blind carbon copies list.
			 */
			hp->h_bcc = addto(hp->h_bcc, &linebuf[2]);
			hp->h_seq++;
			break;

		case 'A':
		case 'a':
		case 'i':
			/*
			 * Insert the autograph string defined by 
			 * the environment variable Sign.
			 * In the case of ~i var, where var is not sign
			 * or Sign, then just insert string and don't 
			 * worry about \t or \n. 
			 */
			if (c == 'A')
				cp = value("Sign");
			else if (c == 'a')	
				cp = value("sign");
			     else { 
				cp = value(&linebuf[3]);
				if (!strcmp(linebuf[3], "Sign") && 
				    !strcmp(linebuf[3], "sign")) {
					if (*cp != NULL) {
					printf("%s\n", cp);
					if (( t == putline(collf, cp)) < 0)
						goto err;
					}
					break;
				}
			     }

			if (*cp != NULL) {	
				char *p = NULL, *temp;
				int count;
				extern char *find_nlortab();

				while ((p = find_nlortab(cp)) != NULL) {
				   count = p-cp;
				   if ((temp = (char *)malloc(count)) == NULL)
					panic(MSGSTR(MEMERR, "Internal error, malloc() failed"));
				   strncpy(temp, cp, count);
				   temp[count-1] = '\0';
				   if (*p == 'n') {
					printf("%s\n", temp);
					tab_seq = FALSE;
				   }
				   if (*p == 't') {
					printf("%s\t", temp);
					tab_seq = TRUE;
				   }
				   if ((t = putline(collf, temp)) < 0)
					goto err;
				   cp = ++p;
				}
				tab_seq = FALSE;
				printf("%s\n", cp);
				if ((t = putline(collf, cp)) < 0)
					goto err;
			}
			break;	

		case 'x':
			/*
			 * Force a quit of sending mail.
			 * Act like an interrupt happened.
			 * Don't put copy of message in dead.letter.
			 */
			hadintr++;
			assign("nosave", "");
			collint(SIGINT);
			vfree("nosave");
			exit(1);

		case 'd':
			copy(deadletter, &linebuf[2]);
			/* fall into . . . */

		case 'r':
		case '<':
			/*
			 * Invoke a file:
			 * If shell escape, insert standard output in
			 * to message (XPG3).
			 * Search for the file name,
			 * then open it and copy the contents to collf.
			 */

			cp = &linebuf[2];
			while (isspace(*cp))
				cp++;
			if (*cp == '!') {
				readshell(collf, &linebuf[3]);
				break;
			}
			while (isspace(*cp))
				cp++;
			if (*cp == '\0') {
				printf(MSGSTR(WHAT, "Interpolate what file?\n")); /*MSG*/
				break;
			}
			cp = expand(cp);
			if (cp == NOSTR)
				break;
			if (isdir(cp)) {
				printf(MSGSTR(DIR, "%s: directory\n"), cp); /*MSG*/
				break;
			}
			if ((fbuf = Fopen(cp, "r")) == NULL) {
				perror(cp);
				break;
			}
			printf("\"%s\" ", cp);
			fflush(stdout);
			lc = 0;
			cc = 0;
			while (readline(fbuf, linebuf) > 0) {
				lc++;
				if ((t = putline(collf, linebuf)) < 0) {
					Fclose(fbuf);
					goto err;
				}
				cc += t;
			}
			Fclose(fbuf);
			printf("%d/%d\n", lc, cc);
			break;

		case 'w':
			/*
			 * Write the message on a file.
			 */

			cp = &linebuf[2];
			while (any(*cp, " \t"))
				cp++;
			if (*cp == '\0') {
				fprintf(stderr, MSGSTR(SAYWHAT, "Write what file!?\n")); /*MSG*/
				break;
			}
			if ((cp = expand(cp)) == NOSTR)
				break;
			rewind(collf);
			exwrite(cp, collf, 0);
			break;

		case 'm':
		case 'M':
		case 'f':
		case 'F':
			/*
			 * Interpolate the named messages, if we
			 * are in receiving mail mode.  Does the
			 * standard list processing garbage.
			 * If ~f is given, we don't shift over.
			 */

			if (!rcvmode) {
				printf(MSGSTR(NOTOSEND, "No messages to send from!?!\n")); /*MSG*/
				break;
			}
			cp = &linebuf[2];
			while (any(*cp, " \t"))
				cp++;
			if (forward(cp, collf, c) < 0)
				goto err;
			goto cont;

		case '?':
			for (hcount = 0; tilde[hcount]; hcount++)
			    puts(MSGSTR(THELP+hcount, tilde[hcount]));
			break;

		case 'p':
			/*
			 * Print out the current state of the
			 * message without altering anything.
			 */

			obuf = stdout;

			rewind(collf);
			while ((t = getc(collf)) != EOF)
				if (t == '\n')
					plines++;
			if ((cp = value("crt")) != NOSTR) {
				if (plines > atoi(cp)) {
					if (setjmp(pipestop)) {
						if (obuf != stdout) {
							pipef = NULL;
							Pclose(obuf);
						}
						signal(SIGPIPE, SIG_DFL);
						goto cont;
					}
			
					cp = value("PAGER");
					if (cp == NULL || *cp == '\0')
						cp = MORE;
					obuf = Popen(cp, "w");
					if (obuf == NULL) {
						perror(cp);
						obuf = stdout;
					}
					else {
						pipef = obuf;
						signal(SIGPIPE, 
						(void(*)(int)) brokpipe);
					}
				}
			}
			
			rewind(collf);

			fprintf(obuf, MSGSTR(CONTAINS, "-------\nMessage contains:\n")); /*MSG*/
			puthead(hp, obuf, GTO|GSUBJECT|GCC|GBCC|GNL);
			while ((t = getc(collf)) != EOF) 
				(void)putc(t, obuf);
			
			if (obuf != stdout) {
				pipef = NULL;
				Pclose(obuf);
				signal(SIGPIPE, SIG_DFL);
			}
			goto cont;

		case '^':
		case '|':
			/*
			 * Pipe message through command.
			 * Collect output as new message.
			 */

			rewind(collf);
			collf = mespipe(collf, &linebuf[2]);
			goto cont;

		case 'v':
		case 'e':
			/*
			 * Edit the current message.
			 * 'e' means to use EDITOR
			 * 'v' means to use VISUAL
			 */

			rewind(collf);
			if ((collf = mesedit(collf, c)) == NULL)
				goto err;
			goto cont;
		}
	}
	if (intty) {
		if ((value("askcc") == NOSTR) && (value("askbcc") == NOSTR)) {
			printf(MSGSTR(EOT, "EOT\n")); /*MSG*/
			fflush(stdout);
		} else {
			if (value("askcc") != NOSTR)
				grabh(hp, GCC);
			if (value("askbcc") != NOSTR)
				grabh(hp, GBCC);
		}
	}

	goto out;

err:
	senderr++;
	if (collf != NULL) {
		Fclose(collf);
		collf = NULL;
	}
out:
	if (collf != NULL)
		rewind(collf);
	noreset--;
	sigblock(sigmask(SIGINT) | sigmask(SIGHUP));
	signal(SIGINT, (void(*)(int))saveint);
	signal(SIGHUP, (void(*)(int))savehup);
	signal(SIGTSTP, (void(*)(int))savetstp);
	signal(SIGTTOU, (void(*)(int))savettou);
	signal(SIGTTIN, (void(*)(int))savettin);
	sigsetmask(omask);
	return(collf);
}

/*
 * Write a file, ex-like if f set.
 */

exwrite(name, ibuf, f)
	char name[];
	FILE *ibuf;
{
	register FILE *of;
	register int c;
	long cc;
	int lc;
	struct stat junk;
	char *disp;

	printf("\"%s\" ", name);
	fflush(stdout);
	if (stat(name, &junk) >= 0 && (junk.st_mode & S_IFMT) == S_IFREG) {
		if (f) {
			fprintf(stderr, MSGSTR(EXISTS, "File exists\n"), name);
			return(-1);
		}
		else
			disp = MSGSTR(APP, "[Appended]");
	}
	else
		if (!f)
			disp = MSGSTR(NEW, "[New file]");
	
	if (f) {
		if ((of = Fopen(name, "w")) == NULL) {
			perror(NOSTR);
			return(-1);
		}
	}
	else {
		if ((of = Fopen(name, "a")) == NULL) {
			perror(NOSTR);
			return(-1);
		}
	}
	lc = 0;
	cc = 0;
	while ((c = getc(ibuf)) != EOF) {
		cc++;
		if (c == '\n')
			lc++;
		putc(c, of);
		if (ferror(of)) {
			perror(name);
			Fclose(of);
			return(-1);
		}
	}
	Fclose(of);
	if (f)
		printf("%d/%ld\n", lc, cc);
	else	
		printf("%s %d/%ld\n",disp, lc, cc);
	fflush(stdout);
	return(0);
}

/*
 * Edit the message being collected on ibuf and obuf.
 * Write the message out onto some poorly-named temp file
 * and point an editor at it.
 *
 * On return, make the edit file the new temp file.
 */

FILE *
mesedit(fp, c)
	FILE *fp;
{
	int pid, s;
	FILE *fbuf;
	register int t;
	void (*sigint)(int), (*sigcont)(int);
	struct stat sbuf;
	extern char tempEdit[];
	register char *edit;

	sigint = signal(SIGINT, SIG_IGN);
	sigcont = signal(SIGCONT, SIG_DFL);

	if (stat(tempEdit, &sbuf) >= 0) {
		printf(MSGSTR(FEXISTS, "%s: file exists\n"), tempEdit); /*MSG*/
		goto out;
	}
	close(creat(tempEdit, 0600));
	if ((fbuf = Fopen(tempEdit, "w")) == NULL) {
		perror(tempEdit);
		goto out;
	}
	while ((t = getc(fp)) != EOF) 
		putc(t, fbuf);
	fflush(fbuf);
	if (ferror(fbuf)) {
		perror(tempEdit);
		remove_file(tempEdit);
		goto fix;
	}
	Fclose(fbuf);
	edit = value(c == 'e' ? "EDITOR" : "VISUAL");
	if (edit == NOSTR || *edit == NULL)
		edit = c == 'e' ? EDITOR : VISUAL;
	pid = vfork();
	if (pid == 0) {
		if (sigint != SIG_IGN)
			signal(SIGINT, SIG_DFL);
		execlp(edit, edit, tempEdit, 0);
		perror(edit);
		_exit(1);
	}
	if (pid == -1) {
		perror("fork");
		remove_file(tempEdit);
		goto out;
	}
	while (wait(&s) != pid)
		;
	if (s != 0) {
		printf(MSGSTR(FERROR, "Fatal error in \"%s\"\n"), edit); /*MSG*/
		remove_file(tempEdit);
		goto out;
	}

	/*
	 * Now switch to new file.
	 */

	if ((fbuf = Fopen(tempEdit, "a+")) == NULL) {
		perror(tempEdit);
		remove_file(tempEdit);
		goto out;
	}
	remove_file(tempEdit);
	Fclose(fp);
	fp = fbuf;
	goto out;
fix:
	perror(tempEdit);
out:
	signal(SIGCONT, (void(*)(int))sigcont);
	signal(SIGINT, (void(*)(int))sigint);
	return(fp);
}

/*
 * Pipe the message through the command.
 * Old message is on stdin of command;
 * New message collected from stdout.
 * Sh -c must return 0 to accept the new message.
 */

FILE *
mespipe(fp, cmd)
	FILE *fp;
	char cmd[];
{
	register FILE *nf;
	int pid, s;
	void (*saveint)(int);
	char *Shell;
	extern char tempEdit[];

	if ((nf = Fopen(tempEdit, "w+")) == NULL) {
		perror(tempEdit);
		return(fp);
	}
	remove_file(tempEdit);
	saveint = signal(SIGINT, SIG_IGN);
	if ((Shell = value("SHELL")) == NULL)
		Shell = _PATH_BSHELL;
	if ((pid = vfork()) == -1) {
		perror("fork");
		goto err;
	}
	if (pid == 0) {
		int fd;
		/*
		 * stdin = current message.
		 * stdout = new message.
		 */

		close(0);
		dup(fileno(fp));
		close(1);
		dup(fileno(nf));
		for (fd = getdtablesize(); --fd > 2;)
			close(fd);
		execl(Shell, Shell, "-c", cmd, 0);
		perror(Shell);
		_exit(1);
	}
	while (wait(&s) != pid)
		;
	if (s != 0 || pid == -1) {
		fprintf(stderr, MSGSTR(FAILED, "\"%s\" failed!?\n"), cmd); /*MSG*/
		goto err;
	}
	if (fsize(nf) == 0) {
		fprintf(stderr, MSGSTR(NOBYTES, "No bytes from \"%s\" !?\n"), cmd); /*MSG*/
		goto err;
	}

	/*
	 * Take new files.
	 */

	fseek(nf, 0L, SEEK_END);
	Fclose(fp);
	signal(SIGINT, (void(*)(int))saveint);
	return(nf);

err:
	Fclose(nf); 
	signal(SIGINT, (void(*)(int))saveint);
	fseek(fp, 0L, SEEK_END);
	return(fp);
}

/*
 * Execute a shell command and insert its standard output
 * into the message.
 */
readshell(fp, cmd)
	FILE *fp;
	char cmd[];
{
	void (*saveint)();
	int pid, s;
	char *Shell;

	saveint = signal(SIGINT, SIG_IGN);
	fflush(fp);
	if ((Shell = value("SHELL")) == NULL)
		Shell = _PATH_BSHELL;
	if ((pid = vfork()) == -1) {
		perror("fork");
		goto err;
	}
	if (pid == 0) {
		int fd;
		/*
		 * stdout = message 
		 */
		close(1);
		dup(fileno(fp));	
		for (fd = getdtablesize(); --fd > 2;)
			close(fd);
		execl(Shell, Shell, "-c", cmd, 0);
		perror(Shell);
		_exit(1);
	}
	while (wait(&s) != pid)
		;
	if (s != 0 || pid == -1) {
		fprintf(stderr, MSGSTR(FAILED, "\"%s\" failed!?\n"), cmd);
		goto err;
	}
	signal(SIGINT, (void(*)(int))saveint);
	return(0);

err:
	return(-1);
}

/*
 * Interpolate the named messages into the current
 * message, preceding each line with a tab.
 * Return a count of the number of characters now in
 * the message, or -1 if an error is encountered writing
 * the message temporary.  The flag argument is 'm' if we
 * should shift over and 'f' if not.
 */
forward(ms, fp, f)
	char ms[];
	FILE *fp;
{
	register int *msgvec;
	extern char tempMail[];
	char *tabstr;
	int ig;

	msgvec = (int *) salloc((msgCount+1) * sizeof *msgvec);
	if (msgvec == (int *) NOSTR)
		return(0);
	if (getmsglist(ms, msgvec, 0) < 0)
		return(0);
	if (*msgvec == (int)NULL) {
		*msgvec = first(0, MMNORM);
		if (*msgvec == (int)NULL) {
			printf(MSGSTR(NOAMSGS, "No appropriate messages\n")); /*MSG*/
			return(0);
		}
		msgvec[1] = (int)NULL;
	}
	if (f == 'f' || f == 'F')
		tabstr = NOSTR;
	else if ((tabstr = value("indentprefix")) == NOSTR)
		tabstr = "\t";
	ig = isupper(f) ? 0 : 1;
	printf(MSGSTR(INTER, "Interpolating:")); /*MSG*/
	for (; *msgvec != 0; msgvec++) {
		struct message *mp = message + *msgvec - 1;
		touch(mp);
		printf(" %d", *msgvec);
		if (send(mp, fp, ig, tabstr) < 0) {
				perror(tempMail);
				return(-1);
		}
	}
	printf("\n");
	return(0);
}

void
collstop(int s)
{
	void (*old_action)(int);
	
	old_action = signal(s, SIG_DFL);
	sigsetmask(sigblock(0) & ~sigmask(s));
	kill(0, s);
	sigblock(sigmask(s));
	signal(s, (void(*)(int))old_action);
	if (colljmp_p) {
		colljmp_p = 0;
		hadintr = 0;
		longjmp(colljmp, 1);
	}
}

/*
 * On interrupt, go here to save the partial
 * message on ~/dead.letter. Then jump out of
 * the collection loop.
 */

void collint(int s)
{
	/*
	 * the control flow is subtle, because we can be called from ~q.
	 */
	if (!hadintr) {
		if (value("ignore") != NOSTR) {
			puts("@");
			fflush(stdout);
			clearerr(stdin);
			return;
		}
		hadintr++;
		longjmp(colljmp, 1);
	}
	rewind(collf);
	if (value("nosave") == NOSTR)
		savedeadletter(collf);
	longjmp(collabort, 1);
}


void collhup(int s)
{
	rewind(collf);
	savedeadletter(collf);
	/*
	 * Let's pretend nobody else wants to clean up,
	 * a true statement at this time.
	 */
	exit(1);
}

/*
 * Add a string to the end of a header entry field.
 */

char *
addto(hf, news)
	char hf[], news[];
{
	register char *cp, *cp2, *linebuf;

	if (hf == NOSTR)
		hf = "";
	if (*news == '\0')
		return(hf);
	linebuf = salloc(strlen(hf) + strlen(news) + 2);
	for (cp = hf; any(*cp, " \t"); cp++)
		;
	for (cp2 = linebuf; *cp;)
		*cp2++ = *cp++;
	*cp2++ = ' ';
	for (cp = news; any(*cp, " \t"); cp++)
		;
	while (*cp != '\0')
		*cp2++ = *cp++;
	*cp2 = '\0';
	return(linebuf);
}

int
savedeadletter(fp)
	register FILE *fp;
{
	register FILE *dbuf;
	register int c;
	char *cp;
	if (fsize(fp) == 0)
		return;
	cp = deadletter;
	c = umask(077);
	dbuf = Fopen(cp, "w");
	(void) umask(c);
	if (dbuf == NULL)
		return;
	while(( c = getc(fp)) != EOF)
		(void)putc(c, dbuf);
	fprintf(stderr, MSGSTR(LINTRP, "\n(Last Interrupt -- letter saved in %s)\n"),deadletter);
	Fclose(dbuf);
	rewind(fp);
}

char *
find_nlortab(a_string)
	register char *a_string;
{
	register char *sp = NULL;
	char *next;

	if (a_string == 0)
		return(0);
	
	next = a_string;
	while ((sp = strchr(next, '\\')) != NULL) {
		sp++;
		if (*sp == 'n' || *sp == 't')
			return (sp);
		else {
			sp++;
			next = sp;
		}
	}
	return ((char *)0);
}
