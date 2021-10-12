static char sccsid[] = "@(#)98	1.31  src/bos/usr/bin/bsh/fault.c, cmdbsh, bos412, 9443A412c 10/26/94 09:52:52";
/*
 * COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
 *
 * FUNCTIONS: stdsigs specsigs fault ignsig getsig setsig oldsigs clrsig
 *	      chktrap
 *
 * ORIGINS: 3, 26, 27, 71
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
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * 1.19  com/cmd/sh/sh/fault.c, cmdsh, bos320, 9125320 6/6/91 23:10:38
 * 
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 * 
 *
 * OSF/1 1.1
 */

#include	"defs.h"
#include        "timeout.h"
#include        <sys/user.h>
#include        <sys/context.h>
#include        <sys/mstsave.h>


extern void	done(int);
extern void	fault(int, int, struct sigcontext *);

static struct sigaction fault_action	= { fault, 0, 0, 0 };
static struct sigaction done_action	= { done, 0, 0, 0 };
static struct sigaction dfl_action	= { SIG_DFL, 0, 0, 0 };
static struct sigaction ign_action	= { SIG_IGN, 0, 0, 0 };
static struct sigaction old_action	= { 0, 0, 0, 0 };

uchar_t	*trapcom[SIGMAX+1];
BOOL	trapflg[SIGMAX+1];	/*  init to 0  */


/* Initialize signal handling, use sigaction */
struct sigaction	*sigact [] = {
		&dfl_action,		/*  0 */
		&done_action,		/*  1 - SIGHUP */
		&fault_action,		/*  2 - SIGINT */
		&fault_action,		/*  3 - SIGQUIT */
		&done_action,		/*  4 - SIGILL */
		&done_action,		/*  5 - SIGTRAP */
		&done_action,		/*  6 - SIGABRT */
		&done_action,		/*  7 - SIGEMT */
		&done_action,		/*  8 - SIGFPE */
		&dfl_action,		/*  9 - SIGKILL */
		&done_action,		/* 10 - SIGBUS */
		&fault_action,		/* 11 - SIGSEGV */
		&done_action,		/* 12 - SIGSYS */
		&done_action,		/* 13 - SIGPIPE */
		&fault_action,		/* 14 - SIGALRM */
		&fault_action,		/* 15 - SIGTERM */
		&dfl_action,		/* 16 - SIGURG */
		&dfl_action,		/* 17 - SIGSTOP */
		&dfl_action,		/* 18 - SIGTSTP */
		&dfl_action,		/* 19 - SIGCONT */
		&dfl_action,		/* 20 - SIGCHLD */
		&dfl_action,		/* 21 - SIGTTIN */
		&dfl_action,		/* 22 - SIGTTOU */
		&dfl_action,		/* 23 - SIGIO */
		&dfl_action,		/* 24 - SIGXCPU */
		&dfl_action,		/* 25 - SIGXFSZ */
		&dfl_action,		/* 26 - */
		&dfl_action,		/* 27 - SIGMSG */
		&dfl_action,		/* 28 - SIGWINCH */
		&dfl_action,		/* 29 - SIGPWR */
		&done_action,		/* 30 - SIGUSR1 */
		&done_action,		/* 31 - SIGUSR2 */
		&dfl_action,		/* 32 - SIGPROF */
		&dfl_action,		/* 33 - SIGDANGER */
		&dfl_action,		/* 34 - SIGVTALRM */
		&dfl_action,		/* 35 - SIGMIGRATE */
		&dfl_action,		/* 36 - SIGPRE */
		&dfl_action,		/* 37 - */
		&dfl_action,		/* 38 - */
		&dfl_action,		/* 39 - */
		&dfl_action,		/* 40 - */
		&dfl_action,		/* 41 - */
		&dfl_action,		/* 42 - */
		&dfl_action,		/* 43 - */
		&dfl_action,		/* 44 - */
		&dfl_action,		/* 45 - */
		&dfl_action,		/* 46 - */
		&dfl_action,		/* 47 - */
		&dfl_action,		/* 48 - */
		&dfl_action,		/* 49 - */
		&dfl_action,		/* 50 - */
		&dfl_action,		/* 51 - */
		&dfl_action,		/* 52 - */
		&dfl_action,		/* 53 - */
		&dfl_action,		/* 54 - */
		&dfl_action,		/* 55 - */
		&dfl_action,		/* 56 - */
		&dfl_action,		/* 57 - */
		&dfl_action,		/* 58 - */
		&dfl_action,		/* 59 - */
		&dfl_action,		/* 60 - SIGGRANT */
		&dfl_action,		/* 61 - SIGRETRACT */
		&dfl_action,		/* 62 - SIGSOUND */
		&dfl_action };		/* 63 - SIGSAK */


int	trap_waitrc = 0;
int	trap_status = 0;

/* ========	fault handling routines	   ======== */

extern long mailtime;
extern int mailchk;

void
stdsigs()
{

	int	i;

	for(i = 1; i <= SIGMAX; i++) {
		switch(i) {
			case SIGHUP:
			case SIGINT:
			case SIGILL:
			case SIGTRAP:
			case SIGABRT:
			case SIGEMT:
			case SIGFPE:
			case SIGBUS:
			case SIGSEGV:
			case SIGSYS:
			case SIGPIPE:
			case SIGALRM:
			case SIGTERM:
			case SIGUSR1:
			case SIGUSR2:
			default:
				setsig(i);
				break;
			case SIGQUIT:
			case SIGXFSZ:
				ignsig(i);
				break;
		}
	}

}

void
specsigs()
{

#define	SIGS	14
int	count;
static int	 signals[SIGS]={SIGHUP, SIGINT, SIGILL, SIGQUIT,
			  SIGTRAP, SIGABRT, SIGEMT, SIGFPE, 
			  SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, 
			  SIGALRM, SIGTERM};

	for(count = 0; count < SIGS; count++) {
		if(signals[count] == SIGQUIT)
			ignsig(signals[count]);
		else
			setsig(signals[count]);
	}

}

#define MST		ctx->sc_jmpbuf.jmp_context
#define PRIVORG		0x20000000

struct {
   unsigned int   locatingString ;
   struct mstsave  save ;
} loggingException = { 0xCCCCDDDD } ;


void
fault(int sig, int foo, struct sigcontext *ctx)
{
	register int	flag;

	if ( sig == SIGCHLD )		/* Wait for child here */
		trap_waitrc = wait (&trap_status);

	sigaction ( sig, &fault_action, (struct sigaction *)0 );
	if (sig == SIGSEGV) {
		int retval = 0 ;
		/**********************************************************
		* - except[0] contains the adddress that caused the SIGSEGV.
		* - bsh historically assumes a SIGSEGV indicates the need
		*   to grow its data segment.
		* - The following checks insure the address causing the
		*   fault resides within the process private segment.
		* - If the address were not checked, a SIGSEGV from outside
		*   the process private segment would cause the data segment
		*   to grow indiscrimately; eventually going ENOMEM or dying.
		*
		* NOTES:
		*   (1) The range test condition could be made stricter.
		*       - will not handle between HEAP and STACK errors 
		*       - will not work under the "large data segment model"
		*
		**********************************************************/

		if ((unsigned int)MST.except[0] > &errno ||
		    (unsigned int)MST.except[0] < PRIVORG ||
		    (retval = setbrk(brkincr)) == -1) {
			sigset_t newset;

			sigemptyset(&newset);
			sigaddset(&newset,SIGSEGV);

			/* die if another SIGSEGV occurs */
			sigprocmask(SIG_UNBLOCK,&newset,NULL);
			{
			/* generate a FULLDUMP (with the data segment) */
				struct sigaction dfl={SIG_DFL,0,0,SA_FULLDUMP};
				sigaction(sig,&dfl,(struct sigaction *)0);
			}
			loggingException.save = MST;

			/* No space error message when sbrk fails */
			if ( retval == -1 )
				error(MSGSTR(M_NOSPACE,nospace));
		}
	}
	else if (sig == SIGALRM)
	{
		long int curtime;

		time(&curtime);
		if(mailchk) {
			chkmail();
			mailalarm = 1;
			alarm(mailchk);
			mailtime = curtime;
		}
		if (timecroak && flags&waiting && (curtime >= timecroak))
			done(0);
	}
	else
	{
		flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
		if (sig == SIGINT)
			wasintr++;
	}
}

int
ignsig(register int i)
{
	register int    s;

	if (i == SIGSEGV)
	{
		clrsig(i);
		{
		  uchar_t buf[NL_TEXTMAX];
		  strcpy ((char *)buf, MSGSTR(M_BADTRAP, (char *)badtrap));
		  failed(buf, MSGSTR(M_TRAP,"cannot trap SIGSEGV"));
		}
	}
	else {
		sigaction(i, &ign_action, &old_action);
		if ((s = (old_action.sa_handler == SIG_IGN)) == 0)
			trapflg[i] |= SIGMOD;
	}
	return(s);
}

int
setsig(int n)
{

	if ( n == SIGSEGV )
		sigaction( n, sigact[n], (struct sigaction *)0 );
	else if (ignsig(n) == 0)
		sigaction( n, sigact[n], (struct sigaction *)0 );
}

void
getsig(int n)
{
	register int i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0)
		sigaction(i, &fault_action, (struct sigaction *)0);
}


void
oldsigs()
{
	register int	i = SIGMAX +1;
	register uchar_t	*t;

	while (i--)
	{
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

int
clrsig(int i)
{
	free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD)
	{
		trapflg[i] &= ~SIGMOD;
		sigaction(i, sigact[i], (struct sigaction *)0);
	}
}

/*
 * check for traps
 */
void
chktrap()
{
	register int	i = SIGMAX +1;
	register uchar_t	*t;

	trapnote &= ~TRAPSET;

	while (--i)
	{
		if (trapflg[i] & TRAPSET)
		{
			trapflg[i] &= ~TRAPSET;
			if (t = trapcom[i])
			{
				int	savxit = exitval;

				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}
