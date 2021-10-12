static char sccsid[] = "@(#)69	1.11  src/bos/usr/bin/capture/capture.c, cmdsh, bos41B, 412_41B_sync 12/15/94 12:11:34";
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
 * capture.c - allows screen dumps to a file with Control-P (^P)
 *
 */
 
/* define to use faster MACROS instead of functions for performance */
#define _ILS_MACROS

#include <cur00.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/id.h>

#include <termios.h>

#include <sys/time.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <langinfo.h>
#include <ctype.h>
#define  TSSIZE 128

#include "vtparse.h"
#include "vtif.h"

#include "capture_msg.h" 
static nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_CAPTURE,n,s) 

static char *d_t_format;
static char buf[TSSIZE];



#define	CNTRL_P	0x10 		/* Control-P (^P) causes screen dump */

static struct	termios b;
static struct	winsize old_win;
static struct	winsize new_win = { 24, 80, 0, 0 };

static char	*shell;
static char	cntrl_key = CNTRL_P;	/* Default screen dump key is Control-P */
static FILE	*fscript;
static int	master;
static int	slave;
static int	child;
static int	subchild;
static char	*fname = "screen.out";
int	finish(void);
static void dooutput();

static struct	tchars tc;
static struct	ltchars lc;
static int	lb;
static int	l;
static char	line[12];
static int	aflg;
static int     input_esc = 0;          /* are we parsing an esc sequence on input? */
static int     found_esc = 0;          /* are we parsing an esc sequence? */
void	screen_dump(int);	/* perform the screen dump to a file */
static char	*oldtty;
static char	*newtty;
static uid_t	real_uid;
static uid_t	effective_uid;

main(argc, argv)
	int argc;
	char *argv[];
{
	struct sigaction	chld;
	char c, *octal_escape, *control();

	real_uid = getuidx(ID_REAL);
	effective_uid = getuidx(ID_EFFECTIVE);

	/*
	 * this program has become a SUID program and privileges need
	 * to be lowered until needed in doshell() and done().
	 */
	seteuid(real_uid);

	oldtty = strdup((char *) ttyname(2));

	strcpy(line, "/dev/ptyXX");
	setlocale(LC_ALL, "");

	catd = catopen(MF_CAPTURE,NL_CAT_LOCALE);

	d_t_format= nl_langinfo(D_T_FMT);

	sigaction(SIGCHLD, (struct sigaction *)0, &chld);
	chld.sa_handler = (void (*)(int))finish ;
	sigaction(SIGCHLD, &chld, (struct sigaction *)0);
	shell = getenv("SHELL");
	if (shell == 0)
		shell = "/usr/bin/bsh";
	/* capture has the "interesting" feature that the user can specify the
	 * desired screen dump key in OCTAL through the SCREENDUMP environment
	 * variable
	 */
	if (octal_escape = getenv("SCREENDUMP")) {
		if (!(cntrl_key = (cc_t) otoi(octal_escape))) {
		    printf(MSGSTR(INV_OCT_ESC,
		    "Invalid octal escape value %s\n"), octal_escape); /*MSG */
			cntrl_key = CNTRL_P;
		}
	}
	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch (argv[0][1]) {

		case 'a':
			aflg++;
			break;

		default:
			fprintf(stderr,
			    MSGSTR(USAGE, "usage: capture [ -a ] [ %s ]\n"), fname); /*MSG*/
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

	/*
	 * newtty is the pty for the new shell. Save it off to set
	 * correct permissions - doshell() - and reset it at the
	 * end of the process - done()
	 */
	newtty = strdup((char *) ttyname(master));

	printf(MSGSTR(CAPTURESTART, "Capture started, file is %s\n"), fname); /*MSG*/
	printf(MSGSTR(CAPTURESTART1, "Use %s to dump screen to file %s\n"), control(cntrl_key), fname); /*MSG*/
	printf(MSGSTR(CAPTURESTART2, "You are now emulating a vt100 terminal.\n")); /*MSG*/

	fixtty();
	printf(MSGSTR(CAPTURESTART3, "Press Any Key to continue.\n")); /*MSG*/
	if (read(fileno(stdin), &c, 1) < 0)
		perror("read");

	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) {
		subchild = child = fork();
		if (child < 0) {
			perror("fork");
			fail();
		}
		if (child) {
			time_t tvec;

			(void) close(0);
			tvec = time((time_t *)0);
			strftime(buf, TSSIZE, d_t_format, localtime(&tvec));
			fprintf(fscript,
				MSGSTR(STARTCAP,
				"Capture started on %s"), buf); /*MSG*/
			fprintf(fscript,"\n");
			vtinit();     /* initialize curses and ANSI emulation */
			(void) signal(SIGUSR2, (void(*)(int))screen_dump);
			dooutput();
		} else 
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
	char ibuf[BUFSIZ], *p;
	int cc;

	(void) fclose(fscript);
	while ((cc = read(0, ibuf, sizeof(ibuf))) > 0) {
		for (p = ibuf; p < ibuf + cc ; p++) {
			/*
			 * Here we need to look at the characters to see if
			 * they are an ESC. If so do the correct start
			 * parsing the ANSI escape sequence.
			 */
			if (*p == ESC) {
				input_esc = 1;
				continue;
			}
			if (input_esc) {
				/* parse ANSI esc sequence */
				input_esc = vt1(*p, master);
			} else {
				if (*p == cntrl_key) 	/* if cntrl_key dump */
					(void) kill(child, SIGUSR2);
				else
					(void) write(master, p, 1);
			}
		}
	}
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

static void
dooutput()
{
	char obuf;
	int cc;

	for (;;) {
		cc = read(master, &obuf, sizeof (obuf));
		if (cc <= 0)
			break;
		/*
		 * Here we need to read in characters to determine if
		 * they are an ESC. If so do the correct start parsing the
		 * ANSI escape sequence.
		 */
		if (obuf == ESC) {
			found_esc = 1;
			continue;
		}
		if (found_esc) {
			/* parse ANSI esc sequence */
			cc = vt2(obuf);
                        switch(cc) {
                        case -1:        /* abort */
				/* send out the original ESC char */
				vtaddch(ESC);
				vtaddch(obuf);
                                /* FALL THROUGH */
                        case 0:         /* normal end */
				found_esc = 0;
                                break;
                        default:        /* still handling */
                                break;
                        }
		} else {
			vtaddch(obuf);
		}
	}
	vtclose();
	done();
}


/*
 * NAME:  doshell
 *
 * FUNCTION:  Run a shell to execute commands in.
 *
 */

static doshell()
{
	gid_t	us_gid;
	struct stat buf;

	us_gid = getgid();

	/*
	 * get the mode of the pty where this program was called from
	 * to be able to set the correct permissions on the new pty
	 */
	if ( stat(oldtty, &buf) != 0 )
	    buf.st_mode = (S_IRUSR|S_IWUSR|S_IWGRP|S_IWOTH|S_ISVTX);

	setsid();
	getslave();
	(void) close(master);
	(void) fclose(fscript);
	(void) dup2(slave, 0);
	(void) dup2(slave, 1);
	(void) dup2(slave, 2);
	(void) close(slave);
	(void) putenv("TERM=vt100");


	/*
	 * Raise the privilege of the program to set the ownership and
	 * mode of the new pty to be the same as the pty where the
	 * command was issued from. Lower the privilege after done.
	 */

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

	execl(shell, "bsh", "-is", 0);
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

/*	sbuf.c_iflag &= ~(INLCR | IUCLC | ISTRIP | IXON | BRKINT ); */
	sbuf.c_iflag &= ~ISIG;
	sbuf.c_lflag &= ~(ECHO | ICANON); 
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
		strftime(buf, TSSIZE, d_t_format, localtime(&tvec));
		fprintf(fscript,MSGSTR(ENDCAP, "\nCapture done on %s"), buf); /*MSG*/
		fprintf(fscript, "\n");
		(void) fclose(fscript);
		(void) close(master);
	} else {
		if ( -1 == tcsetattr((int)fileno(stdin), TCSANOW, &b) ) {
			perror("tcsetattr 3");
		}
		printf(MSGSTR(DONECAP, "Capture done, file is %s\n"), fname); /*MSG*/
		printf(MSGSTR(DONECAP2, "You are NO LONGER emulating a vt100 terminal.\n")); /*MSG*/
	}

	/*
	 * Raise privilege to reset pty to original mode and
	 * ownership : 666/root,system. Lower privilege when done.
	 */
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
	if ( -1 == ioctl(fileno(stdin), TIOCGWINSZ, (char *)&old_win) ) {
		perror("ioctl TIOCGWINSZ");
	}
	if (old_win.ws_row < 24)
		new_win.ws_row = old_win.ws_row;
	if (old_win.ws_col < 80)
		new_win.ws_col = old_win.ws_col;
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

static void
screen_dump(int s)
{
	int x, y;

	fputwc('\n', fscript);
	for (y = 0; y < stdscr->_maxy; y++) {
		for (x = 0; x < stdscr->_maxx; x++)
			fputwc( stdscr->_y[y][x], fscript);
		fputwc('\n', fscript);
	}
	(void) signal(SIGUSR2, (void(*)(int)) screen_dump);
	dooutput();
}

/*
 * Construct a control character sequence
 * for a special character.
 */
static char *
control(c)
        register cc_t c;
{
        static char buf[3];

        if (c == 0x7f)
                return ("^?");
        if (c == '\377') {
		return control(CNTRL_P);
        }
        if (c >= 0x20) {
                buf[0] = c;
                buf[1] = 0;
        } else {
                buf[0] = '^';
                buf[1] = '@'+c;
                buf[2] = 0;
        }
        return (buf);
}

/* silly */
static otoi(string)
register char *string;
{
        register int answer = 0;

        while (isdigit(*string)) {
                answer <<= 3;
                answer += (*string) - '0';
                ++string;
        }
        return answer;
}
