static char sccsid[] = "@(#)95	1.1  src/bos/usr/ccs/lib/libc/siginter.c, libcproc, bos411, 9428A410j 2/26/91 17:47:47";
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: siginterrupt
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <signal.h>


/*
 * NAME: siginterrupt()			(BSD)
 *
 * FUNCTION: Changes the SA_RESTART signal action flag to allow/prevent
 * 	     restart of system calls after an instance of the specified signal.
 *
 * WARNING: siginterrupt() is implemented with two sigaction() system calls,
 *	  so it is not atomic.  There is a chance that other signal actions
 *	  could inadvertently change along with the SA_INTERRUPT flag.
 *	  For this to happen, some signal handler would have to be changing
 *	  the action of this signal between these two calls.
 *
 * RETURN VALUES: 
 *	0 => successfully set/reset SA_RESTART flag
 *	-1 => failed, SA_RESTART flag not changed
 */

int siginterrupt(signo, flag)
int signo;		/* signal whose SA_RESTART action should change */
int flag;		/* 1=> reset SA_RESTART, 0=> set SA_RESTART */
{
	struct sigaction act;
	int rc;

	/* use sigaction() system call to get current signal actions */
	rc = sigaction(signo, (struct sigaction *)NULL, &act);

	/* change just the SA_RESTART action according to "flag" */
	if ( rc == 0 )
	{	if (!flag)
			act.sa_flags |= SA_RESTART;
		else
			act.sa_flags &= ~SA_RESTART;
		rc = sigaction(signo, &act, (struct sigaction *)NULL);
	}
	return (rc);
}
