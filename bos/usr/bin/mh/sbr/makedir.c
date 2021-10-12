static char sccsid[] = "@(#)94	1.5  src/bos/usr/bin/mh/sbr/makedir.c, cmdmh, bos411, 9428A410j 3/27/91 17:51:04";
/* 
 * COMPONENT_NAME: CMDMH makedir.c
 * 
 * FUNCTIONS: MSGSTR, makedir 
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
/* static char sccsid[] = "makedir.c	7.1 87/10/13 17:14:24"; */

/* makedir.c - make a directory */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


makedir (dir)
register char *dir;
{
    int     pid;
    register char  *cp;

    m_update ();
    (void) fflush (stdout);

#ifdef	BSD42
    if (getuid () == geteuid ()) {
	if (mkdir (dir, 0755) == NOTOK) {
	    advise (dir, MSGSTR(NODIR, "unable to create directory %s"), dir); /*MSG*/
	    return 0;
	}
    }
    else
#endif	BSD42
    switch (pid = vfork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    return 0;

	case OK: 
	    (void) setgid (getgid ());
	    (void) setuid (getuid ());

	    execl ("/bin/mkdir", "mkdir", dir, NULLCP);
	    execl ("/usr/bin/mkdir", "mkdir", dir, NULLCP);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror ("mkdir");
	    _exit (-1);

	default: 
	    if (pidXwait (pid, "mkdir"))
		return 0;
	    break;
    }

    if ((cp = m_find ("folder-protect")) == NULL)
	cp = foldprot;
    (void) chmod (dir, atooi (cp));
    return 1;
}

