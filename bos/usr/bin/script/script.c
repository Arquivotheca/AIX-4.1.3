static char sccsid[] = "@(#)26  1.18  src/bos/usr/bin/script/script.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:12:27";
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27
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
 */
/*
 * script.c - makes a typescript of everything printed on the terminal.
 *
 */
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <termios.h>

#include <sys/time.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/id.h>

#include "script_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SCRIPT,n,s) 
#include <langinfo.h>
#define TSSIZE 128
static char *d_t_format;
static char buf[TSSIZE];


static struct	termios b;

static char	*shell;
static FILE	*fscript;
static int	master;
static int	slave;
static int	child;
static int	subchild;
static char	*fname = "typescript";
int	finish(void);

static struct	tchars tc;
static struct	ltchars lc;
static int	lb;
static int	l;
static char	line[12];
static int	aflg;
static char	*oldtty;
static char	*newtty;
static int	p[2];
static char	inbuf[25];
static uid_t	real_uid;
static uid_t	effective_uid;

main(argc, argv)
	int argc;
	char *argv[];
{
	struct sigaction	chld;

	real_uid = getuidx(ID_REAL);
	effective_uid = getuidx(ID_EFFECTIVE);

	seteuid(real_uid);

	oldtty = strdup((char *) ttyname(2));

	strcpy(line, "/dev/ptyXX");
	setlocale(LC_ALL, "");

	catd = catopen(MF_SCRIPT,NL_CAT_LOCALE);

	d_t_format = nl_langinfo(D_T_FMT);

	sigaction(SIGCHLD, (struct sigaction *)0, &chld);
	chld.sa_handler = (void (*)(int))finish ;
	sigaction(SIGCHLD, &chld, (struct sigaction *)0);
	shell = getenv("SHELL");
	if (shell == 0)
		shell = "/usr/bin/sh";
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch (argv[0][1]) {

		case 'a':
			aflg++;
			break;

		default:
			fprintf(stderr,
			    MSGSTR(USAGE, "usage: script [ -a ] [ typescript ]\n")); /*MSG*/
			exit(1);
		}
		argc--, argv++;
	}
	if (argc > 0)
		fname = argv[0];
	if ((fscript = fopen(fname, aflg ? "a" : "w")) == NULL) {
		perror(fname);
		fail();
	}
	getmaster();
	newtty = strdup((char *) ttyname(master));
	printf(MSGSTR(SCRIPTSTART, "Script started, file is %s\n"), fname); /*MSG*/
		fflush(stdout);
	fixtty();

	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) {
		if (pipe(p) < 0) {
			perror("script");
			fail();
		}
		subchild = child = fork();
		if (child < 0) {
			perror("fork");
			fail();
		}
		if (child)
			dooutput();
		else 
		 	doshell();
	}
	doinput();
}

/*
 * NAME: doinput
 *
 * FUNCTION:  Gets input from the keyboard and puts it into a buffer.
 *
 */

static doinput()
{
	char ibuf[BUFSIZ];
	int cc;

	(void) fclose(fscript);
	while ((cc = read(0, ibuf, (unsigned) BUFSIZ)) > 0)
		(void) write(master, ibuf, (unsigned)cc);
	done();
}


/*
 * NAME: finish 
 *
 * FUNCTION:  Waits for all children to die before exiting. 
 *
 */

static finish(void)
{
	int status;
	register int pid;
	register int die = 0;

	while ((pid = wait3(&status, WNOHANG, 0)) > 0)
		if (pid == child)
			die = 1;

	if (die)
		done();
}


/*
 * NAME:  dooutput
 *
 * FUNCTION:  Writes terminal output to file.
 *
 */

static dooutput()
{
	time_t tvec;
	char obuf[BUFSIZ];
	int cc;

	(void) close(p[1]);
	(void) close(0);
	tvec = time((time_t *)0);
	strftime(buf, TSSIZE, d_t_format,localtime(&tvec));
	fprintf(fscript, MSGSTR(STARTSCR, "Script started on %s"), buf); /*MSG*/
	fflush(fscript);
	read(p[0], inbuf, 25);
	(void) close(p[0]);
	for (;;) {
		cc = read(master, obuf, sizeof (obuf));
		if (cc <= 0)
			break;
		(void) write(1, obuf, (unsigned)cc);
		(void) fwrite((void *)obuf, (size_t)1, (size_t)cc, fscript);
	}
	done();
}


/*
 * NAME:  doshell
 *
 * FUNCTION:  Run a shell to execute commands in.
 *            Change the ownership and permissions of the controlling
 *            termial.
 *
 */

static doshell()
{
	int t;
	gid_t	us_gid;
	struct stat buf;

	(void) close(p[0]);
	us_gid = getgid();
	if ( stat(oldtty, &buf) != 0 )
	    buf.st_mode = (S_IRUSR|S_IWUSR|S_IWGRP|S_IWOTH|S_ISVTX);

	setsid();
	getslave();
	write(p[1], "OK!  You can read now.", 25);
	(void) close(p[1]);
	(void) close(master);
	(void) fclose(fscript);
	(void) dup2(slave, 0);
	(void) dup2(slave, 1);
	(void) dup2(slave, 2);
	(void) close(slave);

	if ( seteuid(effective_uid) == 0 ) {
	    if ( chmod(newtty, buf.st_mode) != 0 ) {
		fprintf(stderr,MSGSTR(CHMOD_ERR, "chmod failed on tty. errno = %d\n"),
		    errno);
	    }
	    if ( chown(newtty, real_uid, us_gid) != 0 ) {
		fprintf(stderr,MSGSTR(CHOWN_ERR, "chown failed on tty. errno = %d\n"),
		    errno);
	    }
	    seteuid(real_uid);
	}

	execl(shell, "sh", "-is", 0);
	perror(shell);
	fail();
}


/*
 * NAME:  fixtty
 *
 * FUNCTION:  Set up the terminal.
 *
 */

static fixtty()
{
	struct termios sbuf;

	sbuf = b;

	sbuf.c_iflag &= ~( BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP |
			   			INLCR | IGNCR | ICRNL | IUCLC ); 
	sbuf.c_oflag = 0;
	sbuf.c_lflag &= ~( ISIG | ICANON | ECHO );

	sbuf.c_cc[VMIN] = 1;
	if ( -1 == tcsetattr((int)fileno(stdin), TCSANOW, &sbuf) ) {
		perror("tcsetattr 1");
	}
}


/*
 * NAME:  fail
 *
 * FUNCTION:  Kill the program on failure.
 *
 */

static fail()
{

	(void) kill(0, SIGTERM);
	done();
}


/*
 * NAME:  done
 *
 * FUNCTION:  Cleanup on exit. Close output file and respond to
 *            user with exit message.
 *
 */

static done()
{
	time_t tvec;
 
	if (subchild) {
		tvec = time((time_t *)0);
		strftime(buf, TSSIZE, d_t_format,localtime(&tvec));
		fprintf(fscript,MSGSTR(ENDSCR, "\nscript done on %s"), buf); /*MSG*/
		fflush(fscript);
		(void) fclose(fscript);
		(void) close(master);
	} else {
		if ( -1 == tcsetattr((int)fileno(stdin), TCSANOW, &b) ) {
			perror("tcsetattr 3");
		}
		printf(MSGSTR(DONESCRIPT, "Script done, file is %s\n"), fname); /*MSG*/
		fflush(stdout);
	}
	/* pop the berkeley line discipline before exiting */

	if ( seteuid(effective_uid) == 0 )
	{
	    if ( acl_set(newtty, R_ACC|W_ACC, R_ACC|W_ACC, R_ACC|W_ACC) != 0 ) {
		fprintf(stderr,MSGSTR(ACLSET_ERR, "acl_set failed on tty. errno = %d\n"),
		    errno);
	    }
	    if ( chown(newtty, 0, 0) != 0 ) {
		fprintf(stderr,MSGSTR(CHOWN_ERR, "chown failed on tty. errno = %d\n"),
		    errno);
	    }
	    seteuid(real_uid);
	}

	exit(0);
}


/*
 * NAME:  getmaster
 *
 * FUNCTION:  Get a controller pty.
 *
 */

static getmaster()
{
	char *pty, *bank, *cp;
	struct stat stb;

#ifdef _DEBUG
	fprintf(stderr, "getmaster() -- Attempting to use %s\n",
		line);
#endif

	master = open("/dev/ptc", O_RDWR);
	if ( -1 == master ) {
		perror("open master");
		fail();
	}
	if ( -1 == tcgetattr((int)fileno(stdin), &b) ) {
		perror("tcgetattr");
		fail();
	}
}


/*
 * NAME:  getslave
 *
 * FUNCTION:  Get a slave pty.
 *
 */

static getslave()
{

	slave = open(ttyname(master), O_RDWR);
	if (slave < 0) {
		perror(line);
		fail();
	}
	if ( -1 == tcsetattr(slave, TCSANOW, &b) ) {
		perror("tcsetattr 2");
	}
}
