static char sccsid[] = "@(#)88	1.7.1.4  src/bos/usr/bin/lock/lock.c, cmdmisc, bos41B, 9504A 1/4/95 14:12:01";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS: lock
 *
 * ORIGINS: 26, 27
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

/*
 * Lock a terminal up until the given key is entered,
 * or until the root password is entered,
 * or the given interval times out.
 *
 * Timeout interval is by default TIMEOUT, it can be changed with
 * an argument of the form -time where time is in minutes
 */

#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/termio.h>
#include <errno.h>

#include "lock_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LOCK,n,s) 

#define TIMEOUT 15

int	quit(void);
int	bye(void);
int	hi(void);

static struct timeval	timeout	= {0, 0};
static struct timeval	zerotime = {0, 0};
static struct termio 	tty, ntty;
static long	nexttime;		/* keep the timeout time */

main(argc, argv)
	int argc;
	char **argv;
{
	char	*ttynam;
	char	*ap;
	int	sectimeout = TIMEOUT;
	char	s[BUFSIZ], s1[BUFSIZ];
	char	hostname[32];
	char date_buffer[100];
	struct timeval	timval;
	struct itimerval	ntimer, otimer;
	struct timezone	timzone;
	struct tm	*timp;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_LOCK, NL_CAT_LOCALE);

	/* process arguments */

	if (argc > 1){
		if (argv[1][0] != '-')
			usage();
		if (sscanf(&(argv[1][1]), "%d", &sectimeout) != 1)
			usage();
	}
	if (sectimeout < 0)
		usage();
	timeout.tv_sec = sectimeout * 60;

	/* get information for header */

	if (ioctl(0, TCGETA, &tty))
		exit(1);
	gethostname(hostname, sizeof(hostname));
	if (!(ttynam = ttyname(0))){
		fprintf(stderr, MSGSTR(LNOTTERM, "lock: not a terminal?\n"));
		exit (1);
	}
	gettimeofday(&timval, &timzone);
	nexttime = timval.tv_sec + (sectimeout * 60);
	timp = localtime((time_t *)&timval.tv_sec);

	/* get key and check again */

	signal(SIGINT,(void (*)(int)) quit);
	signal(SIGQUIT, (void (*)(int))quit);
	ntty = tty; ntty.c_lflag &= ~ECHO;
	ioctl(0, TCSETA, &ntty);
	printf(MSGSTR(LKEY, "Key: "));
	if (fgets(s, (int)sizeof(s), stdin) == NULL) {
		putchar('\n');
		quit();
	}
	printf(MSGSTR(LAGAIN, "\nAgain: "));
	/*
	 * Don't need EOF test here, if we get EOF, then s1 != s
	 * and the right things will happen.
	 */
	(void) fgets(s1, (int)sizeof(s1), stdin);
	putchar('\n');
	if (strcmp(s1, s)) {
		putchar(07);
		ioctl(0, TCSETA, &tty);
		exit(1);
	}
	s[0] = 0;

	/* Set signal handlers */

	signal(SIGINT, (void (*)(int))hi);
	signal(SIGQUIT, (void (*)(int))hi);
	signal(SIGTSTP, (void (*)(int))hi);
	signal(SIGALRM, (void (*)(int))bye);
	ntimer.it_interval = zerotime;
	ntimer.it_value = timeout;
	setitimer(ITIMER_REAL, &ntimer, &otimer);

	/* Header info */

	printf (MSGSTR(LWHENTIMEOUT, "lock: %s on %s. timeout in %d minutes\n"),
		ttynam, hostname, sectimeout);
	strftime(date_buffer, (size_t)sizeof(date_buffer), "%c", timp);
	printf(MSGSTR(LTIMENOWIS, "time now is %s\n"), date_buffer);

	/* wait */

	for (;;) {
		printf(MSGSTR(LKEY, "Key: "));
		if (fgets(s, (int)sizeof(s), stdin) == NULL) {
			clearerr(stdin);
			hi();
			continue;
		}
		if (strcmp(s1, s) == 0)
			break;
		printf("\07\n");
		if (ioctl(0, TCGETA, &ntty))
			exit(1);
	}
	ioctl(0, TCSETA, &tty);
	putchar('\n');
	exit (0);
}

/*
 * NAME: usage
 * FUNCTION: displays the usage statement to the user
 */
static usage()
{
	fprintf(stderr, MSGSTR(LUSAGE, "Usage: lock [-timeout]\n"));
	exit (1);
}

/*
 * NAME: quit
 * FUNCTION: get out of here
 */
static quit(void)
{
	ioctl(0, TCSETA, &tty);
	exit (0);
}

/*
 * NAME: bye
 * FUNCTION: inform user time has run out and exit
 */
static bye(void)
{
	ioctl(0, TCSETA, &tty);
	printf(MSGSTR(LLOCKTIMOUT, "lock: timeout\n"));
	kill (getppid(),SIGHUP);
	exit (1);
}

/*
 * NAME: hi
 * FUCNTION: tell the user we are waiting
 */
static hi(void)
{
	long	curtime;
	struct timeval	timval;
	struct timezone	timzone;

	signal(SIGINT, (void (*)(int))hi);
	signal(SIGQUIT, (void (*)(int))hi);
	signal(SIGTSTP, (void (*)(int))hi);

	gettimeofday(&timval, &timzone);
	curtime = timval.tv_sec;
	printf(MSGSTR(LTYPINULK, "lock: type in the unlock key. timeout in %d minutes\n"),
		(nexttime-curtime)/60);
}
