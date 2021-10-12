static char sccsid[] = "@(#)99        1.5  src/bos/usr/bin/mh/sbr/pidwait.c, cmdmh, bos411, 9428A410j 10/10/90 15:38:17";
/* 
 * COMPONENT_NAME: CMDMH pidwait.c
 * 
 * FUNCTIONS: pidwait 
 *
 * ORIGINS: 10  26  27  28  35 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* pidwait.c - wait for child to exit */

#include "mh.h"
#include <signal.h>
#include <stdio.h>
#ifdef	BSD42
#include <sys/wait.h>
#endif	BSD42


int     pidwait (id, sigsok)
register int     id,
		 sigsok;
{
    register int    pid;
#ifndef	BSD42
    int     status;
#else	BSD42
    union wait status;
#endif	BSD42
    void     (*hstat) (int), (*istat) (int), (*qstat) (int), (*tstat) (int);

    if (sigsok == NOTOK) {
	hstat = signal (SIGHUP, SIG_IGN);
	istat = signal (SIGINT, SIG_IGN);
	qstat = signal (SIGQUIT, SIG_IGN);
	tstat = signal (SIGTERM, SIG_IGN);
    }

    while ((pid = wait ((int *)&status)) != NOTOK && pid != id)
	continue;

    if (sigsok == NOTOK) {
	(void) signal (SIGHUP, (void(*)(int)) hstat);
	(void) signal (SIGINT, (void(*)(int)) istat);
	(void) signal (SIGQUIT, (void(*)(int)) qstat);
	(void) signal (SIGTERM, (void(*)(int)) tstat);
    }

#ifndef	BSD42
    return (pid == NOTOK ? NOTOK : status);
#else	BSD42
    return (pid == NOTOK ? NOTOK : status.w_status);
#endif	BSD42
}
