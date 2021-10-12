static char sccsid[] = "@(#)73	1.9  src/bos/usr/bin/tip/aculib/courier.c, cmdtip, bos411, 9428A410j 1/15/93 08:55:03";
/* 
 * COMPONENT_NAME: cmdtip courier.c
 * 
 * FUNCTIONS: MSGSTR, cour_abort, cour_connect, cour_dialer, 
 *            cour_disconnect, cour_nap, cour_napx, cour_swallow, 
 *            cour_write, coursync, mask, setvec, sigALRM, 
 *            verbose_read 
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
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* static char sccsid[] = "courier.c	5.1 (Berkeley) 4/3/86"; */

/*
 * Routines for calling up on a Hayes Smartmodem
 */
#include "../tip.h"
#include <stdio.h>
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

#define	MAXRETRY	5

static	void  sigALRM(int);
static	int timeout = 0;
static	jmp_buf timeoutbuf, intbuf;
static	int (*osigint)();
static  int cour_connect();
static  int coursync();
static  cour_napx(void);

cour_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
#ifdef ACULOG
	char line[80];
#endif
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(USING, "Using \"%s\"\n"), acu); /*MSG*/

#ifdef _AIX
	ioctl(FD, TCGETA, &tbuf);
	tbuf.c_cflag |= HUPCL;
	ioctl(FD, TCSETAW, &tbuf);
#else
	ioctl(FD, TIOCHPCL, 0);
#endif
	/*
	 * Get in synch.
	 */
	if (!coursync()) {
		printf(MSGSTR(CANTSYNC, "can't synchronize with courier\n")); /*MSG*/
#ifdef ACULOG
		logent(value(HOST), num, "courier", MSGSTR(CANTSYNC2, "can't synch up")); /*MSG*/
#endif
		return (0);
	}
	fflush(stdout);
	cour_write(FD, "AT D", 4);
	for (cp = num; *cp; cp++)
		if (*cp == '=')
			*cp = ',';
	cour_write(FD, num, strlen(num));
	cour_write(FD, "\r", 1);
	connected = cour_connect();
#ifdef ACULOG
	if (timeout) {
		sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
			number(value(DIALTIMEOUT)));
		logent(value(HOST), num, "cour", line);
	}
#endif
	if (timeout)
		cour_disconnect();
	return (connected);
}

cour_disconnect()
  {
	close(FD);
}

cour_abort()
  {
	cour_write(FD, "\rAT Z\r", 6);
	close(FD);
}

static void
sigALRM(int s)
{
	printf(MSGSTR(TIMEOUT, "\07timeout waiting for reply\n")); /*MSG*/
	timeout = 1;
	longjmp(timeoutbuf, 1);
}

static int
cour_swallow( register char *match)
  {
	char c;
	void (*f)(int);

	f = signal(SIGALRM, sigALRM);
	timeout = 0;
	do {
		if (*match =='\0') {
			signal(SIGALRM, f);
			return 1;
		}
		if (setjmp(timeoutbuf)) {
			signal(SIGALRM, f);
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
		if (boolean(value(VERBOSE)))
			putchar(c);
	} while (c == *match++);
	if (boolean(value(VERBOSE)))
		fflush(stdout);
	signal(SIGALRM, SIG_DFL);
	return (0);
}

struct baud_msg {
	char *msg;
	int baud;
} baud_msg[] = {
	"",		B300,
	" 1200",	B1200,
	" 2400",	B2400,
	0,		0,
};

static int
cour_connect()
{
	char c;
	int nc, nl, n;
#ifdef _AIX
	struct termios sb;
#else
	struct sgttyb sb;
#endif
	char dialer_buf[64];
	struct baud_msg *bm;
	void (*f)(int);

	if (cour_swallow("\r\n") == 0)
		return (0);
	f = signal(SIGALRM, sigALRM);
again:
	nc = 0; nl = sizeof(dialer_buf)-1;
	bzero(dialer_buf, sizeof(dialer_buf));
	timeout = 0;
	for (nc = 0, nl = sizeof(dialer_buf)-1 ; nl > 0 ; nc++, nl--) {
		if (setjmp(timeoutbuf))
			break;
		alarm(number(value(DIALTIMEOUT)));
		n = read(FD, &c, 1);
		alarm(0);
		if (n <= 0)
			break;
		c &= 0x7f;
		if (c == '\r') {
			if (cour_swallow("\n") == 0)
				break;
			if (!dialer_buf[0])
				goto again;
			if (strcmp(dialer_buf, "RINGING") == 0) {
				printf("%s\r\n", dialer_buf);
				goto again;
			}
			if (strncmp(dialer_buf, "CONNECT",
				    sizeof("CONNECT")-1) != 0)
				break;
			for (bm = baud_msg ; bm ; bm++)
				if (strcmp(bm->msg,
				    dialer_buf+sizeof("CONNECT")-1) == 0) {
#ifdef _AIX
					if (ioctl(FD, TCGETA, &sb) < 0)	{
						perror("TCGETA");
						goto error;
					}
					sb.c_cflag |= bm->baud;
					if (ioctl(FD, TCSETAF, &sb) < 0)  {
						perror("TCSETAF");
						goto error;
					}
#else
					if (ioctl(FD, TIOCGETP, &sb) < 0) {
						perror("TIOCGETP");
						goto error;
					}
					sb.sg_ispeed = sb.sg_ospeed = bm->baud;
					if (ioctl(FD, TIOCSETP, &sb) < 0) {
						perror("TIOCSETP");
						goto error;
					}
#endif
					signal(SIGALRM, f);
					if (boolean(value(VERBOSE)))
						printf("%s\r\n", dialer_buf);
					return (1);
				}
			break;
		}
		dialer_buf[nc] = c;
#ifdef notdef
		if (boolean(value(VERBOSE)))
			putchar(c);
#endif
	}
error1:
	printf("%s\r\n", dialer_buf);
error:
	signal(SIGALRM, f);
	return (0);
}

/*
 * This convoluted piece of code attempts to get
 * the courier in sync.  If you don't have FIONREAD
 * there are gory ways to simulate this.
 */
#undef sleep
static int
coursync()
{
	int already = 0;

	/*
	 * Toggle DTR to force anyone off that might have left
	 * the modem connected, and insure a consistent state
	 * to start from.
	 *
	 * If you don't have the ioctl calls to diddle directly
	 * with DTR, you can always try setting the baud rate to 0.
	 */
#ifdef _AIX
	struct termios tmp;

	tmp = tbuf;
	tmp.c_cflag |= (HUPCL | B0);	/* Set baud rate to 0 */
	ioctl(FD, TCSETA, &tmp);
	sleep(2);
	ioctl(FD, TCSETA, &tbuf);	/* return to proper values */
#else
	ioctl(FD, TIOCCDTR, 0);
	sleep(2);
	ioctl(FD, TIOCSDTR, 0);
#endif
	while (already++ < MAXRETRY) {
#ifdef _AIX
		ioctl(FD, TCFLSH, 2);	/* flush input and output queue */
#else
		ioctl(FD, TIOCFLUSH, 0);	/* flush any clutter */
#endif
		cour_write(FD, "\rAT Z\r", 6);	/* reset modem */
		sleep(2);
		verbose_read();
		cour_write(FD, "AT E0\r", 6);	/* turn off echoing */
		sleep(2);
		verbose_read();
#ifdef _AIX
		ioctl(FD, TCFLSH, 2);	/* flush input and output queue */
#else
		ioctl(FD, TIOCFLUSH, 0);	/* flush any clutter */
#endif
		sleep(1);
		cour_write(FD, "AT C1 E0 H0 Q0 X6 V1\r", 21);
		if (cour_swallow("\r\nOK\r\n")) {
#ifdef _AIX
			ioctl(FD, TCFLSH, 2);	/* flush input and output queue */
#else
			ioctl(FD, TIOCFLUSH, 0);
#endif
			return 1;
		}
		sleep(2);
		cour_write(FD, "+++", 3);
		sleep(2);
	}
	cour_write(FD, "\rAT Z\r", 6);
	return 0;
}

cour_write(fd, cp, n)
int fd;
char *cp;
int n;
{
#ifdef _AIX
	struct termios sb;
#else
	struct sgttyb sb;
#endif
	if (boolean(value(VERBOSE)))
		write(1, cp, n);
#ifdef _AIX
	ioctl(fd, TCFLSH, 2);	/* flush input/output queue */
#else
	ioctl(fd, TIOCGETP, &sb);
	ioctl(fd, TIOCSETP, &sb);
#endif
	cour_nap();
	for ( ; n-- ; cp++) {
		write(fd, cp, 1);
#ifdef _AIX
		ioctl(fd, TCFLSH, 2);	/* flush input/output queue */
#else
		ioctl(fd, TIOCGETP, &sb);
		ioctl(fd, TIOCSETP, &sb);
#endif
		cour_nap();
	}
}

verbose_read()
{
	int n = 0;
	char buf[BUFSIZ];
	if (!boolean(value(VERBOSE)))
		return;
#ifdef _AIX
	if (ioctl(FD, TIONREAD, &n) < 0)
#else
	if (ioctl(FD, FIONREAD, &n) < 0)
#endif
		return;
	if (n <= 0)
		return;
	if (read(FD, buf, n) != n)
		return;
	write(1, buf, n);
}

/*
 * Code stolen from /usr/src/lib/libc/gen/sleep.c
 */
#include <sys/time.h>

#define mask(s) (1<<((s)-1))
#define setvec(vec, a) \
        vec.sv_handler = (void (*)(int))a; vec.sv_mask = vec.sv_onstack = 0

static napms = 50; /* Give the courier 50 milliseconds between characters */

static int ringring;

cour_nap()
{
	
	int omask;
        struct itimerval itv, oitv;
        register struct itimerval *itp = &itv;
        struct sigvec vec, ovec;

        timerclear(&itp->it_interval);
        timerclear(&itp->it_value);
        if (setitimer(ITIMER_REAL, itp, &oitv) < 0)
                return;
        setvec(ovec, SIG_DFL);
        omask = sigblock(mask(SIGALRM));
        itp->it_value.tv_sec = napms/1000;
	itp->it_value.tv_usec = ((napms%1000)*1000);
        setvec(vec, cour_napx);
        ringring = 0;
        (void) sigvec(SIGALRM, &vec, &ovec);
        (void) setitimer(ITIMER_REAL, itp, (struct itimerval *)0);
        while (!ringring)
                sigpause(omask &~ mask(SIGALRM));
        (void) sigvec(SIGALRM, &ovec, (struct sigvec *)0);
        (void) setitimer(ITIMER_REAL, &oitv, (struct itimerval *)0);
	(void) sigsetmask(omask);
}

static 
cour_napx(void)
{
        ringring = 1;
}


