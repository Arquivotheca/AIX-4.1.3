static char sccsid[] = "@(#)62  1.18  src/bos/usr/bin/mail/main.c, cmdmailx, bos41J, 9523A_all 6/5/95 14:27:23";
/* 
 * COMPONENT_NAME: CMDMAILX main.c
 * 
 * FUNCTIONS: MSGSTR, Mmain, hdrstop, usage
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
 * static char *sccsid = "main.c       5.3 (Berkeley) 9/15/85";
 * #endif not lint
 */

#include <unistd.h>
#include "rcv.h"
#include <sys/stat.h>
#include <sys/syspest.h>
#include <locale.h>

#include "mail_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/* define BUGLPR variable */
BUGVDEF(mailbug, 0)


/*
 * Mail -- a mail program
 *
 * Startup -- interface with user.
 */

int		_open_max;
struct	sstack	*sstack;
jmp_buf		hdrjmp;
short		file_recip;
char		*recip_name;

void
usage(prognm) 
	char * prognm;
{
	fprintf(stderr, MSGSTR(USAGE2,
		"Usage: %s [-h number][-r address][-s subject][-c user(s)] user...\n"), prognm);
	fprintf(stderr, MSGSTR(USAGE3, 
		"    %s [-deFHinNv][-f file][-u user]\n"), prognm);
	exit(1);
}

/*
 * Find out who the user is, copy his mail file (if exists) into
 * /tmp/Rxxxxx and set up the message pointers.  Then, print out the
 * message headers and read user commands.
 *
 * Command line syntax:
 *	Mail [ -i ] [ -r address ] [ -h number ] [ -f [ name ] ]
 * or:
 *	Mail [ -i ] [ -r address ] [ -h number ] people ...
 */
void hdrstop(int), (*prevint)(int);

main(argc, argv)
	char **argv;
{
	register char *ef;
	register int i, argp;
	int mustsend, uflag, f;
	FILE *ibuf, *ftat;
	struct sgttyb tbuf;
	short header_only = FALSE;
	char *progname;
	
        setlocale(LC_ALL,"");
	catd = catopen(MF_MAIL,NL_CAT_LOCALE);
	file_recip = FALSE;

	if ((_open_max = sysconf(_SC_OPEN_MAX)) == -1) {
		perror("Mail: sysconf error");
		exit(1);
	}
	if ((sstack = (struct sstack *)malloc(
		_open_max * (sizeof(struct sstack)))) == NULL) {
		perror("Mail: no memory");
		exit(1);
	}

	/*
	 * Set up a reasonable environment.  We clobber the last
	 * element of argument list for compatibility with version 6,
	 * figure out whether we are being run interactively, set up
	 * all the temporary files, buffer standard output, and so forth.
	 */

	uflag = 0;
#ifdef	GETHOST
	inithost();
#endif	GETHOST
	mypid = getpid();
	intty = isatty(0);
	outtty = isatty(1);
	if (outtty) {
		gtty(1, &tbuf);
		baud = tbuf.sg_ospeed;
	}
	else
		baud = B9600;
	image = -1;

	/*
	 * Now, determine how we are being used.
	 * We successively pick off instances of -r, -h, -f, and -i.
	 * If called as "rmail" we note this fact for letter sending.
	 * If there is anything left, it is the base of the list
	 * of users to mail to.  Argp will be set to point to the
	 * first of these users.
	 */

	ef = NOSTR;
	argp = -1;
	mustsend = 0;
	if (argc > 0 && **argv == 'r')
		rmail++;
	if ((progname = strrchr(argv[0], '/')) == NULL)
		progname = argv[0];
	else
		progname;

	while (( i = getopt(argc, argv, "eHFr:T:u:idh:s:c:fnNvI")) != -1) {
		switch (i) {
		case 'e':
			/*
			 * Test for presence of mail, returning 0 if
			 * the user has mail and 1 if the user does not.
			 */
			mail_test = TRUE;
			break;

		case 'H':
			/*
			 * Print header summary only.
			 */
			header_only = TRUE;
			break;

		case 'F':
			/*
			 * Record the message in a file named after the
			 * first recipient. This option overrides the 
			 * record variable, if it is set.
			 */
			file_recip = TRUE;
			break;

		case 'r':
			/*
			 * Next argument is address to be sent along
			 * to the mailer.
			 */
			mustsend++;
			rflag = optarg;
			break;

		case 'T':
			/*
			 * Next argument is temp file to write which
			 * articles have been read/deleted for netnews.
			 */
			Tflag = optarg;
			if ((f = creat(Tflag, 0600)) < 0) {
				perror(Tflag);
				exit(1);
			}
			close(f);
			break;

		case 'u':
			/*
			 * Next argument is person to pretend to be.
			 */
			uflag++;
			strcpy(myname, optarg);
			break;

		case 'i':
			/*
			 * User wants to ignore interrupts.
			 * Set the variable "ignore"
			 */
			assign("ignore", "");
			break;

		case 'd':
			debug++;
			break;

		case 'h':
			/*
			 * Specified sequence number for network.
			 * This is the number of "hops" made so
			 * far (count of times message has been
			 * forwarded) to help avoid infinite mail loops.
			 */
			mustsend++;
			hflag = atoi(optarg);
			if (hflag == 0) {
				fprintf(stderr, MSGSTR(NEEDNOZ, "-h needs non-zero number\n")); /*MSG*/
				exit(1);
			}
			break;

		case 's':
			/*
			 * Give a subject field for sending from
			 * non terminal
			 */
			mustsend++;
			sflag = optarg;
			break;

		case 'c':
			/*
			 * Give a carbon copy field for sending from 
			 * non terminal
			 */
			mustsend++;
			ccflag = optarg;
			break;

		case 'f':
			/*
			 * User is specifying file to "edit" with Mail,
			 * as opposed to reading system mailbox.
			 * If no argument is given after -f, we read his
			 * mbox file in his home directory.
			 *
			 * getopt() can't handle optional arguments, so here
			 * is an ugly hack to get around it.
			 */
			if ((argv[optind]) && (argv[optind][0] != '-'))
				ef = argv[optind++];
			else
				ef = mbox;
			break;

		case 'n':
			/*
			 * User doesn't want to source /usr/lib/Mail.rc
			 */
			nosrc++;
			break;

		case 'N':
			/*
			 * Avoid initial header printing.
			 */
			noheader++;
			break;

		case 'v':
			/*
			 * Send mailer verbose flag
			 */
			assign("verbose", "");
			break;

		case 'I':
			/*
			 * We're interactive
			 */
			intty = 1;
			break;

		case '?':
			/* missing argument */
			switch (optopt) {
			case 'r':
				fprintf(stderr,
				MSGSTR(NEEDADR, "Address required after -r\n"));
				break;
			case 'T':
				fprintf(stderr,
				MSGSTR(NEEDNM, "Name required after -T\n"));
				break;
			case 'u':
				fprintf(stderr, 
				MSGSTR(NONAME, "Missing user name for -u\n"));
				break;
			case 'h':
				fprintf(stderr, 
				MSGSTR(NEEDNUM, "Number required for -h\n"));
				break;
			case 's':
				fprintf(stderr, 
				MSGSTR(NEEDSUBJ, "Subject required for -s\n"));
				break;
			case 'c':
				fprintf(stderr,
				MSGSTR(NEEDCC, "Address required with -c\n"));
				break;
			}
			exit(1);

		default:
			usage (progname);
		}
	}

	argv += optind;
	argc -= optind;

	if (file_recip == TRUE)
		recip_name = argv[0];	/* first recipient (in send mode) */

	/*
	 * Check for inconsistent arguments.
	 */

	if ((uflag != NOSTR || ef != NOSTR) && argc > 0) {
		fprintf(stderr, MSGSTR(NOWORK, "Cannot give this flag and people to send to.\n")); /*MSG*/
		exit(1);
	}
	if (mustsend && argc == 0) {
		fprintf(stderr, MSGSTR(NOSENSE, "The flags you gave make no sense since you're not sending mail.\n")); /*MSG*/
		exit(1);
	}
	tinit();
	input = stdin;
	rcvmode = (argc == 0);
	if (!nosrc)
		mail_load(MASTER);
	mail_load(mailrc);
	findmail();  /* set up maildir */
	if (!debug && value("debug") != NOSTR) debug++;
	if (debug) {
		fprintf(stderr, "uid = %d, user = %s, mailname = %s\n",
		    uid, myname, mailname);
		fprintf(stderr, "deadletter = %s, mailrc = %s, mbox = %s\n",
		    deadletter, mailrc, mbox);
	}
	if (!rcvmode) {
		mail(argv);

		/*
		 * why wait?
		 */

		exit(senderr);
	}

	/*
	 * Ok, we are reading mail.
	 * Decide whether we are editing a mailbox or reading
	 * the system mailbox, and open up the right stuff.
	 */

	if (ef != NOSTR) {
		char *ename;

		edit++;
		ename = expand(ef);
		if (ename != ef) {
			if ((ef = (char *)calloc(1, strlen(ename)+1)) == NULL) {
				fprintf(stderr, MSGSTR(MEMERR3, "Internal error, calloc() failed.\n"));
				exit(1);
			}
			strcpy(ef, ename);
		}
		editfile = ef;
		strcpy(mailname, ef);
	}
	if (setfile(mailname, edit) < 0) 
		exit (1);	/* error already reported */

	if (mail_test == TRUE)
		return(0);

	if (!noheader && value("noheader") == NOSTR) {
		if (setjmp(hdrjmp) == 0) {
			if ((prevint = signal(SIGINT, SIG_IGN)) != SIG_IGN)
				signal(SIGINT, hdrstop);
			announce(!0);
			fflush(stdout);
			signal(SIGINT, (void(*)(int))prevint);
		}
	}

	if (header_only == TRUE)
		return(0);

	if (!edit && msgCount == 0) {
		printf(MSGSTR(NMAIL, "No mail\n")); /*MSG*/
		fflush(stdout);
		exit(0);
	}
	commands();
	if (!edit) {
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		quit();
	}
	exit(0);
}

/*
 * Interrupt printing of the headers.
 */
void
hdrstop(int s)
{
	fflush(stdout);
	fprintf(stderr, MSGSTR(INTERPT, "\nInterrupt\n")); /*MSG*/
	longjmp(hdrjmp, 1);
}
