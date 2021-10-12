static char sccsid[] = "@(#)06	1.5  src/bos/usr/bin/mh/sbr/remdir.c, cmdmh, bos411, 9428A410j 3/27/91 17:52:49";
/* 
 * COMPONENT_NAME: CMDMH remdir.c
 * 
 * FUNCTIONS: MSGSTR, remdir 
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
/* static char sccsid[] = "remdir.c	7.1 87/10/13 17:17:31"; */

/* remdir.c - remove a directory */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


remdir (dir)
char *dir;
{
#ifndef	BSD42
    int     pid;
#endif	not BSD42

    m_update ();
    (void) fflush (stdout);

#ifndef	BSD42
    switch (pid = vfork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    return 0;

	case OK: 
	    execl ("/bin/rmdir", "rmdir", dir, NULLCP);
	    execl ("/usr/bin/rmdir", "rmdir", dir, NULLCP);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror ("rmdir");
	    _exit (-1);

	default: 
	    if (pidXwait (pid, "rmdir"))
		return 0;
	    break;
    }
#else	BSD42
    if (rmdir (dir) == NOTOK) {
	admonish (dir, MSGSTR(NORMDIR, "unable to remove directory %s"), dir); /*MSG*/
	return 0;
    }
#endif	BSD42

    return 1;
}
