static char sccsid[] = "@(#)76	1.3  src/bos/usr/ccs/lib/libc/abort.c, libcproc, bos411, 9428A410j 4/20/94 17:38:06";
/*
 *   COMPONENT_NAME: LIBCPROC
 *
 *   FUNCTIONS: _abort
 *		abort
 *
 *   ORIGINS: 3,26,27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/* abort.c,v $ $Revision: 2.6.2.4 $ (OSF) */

/*
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <signal.h>
#include <stdlib.h>

#include "ts_supp.h"
#include "push_pop.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _abort_rmutex;
#endif	/* _THREAD_SAFE */

/*
 * FUNCTION:	Causes abnormal program termination to occur, unless the
 *		signal SIGABRT is being caught and the signal handler does
 *		not return.
 *
 * RETURN VALUE DESCRIPTION:
 *              The abort function cannot return to its caller.
 */

void
abort(void)
{
	struct sigaction action;
	sigset_t	mask;
	static int	abortcnt;


	TS_LOCK(&_abort_rmutex);
	if (!abortcnt) {
		abortcnt++;
		TS_PUSH_CLNUP(&_abort_rmutex);
		_cleanup();
		TS_POP_CLNUP(0);
	}
	TS_UNLOCK(&_abort_rmutex);

	(void) raise(SIGABRT);	/* Terminate the process */

	/*
	 * The signal handler for SIGABRT has returned or SIGABRT is blocked.
	 * Take additional steps to terminate the process.  If there is a
	 * pending SIGABRT then unblocking it may terminate the process.
	 */
	(void) sigemptyset(&mask);
	(void) sigaddset(&mask, SIGABRT);
 	(void) sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)NULL);

	/*
	 * Still alive?  Replace the signal handler for SIGABRT with SIG_DFL
	 * and raise SIGABRT again.  This should terminate the process.
	 */
	action.sa_handler = SIG_DFL;
	action.sa_flags = 0;
	(void) sigemptyset(&action.sa_mask);
 	(void) sigaction(SIGABRT, &action, (struct sigaction *)NULL);
	(void) raise(SIGABRT);

	exit(SIGABRT);	/* Should never ever get this far */
}

void
_abort(void)
{
	abort();
}
