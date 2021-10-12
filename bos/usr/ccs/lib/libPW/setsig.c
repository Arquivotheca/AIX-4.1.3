static char sccsid[] = "@(#)85	1.12  src/bos/usr/ccs/lib/libPW/setsig.c, libPW, bos411, 9428A410j 11/10/93 15:18:06";
/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: setsig, setsig1
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 */
 
# include	"signal.h"
# include	"sys/types.h"
# include	"macros.h"
# include	"pw_msg.h"

/*
	General-purpose signal setting routine.
	All non-ignored, non-caught signals are caught.
	If a signal other than hangup, interrupt, or quit is caught,
	a "user-oriented" message is printed on file descriptor 2 with
	a number for help(I).
	If hangup, interrupt or quit is caught, that signal	
	is set to ignore.
	Termination is like that of "fatal",
	via "clean_up(sig)" (sig is the signal number)
	and "exit(userexit(1))".
 
	If the file "dump.core" exists in the current directory
	the function commits suicide to produce a core dump
	(after calling clean_up, but before calling userexit).
*/
#define NUMSIGS 16 /*Number of signals defined*/

setsig()
{
	extern int setsig1(int);
	register int signo;
	register void (* func)(int);

	for (signo = 1; signo < NUMSIGS; signo++) {
		if ((func = signal(signo, SIG_IGN)) == SIG_DFL)
			func = (void (*)(int))setsig1;
		signal(signo,(void (*)(int))func);
	}
}



setsig1(int sig)
{
	register char *p;
	register int msgno;

	int i;
	nl_catd catd;

	if (sig<NUMSIGS && (sig)) {
		catd = catopen(MF_PW, NL_CAT_LOCALE);
		switch (sig) {
		case 4:
			msgno=XILINST;
			p="SIGNAL: Illegal instruction(ut12)";
			break;
		case 5:
			msgno=XTRACE;
			p="SIGNAL: Trace/BPT trap(ut12)";
			break;
		case 6:
			msgno=XIOTTR;
			p="SIGNAL: IOT trap(ut12)";
			break;
		case 7:
			msgno=XDANG;
			p="SIGNAL: Danger(ut12)";
			break;
		case 8:
			msgno=XARITHEX;
			p="SIGNAL: Arithmetic exception(ut12)";
			break;
		case 9:
			msgno=XKILD;
			p="SIGNAL: Killed(ut12)";
			break;
		case 10:
			msgno=XBUSER;
			p="SIGNAL: Bus error(ut12)";
			break;
		case 11:
			msgno=XMEMFLT;
			p="SIGNAL: Memory fault(ut12)";
			break;
		case 12:
			msgno=XBADSYS;
			p="SIGNAL: Bad system call(ut12)";
			break;
		case 13:
			msgno=XBRKPIP;
			p="SIGNAL: Broken pipe(ut12)";
			break;
		case 14:
			msgno=XAMCLOK;
			p="SIGNAL: Alarm clock(ut12)";
			break;
		case 15:
			msgno=XTERMAS;
			p="SIGNAL: Terminated(ut12)";
			break;
		default:
			signal(sig,SIG_IGN);
			msgno=XTERMAS + 1; /* XTERMAS is the last msg. in pw.msg; */
					   /* therefore, catgets will return the  */
					   /* default message                     */
			p="SIGNAL: SIGHUP, SIGINT and SIGQUIT are ignored";
			break;
		}/*switch*/

		fprintf(stderr, "%s\n", catgets(catd, MS_PW2, msgno,p));
		catclose(catd);
	} /*if*/
	else
		signal(sig,SIG_IGN);
	clean_up(sig);

	if(open("dump.core",0) > 0) {
		signal(SIGIOT,SIG_DFL);
		abort();
	}
	exit(userexit(1));
}
