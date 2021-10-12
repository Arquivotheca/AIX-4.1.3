static char sccsid[] = "@(#)75	1.6  src/bos/usr/bin/tip/aculib/dn11.c, cmdtip, bos411, 9428A410j 6/9/91 14:13:50";
/* 
 * COMPONENT_NAME: UUCP dn11.c
 * 
 * FUNCTIONS: MSGSTR, alarmtr, dn_abort, dn_dialer, dn_disconnect 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "dn11.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for dialing up on DN-11
 */
#include "../tip.h"
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

int dn_abort();
void alarmtr(int);
static jmp_buf jmpbuf;
static int child = -1, dn;
#ifdef _AIX
static struct termios tmp;
#endif

dn_dialer(num, acu)
	char *num, *acu;
{
	extern errno;
	char *p, *q, phone[40];
	int lt, nw, connected = 1;
	register int timelim;

	if (boolean(value(VERBOSE)))
		printf(MSGSTR(STARTCALL, "\nstarting call...")); /*MSG*/
	if ((dn = open(acu, 1)) < 0) {
		if (errno == EBUSY)
			printf(MSGSTR(LINEBUSY, "line busy...")); /*MSG*/
		else
			printf(MSGSTR(ACUERR, "acu open error...")); /*MSG*/
		return (0);
	}
	if (setjmp(jmpbuf)) {
		kill(child, SIGKILL);
		close(dn);
		return (0);
	}
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(num);
	alarm(timelim < 30 ? 30 : timelim);
	if ((child = fork()) == 0) {
		/*
		 * ignore this stuff for aborts
		 */
		signal(SIGALRM, SIG_IGN);
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		sleep(2);
		nw = write(dn, num, lt = strlen(num));
		exit(nw != lt);
	}
	/*
	 * open line - will return on carrier
	 */
	if ((FD = open(DV, 2)) < 0) {
		if (errno == EIO)
			printf(MSGSTR(LOSTCARRIER, "lost carrier...")); /*MSG*/
		else
			printf(MSGSTR(LINEFAILED, "dialup line open failed...")); /*MSG*/
		alarm(0);
		kill(child, SIGKILL);
		close(dn);
		return (0);
	}
	alarm(0);
#ifdef _AIX
	ioctl(dn, TCGETA, &tbuf);
	tbuf.c_cflag |= HUPCL;
	ioctl(dn, TCSETA, &tbuf);
#else
	ioctl(dn, TIOCHPCL, 0);
#endif
	signal(SIGALRM, SIG_DFL);
	while ((nw = wait(&lt)) != child && nw != -1)
		;
	fflush(stdout);
	close(dn);
	if (lt != 0) {
		close(FD);
		return (0);
	}
	return (1);
}

void alarmtr(int s)
{

	alarm(0);
	longjmp(jmpbuf, 1);
}

/*
 * Insurance, for some reason we don't seem to be
 *  hanging up...
 */
dn_disconnect()
{

	sleep(2);
	if (FD > 0)
#ifdef _AIX
		tmp = tbuf;
		tmp.c_cflag |= (HUPCL | B0);	/* set Baud rate to 0 */
		ioctl(FD, TCSETA, &tmp);
		ioctl(FD, TCSETAW, &tbuf); /* restore proper values */
#else
		ioctl(FD, TIOCCDTR, 0);
#endif
	close(FD);
}

dn_abort()
{

	sleep(2);
	if (child > 0)
		kill(child, SIGKILL);
	if (dn > 0)
		close(dn);
	if (FD > 0)
#ifdef _AIX
		tmp = tbuf;
		tmp.c_cflag |= (HUPCL | B0);	/* set Baud rate to 0 */
		ioctl(FD, TCSETA, &tmp);
		ioctl(FD, TCSETAW, &tbuf); /* restore proper values */
#else
		ioctl(FD, TIOCCDTR, 0);
#endif
	close(FD);
}
