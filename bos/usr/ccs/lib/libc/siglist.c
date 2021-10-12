static char sccsid[] = "@(#)96	1.2  src/bos/usr/ccs/lib/libc/siglist.c, libcproc, bos411, 9428A410j 11/30/93 10:45:54";
/*
 *   COMPONENT_NAME: libcgen
 *
 * FUNCTIONS: sys_siglist
 *
 *   ORIGINS: 26,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <signal.h>

/*
 * NAME: sys_siglist
 *
 * FUNCTION: external array of signal message strings
 *
 * RETURN VALUES: 
 *	NONE (not applicable)
 */

char	*sys_siglist[SIGMAX+1] = {
	"Signal 0",
	"Hangup",			/* SIGHUP */
	"Interrupt",			/* SIGINT */
	"Quit",				/* SIGQUIT */
	"Illegal instruction",		/* SIGILL */
	"Trace/BPT trap",		/* SIGTRAP */
	"IOT/Abort trap",		/* SIGABRT */
	"EMT trap",			/* SIGEMT */
	"Floating point exception",	/* SIGFPE */
	"Killed",			/* SIGKILL */
	"Bus error",			/* SIGBUS */
	"Segmentation fault",		/* SIGSEGV */
	"Bad system call",		/* SIGSYS */
	"Broken pipe",			/* SIGPIPE */
	"Alarm clock",			/* SIGALRM */
	"Terminated",			/* SIGTERM */
	"Urgent I/O condition",		/* SIGURG */
	"Stopped (signal)",		/* SIGSTOP */
	"Stopped",			/* SIGTSTP */
	"Continued",			/* SIGCONT */
	"Child exited",			/* SIGCHLD */
	"Stopped (tty input)",		/* SIGTTIN */
	"Stopped (tty output)",		/* SIGTTOU */
	"I/O possible/complete",	/* SIGIO */
	"Cputime limit exceeded",	/* SIGXCPU */
	"Filesize limit exceeded",	/* SIGXFSZ */
	"Signal 26",
	"Input device data",		/* SIGMSG */
	"Window size changes",		/* SIGWINCH */
	"Power-failure",		/* SIGPWR */
	"User defined signal 1",	/* SIGUSR1 */
	"User defined signal 2",	/* SIGUSR2 */
	"Profiling timer expired",	/* SIGPROF */
	"Paging space low",		/* SIGDANGER */
	"Virtual timer expired",	/* SIGVTALRM */
	"Signal 35",
	"Signal 36",
	"Signal 37",
	"Signal 38",
	"Signal 39",
	"Signal 40",
	"Signal 41",
	"Signal 42",
	"Signal 43",
	"Signal 44",
	"Signal 45",
	"Signal 46",
	"Signal 47",
	"Signal 48",
	"Signal 49",
	"Signal 50",
	"Signal 51",
	"Signal 52",
	"Signal 53",
	"Signal 54",
	"Signal 55",
	"Signal 56",
	"Signal 57",
	"Signal 58",
	"Secure attention",		/* SIGSAK */
	"Monitor mode granted",		/* SIGGRANT */
	"Monitor mode retracted",	/* SIGRETRACT */
	"Sound completed",		/* SIGSOUND */
	"Signal 63",
};
