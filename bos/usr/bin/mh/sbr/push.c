static char sccsid[] = "@(#)01	1.4  src/bos/usr/bin/mh/sbr/push.c, cmdmh, bos411, 9428A410j 6/15/90 22:14:56";
/* 
 * COMPONENT_NAME: CMDMH push.c
 * 
 * FUNCTIONS: MSGSTR, push 
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
/* static char sccsid[] = "push.c	7.1 87/10/13 17:16:05"; */

/* push.c - push a fork into the background */

#include "mh.h"
#include <stdio.h>
#include <signal.h>

#ifdef MSG
#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_MH,n,s) 
#else
#define MSGSTR(n,s) s
#endif


void	push () {
    register int     i;

    for (i = 0; i < 5; i++) {
	switch (fork ()) {
	    case NOTOK: 
		sleep (5);
		continue;

	    case OK: 
		(void) signal (SIGHUP, SIG_IGN);
		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGQUIT, SIG_IGN);
		(void) signal (SIGTERM, SIG_IGN);
#ifdef	SIGTSTP
		(void) signal (SIGTSTP, SIG_IGN);
		(void) signal (SIGTTIN, SIG_IGN);
		(void) signal (SIGTTOU, SIG_IGN);
#endif	SIGTSTP
		(void) freopen ("/dev/null", "r", stdin);
		(void) freopen ("/dev/null", "w", stdout);
		break;

	    default: 
		done (0);
	}
	break;
    }
    if (i >= 5)
	advise (NULLCP, MSGSTR(NOPUSH, "unable to fork, so can't push...")); /*MSG*/
}
