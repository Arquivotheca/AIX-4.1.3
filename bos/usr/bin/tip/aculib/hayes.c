static char sccsid[] = "@(#)76	1.8  src/bos/usr/bin/tip/aculib/hayes.c, cmdtip, bos411, 9428A410j 3/11/94 08:51:19";
/* 
 * COMPONENT_NAME: UUCP hayes.c
 * 
 * FUNCTIONS: MSGSTR, error_rep, gobble, goodbye, hay_abort, 
 *            hay_dialer, hay_disconnect, hay_sync, min, sigALRM,
 *            strgobble
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

/* static char sccsid[] = "hayes.c	5.1 (Berkeley) 4/30/85"; */

/*
 * Routines for calling up on a Hayes Modem
 * (based on the old VenTel driver).
 * The modem is expected to be strapped for "echo".
 * Also, the switches enabling the DTR and CD lines
 * must be set correctly.
 * NOTICE:
 * The easy way to hang up a modem is always simply to
 * clear the DTR signal. However, if the +++ sequence
 * (which switches the modem back to local mode) is sent
 * before modem is hung up, removal of the DTR signal
 * has no effect (except that it prevents the modem from
 * recognizing commands).
 * (by Helge Skrivervik, Calma Company, Sunnyvale, CA. 1984) 
 */
#include "../tip.h"
#include "../tip_msg.h" 
#include <nl_types.h>
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 

#define	min(a,b)	((a < b) ? a : b)

static	void sigALRM(int);
static	int timeout = 0;
static	jmp_buf timeoutbuf;
static 	char gobble();
#define DUMBUFLEN	40
static char dumbuf[DUMBUFLEN];

#define	DIALING		1
#define IDLE		2
#define CONNECTED	3
#define	FAILED		4
static	int state = IDLE;

hay_dialer(num, acu)
	register char *num;
	char *acu;
{
	register char *cp;
	register int connected = 0;
	char dummy;
#ifdef ACULOG
	char line[80];
#endif
	if (hay_sync() == 0)		/* make sure we can talk to the modem */
		return(0);
	usleep(250000);
	if (boolean(value(VERBOSE)))
		printf(MSGSTR(DIALINGIT, "\ndialing...")); /*MSG*/
	fflush(stdout);
#ifdef _AIX
	tcgetattr(FD,&FDbuf);
	tbuf.c_cflag |= HUPCL;
	tcsetattr(FD,TCSANOW,&FDbuf);
	tcflush(FD,TCIOFLUSH);	/* flush input and output queues */
#else
	ioctl(FD, TIOCHPCL, 0);
	ioctl(FD, TIOCFLUSH, 0);	/* get rid of garbage */
#endif
	connected = 0;
	write(FD, "ATV1\r", 5);	/* tell modem to use verbose status codes */
	if (strgobble("OK\r")) {
		usleep(250000);
		write(FD, "ATTD", 4); /* send dial command */
		write(FD, num, strlen(num)); /* send dial command */
		state = DIALING;
		write(FD, "\r", 1);
        	if (strgobble("CONNECT\r"))
				connected = 1;
			else
				error_rep(dummy);
		if (connected)
			state = CONNECTED;
		else {
			state = FAILED;
			return (connected);	/* lets get out of here.. */
		}
#ifdef _AIX
		tcflush(FD,TCIOFLUSH); /* flush input and output queues */
#else
		ioctl(FD, TIOCFLUSH, 0);
#endif
#ifdef ACULOG
		if (timeout) {
			sprintf(line, MSGSTR(DIALTIMEDOUT, "%d second dial timeout"), /*MSG*/
				number(value(DIALTIMEOUT)));
			logent(value(HOST), num, "hayes", line);
		}
#endif
		if (timeout)
			hay_disconnect();	/* insurance */
	}
	return (connected);
}


hay_disconnect()
{
	char c;
	int len, rlen;
#ifdef _AIX
	struct termios tmp;
#endif 

	/* first hang up the modem*/
#ifdef DEBUG
	printf(MSGSTR(DISCONNECTING2, "\rdisconnecting modem....\n\r")); /*MSG*/
#endif
#ifdef _AIX
	tcgetattr(FD,&FDbuf);
	tmp = FDbuf;
	tmp.c_cflag |= (HUPCL | B0);	/* set baud rate to 0 */
	tcsetattr(FD,TCSANOW,&tmp);
	sleep(1);
	tcsetattr(FD,TCSAFLUSH,&FDbuf); /* restore proper settings */
#else
	ioctl(FD, TIOCCDTR, 0);
	sleep(1);
	ioctl(FD, TIOCSDTR, 0);
#endif
	goodbye();
}

hay_abort()
{

	char c;

	write(FD, "\r", 1);	/* send anything to abort the call */
	hay_disconnect();
}

static void
sigALRM(int s)
{

	printf(MSGSTR(TIMEOUT2, "\07timeout waiting for reply\n\r")); /*MSG*/
	timeout = 1;
	longjmp(timeoutbuf, 1);
}

strgobble(strng)
	char *strng;
{
	char *current, tempstr[2];
	int result;

	current = strng;
	if (current != NULL) {
		while (*current != '\0') {
			tempstr[0] = *current;
			tempstr[1] = '\0';
			result=gobble(tempstr);
			if (result != *current)
				return(0);
			current++;
		}
	}
	return(1);
}

static char
gobble(match)
	register char *match;
{
	char c;
	void (*f)(int);
	int i, status = 0;

	signal(SIGALRM, sigALRM);
	timeout = 0;
#ifdef DEBUG
	printf("\ngobble: waiting for %s\n", match);
#endif
	do {
		if (setjmp(timeoutbuf)) {
			signal(SIGALRM, f);
			return (0);
		}
		alarm(number(value(DIALTIMEOUT)));
		read(FD, &c, 1);
		alarm(0);
		c &= 0177;
#ifdef DEBUG
		printf("%c 0x%x ", c, c);
#endif
		for (i = 0; i < strlen(match); i++)
			if (c == match[i])
				status = c;
	} while (status == 0);
	signal(SIGALRM, SIG_DFL);
#ifdef DEBUG
	printf("\n");
#endif
	return (status);
}

error_rep(c)
	register char c;
{
	printf("\n\r");
	switch (c) {

	case '0':
		printf(MSGSTR(OK, "OK")); /*MSG*/
		break;

	case '1':
		printf(MSGSTR(CONNECT, "CONNECT")); /*MSG*/
		break;
	
	case '2':
		printf(MSGSTR(RING, "RING")); /*MSG*/
		break;
	
	case '3':
		printf(MSGSTR(NOCARRIER, "NO CARRIER")); /*MSG*/
		break;
	
	case '4':
		printf(MSGSTR(ERROR, "ERROR in input")); /*MSG*/
		break;
	
	case '5':
		printf(MSGSTR(CONNECT12, "CONNECT 1200")); /*MSG*/
		break;
	
	default:
		printf(MSGSTR(UNKERR, "Unknown Modem error: %c (0x%x)"), c, c); /*MSG*/
	}
	printf("\n\r");
	return;
}

/*
 * set modem back to normal verbose status codes.
 */
goodbye()
{
	int len, rlen;
	char c;

#ifdef _AIX
	struct termios tmp;

	tcflush(FD,TCIOFLUSH); /* flush input and output queues */
#else
	/* this probably never worked since len was never initialized */
	ioctl(FD, TIOCFLUSH, &len);	/* get rid of trash */
#endif
	if (hay_sync()) {
		sleep(1);
#ifndef DEBUG
#ifdef _AIX
	tcflush(FD,TCIOFLUSH); /* flush input and output queues */
#else
	ioctl(FD, TIOCFLUSH, 0);
#endif
#endif
		vtable[DIALTIMEOUT].v_value = 2; /* lower rcv timeout */
		write(FD, "ATH0\r", 5);
		c = strgobble("OK\r");
		if (!c) {			/* insurance */
			sleep(2);
			write(FD, "+++", 3);
			sleep(2);
			c = strgobble("OK\r");
			write(FD, "ATH0\r", 5);
		}
		if (!c) {
			printf(MSGSTR(CANTHANGUP, "cannot hang up modem\n\r")); /*MSG*/
			printf(MSGSTR(PLEASE, "please use 'tip dialer' to make sure the line is hung up\n\r")); /*MSG*/
		}
		sleep(1);
#ifdef _AIX
		ioctl(FD, TIONREAD, &len);
#else
		ioctl(FD, FIONREAD, &len);
#endif
#ifdef DEBUG
		printf("goodbye1: len=%d -- ", len);
		rlen = read(FD, dumbuf, min(len, DUMBUFLEN));
		dumbuf[rlen] = '\0';
		printf("read (%d): %s\r\n", rlen, dumbuf);
#endif
		write(FD, "ATZ1\r", 5);
		sleep(1);
#ifdef DEBUG
#ifdef _AIX
		ioctl(FD, TIONREAD, &len);
#else
		ioctl(FD, FIONREAD, &len);
#endif
		printf("goodbye2: len=%d -- ", len);
		rlen = read(FD, dumbuf, min(len, DUMBUFLEN));
		dumbuf[rlen] = '\0';
		printf("read (%d): %s\r\n", rlen, dumbuf);
#endif
	}
#ifdef _AIX
	tcflush(FD,TCIOFLUSH); /* flush input and output queues */

	tcgetattr(FD,&FDbuf);
	tmp = FDbuf;
	tmp.c_cflag |= (HUPCL | B0);	/* set baud rate to 0 */
	tcsetattr(FD,TCSANOW,&tmp);
	sleep(1);
	tcsetattr(FD,TCSAFLUSH,&FDbuf); /* restore proper settings */
#else
	ioctl(FD, TIOCFLUSH, 0);	/* clear the input buffer */
	ioctl(FD, TIOCCDTR, 0);		/* clear DTR (insurance) */
#endif
	close(FD);
}

#define MAXRETRY	5

hay_sync()
{
	int len, retry = 0;
#ifdef _AIX
	struct termios tmp;
#endif
#ifdef _AIX
	tcflush(FD,TCIFLUSH); /* flush input queues */
#else
	ioctl(FD, TIOCFLUSH, 0);	/* clear the input buffer */
#endif

	while (retry++ <= MAXRETRY) {
		write(FD, "AT\r", 3);
		sleep(1);
#ifdef _AIX
		ioctl(FD, TIONREAD, &len);
#else
		ioctl(FD, FIONREAD, &len);
#endif
		if (len) {
			len = read(FD, dumbuf, min(len, DUMBUFLEN));
			if (index(dumbuf, '0') || 
		   	(index(dumbuf, 'O') && index(dumbuf, 'K'))
			|| index(dumbuf,'\r'))
				return(1);
#ifdef DEBUG
			dumbuf[len] = '\0';
			printf("hay_sync: (\"%s\") %d\n\r", dumbuf, retry);
#endif
		}
#ifdef _AIX

		tcgetattr(FD,&FDbuf);
		tmp = FDbuf;
		tmp.c_cflag |= (HUPCL | B0);	/* set baud rate to 0 */
		tcsetattr(FD,TCSANOW,&tmp);
		sleep(1);
		tcsetattr(FD,TCSANOW,&FDbuf); /* restore proper settings */
#else
		ioctl(FD, TIOCCDTR, 0);
		ioctl(FD, TIOCSDTR, 0);
#endif
	}
	printf(MSGSTR(CANTSYNC3, "Cannot synchronize with hayes...\n\r")); /*MSG*/
	return(0);
}
