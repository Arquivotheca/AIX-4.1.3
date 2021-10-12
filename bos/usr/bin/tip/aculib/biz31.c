static char sccsid[] = "@(#)72	1.5  src/bos/usr/bin/tip/aculib/biz31.c, cmdtip, bos411, 9428A410j 4/10/91 09:12:35";
/* 
 * COMPONENT_NAME: UUCP biz31.c
 * 
 * FUNCTIONS: MSGSTR, biz31_abort, biz31_disconnect, biz31f_dialer, 
 *            biz31w_dialer, biz_dialer, bizsync, detect, echo, flush, 
 *            sigALRM 
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

/* static char sccsid[] = "biz31.c	5.1 (Berkeley) 6/6/85"; */

#include "../tip.h"
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

#define MAXRETRY	3		/* sync up retry count */
#define DISCONNECT_CMD	"\21\25\11\24"	/* disconnection string */

static	void sigALRM(int);
static	int timeout = 0;
static	jmp_buf timeoutbuf;
static  int detect(register char *);
static  int echo(register char *);
static  int flush(register char *);
static  int bizsync(int);
/*
 * Dial up on a BIZCOMP Model 1031 with either
 * 	tone dialing (mod = "f")
 *	pulse dialing (mod = "w")
 */
static int
biz_dialer(num, mod)
	char *num, *mod;
{
	register int connected = 0;

	if (!bizsync(FD)) {
		logent(value(HOST), "", "biz", MSGSTR(OUTOFSYNC, "out of sync")); /*MSG*/
		printf(MSGSTR(OUTOFSYNC2, "bizcomp out of sync\n")); /*MSG*/
		delock(uucplock);
		exit(0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(STARTCALL, "\nstarting call...")); /*MSG*/
	echo("#\rk$\r$\n");			/* disable auto-answer */
	echo("$>$.$ #\r");			/* tone/pulse dialing */
	echo(mod);
	echo("$\r$\n");
	echo("$>$.$ #\re$ ");			/* disconnection sequence */
	echo(DISCONNECT_CMD);
	echo("\r$\n$\r$\n");
	echo("$>$.$ #\rr$ ");			/* repeat dial */
	echo(num);
	echo("\r$\n");
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(RINGING, "ringing...")); /*MSG*/
	/*
	 * The reply from the BIZCOMP should be:
	 *	`^G NO CONNECTION\r\n^G\r\n'	failure
	 *	` CONNECTION\r\n^G'		success
	 */
	connected = detect(" ");
#ifdef ACULOG
	if (timeout) {
		char line[80];

		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "biz", line);
	}
#endif
	if (!connected)
		flush(MSGSTR(NOCONNECT, " NO CONNECTION\r\n\07\r\n")); /*MSG*/
	else
		flush(MSGSTR(CONNECTION, "CONNECTION\r\n\07")); /*MSG*/
	if (timeout)
		biz31_disconnect();	/* insurance */
	return (connected);
}

biz31w_dialer(num, acu)
	char *num, *acu;
{

	return (biz_dialer(num, "w"));
}

biz31f_dialer(num, acu)
	char *num, *acu;
{

	return (biz_dialer(num, "f"));
}

#undef sleep
biz31_disconnect()
{

	write(FD, DISCONNECT_CMD, 4);
	sleep(2);
#ifdef _AIX
	ioctl(FD, TCFLSH, FWRITE); /* probably wants to flush output queue */
#else
	ioctl(FD, TIOCFLUSH);	/* missing last argument - bug? */
#endif
}

biz31_abort()
{

	write(FD, "\33", 1);
}

static int
echo(	register char *s)
{
	char c;

	while (c = *s++) switch (c) {

	case '$':
		read(FD, &c, 1);
		s++;
		break;

	case '#':
		c = *s++;
		write(FD, &c, 1);
		break;

	default:
		write(FD, &c, 1);
		read(FD, &c, 1);
	}
}

static void
sigALRM(int s)
{

	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
detect(	register char *s)
{
	char c;
	void (*f)(int);

	f = signal(SIGALRM, sigALRM);
	timeout = 0;
	while (*s) {
		if (setjmp(timeoutbuf)) {
			printf(MSGSTR(TIMEOUT, "\07timeout waiting for reply\n")); /*MSG*/
			biz31_abort();
			break;
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		if (c != *s++)
			break;
	}
	signal(SIGALRM, f);
	return (timeout == 0);
}

static int
flush(	register char *s)
{
	char c;
	void (*f)(int);

	f = signal(SIGALRM, sigALRM);
	while (*s++) {
		if (setjmp(timeoutbuf))
			break;
		alarm(10);
		read(FD, &c, 1);
		alarm(0);
	}
	signal(SIGALRM, f);
	timeout = 0;			/* guard against disconnection */
}

/*
 * This convoluted piece of code attempts to get
 *  the bizcomp in sync.  If you don't have the capacity or nread
 *  call there are gory ways to simulate this.
 */
static int
bizsync(int fd)
{
#ifdef FIOCAPACITY
	struct capacity b;
#	define chars(b)	((b).cp_nbytes)
#	define IOCTL	FIOCAPACITY
#endif
#ifdef FIONREAD
	long b;
#	define chars(b)	(b)
#	define IOCTL	FIONREAD
#endif
	register int already = 0;
	char buf[10];

retry:
#ifdef _AIX
	if (ioctl(fd, TIONREAD, (int)&b) >= 0 && chars(b) > 0);
		ioctl(fd, TCFLSH, FWRITE); /* flush output queue */
#else
	if (ioctl(fd, IOCTL, (caddr_t)&b) >= 0 && chars(b) > 0)
		ioctl(fd, TIOCFLUSH);
#endif
	write(fd, "\rp>\r", 4);
	sleep(1);
#ifdef _AIX
	if (ioctl(fd, TIONREAD, (int)&b) >= 0)  {
#else
	if (ioctl(fd, IOCTL, (caddr_t)&b) >= 0) {
#endif
		if (chars(b) != 10) {
	nono:
			if (already > MAXRETRY)
				return (0);
			write(fd, DISCONNECT_CMD, 4);
			sleep(2);
			already++;
			goto retry;
		} else {
			read(fd, buf, 10);
			if (strncmp(buf, "p >\r\n\r\n>", 8))
				goto nono;
		}
	}
	return (1);
}
