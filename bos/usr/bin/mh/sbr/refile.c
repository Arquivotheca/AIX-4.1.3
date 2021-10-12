static char sccsid[] = "@(#)05	1.5  src/bos/usr/bin/mh/sbr/refile.c, cmdmh, bos411, 9428A410j 3/27/91 17:53:53";
/* 
 * COMPONENT_NAME: CMDMH refile.c
 * 
 * FUNCTIONS: MSGSTR, refile 
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
/* static char sccsid[] = "refile.c	7.1 87/10/13 17:17:14"; */

/* refile.c - refile the draft into another folder */


#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

int     refile (arg, file)
register char **arg,
               *file;
{
    int     pid;
    register int    vecp;
    char   *vec[MAXARGS];

    vecp = 0;
    vec[vecp++] = r1bindex (fileproc, '/');
    vec[vecp++] = "-file";
    vec[vecp++] = file;

    if (arg)
	while (*arg)
	    vec[vecp++] = *arg++;
    vec[vecp] = NULL;

    m_update ();
    (void) fflush (stdout);

    switch (pid = vfork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    return NOTOK;

	case OK: 
	    execvp (fileproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (fileproc);
	    _exit (-1);

	default: 
	    return (pidwait (pid, NOTOK));
    }
}
