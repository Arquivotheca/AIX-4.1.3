static char sccsid[] = "@(#)34	1.12  src/bos/usr/bin/tip/cmds.c, cmdtip, bos411, 9428A410j 5/11/94 10:11:48";
/* 
 * COMPONENT_NAME: UUCP cmds.c
 * 
 * FUNCTIONS: MSGSTR, abort, anyof, args, chdirectory, consh, cu_put, 
 *            cu_take, execute, expand, finish, genbrk, getfl, 
 *            intcopy, pipefile, pipeout, prtime, send, sendfile, 
 *            setscript, shell, stopsnd, suspend, tandem, timeout, 
 *            transfer, transmit, variable 
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

/* static char sccsid[] = "cmds.c	5.4 (Berkeley) 5/5/86"; */

#include "tip.h"
#include <nl_types.h>    
#include "tip_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TIP,n,s) 
/*
 * tip
 *
 * miscellaneous commands
 */

int	quant[] = { 60, 60, 24 };

char	null = '\0';
char	sep[][50] = { "second", "minute", "hour" };
static char *argv[10];		/* argument vector for take and put */

void	timeout(int);		/* timeout function called on alarm */
void	stopsnd(int);		/* SIGINT handler during file transfers */
int	intprompt();		/* used in handling SIG_INT during prompt */
void	intcopy(int);		/* interrupt routine for file transfers */

/*
 * FTP - remote ==> local
 *  get a file from the remote host
 */
getfl(c)
	char c;
{
	char buf[256], *cp, *expand();
	
	putchar(c);
	/*
	 * get the UNIX receiving file's name
	 */
	if (prompt(MSGSTR(LOCALFILE, "Local file name? "), copyname)) /*MSG*/
		return;
	cp = expand(copyname);
	if ((sfd = creat(cp, 0666)) < 0) {
		printf(MSGSTR(CANTCREAT, "\r\n%s: cannot creat\r\n"), copyname); /*MSG*/
		return;
	}
	
	/*
	 * collect parameters
	 */
	if (prompt(MSGSTR(LISTCOMM, "List command for remote system? "), buf)) { /*MSG*/
		unlink(copyname);
		return;
	}
	transfer(buf, sfd, value(EOFREAD));
}

/*
 * Cu-like take command
 */
cu_take(cc)
	char cc;
{
	int fd, argc;
	char line[BUFSIZ], *expand(), *cp;

	if (prompt(MSGSTR(TAKE, "[take] "), copyname)) /*MSG*/
		return;
	if ((argc = args(copyname, argv)) < 1 || argc > 2) {
		printf(MSGSTR(USAGE, "usage: <take> from [to]\r\n")); /*MSG*/
		return;
	}
	if (argc == 1)
		argv[1] = argv[0];
	cp = expand(argv[1]);
	if ((fd = creat(cp, 0666)) < 0) {
		printf(MSGSTR(CANTCREAT2, "\r\n%s: cannot create\r\n"), argv[1]); /*MSG*/
		return;
	}
#ifdef _DEBUG
	fprintf(stderr, "cu_take: fd = %d\n", fd);
#endif
	sprintf(line, "/usr/bin/ksh -c \"/bin/cat %s; echo '\\01'\"", argv[0]);
	transfer(line, fd, "\01");
}

static	jmp_buf intbuf;
/*
 * Bulk transfer routine --
 *  used by getfl(), cu_take(), and pipefile()
 */
transfer(buf, fd, eofchars)
	char *buf, *eofchars;
	int fd;
{
	register int ct;
	char c, buffer[BUFSIZ];
	register char *p = buffer;
	register int cnt, eof;
	time_t start;
	void (*f)(int);

	pwrite(FD, buf, size(buf));
	quit = 0;
	kill(pid, SIGIOT);
	read(repdes[0], (char *)&ccc, 1);  /* Wait until read process stops */
	
	/*
	 * finish command
	 */
	pwrite(FD, "\r", 1);
#ifdef _DEBUG
	fprintf(stderr,"pmask = %0o\n", pmask);
#endif
	do {
		read(FD, &c, 1); 
#ifdef _DEBUG
	fprintf(stderr,"%c", (c&pmask));
#endif
	}
	while ((c&pmask) != '\n');
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag |= ISIG;	/* enable quit and interrupt */
	tcsetattr(0,TCSANOW,&tbuf);
#else
	ioctl(0, TIOCSETC, &defchars);
#endif
	
	(void) setjmp(intbuf);
	f = signal(SIGINT, (void(*)(int)) intcopy);
	start = time(&start);
	for (ct = 0; !quit;) {
		eof = (read(FD, &c, 1) <= 0);
		c &= pmask;
#ifdef _DEBUG
	fprintf(stderr,"read(%c) = %d\n",c,eof);
#endif
		if (quit)
			continue;
		if (eof || any(c, eofchars))
			break;
		if (c == 0)
			continue;	/* ignore nulls */
		if (c == '\r')
			continue;
		*p++ = c;

		if (c == character(value(PROMPT)) && boolean(value(VERBOSE)))
			printf("\r%d", ++ct);
		if ((cnt = (p-buffer)) == number(value(FRAMESIZE))) {
#ifdef _DEBUG
		fprintf(stderr,"\n\nwrite(%d,buffer,%d)\n\n\n",fd,cnt);
#endif
			if (write(fd, buffer, cnt) != cnt) {
				printf(MSGSTR(WRITERR, "\r\nwrite error\r\n")); /*MSG*/
				quit = 1;
			}
			p = buffer;
		}
	}
	if (cnt = (p-buffer)) {
#ifdef _DEBUG
		fprintf(stderr,"\n\nwrite(%d,buffer,%d)\n\n\n",fd,cnt);
#endif
		if (write(fd, buffer, cnt) != cnt)
			printf(MSGSTR(WRITERR, "\r\nwrite error\r\n")); /*MSG*/
	}

	if (boolean(value(VERBOSE)))
		prtime(MSGSTR(LINESXFER, " lines transferred in "), time(0)-start); /*MSG*/
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag &= ~ISIG; /* disable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &tchars);
#endif
	write(fildes[1], (char *)&ccc, 1);
	signal(SIGINT, f);
	close(fd);
}

/*
 * FTP - remote ==> local process
 *   send remote input to local process via pipe
 */
pipefile()
{
	int cpid, pdes[2];
	char buf[256];
	int status, p;
	extern int errno;

	if (prompt(MSGSTR(LOCALCOMM, "Local command? "), buf)) /*MSG*/
		return;

	if (pipe(pdes)) {
		printf(MSGSTR(CANTPIPE, "can't establish pipe\r\n")); /*MSG*/
		return;
	}

	if ((cpid = fork()) < 0) {
		printf(MSGSTR(CANTFORK, "can't fork!\r\n")); /*MSG*/
		return;
	} else if (cpid) {
		if (prompt(MSGSTR(LISTCOMM, "List command for remote system? "), buf)) { /*MSG*/
			close(pdes[0]), close(pdes[1]);
			kill (cpid, SIGKILL);
		} else {
			close(pdes[0]);
			signal(SIGPIPE, (void(*)(int)) intcopy);
			transfer(buf, pdes[1], value(EOFREAD));
			signal(SIGPIPE, SIG_DFL);
			while ((p = wait(&status)) > 0 && p != cpid)
				;
		}
	} else {
		register int f;

		dup2(pdes[0], 0);
		close(pdes[0]);
		for (f = 3; f < 20; f++)
			close(f);
		execute(buf);
		printf(MSGSTR(CANTEXECL, "can't execl!\r\n")); /*MSG*/
		exit(0);
	}
}

/*
 * Interrupt service routine for FTP
 */
void stopsnd(int s)
{

	stop = 1;
	signal(SIGINT, SIG_IGN);
}

/*
 * FTP - local ==> remote
 *  send local file to remote host
 *  terminate transmission with pseudo EOF sequence
 */
sendfile(cc)
	char cc;
{
	FILE *fd;
	char *fnamex;
	char *expand();

	putchar(cc);
	/*
	 * get file name
	 */
	if (prompt(MSGSTR(LOCALFILE, "Local file name? "), fname)) /*MSG*/
		return;

	/*
	 * look up file
	 */
	fnamex = expand(fname);
	if ((fd = fopen(fnamex, "r")) == NULL) {
		printf(MSGSTR(CANTOPEN, "%s: cannot open\r\n"), fname); /*MSG*/
		return;
	}
	transmit(fd, value(EOFWRITE), NULL);
	if (!boolean(value(ECHOCHECK))) {
#ifdef _AIX
		tcflush(FD, TCIOFLUSH);	/* flush input/output queues */
#else
		struct sgttyb buf;

		ioctl(FD, TIOCGETP, &buf);	/* this does a */
		ioctl(FD, TIOCSETP, &buf);	/*   wflushtty */
#endif
	}
}

/*
 * Bulk transfer routine to remote host --
 *   used by sendfile() and cu_put()
 */
transmit(fd, eofchars, command)
	FILE *fd;
	char *eofchars, *command;
{
	char *pc, lastc;
	int c, ccount, lcount;
	time_t start_t, stop_t;
	void (*f)(int);

	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	stop = 0;
	f = signal(SIGINT, (void(*)(int)) stopsnd);
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag |= ISIG;	/* enable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &defchars);
#endif
	read(repdes[0], (char *)&ccc, 1);
	if (command != NULL) {
		for (pc = command; *pc; pc++)
			send(*pc);
		if (boolean(value(ECHOCHECK)))
			read(FD, (char *)&c, 1);	/* trailing \n */
		else {
#ifdef _AIX
		tcflush(FD, TCIOFLUSH);	/* flush input/output queues */
#else
			struct sgttyb buf;

			ioctl(FD, TIOCGETP, &buf);	/* this does a */
			ioctl(FD, TIOCSETP, &buf);	/*   wflushtty */
#endif
#undef sleep
			sleep(5); /* wait for remote stty to take effect */
		}
	}
	lcount = 0;
	lastc = '\0';
	start_t = time(0);
	while (1) {
		ccount = 0;
		do {
			c = getc(fd);
			if (stop)
				goto out;
			if (c == EOF)
				goto out;
			if (c == 0177 && !boolean(value(RAWFTP)))
				continue;
			lastc = c;
			if (c < 040) {
				if (c == '\n') {
					if (!boolean(value(RAWFTP)))
						c = '\r';
				}
				else if (c == '\t') {
					if (!boolean(value(RAWFTP))) {
						if (boolean(value(TABEXPAND))) {
							send(' ');
							while ((++ccount % 8) != 0)
								send(' ');
							continue;
						}
					}
				} else
					if (!boolean(value(RAWFTP)))
						continue;
			}
			send(c);
		} while (c != '\r' && !boolean(value(RAWFTP)));
		if (boolean(value(VERBOSE)))
			printf("\r%d", ++lcount);
		if (boolean(value(ECHOCHECK))) {
			timedout = 0;
			alarm(value(ETIMEOUT));
			do {	/* wait for prompt */
				read(FD, (char *)&c, 1);
				if (timedout || stop) {
					if (timedout)
						printf(MSGSTR(TIMEDOUT, "\r\ntimed out at eol\r\n")); /*MSG*/
					alarm(0);
					goto out;
				}
			} while ((c&pmask) != character(value(PROMPT)));
			alarm(0);
		}
	}
out:
	if (lastc != '\n' && !boolean(value(RAWFTP)))
		send('\r');
	for (pc = eofchars; *pc; pc++)
		send(*pc);
	stop_t = time(0);
	fclose(fd);
	signal(SIGINT, f);
	if (boolean(value(VERBOSE)))
		if (boolean(value(RAWFTP)))
			prtime(MSGSTR(CHARSXFER, " chars transferred in "), stop_t-start_t); /*MSG*/
		else
			prtime(MSGSTR(LINESXFER, " lines transferred in "), stop_t-start_t); /*MSG*/
	write(fildes[1], (char *)&ccc, 1);
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag &= ~ISIG; /* disable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &tchars);
#endif
}

/*
 * Cu-like put command
 */
cu_put(cc)
	char cc;
{
	FILE *fd;
	char line[BUFSIZ];
	int argc;
	char *expand();
	char *copynamex;

	if (prompt(MSGSTR(PUT, "[put] "), copyname)) /*MSG*/
		return;
	if ((argc = args(copyname, argv)) < 1 || argc > 2) {
		printf(MSGSTR(USAGE2, "usage: <put> from [to]\r\n")); /*MSG*/
		return;
	}
	if (argc == 1)
		argv[1] = argv[0];
	copynamex = expand(argv[0]);
	if ((fd = fopen(copynamex, "r")) == NULL) {
		printf(MSGSTR(CANTOPEN, "%s: cannot open\r\n"), copynamex); /*MSG*/
		return;
	}
	if (boolean(value(ECHOCHECK)))
		sprintf(line, "/bin/cat>%s\r", argv[1]);
	else
		sprintf(line, "/bin/stty -echo;/bin/cat>%s;/bin/stty echo\r", argv[1]);
	transmit(fd, "\04", line);
}

/*
 * FTP - send single character
 *  wait for echo & handle timeout
 */
send(c)
	char c;
{
	char cc;
	int retry = 0;

	cc = c;
	pwrite(FD, &cc, 1);
#ifdef notdef
	if (number(value(CDELAY)) > 0 && c != '\r')
		nap(number(value(CDELAY)));
#endif
	if (!boolean(value(ECHOCHECK))) {
#ifdef notdef
		if (number(value(LDELAY)) > 0 && c == '\r')
			nap(number(value(LDELAY)));
#endif
		return;
	}
tryagain:
	timedout = 0;
	alarm(value(ETIMEOUT));
	read(FD, &cc, 1);
	alarm(0);
	if (timedout) {
		printf(MSGSTR(TIMEOUTERR, "\r\ntimeout error (%s)\r\n"), ctrl(c)); /*MSG*/
		if (retry++ > 3)
			return;
		pwrite(FD, &null, 1); /* poke it */
		goto tryagain;
	}
}

void timeout(int s)
{
	signal(SIGALRM, (void(*)(int)) timeout);
	timedout = 1;
}

/*
 * Stolen from consh() -- puts a remote file on the output of a local command.
 *	Identical to consh() except for where stdout goes.
 */
pipeout(c)
{
	char buf[256];
	int cpid, status, p;
	time_t start;

	putchar(c);
	if (prompt(MSGSTR(LOCALCOMM, "Local command? "), buf)) /*MSG*/
		return;
	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag |= ISIG;	/* enable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &defchars);
#endif
	read(repdes[0], (char *)&ccc, 1);
	/*
	 * Set up file descriptors in the child and
	 *  let it go...
	 */
	if ((cpid = fork()) < 0)
		printf(MSGSTR(CANTFORK, "can't fork!\r\n")); /*MSG*/
	else if (cpid) {
		start = time(0);
		while ((p = wait(&status)) > 0 && p != cpid)
			;
	} else {
		register int i;

		dup2(FD, 1);
		for (i = 3; i < 20; i++)
			close(i);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execute(buf);
		printf(MSGSTR(CANTFIND, "can't find `%s'\r\n"), buf); /*MSG*/
		exit(0);
	}
	if (boolean(value(VERBOSE)))
		prtime(MSGSTR(AWAYFOR, "away for "), time(0)-start); /*MSG*/
	write(fildes[1], (char *)&ccc, 1);
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag &= ~ISIG; /* disable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &tchars);
#endif
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}

#ifdef CONNECT
/*
 * Fork a program with:
 *  0 <-> local tty in
 *  1 <-> local tty out
 *  2 <-> local tty out
 *  3 <-> remote tty in
 *  4 <-> remote tty out
 */
consh(c)
{
	char buf[256];
	int cpid, status, p;
	time_t start;

	putchar(c);
	if (prompt(MSGSTR(LOCALCOMM, "Local command? "), buf)) /*MSG*/
		return;
	kill(pid, SIGIOT);	/* put TIPOUT into a wait state */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag |= ISIG;	/* enable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &defchars);
#endif
	read(repdes[0], (char *)&ccc, 1);
	/*
	 * Set up file descriptors in the child and
	 *  let it go...
	 */
	if ((cpid = fork()) < 0)
		printf(MSGSTR(CANTFORK, "can't fork!\r\n")); /*MSG*/
	else if (cpid) {
		start = time(0);
		while ((p = wait(&status)) > 0 && p != cpid)
			;
	} else {
		register int i;

		dup2(FD, 3);
		dup2(3, 4);
		for (i = 5; i < 20; i++)
			close(i);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		execute(buf);
		printf(MSGSTR(CANTFIND, "can't find `%s'\r\n"), buf); /*MSG*/
		exit(0);
	}
	if (boolean(value(VERBOSE)))
		prtime(MSGSTR(AWAYFOR, "away for "), time(0)-start); /*MSG*/
	write(fildes[1], (char *)&ccc, 1);
#ifdef _AIX
	tcgetattr(0,&tbuf);
	tbuf.c_lflag &= ~ISIG; /* disable quit and interrupt */
	tcsetattr(0,TCSAFLUSH,&tbuf);
#else
	ioctl(0, TIOCSETC, &tchars);
#endif
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
}
#endif

/*
 * Escape to local shell
 */
shell()
{
	int shpid, status;
	extern char **environ;
	char *cp;

	printf(MSGSTR(SH, "[sh]\r\n")); /*MSG*/
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	unraw();
	if (shpid = fork()) {
		while (shpid != wait(&status));
		raw();
		printf("\r\n!\r\n");
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		return;
	} else {
		signal(SIGQUIT, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		if ((cp = rindex(value(SHELL), '/')) == NULL)
			cp = value(SHELL);
		else
			cp++;
		execl(value(SHELL), cp, 0);
		printf(MSGSTR(CANTEXECL2, "\r\ncan't execl!\r\n")); /*MSG*/
		exit(1);
	}
}

/*
 * TIPIN portion of scripting
 *   initiate the conversation with TIPOUT
 */
setscript()
{
	char c;
	/*
	 * enable TIPOUT side for dialogue
	 */
#ifdef _AIX221
	kill(pid, SIGDANGER);
#else
	kill(pid, SIGUSR1);
#endif
	if (boolean(value(SCRIPT)))
		write(fildes[1], value(RECORD), size(value(RECORD)));
	write(fildes[1], "\n", 1);
	/*
	 * wait for TIPOUT to finish
	 */
	read(repdes[0], &c, 1);
	if (c == 'n')
		printf(MSGSTR(CANTCREAT3, "can't create %s\r\n"), value(RECORD)); /*MSG*/
}

/*
 * Change current working directory of
 *   local portion of tip
 */
chdirectory()
{
	char dirname[80];
	register char *cp = dirname;

	if (prompt(MSGSTR(CD, "[cd] "), dirname)) { /*MSG*/
		if (stoprompt)
			return;
		cp = value(HOME);
	}
	if (chdir(cp) < 0)
		printf(MSGSTR(BADDIR, "%s: bad directory\r\n"), cp); /*MSG*/
	printf("!\r\n");
}

abort(msg)
	char *msg;
{

	kill(pid, SIGTERM);
	setreuid(euid, euid);
	setregid(egid, egid);
	disconnect(msg);
	if (msg != NOSTR)
		printf("\r\n%s", msg);
	printf(MSGSTR(EOT, "\r\n[EOT]\r\n")); /*MSG*/
	ttyunlock(uucplock);
	unraw();
	exit(0);
}

finish()
{
	char *dismsg;

	if ((dismsg = value(DISCONNECT)) != NOSTR) {
		write(FD, dismsg, strlen(dismsg));
		sleep(5);
	}
	abort(NOSTR);
}

void intcopy(int s)
{

	raw();
	quit = 1;
	longjmp(intbuf, 1);
}

execute(s)
	char *s;
{
	register char *cp;

	if ((cp = rindex(value(SHELL), '/')) == NULL)
		cp = value(SHELL);
	else
		cp++;
	execl(value(SHELL), cp, "-c", s, 0);
}

args(buf, a)
	char *buf, *a[];
{
	register char *p = buf, *start;
	register char **parg = a;
	register int n = 0;

	do {
		while (*p && (*p == ' ' || *p == '\t'))
			p++;
		start = p;
		if (*p)
			*parg = p;
		while (*p && (*p != ' ' && *p != '\t'))
			p++;
		if (p != start)
			parg++, n++;
		if (*p)
			*p++ = '\0';
	} while (*p);

	return(n);
}

prtime(s, a)
	char *s;
	time_t a;
{
	register i;
	int nums[3];

	for (i = 0; i < 3; i++) {
		nums[i] = (int)(a % quant[i]);
		a /= quant[i];
	}
	printf("%s", s);
	while (--i >= 0)
		if (nums[i] || i == 0 && nums[1] == 0 && nums[2] == 0)
			if (nums[i] == 1)
				printf(MSGSTR(NUMSING, "%d %s "), nums[i], sep[i]); /*MSG*/
			else
				printf(MSGSTR(NUMPLURL, "%d %ss "), nums[i], sep[i]); /*MSG*/
	printf("\r\n!\r\n");
}

variable()
{
	char	buf[256];

	if (prompt(MSGSTR(SET, "[set] "), buf)) /*MSG*/
		return;
	vlex(buf);
	if (vtable[BEAUTIFY].v_access&CHANGED) {
		vtable[BEAUTIFY].v_access &= ~CHANGED;
		kill(pid, SIGSYS);
	}
	if (vtable[SCRIPT].v_access&CHANGED) {
		vtable[SCRIPT].v_access &= ~CHANGED;
		setscript();
		/*
		 * So that "set record=blah script" doesn't
		 *  cause two transactions to occur.
		 */
		if (vtable[RECORD].v_access&CHANGED)
			vtable[RECORD].v_access &= ~CHANGED;
	}
	if (vtable[RECORD].v_access&CHANGED) {
		vtable[RECORD].v_access &= ~CHANGED;
		if (boolean(value(SCRIPT)))
			setscript();
	}
	if (vtable[TAND].v_access&CHANGED) {
		vtable[TAND].v_access &= ~CHANGED;
		if (boolean(value(TAND)))
			tandem("on");
		else
			tandem("off");
	}
 	if (vtable[LECHO].v_access&CHANGED) {
 		vtable[LECHO].v_access &= ~CHANGED;
 		HD = boolean(value(LECHO));
 		if (HD == TRUE)
			tbuf.c_lflag |= ECHO;
		else
			tbuf.c_lflag &= ~ECHO;
		tcsetattr(0, TCSANOW, &tbuf);
 	}
	if (vtable[PARITY].v_access&CHANGED) {
		vtable[PARITY].v_access &= ~CHANGED;
		setparity();
	}
}

/*
 * Turn tandem mode on or off for remote tty.
 */
tandem(option)
	char *option;
{
#ifdef _AIX
	struct termio rmtty;

	tcgetattr(FD,&rmtty);
	if (strcmp(option,"on") == 0) {
		rmtty.c_iflag |= IXOFF|IXON;	/* enable input flow control */
		tbuf.c_iflag |= IXOFF|IXON; /* enable input flow control  */
	} else {
		rmtty.c_iflag &= ~(IXOFF|IXON); /* disable input flow control */
		tbuf.c_iflag &= ~(IXOFF|IXON); /* disable input flow control */
	}
	tcsetattr(FD,TCSAFLUSH,&rmtty);
	tcsetattr(FD,TCSAFLUSH,&tbuf);
#else
	struct sgttyb rmtty;

	ioctl(FD, TIOCGETP, &rmtty);
	if (strcmp(option,"on") == 0) {
		rmtty.sg_flags |= TANDEM;
		arg.sg_flags |= TANDEM;
	} else {
		rmtty.sg_flags &= ~TANDEM;
		arg.sg_flags &= ~TANDEM;
	}
	ioctl(FD, TIOCSETP, &rmtty);
	ioctl(0,  TIOCSETP, &arg);
#endif
}

/*
 * Send a break.
 */
genbrk()
{

#ifdef _AIX
	tcsendbreak(FD,25); /* send break for .25 seconds */
#else
	ioctl(FD, TIOCSBRK, NULL);
	sleep(1);
	ioctl(FD, TIOCCBRK, NULL);
#endif
}

/*
 * Suspend tip
 */
suspend(c)
	char c;
{

	unraw();
#if !defined(_AIX)
	kill(c == CTRL(y) ? getpid() : 0, SIGTSTP);
#endif
	raw();
}

/*
 *	expand a file name if it includes shell meta characters
 */

char *
expand(name)
	char name[];
{
	static char xname[BUFSIZ];
	char cmdbuf[BUFSIZ];
	register int pid, l, rc;
	register char *cp, *Shell;
	int s, pivec[2], (*sigint)();

	if (!anyof(name, "~{[*?$`'\"\\"))
		return(name);
	/* sigint = signal(SIGINT, SIG_IGN); */
	if (pipe(pivec) < 0) {
		perror(MSGSTR(PIPE, "pipe")); /*MSG*/
		/* signal(SIGINT, sigint) */
		return(name);
	}
	sprintf(cmdbuf, "/bin/echo %s", name);
#ifdef _AIX
	if ((pid = fork()) == 0) {
#else 
	if ((pid = vfork()) == 0) {
#endif
		Shell = value(SHELL);
		if (Shell == NOSTR)
			Shell = "/usr/bin/ksh";
		close(pivec[0]);
		close(1);
		dup(pivec[1]);
		close(pivec[1]);
		close(2);
		execl(Shell, Shell, "-c", cmdbuf, 0);
		_exit(1);
	}
	if (pid == -1) {
		perror(MSGSTR(FORK, "fork")); /*MSG*/
		close(pivec[0]);
		close(pivec[1]);
		return(NOSTR);
	}
	close(pivec[1]);
	while (wait(&s) != pid)
		;
	l = read(pivec[0], xname, BUFSIZ);
	close(pivec[0]);
	s &= 0377;
	if (s != 0 && s != SIGPIPE) {
		fprintf(stderr, MSGSTR(ECHOFAIL, "\"Echo\" failed\n")); /*MSG*/
		return(NOSTR);
	}
	if (l < 0) {
		perror(MSGSTR(READIT, "read")); /*MSG*/
		return(NOSTR);
	}
	if (l == 0) {
		fprintf(stderr, MSGSTR(NOMATCH, "\"%s\": No match\n"), name); /*MSG*/
		return(NOSTR);
	}
	if (l == BUFSIZ) {
		fprintf(stderr, MSGSTR(BUFFOFLOW, "Buffer overflow expanding \"%s\"\n"), name); /*MSG*/
		return(NOSTR);
	}
	xname[l] = 0;
	for (cp = &xname[l-1]; *cp == '\n' && cp > xname; cp--)
		;
	*++cp = '\0';
	return(xname);
}

/*
 * Are any of the characters in the two strings the same?
 */

anyof(s1, s2)
	register char *s1, *s2;
{
	register int c;

	while (c = *s1++)
		if (any(c, s2))
			return(1);
	return(0);
}
