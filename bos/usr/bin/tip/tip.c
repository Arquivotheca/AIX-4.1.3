static char sccsid[] = "@(#)42	1.12  src/bos/usr/bin/tip/tip.c, cmdtip, bos411, 9428A410j 3/11/94 09:07:20";
/* 
 * COMPONENT_NAME: UUCP tip.c
 * 
 * FUNCTIONS: MSGSTR, Mtip, any, cleanup, ctrl, escape, help, 
 *            interp, intprompt, load_etable, load_sep, prompt, 
 *            pwrite, raw, setparity, size, sname, speed, tipin, 
 *            ttysetup, unraw 
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

#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

/* static char sccsid[] = "tip.c	5.4 (Berkeley) 4/3/86"; */

/*
 * tip - UNIX link to other systems
 *  tip [-v] [-speed] system-name
 * or
 *  cu phone-number [-s speed] [-l line] [-a acu]
 */
#include "tip.h"
#include <locale.h>
#include <nl_types.h>
#include "tip_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 
extern char sep[][50];
/*
 * Baud rate mapping table
 */
int bauds[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600,
	1200, 1800, 2400, 4800, 9600, 19200, 38400, -1
};

#ifndef _AIX
int	disc = OTTYDISC;		/* tip normally runs this way */
#endif
void	intprompt(int);
void	timeout(int);
void	cleanup(int);
char	*sname();
char	PNbuf[256];			/* This limits the size of a number */

main(argc, argv)
	char *argv[];
{
	char *system = NOSTR;
	register int i;
	register char *p;
	char sbuf[12], *ttyname;

	setlocale(LC_ALL, "");
	catd = catopen(MF_TIP,NL_CAT_LOCALE);
	load_sep();
	load_etable();
	if (equal(sname(argv[0]), "cu")) {
		cumode = 1;
		cumain(argc, argv);
		goto cucommon;
	}

	if (argc > 4) {
		fprintf(stderr, MSGSTR(USAGE4, "usage: tip [-v] [-speed] [system-name]\n")); /*MSG*/
		exit(1);
	}
	if (!isatty(0)) {
		fprintf(stderr, MSGSTR(INTERACTIVE, "tip: must be interactive\n")); /*MSG*/
		exit(1);
	}

	for (; argc > 1; argv++, argc--) {
		if (argv[1][0] != '-')
			system = argv[1];
		else switch (argv[1][1]) {

		case 'v':
			vflag++;
			break;

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			BR = atoi(&argv[1][1]);
			break;

		default:
			fprintf(stderr, MSGSTR(UNKOPT, "tip: %s, unknown option\n"), argv[1]); /*MSG*/
			break;
		}
	}

	if (system == NOSTR)
		goto notnumber;
	if (isalpha(*system))
		goto notnumber;
	/*
	 * System name is really a phone number...
	 * Copy the number then stomp on the original (in case the number
	 *	is private, we don't want 'ps' or 'w' to find it).
	 */
	if (strlen(system) > sizeof PNbuf - 1) {
		fprintf(stderr, MSGSTR(NUMTOOLONG, "tip: phone number too long (max = %d bytes)\n"), /*MSG*/
			sizeof PNbuf - 1);
		exit(1);
	}
	strncpy( PNbuf, system, sizeof PNbuf - 1 );
	for (p = system; *p; p++)
		*p = '\0';
	PN = PNbuf;
	system = sbuf;
	(void *)sprintf(sbuf,"tip%d",BR);

notnumber:
	signal(SIGINT, (void(*)(int)) cleanup);
	signal(SIGQUIT, (void(*)(int)) cleanup);
	signal(SIGHUP, (void(*)(int)) cleanup);
	signal(SIGTERM, (void(*)(int)) cleanup);

	if ((ttyname = hunt(system)) == NULL) {
		printf(MSGSTR(ALLBUSY, "all ports busy\n")); /*MSG*/
		exit(3);
	}
	if (ttyname == (char *) -1) {
		printf(MSGSTR(LINKDOWN, "link down\n")); /*MSG*/
		ttyunlock(uucplock);
		exit(3);
	}
	setbuf(stdout, NULL);

	/*
	 * Kludge, their's no easy way to get the initialization
	 *   in the right order, so force it here
	 */
	if ((PH = (char *)getenv("PHONES")) == NOSTR)
		PH = "/etc/phones";
	vinit();				/* init variables */
	loginit();
	setparity("even");			/* set the parity table */
	if ((i = speed(number(value(BAUDRATE)))) == (int)NULL) {
		printf(MSGSTR(BADBAUD, "tip: bad baud rate %d\n"), number(value(BAUDRATE))); /*MSG*/
		ttyunlock(uucplock);
		exit(3);
	}

	/*
	 * Now that we have the logfile and the ACU open
	 *  return to the real uid and gid.  These things will
	 *  be closed on exit.  Swap real and effective uid's
	 *  so we can get the original permissions back
	 *  for removing the uucp lock.
	 */
	gid = getgid();
	egid = getegid();
	uid = getuid();
	euid = geteuid();
	setregid(egid, gid);
	setreuid(euid, uid);

	/*
	 * Hardwired connections require the
	 *  line speed set before they make any transmissions
	 *  (this is particularly true of things like a DF03-AC)
	 */
	if (HW)
		ttysetup(i);
	if (p = connect()) {
		printf(MSGSTR(EOT2, "\07%s\n[EOT]\n"), p); /*MSG*/
		setreuid(uid, euid);
		setregid(gid, egid);
		ttyunlock(uucplock);
		exit(1);
	}
	if (!HW)
		ttysetup(i);
cucommon:
	/*
	 * From here down the code is shared with
	 * the "cu" version of tip.
	 */

#ifdef _AIX
	if (tcgetattr(0,&tbuf) == -1)
		perror("tcgetattr");
	tbufsave = tbuf;	/* save original settings */
/* set up for RAW mode */
	tbuf.c_iflag &= ~(INLCR | ICRNL | IGNCR | IXOFF | IUCLC);
	tbuf.c_oflag |= OPOST;
	tbuf.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
	tbuf.c_lflag &= ~(ICANON | ISIG);
	if (boolean(value(LECHO)) == TRUE)
		tbuf.c_lflag |= ECHO;
	else
		tbuf.c_lflag &= ~ECHO;
	tbuf.c_iflag |= IXON;
	tbuf.c_cc[VMIN] = 1;
	tbuf.c_cc[VTIME] = 0;

#else
	ioctl(0, TIOCGETP, (char *)&defarg);
	ioctl(0, TIOCGETC, (char *)&defchars);
	ioctl(0, TIOCGLTC, (char *)&deflchars);
	ioctl(0, TIOCGETD, (char *)&odisc);
	arg = defarg;
	arg.sg_flags = ANYP | CBREAK;
	tchars = defchars;
	tchars.t_intrc = tchars.t_quitc = -1;
	ltchars = deflchars;
	ltchars.t_suspc = ltchars.t_dsuspc = ltchars.t_flushc
		= ltchars.t_lnextc = -1;
#endif

	raw();

	pipe(fildes); pipe(repdes);
	signal(SIGALRM, (void(*)(int)) timeout);

	/*
	 * Everything's set up now:
	 *	connection established (hardwired or dialup)
	 *	line conditioned (baud rate, mode, etc.)
	 *	internal data structures (variables)
	 */

	/* Reset tty to notify us of DCD loss */
	FDbuf.c_cflag &= ~CLOCAL;
	(void) tcsetattr(FD,TCSANOW,&FDbuf);

	/*
	 * Fork one process for local side and one for remote.
	 */
	printf(cumode ? MSGSTR(CONNED, "Connected\r\n") : MSGSTR(CONNECT2, "\07connected\r\n")); /*MSG*/ /*MSG*/
	if (pid = fork())
		tipin();
	else
		tipout();
	/*NOTREACHED*/
}

void cleanup(int s)
{

	if (uid != getuid()) {
		setreuid(uid, euid);
		setregid(gid, egid);
	}
	tcsetattr(FD, TCSANOW, &FDbufsave);
	ttyunlock(uucplock);
#ifdef _AIX
	unraw();	/* restore terminal to original settings */
#else
	if (odisc)
		ioctl(0, TIOCSETD, (char *)&odisc);
#endif
	exit(0);
}

/*
 * put the controlling keyboard into raw mode
 */
raw()
{

#ifdef _AIX

if (tcsetattr(0,TCSAFLUSH,&tbuf) == -1)
	perror("tcsetattr");	
#else
	ioctl(0, TIOCSETP, &arg);
	ioctl(0, TIOCSETC, &tchars);
	ioctl(0, TIOCSLTC, &ltchars);
	ioctl(0, TIOCSETD, (char *)&disc);
#endif
}


/*
 * return keyboard to normal mode
 */
unraw()
{

#ifdef _AIX

if (tcsetattr(0,TCSAFLUSH,&tbufsave) == -1)
	perror("tcsetattr");

#else
	ioctl(0, TIOCSETD, (char *)&odisc);
	ioctl(0, TIOCSETP, (char *)&defarg);
	ioctl(0, TIOCSETC, (char *)&defchars);
	ioctl(0, TIOCSLTC, (char *)&deflchars);
#endif
}

static	jmp_buf promptbuf;

/*
 * Print string ``s'', then read a string
 *  in from the terminal.  Handles signals & allows use of
 *  normal erase and kill characters.
 */
prompt(s, p)
	char *s;
	register char *p;
{
	register char *b = p;
	void (*oint)(int), (*oquit)(int);

	stoprompt = 0;
	oint = signal(SIGINT, (void(*)(int)) intprompt); 
	oint = signal(SIGQUIT, SIG_IGN); /* probably a BUG; should be oquit */
	unraw();
	printf("%s", s);
	if (setjmp(promptbuf) == 0)
		while ((*p = getchar()) != (char) EOF && *p != '\n')
			p++;
	*p = '\0';

	raw();
	signal(SIGINT, oint);
	signal(SIGQUIT, oint); /* matching BUG (probably makes no difference) */
	return (stoprompt || p == b);
}

/*
 * Interrupt service routine during prompting
 */
void intprompt(int s)
{

	signal(SIGINT, SIG_IGN);
	stoprompt = 1;
	printf("\r\n");
	longjmp(promptbuf, 1);
}

/*
 * ****TIPIN   TIPIN****
 */
tipin()
{
	char gch, bol = 1;

	/*
	 * Kinda klugey here...
	 *   check for scripting being turned on from the .tiprc file,
	 *   but be careful about just using setscript(), as we may
	 *   send a SIGUSR1 before tipout has a chance to set up catching
	 *   it; so wait a second, then setscript()
	 */
#undef sleep

	if (boolean(value(SCRIPT))) {
		sleep(1);
		setscript();
	}

	while (1) {
		gch = getchar()&pmask;
		if ((gch == character(value(ESCAPE))) && bol) {
			if (!(gch = escape()))
				continue;
		} else if (!cumode && gch == character(value(RAISECHAR))) {
			boolean(value(RAISE)) = !boolean(value(RAISE));
			continue;
		} else if (gch == '\r') {
			bol = 1;
			pwrite(FD, &gch, 1);
			if (boolean(value(HALFDUPLEX)))
				printf("\r\n");
			continue;
		} else if (!cumode && gch == character(value(FORCE)))
			gch = getchar()&pmask;
		bol = any(gch, value(EOL));
		if (boolean(value(RAISE)) && islower(gch))
			gch = toupper(gch);
		pwrite(FD, &gch, 1);
		if (boolean(value(HALFDUPLEX)))
			printf("%c", gch);
	}
}

/*
 * Escape handler --
 *  called on recognition of ``escapec'' at the beginning of a line
 */
escape()
{
	register char gch;
	register esctable_t *p;
	char c = character(value(ESCAPE));
	extern esctable_t etable[];

	gch = (getchar()&pmask);
	for (p = etable; p->e_char; p++)
		if (p->e_char == gch) {
			if ((p->e_flags&PRIV) && getuid())
				continue;
			printf("%s", ctrl(c));
			(*p->e_func)(gch);
			return (0);
		}
	/* ESCAPE ESCAPE forces ESCAPE */
	if (c != gch)
		pwrite(FD, &c, 1);
	return (gch);
}

speed(n)
	int n;
{
	register int *p;

	for (p = bauds; *p != -1;  p++)
		if (*p == n)
			return (p - bauds);
	return ((int)NULL);
}

any(c, p)
	register char c, *p;
{
	while (p && *p)
		if (*p++ == c)
			return (1);
	return (0);
}

size(s)
	register char	*s;
{
	register int i = 0;

	while (s && *s++)
		i++;
	return (i);
}

char *
interp(s)
	register char *s;
{
	static char buf[256];
	register char *p = buf, c, *q;

	while (c = *s++) {
		for (q = "\nn\rr\tt\ff\033E\bb"; *q; q++)
			if (*q++ == c) {
				*p++ = '\\'; *p++ = *q;
				goto next;
			}
		if (c < 040) {
			*p++ = '^'; *p++ = c + 'A'-1;
		} else if (c == 0177) {
			*p++ = '^'; *p++ = '?';
		} else
			*p++ = c;
	next:
		;
	}
	*p = '\0';
	return (buf);
}

char *
ctrl(c)
	char c;
{
	static char s[3];

	if (c < 040 || c == 0177) {
		s[0] = '^';
		s[1] = c == 0177 ? '?' : c+'A'-1;
		s[2] = '\0';
	} else {
		s[0] = c;
		s[1] = '\0';
	}
	return (s);
}

/*
 * Help command
 */
help(c)
	char c;
{
	register esctable_t *p;
	extern esctable_t etable[];

	printf("%c\r\n", c);
	for (p = etable; p->e_char; p++) {
		if ((p->e_flags&PRIV) && getuid())
			continue;
		printf("%2s", ctrl(character(value(ESCAPE))));
		printf("%-2s %c   %s\r\n", ctrl(p->e_char),
			p->e_flags&EXP ? '*': ' ', p->e_help);
	}
}

/*
 * Set up the "remote" tty's state
 */
ttysetup(speed)
	int speed;
{
#ifdef _AIX
	struct termios temp;

	if (tcgetattr(FD,&temp) == -1)
		perror("tcgetattr");
/* set up for RAW mode */
        temp.c_iflag = temp.c_oflag = temp.c_lflag = (ushort)0;
        temp.c_iflag = (IGNPAR | IGNBRK | IXON | IXOFF);
	temp.c_cc[VMIN] = 1;
	temp.c_cc[VTIME] = 0;
	if (boolean(value(TAND)))
		temp.c_iflag |= IXOFF|IXON; /* TANDEM (enable flow control) mode */
	if (cfsetospeed(&temp, (speed_t)speed) < 0)
		perror("cfsetospeed");
	if (cfsetispeed(&temp, (speed_t)speed) < 0)
		perror("cfsetispeed");

	tcsetattr(FD,TCSAFLUSH,&temp);
#else
	unsigned bits = LDECCTQ;

	arg.sg_ispeed = arg.sg_ospeed = speed;
	arg.sg_flags = RAW;
	if (boolean(value(TAND)))
		arg.sg_flags |= TANDEM;
	ioctl(FD, TIOCSETP, (char *)&arg);
	ioctl(FD, TIOCLBIS, (char *)&bits);
#endif
}

/*
 * Return "simple" name from a file name,
 * strip leading directories.
 */
char *
sname(s)
	register char *s;
{
	register char *p = s;

	while (*s)
		if (*s++ == '/')
			p = s;
	return (p);
}

static char partab[0200];

/*
 * Do a write to the remote machine with the correct parity.
 * We are doing 8 bit wide output, so we just generate a character
 * with the right parity and output it.
 */
pwrite(fd, buf, n)
	int fd;
	char *buf;
	register int n;
{
	register int i;
	register char *bp;
	extern int errno;

	bp = buf;
	if (!equal (value(PARITY),"graphic"))
	    for (i = 0; i < n; i++) {
		*bp = partab[(*bp) & pmask];
		bp++;
	    }
	if (write(fd, buf, n) < 0) {
		if (errno == EIO)
			abort(MSGSTR(LOSTCARRIER, "Lost carrier.")); /*MSG*/
		/* this is questionable */
		perror(MSGSTR(WRITEIT, "write")); /*MSG*/
	}
}

/*
 * Build a parity table with appropriate high-order bit.
 */
setparity(defparity)
	char *defparity;
{
	register int i;
	char *parity;
	extern char evenpartab[];

	if (value(PARITY) == NOSTR)
		value(PARITY) = defparity;
	parity = value(PARITY);
	for (i = 0; i < 0200; i++)
		partab[i] = evenpartab[i];
	if equal(parity, "graphic") {
		pmask = 0377;
		return;
	} else pmask = 0177;
	if (equal(parity, "even"))
		return;
	if (equal(parity, "odd")) {
		for (i = 0; i < 0200; i++)
			partab[i] ^= 0200;	/* reverse bit 7 */
		return;
	}
	if (equal(parity, "none") || equal(parity, "zero")) {
		for (i = 0; i < 0200; i++)
			partab[i] &= ~0200;	/* turn off bit 7 */
		return;
	}
	if (equal(parity, "one")) {
		for (i = 0; i < 0200; i++)
			partab[i] |= 0200;	/* turn on bit 7 */
		return;
	}
	fprintf(stderr, MSGSTR(UNKPARITY, "%s: unknown parity value\n"), PA); /*MSG*/
	fflush(stderr);
}
load_sep() {
	strcpy(sep[0], MSGSTR(LITSECOND, "second")); /*MSG*/
	strcpy(sep[1], MSGSTR(LITMINUTE, "minute")); /*MSG*/
	strcpy(sep[2], MSGSTR(LITHOUR, "hour")); /*MSG*/
}

load_etable() {

	int i = 0;
	extern esctable_t etable[];

	strcpy(etable[i++].e_help, MSGSTR(ETSHELL, "shell")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETRCV, "receive file from remote host")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETSND, "send file to remote host")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETTAKE, "take file from remote UNIX")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETPUT, "put file to remote UNIX")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETPIPE, "pipe remote file")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETPIPLOC, "pipe local command to remote host")); /*MSG*/
#ifdef CONNECT
	strcpy(etable[i++].e_help, MSGSTR(ETCONNECT, "connect program to remote host")); /*MSG*/
#endif CONNECT
	strcpy(etable[i++].e_help, MSGSTR(ETCD, "change directory")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETEXIT, "exit from tip")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETEXIT, "exit from tip")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETSUSPALL, "suspend tip (local+remote)")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETSUSPLOC, "suspend tip (local only)")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETSETVAR, "set variable")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETGETSUM, "get this summary")); /*MSG*/
	strcpy(etable[i++].e_help, MSGSTR(ETSENDBRK, "send break")); /*MSG*/
}
