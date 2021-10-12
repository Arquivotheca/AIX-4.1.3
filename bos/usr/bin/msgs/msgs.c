static char sccsid[] = "@(#)57 1.7.1.6  src/bos/usr/bin/msgs/msgs.c, cmdcomm, bos41B, 9504A 12/23/94 12:51:26";
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: msgs
 *
 * ORIGINS: 10,26,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "msgs_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MSGS,n,s) 

#define ISDIGIT isdigit
/*
 * msgs - a user bulletin board program
 *
 * usage:
 *	msgs [-fhlpq] [[-]number]	to read messages
 *	msgs -s				to place messages
 *	msgs -c [-days]			to clean up the bulletin board
 *
 * prompt commands are:
 *	y Y \n	print message
 *	n N	flush message, go to next message
 *	q Q	flush message, quit
 *	p	print message, turn on 'pipe thru more' mode
 *	P	print message, turn off 'pipe thru more' mode
 *	-	reprint last message
 *	m[-][<num>]	mail with message in temp mbox
 *	x X	exit without flushing this message
 *	<num>	print message number <num>
 */

#define OBJECT		/* will object to messages without Subjects */

#include <stdio.h>
#include <stdlib.h>	/* for MB_CUR_MAX */
#include <locale.h>
#include <sys/limits.h>
#include <sys/access.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pwd.h>
#include <sgtty.h>
#include <setjmp.h>
#include <langinfo.h>

#define USRMSGS	"/var/msgs"		/* msgs directory */
#define MAIL	"/usr/bin/mail -f %s"	/* could set destination also */
#define PAGE	"/usr/bin/more -%d"	/* crt screen paging program */
#define NO	0
#define YES	1
#define SUPERUSER	0	/* superuser uid */
#define DAEMON		1	/* daemon uid */
#define NLINES	24		/* default number of lines/crt screen */
#define NDAYS	21		/* default keep time for messages */
#define DAYS	*24*60*60	/* seconds/day */
#define TEMP	"/tmp/msgXXXXXX"
#define MSGSRC	".msgsrc"	/* user's rc file */
#define BOUNDS	"bounds"	/* message bounds file */
#define TSSIZE	128		/* max # bytes in time string */

typedef	char	bool;

static FILE	*newmsg;
static char	*sep = "-";
static char	inbuf[BUFSIZ];
static char	fname[PATH_MAX];
static char	cmdbuf[PATH_MAX];
static char	subj[MAX_INPUT];
static char	from[MAX_INPUT];
static char	date[PATH_MAX];
static char	tbuf[TSSIZE];		/* buffer to contain formatted date and time */
static char	*ptr;
static char	*in;
static bool	local;
static bool	ruptible;
static bool	totty;
static bool	seenfrom;
static bool	seensubj;
static bool	blankline;
static bool	printing = NO;
static bool	mailing = NO;
static bool	quitit = NO;
static bool	sending = NO;
static bool	intrpflg = NO;
static bool	tstpflag = NO;
static int	uid;
static int	msg;
static int	prevmsg;
static int	lct;
static int	nlines;
static int	flgbk = 0;
static int	restart = 0;
static int	Lpp = 0;
static time_t	t;
static time_t	keep;
static size_t	mb_cur_max;
static struct	sgttyb	otty;

static char	*nxtfld();
static int	onintr(void);
static int	onsusp(void);

extern	int	errno;

/* option initialization */
static bool	hdrs = NO;
static bool	qopt = NO;
static bool	hush = NO;
static bool	send = NO;
static bool	locomode = NO;
static bool    pause_f = NO;
static bool	clean = NO;
static jmp_buf	tstpbuf;

main(argc, argv)
int argc; char *argv[];
{
	bool newrc, already;
	int rcfirst = 0;		/* first message to print (from .rc) */
	int rcback = 0;			/* amount to back off of rcfirst */
	int firstmsg, nextmsg, lastmsg = 0;
	int blast = 0;
	FILE *bounds, *msgsrc;
	struct dirent *dp;
	struct stat stbuf;
	bool seenany = NO;
	DIR	*dirp;

	(void) setlocale(LC_ALL,"");
	mb_cur_max = MB_CUR_MAX;
	catd = catopen(MF_MSGS, NL_CAT_LOCALE);

	/* gtty(fileno(stdout), &otty); */
	ioctl(stdout, TIOCGETP, &otty);
	time(&t);
	setuid(uid = getuid());
	ruptible = ((void (*) (int)) signal(SIGINT, SIG_IGN) == SIG_DFL);
	if (ruptible)
		signal(SIGINT, SIG_DFL);

	argc--, argv++;
	while (argc > 0) {
		if (ISDIGIT(argv[0][0])) {	/* starting message # */
			rcfirst = atoi(argv[0]);
			restart = 1;
		}
		else if (ISDIGIT(argv[0][1])) {	/* backward offset */
			rcback = atoi( &( argv[0][1] ) );
			flgbk = 1;
		}
		else {
			ptr = *argv;
			while (*ptr) switch (*ptr++) {

			case '-':
				break;

			case 'c':
				if (uid != SUPERUSER && uid != DAEMON) {
					fprintf(stderr,MSGSTR(SORRY,"Sorry\n"));
					exit(1);
				}
				clean = YES;
				break;

			case 'f':		/* silently */
				hush = YES;
				break;

			case 'h':		/* headers only */
				hdrs = YES;
				break;

			case 'l':		/* local msgs only */
				locomode = YES;
				break;

			case 'p':		/* pipe thru 'more' during
						 * long msgs
						 */
				pause_f = YES;
				break;

			case 'q':		/* query only */
				qopt = YES;
				break;

			case 's':		/* sending TO msgs */
				send = YES;
				break;

			default:
				fprintf(stderr,MSGSTR(USAGE,
					"usage: msgs [-fhlpq] [[-]number]\n       msgs -s\n       msgs -c [-Days]\n"));
				exit(1);
			}
		}
		argc--, argv++;
	}

	/*
	 * determine current message bounds
	 */
	sprintf(fname, "%s/%s", USRMSGS, BOUNDS);
	bounds = fopen(fname, "r");

	if (bounds != NULL) {
		fscanf(bounds, "%d %d\n", &firstmsg, &lastmsg);
		fclose(bounds);
		blast = lastmsg;	/* save upper bound */
	}

	if (clean)
		keep = t - (rcback? rcback : NDAYS) DAYS;

	if (clean || bounds == NULL) {	/* relocate message bounds */
		dirp = opendir(USRMSGS);
		if (dirp == NULL) {
			perror(USRMSGS);
			exit(errno);
		}

		firstmsg = -1;
		lastmsg = 0;

		for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
			register char *cp = dp->d_name;
			register int i = 0;

			if (dp->d_ino == 0)
				continue;
			if (dp->d_namlen == 0)
				continue;

			if (clean)
				sprintf(inbuf, "%s/%s", USRMSGS, cp);

			while (ISDIGIT(*cp))
				i = i * 10 + *cp++ - '0';
			if (*cp)
				continue;	/* not a message! */

			if (clean) {
				if (stat(inbuf, &stbuf) != 0)
					continue;
				if (stbuf.st_mtime < keep
				    && !accessx(inbuf,W_ACC, ACC_SELF)) {
					unlink(inbuf);
					continue;
				}
			}

			if (i > lastmsg)
				lastmsg = i;
			if (i < firstmsg)
				firstmsg = i;
			seenany = YES;
		}
		closedir(dirp);

		if (!seenany) {
			if (blast != 0)	/* never lower the upper bound! */
				lastmsg = blast;
			firstmsg = lastmsg + 1;
		}
		else if (blast > lastmsg)
			lastmsg = blast;

		if (!send) {
			bounds = fopen(fname, "w");
			if (bounds == NULL) {
				perror(fname);
				exit(errno);
			}
			acl_set(fname,
				R_ACC | W_ACC, R_ACC | W_ACC, R_ACC | W_ACC);
			fprintf(bounds, "%d %d\n", firstmsg, lastmsg);
			fclose(bounds);
		}
	}

	if (send) {
		/*
		 * Send mode - place msgs in USRMSGS
		 */
		bounds = fopen(fname, "w");
		if (bounds == NULL) {
			perror(fname);
			exit(errno);
		}

		nextmsg = lastmsg + 1;
		sprintf(fname, "%s/%d", USRMSGS, nextmsg);
		newmsg = fopen(fname, "w");
		if (newmsg == NULL) {
			perror(fname);
			exit(errno);
		}
		acl_set(fname, R_ACC | W_ACC, R_ACC, R_ACC);

		fprintf(bounds, "%d %d\n", firstmsg, nextmsg);
		fclose(bounds);

		sending = YES;
		if (ruptible)
			signal(SIGINT, (void (*)(int))onintr);


		if (isatty((int)fileno(stdin))) {
			ptr = getpwuid((uid_t)uid)->pw_name;
			strftime(tbuf, sizeof tbuf, nl_langinfo(D_T_FMT),
							localtime(&t));
			printf(MSGSTR(MESSAGE,
				"Message %1$d:\nFrom %2$s %3$s\nSubject: "),
				nextmsg, ptr, tbuf);
			fflush((FILE *)stdout);
			fgets(inbuf, (int)sizeof inbuf, stdin);
			putchar('\n');
			fflush((FILE *)stdout);
			fprintf(newmsg, MSGSTR(FROM,
				"From %1$s %2$s\nSubject: %3$s\n"),
				ptr, tbuf, inbuf);
			blankline = seensubj = YES;
		}
		else
			blankline = seensubj = NO;
		for (;;) {
			fgets(inbuf, (int)sizeof inbuf, stdin);
			if (feof(stdin) || ferror(stdin))
				break;
			blankline = (blankline || (inbuf[0] == '\n'));
			seensubj = (seensubj || (!blankline && (strncmp(inbuf,
					MSGSTR(SUB, "Subj"), 4) == 0)));
			fputs(inbuf, newmsg);
		}
#ifdef OBJECT
		if (!seensubj) {
			printf(MSGSTR(NOTE,
			    "NOTICE: Messages should have a Subject field!\n"));
			exit(1);
		}
#endif
		exit(ferror(stdin));
	}
	if (clean)
		exit(0);

	/*
	 * prepare to display messages
	 */
	totty = (isatty(fileno(stdout)) != 0);
	pause_f = pause_f && totty;

	sprintf(fname, "%s/%s", getenv("HOME"), MSGSRC);
	if (!restart) {
		msgsrc = fopen(fname, "r");
		if (msgsrc) {
			newrc = NO;
			fscanf(msgsrc, "%d\n", &nextmsg);
			fclose(msgsrc);
			if (nextmsg > lastmsg+1) {
				printf(MSGSTR(RESET,
				  "Warning: bounds have been reset (%d, %d)\n"),
				  firstmsg, lastmsg);
				ftruncate(fileno(msgsrc), 0);
				newrc = YES;
			}
			else if (!rcfirst)
				rcfirst = nextmsg - rcback;
		}
		else
			newrc = YES;
	}
	if ((msgsrc = fopen(fname, "r+")) == NULL)
		msgsrc = fopen(fname, "w");
	if (msgsrc == NULL) {
		perror(fname);
		exit(errno);
	}
	if (rcfirst) {
		if (rcfirst > lastmsg+1) {
			printf(MSGSTR(LAST,
				"Warning: the last message is number %d.\n"),
				lastmsg);
			rcfirst = nextmsg;
		}
		if (rcfirst > firstmsg)
			firstmsg = rcfirst;	/* don't set below first msg */
	}
	if (newrc) {
		nextmsg = firstmsg;
		fseek(msgsrc, 0L, 0);
		fprintf(msgsrc, "%d\n", nextmsg);
		fflush(msgsrc);
	}

	if (totty) {
		struct winsize win;
		if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1)
			Lpp = win.ws_row;
		if (Lpp <= 0) {
			if (tgetent(inbuf, getenv("TERM")) <= 0
			    || (Lpp = tgetnum("li")) <= 0) {
				Lpp = NLINES;
			}
		}
	}
	Lpp -= 6;	/* for headers, etc. */

	already = NO;
	prevmsg = firstmsg;
	printing = YES;
	if (ruptible)
		signal(SIGINT, (void (*)(int))onintr);

	/*
	 * Main program loop
	 */
	for (msg = firstmsg; msg <= lastmsg; msg++) {

		sprintf(fname, "%s/%d", USRMSGS, msg);
		newmsg = fopen(fname, "r");
		if (newmsg == NULL)
			continue;

		gfrsub(newmsg);		/* get From and Subject fields */
		if (locomode && !local) {
			fclose(newmsg);
			continue;
		}

		if (qopt) {	/* This has to be located here */
			printf(MSGSTR(NEW, "There are new messages.\n"));
			exit(0);
		}

		if (already && !hdrs)
			putchar('\n');
		already = YES;

		/*
		 * Print header
		 */
		if (totty)
			signal(SIGTSTP, (void (*)(int))onsusp);
		(void) setjmp(tstpbuf);
		nlines = 2;
		if (seenfrom) {
			printf(MSGSTR(MESSAG2,
			     "Message %1$d:\nFrom %2$s %3$s"), msg, from, date);
			nlines++;
		}
		if (seensubj) {
			printf(MSGSTR(SUBJ, "Subject: %s"), subj);
			nlines++;
		}
		else {
			if (seenfrom) {
				putchar('\n');
				nlines++;
			}
			while (nlines < 6
			    && fgets(inbuf, (int)sizeof inbuf, newmsg)
			    && inbuf[0] != '\n') {
				fputs(inbuf, stdout);
				nlines++;
			}
		}

		lct = linecnt(newmsg);
		if (lct) {
            	    if (seensubj)
			printf(MSGSTR(LINE_1, "(%d lines) "), lct);
		    else
			printf(MSGSTR(LINE_2, "(%d more lines) "), lct);
		}

		if (hdrs) {
			printf(MSGSTR(DASH, "\n-----\n"));
			fclose(newmsg);
			continue;
		}

		/*
		 * Ask user for command
		 */
		if (totty)
		  	ask(lct? MORE : (msg==lastmsg? NOMORE : NEXT));
		else
			inbuf[0] = 'y';
		if (totty)
			signal(SIGTSTP, SIG_DFL);
cmnd:
		in = inbuf;
		switch (*in) {
			case 'x':
			case 'X':
				exit(0);
			case 'q':
			case 'Q':
				quitit = YES;
				printf(MSGSTR(POSTPON, "--Postponed--\n"));
				exit(0);
				/* intentional fall-thru */
			case 'n':
			case 'N':
				if (msg >= nextmsg) 
				    sep = MSGSTR(FLUSHED,"Flushed");
				prevmsg = msg;
				break;

			case 'p':
			case 'P':
				pause_f = (*in++ == 'p');
				/* intentional fallthru */
			case '\n':
			case 'y':
			default:
				if (*in == '-') {
					msg = prevmsg-1;
					sep = MSGSTR(REPLAY,"Replay");
					break;
				}
				if (ISDIGIT(*in)) {
					msg = next(in);
					sep = in;
					break;
				}

				prmesg(nlines + lct + (seensubj? 1 : 0));
				prevmsg = msg;

		}

		printf(MSGSTR(SEP, "--%s--\n"), sep);
		sep = "-";
		if (msg >= nextmsg) {
			nextmsg = msg + 1;
			fseek(msgsrc, 0L, 0);
			fprintf(msgsrc, "%d\n", nextmsg);
			fflush(msgsrc);
		}
		if (newmsg)
			fclose(newmsg);
		if (quitit)
			break;
	}

	/*
	 * Make sure .rc file gets updated
	 */
	if (--msg >= nextmsg) {
		nextmsg = msg + 1;
		fseek(msgsrc, 0L, 0);
		fprintf(msgsrc, "%d\n", nextmsg);
		fflush(msgsrc);
	}
	if (!(already || hush || qopt))
		printf(MSGSTR(NONEW, "No new messages.\n"));

	fclose(msgsrc);
	exit(0);
}

/*
 * NAME: prmesg
 * FUNCTION: print out message
 */
static prmesg(length)
int length;
{
	FILE *outf, *inf;
	int c;

	if (pause_f && length > Lpp) {
		signal(SIGPIPE, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		sprintf(cmdbuf, PAGE, Lpp);
		outf = popen(cmdbuf, "w");
		if (!outf)
			outf = stdout;
		else
			setbuf(outf, NULL);
	}
	else
		outf = stdout;

	if (seensubj)
		putc('\n', outf);

	while (fgets(inbuf, (int)sizeof inbuf, newmsg)) {
		fputs(inbuf, outf);
		if (ferror(outf)) {
			clearerr(outf);
			break;
		}
	}

	if (outf != stdout) {
		pclose(outf);
		signal(SIGPIPE, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}
	else {
		fflush((FILE *)stdout);
	}

	/* trick to force wait on output 
	stty(fileno(stdout), &otty); */
	ioctl(stdout, TIOCSETP, &otty);
}

/*
 * NAME: onintr
 * FUNCTION: to catch incoming signals
 */
static onintr(void)
{
	signal(SIGINT, (void (*)(int))onintr);
	if (mailing)
		unlink(fname);
	if (sending) {
		unlink(fname);
		puts(MSGSTR(KILLED, "--Killed--"));
		exit(1);
	}
	if (printing) {
		putchar('\n');
		if (hdrs)
			exit(0);
		sep = "Interrupt";
		if (newmsg)
			fseek(newmsg, 0L, 2);
		intrpflg = YES;
	}
}

/*
 * NAME: onsusp
 * FUNCTION:  We have just gotten a susp.  Suspend and prepare to resume.
 */
static onsusp(void)
{

	signal(SIGTSTP, SIG_DFL);
	sigsetmask(0);
	kill(0, SIGTSTP);
	signal(SIGTSTP, (void (*)(int))onsusp);
	if (!mailing)
		longjmp(tstpbuf,1);
}

/*
 * NAME: linecnt
 * FUNCTION:  count the number of lines in the message
 */
static linecnt(f)
FILE *f;
{
	off_t oldpos = ftell(f);
	int l = 0;
	char lbuf[BUFSIZ];

	while (fgets(lbuf, (int)sizeof lbuf, f))
		l++;
	clearerr(f);
	fseek(f, oldpos, 0);
	return (l);
}

/*
 * NAME: next
 * FUNCTION:  get next message number 
 */
static next(buf)
char *buf;
{
	int i;
	sscanf(buf, "%d", &i);
	sprintf(buf,MSGSTR(GOTO, "Go to %d"), i);
	return(--i);
}

/*
 * NAME: ask
 * FUNCTION: display prompt and get next command
 */
static ask(prompt)
int prompt;
{
	char	inch, *msgstring;
	int	n, cmsg;
	off_t	oldpos;
	FILE	*cpfrom, *cpto;

	switch (prompt) {
	    case NEXT	: msgstring="Next message? [yq]";
			  break;
	    case MORE	: msgstring="More? [ynq]";
			  break;
	    case NOMORE	: msgstring="(No more) [q] ?";
			  break;
	    default	: msgstring="Unknown message";
	}
	printf(MSGSTR(prompt, msgstring));
	fflush((FILE *)stdout);
	intrpflg = NO;
	gets(inbuf);
	if (intrpflg)
		inbuf[0] = 'x';

	/*
	 * Handle 'mail' and 'save' here.
	 */
	if ((inch = inbuf[0]) == 's' || inch == 'm') {
		if (inbuf[1] == '-')
			cmsg = prevmsg;
		else if (ISDIGIT(inbuf[1]))
			cmsg = atoi(&inbuf[1]);
		else
			cmsg = msg;
		sprintf(fname, "%s/%d", USRMSGS, cmsg);

		oldpos = ftell(newmsg);

		cpfrom = fopen(fname, "r");
		if (!cpfrom) {
			printf(MSGSTR(NOTFOUND, "Message %d not found\n"),cmsg);
			ask (prompt);
			return;
		}

		if (inch == 's') {
			in = nxtfld(inbuf);
			if (*in)
			{
				char *sin = in;
				char *sout = fname;
				wchar_t wc;

				if (mb_cur_max == 1)
					while (*sin > ' ')
						*sout++ = *sin++;
				else
				{
					while (*sin)
					{
						n = mbtowc(&wc,sin,mb_cur_max);
						if(iswspace(wc) || iswcntrl(wc))
						   break;
						else
						   while (n--)
							*sout++ = *sin++;
					}
				}
				*sout = (char)NULL;
			}
			else
				strcpy(fname, "Messages");
		}
		else {
			strcpy(fname, TEMP);
			mktemp(fname);
			sprintf(cmdbuf, MAIL, fname);
			mailing = YES;
		}
		cpto = fopen(fname, "a");
		if (!cpto) {
			perror(fname);
			mailing = NO;
			fseek(newmsg, oldpos, 0);
			ask(prompt);
			return;
		}

		while (n = fread((void *)inbuf, (size_t)1,
					(size_t)sizeof inbuf, cpfrom))
			fwrite((void *)inbuf, (size_t)1, (size_t)n, cpto);

		fclose(cpfrom);
		fclose(cpto);
		fseek(newmsg, oldpos, 0);	/* reposition current message */
		if (inch == 's')
			printf(MSGSTR(SAVED, "Message %d saved in \"%s\"\n"),
				cmsg, fname);
		else {
			system(cmdbuf);
			unlink(fname);
			mailing = NO;
		}
		ask(prompt);
	}
}

/*
 * NAME: gfrsub
 * FUNCTION: get subject and from fields from message
 */
static gfrsub(infile)
FILE *infile;
{
	off_t frompos;

	seensubj = seenfrom = NO;
	local = YES;
	subj[0] = from[0] = date[0] = (char)NULL;

	/*
	 * Is this a normal message?
	 */
	if (fgets(inbuf, (int)sizeof inbuf, infile)) {
		if (strncmp(inbuf, MSGSTR(FROM1,"From"), 4)==0) {
			/*
			 * expected form starts with From
			 */
			seenfrom = YES;
			frompos = ftell(infile);
			ptr = from;
			in = nxtfld(inbuf);
			if (*in) while (*in && *in > ' ') {
				if (*in == ':' || *in == '@' || *in == '!')
					local = NO;
				*ptr++ = *in++;
				/* what about sizeof from ? */
			}
			*ptr = (char)NULL;
			if (*(in = nxtfld(in)))
				strncpy(date, in, sizeof date);
			else {
				date[0] = '\n';
				date[1] = (char)NULL;
			}
		}
		else {
			/*
			 * not the expected form
			 */
			fseek(infile, 0L, 0);
			return;
		}
	}
	else
		/*
		 * empty file ?
		 */
		return;

	/*
	 * look for Subject line until EOF or a blank line
	 */
	while (fgets(inbuf, (int)sizeof inbuf, infile)
	    && !(blankline = (inbuf[0] == '\n'))) {
		/*
		 * extract Subject line
		 */
		if (!seensubj && strncmp(inbuf,MSGSTR(SUB, "Subj"), 4)==0) {
			seensubj = YES;
			frompos = ftell(infile);
			strncpy(subj, nxtfld(inbuf), sizeof subj);
		}
	}
	if (!blankline)
		/*
		 * ran into EOF
		 */
		fseek(infile, frompos, 0);

	if (!seensubj)
		/*
		 * for possible use with Mail
		 */
		strncpy(subj, MSGSTR(NOSUB,"(No Subject)\n"), sizeof subj);
}

/*
 * NAME: nxtfld
 * FUNCTION:  find the next field in the message
 */
static char *
nxtfld(s)
char *s;
{
	int n;		/* # bytes in current multi-byte character */
	wchar_t wc;	/* current character represented as a process code */

	if (mb_cur_max == 1)
	{
		while (*s && *s > ' ') s++; /* skip over this field */
		while (*s && *s <= ' ') s++; /* point at next field */
	}
	else
	{
		while (*s)
		{
			n = mbtowc(&wc, s, mb_cur_max);
			if (iswspace(wc) || iswcntrl(wc))
				break;
			else
				s += n;
		}
		while (*s)
		{
			n = mbtowc(&wc, s, mb_cur_max);
			if (iswspace(wc) || iswcntrl(wc))
				s += n;
			else
				break;
		}
	}
	return (s);
}
