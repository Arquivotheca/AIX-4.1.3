static char sccsid[] = "@(#)71	1.7  src/bos/usr/bin/tip/aculib/biz22.c, cmdtip, bos411, 9428A410j 4/10/91 09:12:34";
/* 
 * COMPONENT_NAME: UUCP biz22.c
 * 
 * FUNCTIONS: MSGSTR, biz22_abort, biz22_disconnect, biz22f_dialer, 
 *            biz22w_dialer, biz_dialer, cmd, detect, sigALRM 
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

/* static char sccsid[] = "biz22.c	5.1 (Berkeley) 6/6/85"; */

#include "../tip.h"
#include "../tip_msg.h"
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

#define DISCONNECT_CMD	"\20\04"	/* disconnection string */
static  int cmd(register char *);
static  int detect(register char *);
static	void sigALRM(int);
static	int timeout = 0;
static	jmp_buf timeoutbuf;
/*
 * Dial up on a BIZCOMP Model 1022 with either
 * 	tone dialing (mod = "V")
 *	pulse dialing (mod = "W")
 */
static int
biz_dialer(num, mod)
	char *num, *mod;
{
	register int connected = 0;
	char cbuf[40];

	if (boolean(value(VERBOSE)))
		printf(MSGSTR(STARTCALL, "\nstarting call...")); /*MSG*/
	/*
	 * Disable auto-answer and configure for tone/pulse
	 *  dialing
	 */
	if (cmd("\02K\r")) {
		printf(MSGSTR(CANTINIT, "can't initialize bizcomp...")); /*MSG*/
		return (0);
	}
	strcpy(cbuf, "\02.\r");
	cbuf[1] = *mod;
	if (cmd(cbuf)) {
		printf(MSGSTR(CANTSET, "can't set dialing mode...")); /*MSG*/
		return (0);
	}
	strcpy(cbuf, "\02D");
	strcat(cbuf, num);
	strcat(cbuf, "\r");
	write(FD, cbuf, strlen(cbuf));
	if (!detect("7\r")) {
		printf(MSGSTR(CANTGET, "can't get dial tone...")); /*MSG*/
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(RINGING, "ringing...")); /*MSG*/
	/*
	 * The reply from the BIZCOMP should be:
	 *	2 \r or 7 \r	failure
	 *	1 \r		success
	 */
	connected = detect("1\r");
#ifdef ACULOG
	if (timeout) {
		char line[80];

		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "biz1022", line);
	}
#endif
	if (timeout)
		biz22_disconnect();	/* insurance */
	return (connected);
}

biz22w_dialer(num, acu)
	char *num, *acu;
{

	return (biz_dialer(num, "W"));
}

biz22f_dialer(num, acu)
	char *num, *acu;
{

	return (biz_dialer(num, "V"));
}

biz22_disconnect()
{
	int rw = 2;

	write(FD, DISCONNECT_CMD, 4);
#undef sleep
	sleep(2);
#ifdef _AIX
	ioctl(FD, TCFLSH, FWRITE);	/* flush output queue */
#else
	ioctl(FD, TIOCFLUSH, &rw);
#endif
}

biz22_abort()
{

	write(FD, "\02", 1);
}

static void
sigALRM(int s)
{

	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
cmd(register char *s)
{
	char c;
	void (*f)(int);

	write(FD, s, strlen(s));
	f = signal(SIGALRM, sigALRM);
	if (setjmp(timeoutbuf)) {
		biz22_abort();
		signal(SIGALRM, f);
		return (1);
	}
	alarm(number(value(DIALTIMEOUT)));
	read(FD, &c, 1);
	alarm(0);
	signal(SIGALRM, f);
	c &= 0177;
	return (c != '\r');
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
			biz22_abort();
			break;
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
		if (c != *s++)
			return (0);
	}
	signal(SIGALRM, f);
	return (timeout == 0);
}









