static char sccsid [] = "@(#)68  1.14  src/bos/usr/bin/ex/ex_unix.c, cmdedit, bos41J, 9509A_all 2/21/95 15:34:01";
/*
 * COMPONENT_NAME: (CMDEDIT) ex_unix.c
 *
 * FUNCTIONS: filter, recover, revocer, unix0, unixex, unixwt, waitfor
 *
 * ORIGINS: 3, 10, 13, 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1981 Regents of the University of California
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */

#include "ex.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include "ex_vis.h"
#include <sys/wait.h>

static pid_t other_pid;		/* other direct descendents for filtres */

/*
 * Unix escapes, filtering
 */

/*
 * First part of a shell escape,
 * parse the line, expanding # and % and ! and printing if implied.
 */
void unix0(short warn)
{

	register char *up, *fp, *xfp;
	register int c;
	/* update nc_uxb in parallel with uxb */
	/* Internal vi uses wchar_t's, but the shell uses char's */
	/* vglobp is set to nc_uxb and the . command reads from vglobp */
	register wchar_t *nc_up;
	short printub;
	char puxb[sizeof(uxb)+1];
	wchar_t widec;
	int char_length;

	printub = 0;
	strcpy(puxb, uxb);
	c = ex_getchar();
	if (c == '\n' || c == EOF)
		error(MSGSTR(M_203, "Incomplete shell escape command@- use 'shell' to get a shell"), DUMMY_INT);
	up = uxb;
	nc_up = nc_uxb;
	do {
		switch (c) {

		case '\\':
			if (any(peekchar(), "%#!"))
				c = ex_getchar();
		default:
			if((up += wctomb(up, c))-1 >= &uxb[UXBSIZE]) {
tunix:
				uxb[0] = 0;
				nc_uxb[0] = 0;
				error(MSGSTR(M_204, "Command too long"), DUMMY_INT);
			}
			*nc_up++ = c;
			break;

		case '!':
			fp = puxb;
			if (*fp == 0) {
				uxb[0] = 0;
				nc_uxb[0] = 0;
				error(MSGSTR(M_205, "No previous command@to substitute for !"), DUMMY_INT);
			}
			printub++;
			while (*fp) {
				if (up >= &uxb[UXBSIZE])
					goto tunix;
				*up++ = *fp++;
			}
			fp = puxb;
			while (*fp)
				if((char_length = mbtowc(&widec, fp, MB_CUR_MAX)) > 0){
					fp += char_length;
					*nc_up++ = widec;
				}
			break;

		case '#':
			xfp = fp = altfile;
			if (*fp == 0) {
				uxb[0] = 0;
				nc_uxb[0] = 0;
				error(MSGSTR(M_206, "No alternate filename@to substitute for #"), DUMMY_INT);
			}
			goto uexp;

		case '%':
			xfp = fp = savedfile;
			if (*fp == 0) {
				uxb[0] = 0;
				nc_uxb[0] = 0;
				error(MSGSTR(M_207, "No filename@to substitute for %%"), DUMMY_INT);
			}
uexp:
			printub++;
			while (*fp) {
				if (up >= &uxb[UXBSIZE])
					goto tunix;
				*up++ = *fp++;
			}
			while (*xfp)
				if((char_length = mbtowc(&widec, xfp, MB_CUR_MAX)) > 0){
					xfp += char_length;
					*nc_up++ = widec;
				}
			break;
		}
		c = ex_getchar();
	} while (c == '"' || c == '|' || !endcmd(c));
	if (c == EOF)
		ungetchar(c);
	*up = 0;
	*nc_up = 0;
	if (!inopen)
		resetflav();
	if (warn)
		ckaw();
	if (warn && hush == 0 && chng && xchng != chng && value(WARN) && dol > zero) {
		xchng = chng;
		vnfl();
		mwarn(MSGSTR(M_208, "[No write]|[No write since last change]"), DUMMY_INT);
		noonl();
		flush();
	} else
		warn = 0;
	if (printub) {
		if (uxb[0] == 0)
			error(MSGSTR(M_209, "No previous command@to repeat"), DUMMY_INT);
		if (inopen) {
			splitw++;
			vclean();
			vgoto(WECHO, 0);
		}
		if (warn)
			vnfl();
		if (hush == 0)
			lprintf("!%s", uxb);
		if (inopen && Outchar != termchar) {
			vclreol();
			vgoto(WECHO, 0);
		} else
			putnl();
		flush();
	}
}

/*
 * Do the real work for execution of a shell escape.
 * Mode is like the number passed to open system calls
 * and indicates filtering.  If input is implied, newstdin
 * must have been setup already.
 */
ttymode
unixex(char *opt, char *up, int newstdin, int mode)
	/* opt is "-i" or "-c", up is exec'd */
{
	int pvec[2];
	ttymode f;

	signal(SIGINT, SIG_IGN);
#ifdef SIGTSTP
	if (dosusp)
		signal(SIGTSTP, SIG_DFL);
#endif
	if (inopen)
		f = setty(normf);
	if ((mode & 1) && pipe(pvec) < 0) {
		/* Newstdin should be io so it will be closed */
		if (inopen)
			setty(f);
		error(MSGSTR(M_210, "Can't make pipe for filter"), DUMMY_INT);
	}
#ifndef VFORK
	pid = fork();
#else
	pid = vfork();
#endif
	if (pid < 0) {
		if (mode & 1) {
			close(pvec[0]);
			close(pvec[1]);
		}
		setrupt();
		error(MSGSTR(M_211, "No more processes"), DUMMY_INT);
	}
	if (pid == 0) {
		if (mode & 2) {
			close(0);
			dup(newstdin);
			close(newstdin);
		}
		if (mode & 1) {
			close(pvec[0]);
			close(1);
			dup(pvec[1]);
			if (inopen) {
				close(2);
				dup(1);
			}
			close(pvec[1]);
		}
		if (io)
			close(io);
		if (tfile)
			close(tfile);
		signal(SIGHUP, oldhup);
		signal(SIGQUIT, oldquit);
		if (ruptible)
			signal(SIGINT, SIG_DFL);
		execlp(svalue(SHELL), "sh", opt, up, (char *) 0);
		ex_printf(MSGSTR(M_212, "No %s!\n"), svalue(SHELL));
		error((char *)0, DUMMY_INT);
	}
	if (mode & 1) {
		io = pvec[0];
		close(pvec[1]);
	}
	if (newstdin)
		close(newstdin);
	return (f);
}

/*
 * Wait for the command to complete.
 * F is for restoration of tty mode if from open/visual.
 * C flags suppression of printing.
 */
void unixwt(short c, ttymode f)
{

	waitfor();
#ifdef SIGTSTP
	if (dosusp)
		signal(SIGTSTP, onsusp);
#endif
	if (inopen)
		setty(f);
	setrupt();
	if (!inopen && c && hush == 0) {
		ex_printf("!\n");
		flush();
		termreset();
		gettmode();
	}
}

/*
 * Setup a pipeline for the filtration implied by mode
 * which is like a open number.  If input is required to
 * the filter, then a child editor is created to write it.
 * If output is catch it from io which is created by unixex.
 */
void filter(register int mode)
{
	static int pvec[2];
	ttymode f;	/* mjm: was register */
	register int nlines = lineDOL();

	mode++;
	if (mode & 2) {
		signal(SIGINT, SIG_IGN);
		if (pipe(pvec) < 0)
			error(MSGSTR(M_213, "Can't make pipe"), DUMMY_INT);
		other_pid = fork();
		io = pvec[0];
		if (other_pid < 0) {
			setrupt();
			close(pvec[1]);
			error(MSGSTR(M_214, "No more processes"), DUMMY_INT);
		}
		if (other_pid == 0) {
			setrupt();
			io = pvec[1];
			close(pvec[0]);
			putfile(1);
			catclose(catd);
			exit(exit_status);
		}
		close(pvec[1]);
		io = pvec[0];
		setrupt();
	}
	f = unixex("-c", uxb, (mode & 2) ? pvec[0] : 0, mode);
	if (mode == 3) {
		delete((short)0);
		addr2 = addr1 - 1;
	}
	if (mode == 1)
		deletenone();
	if (mode & 1) {
		if(FIXUNDO)
			undap1 = undap2 = addr2+1;
		ignore(append(getfile, addr2));
#ifdef UNDOTRACE
		if (trace)
			vudump("after append in filter");
#endif
	}
	close(io);
	io = -1;
	unixwt((short)(!inopen), f);
	netchHAD(nlines);
}

/*
 * Set up to do a recover, getting io to be a pipe from
 * the recover process.
 */
void recover(void)
{
	static int pvec[2];

	if (pipe(pvec) < 0)
		error(MSGSTR(M_215, " Can't make pipe for recovery"), DUMMY_INT);
	pid = fork();
	io = pvec[0];
	if (pid < 0) {
		close(pvec[1]);
		error(MSGSTR(M_216, " Can't fork to execute recovery"), DUMMY_INT);
	}
	if (pid == 0) {
		close(2);
		dup(1);
		close(1);
		dup(pvec[1]);
	        close(pvec[1]);
		execl(EXRECOVER, "exrecover", svalue(DIRECTORY), file, (char *) 0);
		close(1);
		dup(2);
		error(MSGSTR(M_217, " No recovery routine"), DUMMY_INT);
	}
	close(pvec[1]);
}

static winch_sig;  /* window change signal received */
static void winchk1(int signumber)
{
	winch_sig = 1; /* window changed */
}

/*
 * Wait for the process (pid an external) to complete.
 */
void waitfor(void)
{
 	struct sigaction hndl, hndl1;
 	sigaction(SIGWINCH, NULL, &hndl); /* get current handler */
 	hndl1 = hndl;
 	hndl.sa_handler = winchk1; 
 	hndl.sa_flags |= SA_RESTART; /* restart wait, if interrupted */
 	sigaction(SIGWINCH, &hndl, NULL); /* set window handler to winchk1 */
	winch_sig = 0;

	do
		rpid = wait(&status);
	while (rpid != pid && rpid != -1);
	if ((status & 0377) == 0) {
		status = (status >> 8) & 0377;
	} else {
		ex_printf(MSGSTR(M_218, "%d: terminated with signal %d"), pid, status & 0177);
		if (status & 0200)
			ex_printf(MSGSTR(M_219, " -- core dumped"));
		ex_putchar('\n');
	}

	if (other_pid) {
		/*
		 * there is another direct descendent that we may have already
		 * picked up in the above wait loop, that we need to make sure
		 * we clean up.
		 */
		(void) kill(other_pid, SIGKILL);
		waitpid(other_pid, &status, 0);
		other_pid = 0;
	}

 	sigaction(SIGWINCH, &hndl1, NULL); /* restore signal handler */
	if (winch_sig) {
		winch_sig = 0;
		kill(getpid(), SIGWINCH); /* send SIGWINCH, if window changed
					     while we were in wait() */
	}
}

/*
 * The end of a recover operation.  If the process
 * exits non-zero, force not edited; otherwise force
 * a write.
 */
void revocer(void)
{

	waitfor();
	if (pid == rpid && status != 0)
		edited = 0;
	else
		change();
}
