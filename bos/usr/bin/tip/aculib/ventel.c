static char sccsid[] = "@(#)79	1.8  src/bos/usr/bin/tip/aculib/ventel.c, cmdtip, bos411, 9428A410j 6/9/91 14:14:27";
/* 
 * COMPONENT_NAME: UUCP ventel.c
 * 
 * FUNCTIONS: DELAY, MSGSTR, busyloop, delay, echo, gobble, min, 
 *            sigALRM, ven_abort, ven_dialer, ven_disconnect, vensync 
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

/* static char sccsid[] = "ventel.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for calling up on a Ventel Modem
 * The Ventel is expected to be strapped for local echo (just like uucp)
 */
#include "../tip.h"
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

#define	MAXRETRY	5

static	void sigALRM(int);
static	int timeout = 0;
static	jmp_buf timeoutbuf;
static  int echo(register char *);
static  int gobble(register char, char *);
static  int vensync(int);
/*
 * some sleep calls have been replaced by this macro
 * because some ventel modems require two <cr>s in less than
 * a second in order to 'wake up'... yes, it is dirty...
 */
#define delay(num,denom) busyloop(CPUSPEED*num/denom)
#define CPUSPEED 1000000	/* VAX 780 is 1MIPS */
#define	DELAY(n)	{ register long N = (n); while (--N > 0); }
busyloop(n) { DELAY(n); }

ven_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
	char *msg, *index(), line[80];

	/*
	 * Get in synch with a couple of carriage returns
	 */
	if (!vensync(FD)) {
		printf(MSGSTR(CANTSYNC5, "can't synchronize with ventel\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "ventel", MSGSTR(CANTSYNC2, "can't synch up")); /*MSG*/
#endif
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	fflush(stdout);
#ifdef _AIX
	ioctl(FD, TCGETA, &tbuf);
	tbuf.c_cflag |= HUPCL;
	ioctl(FD, TCSETA, &tbuf);
#else
	ioctl(FD, TIOCHPCL, 0);
#endif
	echo("#k$\r$\n$D$I$A$L$:$ ");
	for (cp = num; *cp; cp++) {
		delay(1, 10);
		write(FD, cp, 1);
	}
	delay(1, 10);
	write(FD, "\r", 1);
	gobble('\n', line);
	if (gobble('\n', line))
		connected = gobble('!', line);
#ifdef _AIX
	ioctl(FD, TCFLSH, 2);	/* flush input and output queues */
#else
	ioctl(FD, TIOCFLUSH);
#endif
#ifdef ACULOG
	if (timeout) {
		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "ventel", line);
	}
#endif
	if (timeout)
		ven_disconnect();	/* insurance */
	if (connected || timeout || !boolean(value(VERBOSE)))
		return (connected);
	/* call failed, parse response for user */
	cp = index(line, '\r');
	if (cp)
		*cp = '\0';
	for (cp = line; cp = index(cp, ' '); cp++)
		if (cp[1] == ' ')
			break;
	if (cp) {
		while (*cp == ' ')
			cp++;
		msg = cp;
		while (*cp) {
			if (isupper(*cp))
				*cp = tolower(*cp);
			cp++;
		}
		printf("%s...", msg);
	}
	return (connected);
}

ven_disconnect()
{

	close(FD);
}

ven_abort()
{

	write(FD, "\03", 1);
	close(FD);
}

static int
echo(register char *s)
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

	printf(MSGSTR(TIMEOUT, "\07timeout waiting for reply\n")); /*MSG*/
	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
gobble(	register char match, 
	char response[])
{
	register char *cp = response;
	char c;
	void (*f)(int);

	signal(SIGALRM, sigALRM);
	timeout = 0;
	do {
		if (setjmp(timeoutbuf)) {
			signal(SIGALRM, f);
			*cp = '\0';
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, cp, 1);
		alarm(0);
		c = (*cp++ &= 0177);
#ifdef notdef
		if (boolean(value(VERBOSE)))
			putchar(c);
#endif
	} while (c != '\n' && c != match);
	signal(SIGALRM, SIG_DFL);
	*cp = '\0';
	return (c == match);
}

#define min(a,b)	((a)>(b)?(b):(a))
/*
 * This convoluted piece of code attempts to get
 * the ventel in sync.  If you don't have FIONREAD
 * there are gory ways to simulate this.
 */
static int
vensync(int fd)
{
	int already = 0, nread;
	char buf[60];
#ifdef _AIX
	struct termios tmp;
#endif
	/*
	 * Toggle DTR to force anyone off that might have left
	 * the modem connected, and insure a consistent state
	 * to start from.
	 *
	 * If you don't have the ioctl calls to diddle directly
	 * with DTR, you can always try setting the baud rate to 0.
	 */
#ifdef _AIX
	tmp = tbuf;
	tmp.c_cflag |= (HUPCL | B0);	/* set baud rate to 0 */
	ioctl(FD, TCSETA, &tmp);
	sleep(1);
	ioctl(FD, TCSETAW, &tbuf);	/* restore proper settings */
#else
	ioctl(FD, TIOCCDTR, 0);
	sleep(1);
	ioctl(FD, TIOCSDTR, 0);
#endif
	while (already < MAXRETRY) {
		/*
		 * After reseting the modem, send it two \r's to
		 * autobaud on. Make sure to delay between them
		 * so the modem can frame the incoming characters.
		 */
		write(fd, "\r", 1);
		delay(1,10);
		write(fd, "\r", 1);
		sleep(2);
#ifdef _AIX
		if (ioctl(fd, TIONREAD, &nread) < 0) {
#else
		if (ioctl(fd, FIONREAD, (caddr_t)&nread) < 0) {
#endif
			perror(MSGSTR(TIPIOCTL, "tip: ioctl")); /*MSG*/
			continue;
		}
		while (nread > 0) {
			read(fd, buf, min(nread, 60));
			if ((buf[nread - 1] & 0177) == '$')
				return (1);
			nread -= min(nread, 60);
		}
		sleep(1);
		already++;
	}
	return (0);
}

