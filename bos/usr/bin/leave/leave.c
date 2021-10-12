static char sccsid[] = "@(#)23	1.15  src/bos/usr/bin/leave/leave.c, cmdmisc, bos41B, 9504A 1/4/95 14:11:54";
/*
 * COMPONENT_NAME: (CMDMISC) miscellaneous commands
 *
 * FUNCTIONS: leave
 *
 * ORIGINS: 9, 26, 27, 71
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
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 * OSF/1 1.1
 */
 
#include <stdio.h>
#include <locale.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
/*
 * leave [[+]hhmm]
 *
 * Reminds you when you have to leave.
 * Leave prompts for input and goes away if you hit return.
 * It nags you like a mother hen.
 */
static char buff[100];

#include "leave_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEAVE,n,s) 

main(argc, argv)
char **argv;
{

	long when, tod, now, diff, hours, minutes;
	char *cp;
	int *nv;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_LEAVE, NL_CAT_LOCALE);
	if (argc < 2) {
		printf(MSGSTR(WHEN, "At what time do you need to leave?\n")); /*MSG*/
		if ((cp = fgets(buff, sizeof(buff), stdin)) == '\0') {
			printf("\n");
			exit(1);
		}
		if (*cp == '\n')
			exit(0);
	} else
		cp = argv[1];
	if (*cp == '+') {
		cp++;
		if (*cp < '0' || *cp > '9')
			usage();
		tod = strtoul(cp, &cp, 10);
		if (*cp != '\0')
			usage();
		/* shift 2 digits to get hours */
		hours = tod / 100;
		/* extract last two digits for minutes */
		minutes = tod % 100;
		if (minutes < 0 || minutes > 59)
			usage();
		diff = 60*hours+minutes;
		doalarm(diff);
		exit(0);
	}
	if (*cp < '0' || *cp > '9')
		usage();
	tod = strtoul(cp, &cp, 10);
	if (*cp != '\0')
		usage();
	hours = tod / 100;
	if (hours > 12)
		hours -= 12;
	if (hours == 12)
		hours = 0;
	minutes = tod % 100;

	if (hours < 0 || hours > 12 || minutes < 0 || minutes > 59)
		usage();

	time(&now);
	nv = (int *)localtime((time_t *)&now);
	when = 60*hours+minutes;
	if (nv[2] > 12)
		nv[2] -= 12;	/* do am/pm bit */
	now = 60*nv[2] + nv[1];
	diff = when - now;
	while (diff < 0)
		diff += 12*60;
	doalarm(diff);
	exit(0);
}

/*
 * NAME: usage
 * FUNCTION: displays the usage statement to the user
 */
static usage()
{
	fprintf(stderr,MSGSTR(USAGE, "usage: leave [[+]hhmm]\n")); /*MSG*/
	exit(1);
}

/*
 * NAME: doalarm
 * FUNCTION: send user messages at the proper time
 */
static doalarm(nmins)
long nmins;
{
	char *msg1, *msg2, *msg3, *msg4;
	register int i;
	int slp1, slp2, slp3, slp4;
	int seconds, gseconds;
	long daytime;
	char whenleave[NLTBMAX];

	seconds = 60 * nmins;
	if (seconds <= 0)
		seconds = 1;
	gseconds = seconds;

	msg1 = MSGSTR(FIVE, "\7\7\7You have to leave in 5 minutes.\n"); /*MSG*/
	if (seconds <= 60*5) {
		slp1 = 0;
	} else {
		slp1 = seconds - 60*5;
		seconds = 60*5;
	}

	msg2 = MSGSTR(ONE, "\7\7\7Just one more minute!\n"); /*MSG*/
	if (seconds <= 60) {
		slp2 = 0;
	} else {
		slp2 = seconds - 60;
		seconds = 60;
	}

	msg3 = MSGSTR(TIME, "\7\7\7It is time to leave!\n"); /*MSG*/
	slp3 = seconds;

	msg4 = MSGSTR(LATE, "\7\7\7You are going to be late!\n"); /*MSG*/
	slp4 = 60;

	time(&daytime);
	daytime += gseconds;
	if (strftime(whenleave, NLTBMAX, "%c", localtime(&daytime)))
		printf(MSGSTR(SET,"The alarm is set for %s.\n"),whenleave);
	if (fork())
		exit(0);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	if (slp1)
		bother(slp1, msg1);
	if (slp2)
		bother(slp2, msg2);
	bother(slp3, msg3);
	for (i = 0; i < 10; i++)
		bother(slp4, msg4);
	printf(MSGSTR(BYE, "This is the last notice! Bye.\n"));  /*MSG*/
	exit(0);
}

/*
 * NAME: bother
 * FUNCTION: wait slp seconds and then beep user (sound bell)
 */
static bother(slp, msg)
int slp;
char *msg;
{
	int len = strlen(msg);
	delay(slp);
	/*
	 * if write fails, we've lost the terminal through someone else
	 * causing a vhangup by logging in.
	 */
	if (write(1, msg, len) != len)
		exit(0);
}

/*
 * delay is like sleep but does it in 100 sec pieces and
 * knows what zero means.
 */
static delay(secs)
int secs;
{
	int n;

	while (secs > 0) {
		n = 100;
		if (secs < n)
			n = secs;
		secs -= n;
		if (n > 0)
			sleep((unsigned)n);
	}
}
