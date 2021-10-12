static char sccsid[] = "@(#)97	1.5  src/bos/usr/bin/mh/sbr/pidstatus.c, cmdmh, bos411, 9428A410j 3/27/91 18:12:50";
/* 
 * COMPONENT_NAME: CMDMH pidstatus.c
 * 
 * FUNCTIONS: MSGSTR, pidstatus 
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
/* static char sccsid[] = "pidstatus.c	7.1 87/10/13 17:15:16"; */

/* pidstatus.c - report child's status */

#include "mh.h"
#include <signal.h>
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#ifndef	BSD42
static char *sigs[27];
#else
extern  char *sys_siglist[];
#endif	BSD42

/*  */

int	pidstatus (status, fp, cp)
register int   status;
register FILE *fp;
register char *cp;
{
    int     signum;
    char    *sigmsg = NULL;

    if ((status & 0xff00) == 0xff00)
	return status;

    switch (signum = status & 0x007f) {
	case OK: 
	    if (signum = ((status & 0xff00) >> 8)) {
		if (cp)
		    fprintf (fp, "%s: ", cp);
		fprintf (fp, MSGSTR(EXIT, "Exit %d\n"), signum); /*MSG*/
	    }
	    break;

	case SIGINT: 
	    break;

	default: 
	    if (cp)
		fprintf (fp, "%s: ", cp);
#ifndef	BSD42
	    if (signum < sizeof sigs)
		sigmsg = MSGSTR(SIGS+signum, sigs[signum]); /*MSG*/
	    if (sigmsg == NULL)
		fprintf (fp, MSGSTR(SIGNAL, "Signal %d"), signum); /*MSG*/
	    else
		fprintf (fp, "%s", sigmsg);
#else	BSD42
	    if (signum >= NSIG)
		fprintf (fp, MSGSTR(SIGNAL, "Signal %d"), signum); /*MSG*/
	    else
		fprintf (fp, "%s", sys_siglist[signum]);
#endif	BSD42
	    fprintf (fp, "%s\n", status & 0x80 ? MSGSTR(CORE, " (core dumped)") : ""); /*MSG*/
	    break;
    }

    return status;
}
