static char sccsid[] = "@(#)07	1.5  src/bos/usr/bin/mh/sbr/showfile.c, cmdmh, bos411, 9428A410j 3/27/91 17:52:53";
/* 
 * COMPONENT_NAME: CMDMH showfile.c
 * 
 * FUNCTIONS: MSGSTR, showfile 
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
/* static char sccsid[] = "showfile.c	7.1 87/10/13 17:17:49"; */

/* showfile.c - invoke lproc */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


showfile (arg, file)
register char **arg,
               *file;
{
    int     isdraft,
            pid;
    register int    vecp;
    char   *vec[MAXARGS];

    m_update ();
    (void) fflush (stdout);

    if (strcmp (r1bindex (lproc, '/'), "mhl") == 0)
	lproc = mhlproc;

    switch (pid = vfork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    return 1;

	case OK: 
	    vecp = 0;
	    vec[vecp++] = r1bindex (lproc, '/');
	    isdraft = 1;
	    if (arg)
		while (*arg) {
		    if (**arg != '-')
			isdraft = 0;
		    vec[vecp++] = *arg++;
		}
	    if (isdraft) {
		if (strcmp (vec[0], "show") == 0)
		    vec[vecp++] = "-file";
		vec[vecp++] = file;
	    }
	    vec[vecp] = NULL;

	    execvp (lproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (lproc);
	    _exit (-1);

	default: 
	    return (pidwait (pid, NOTOK) & 0377 ? 1 : 0);
    }
}
