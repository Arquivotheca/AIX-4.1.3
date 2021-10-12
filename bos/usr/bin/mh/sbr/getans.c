static char sccsid[] = "@(#)57        1.7  src/bos/usr/bin/mh/sbr/getans.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:28";
/* 
 * COMPONENT_NAME: CMDMH getans.c
 * 
 * FUNCTIONS: MSGSTR, getans, intrser 
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
/* static char sccsid[] = "getans.c	7.1 87/10/13 17:06:14"; */

/* getans.c - get an answer from the user and return a string array */

#include "mh.h"
#ifdef	BSD42
#include <setjmp.h>
#endif	BSD42
#include <signal.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


static	char ansbuf[BUFSIZ];
#ifndef	BSD42
static	int interrupted;
#else	BSD42
static	jmp_buf sigenv;
#endif	BSD42
static void	intrser (int);

char  **getans (prompt, ansp)
register char  *prompt;
register struct swit   *ansp;
{
    register int    i;
    void    (*istat) (int);
    register char  *cp,
                  **cpp;

#ifndef	BSD42
    interrupted = 0;
    istat = signal (SIGINT, intrser);
#else	BSD42
    switch (setjmp (sigenv)) {
	case OK: 
	    istat = signal (SIGINT, (void(*)(int)) intrser);
	    break;

	default: 
	    (void) signal (SIGINT, (void(*)(int)) istat);
	    return NULL;
    }
#endif	BSD42
    for (;;) {
	printf ("%s", prompt);
	(void) fflush (stdout);
	cp = ansbuf;
	while ((i = getchar ()) != '\n') {
#ifndef	BSD42
	    if (i == EOF || interrupted) {
		interrupted = 0;
		(void) signal (SIGINT, istat);
		return NULL;
	    }
#else	BSD42
	    if (i == EOF)
		longjmp (sigenv, DONE);
#endif	BSD42
	    if (cp < &ansbuf[sizeof ansbuf - 1])
		*cp++ = i;
	}
	*cp = 0;
	if (ansbuf[0] == '?' || cp == ansbuf) {
	    printf (MSGSTR(OPTIONS, "Options are:\n")); /*MSG*/
	    printsw (ALL, ansp, "");
	    continue;
	}
	cpp = brkstring (ansbuf, " ", NULLCP);
	switch (smatch (*cpp, ansp)) {
	    case AMBIGSW: 
		ambigsw (*cpp, ansp);
		continue;
	    case UNKWNSW: 
		printf (MSGSTR(HELP, " -%s unknown. Hit <CR> for help.\n"), *cpp); /*MSG*/
		continue;
	    default: 
		(void) signal (SIGINT, (void(*)(int)) istat);
		return cpp;
	}
    }
}


static	void intrser (int s) {
#ifndef	BSD42
	(void) signal(SIGINT, intrser);
	interrupted = 1;
#else	BSD42
	longjmp (sigenv, NOTOK);
#endif	BSD42
}




