static char sccsid[] = "@(#)74	1.6  src/bos/usr/bin/tip/aculib/df.c, cmdtip, bos411, 9428A410j 6/9/91 14:13:39";
/* 
 * COMPONENT_NAME: UUCP df.c
 * 
 * FUNCTIONS: MSGSTR, df02_dialer, df03_dialer, df_abort, df_dialer, 
 *            df_disconnect, timeout 
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

/* static char sccsid[] = "df.c	5.1 (Berkeley) 6/6/85"; */

/*
 * Dial the DF02-AC or DF03-AC
 */

#include "../tip.h"
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

static jmp_buf Sjbuf;
static void timeout(int);

df02_dialer(num, acu)
	char *num, *acu;
{

	return (df_dialer(num, acu, 0));
}

df03_dialer(num, acu)
	char *num, *acu;
{

	return (df_dialer(num, acu, 1));
}

df_dialer(num, acu, df03)
	char *num, *acu;
	int df03;
{
	register int f = FD;
/*
#ifdef _AIX
	struct termios buf;
#else
*/
	struct sgttyb buf;
/*
#endif
*/
	int speed = 0, rw = 2;
	char c = '\0';

#ifdef _AIX
	ioctl(f, TCGETA, &tbuf);
	tbuf.c_cflag |= HUPCL;
	ioctl(f, TCSETA, &tbuf);
#else
	ioctl(f, TIOCHPCL, 0);		/* make sure it hangs up when done */
#endif
	if (setjmp(Sjbuf)) {
		printf(MSGSTR(TIMEDOUT2, "connection timed out\r\n")); /*MSG*/
		df_disconnect();
		return (0);
	}
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	fflush(stdout);
#ifdef TIOCMSET
	if (df03) {
		int st = TIOCM_ST;	/* secondary Transmit flag */
		ioctl(f, TIOCGETP, &buf);
		if (buf.sg_ospeed != B1200) {	/* must dial at 1200 baud */
			speed = buf.sg_ospeed;
			buf.sg_ospeed = buf.sg_ispeed = B1200;
			ioctl(f, TIOCSETP, &buf);
			ioctl(f, TIOCMBIC, &st); /* clear ST for 300 baud */
		} else
			ioctl(f, TIOCMBIS, &st); /* set ST for 1200 baud */
	}
#endif
#undef sleep
	signal(SIGALRM, timeout);
	alarm(5 * strlen(num) + 10);
#ifdef _AIX
	ioctl(f, TCFLSH, 1);	/* flush output queue */
#else
	ioctl(f, TIOCFLUSH, &rw);
#endif
	write(f, "\001", 1);
	sleep(1);
	write(f, "\002", 1);
	write(f, num, strlen(num));
	read(f, &c, 1);
#ifdef TIOCMSET
	if (df03 && speed) {
		buf.sg_ispeed = buf.sg_ospeed = speed;
		ioctl(f, TIOCSETP, &buf);
	}
#endif
	return (c == 'A');
}

df_disconnect()
{
	int rw = 2;

	write(FD, "\001", 1);
	sleep(1);
#ifdef _AIX
	ioctl(FD, TCFLSH, 1);	/* flush output queue */
#else
	ioctl(FD, TIOCFLUSH, &rw);
#endif
}


df_abort()
{

	df_disconnect();
}


static void
timeout(int s)
{

	longjmp(Sjbuf, 1);
}
