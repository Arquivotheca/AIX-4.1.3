static char sccsid[] = "@(#)92  1.7  src/bos/usr/bin/mh/sbr/m_update.c, cmdmh, bos411, 9428A410j 3/27/91 17:50:59";
/* 
 * COMPONENT_NAME: CMDMH m_update.c
 * 
 * FUNCTIONS: MSGSTR, m_chkids, m_update 
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
/* static char sccsid[] = "m_update.c	7.1 87/10/13 17:13:52"; */

/* m_update.c - update the profile */

#include "mh.h"
#include <stdio.h>
#include <signal.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

static int  m_chkids();

void m_update () {
    int     action;
    void     (*hstat) (int), (*istat) (int), (*qstat) (int), (*tstat) (int);
    register struct node   *np;
    FILE * out;

    if (!(ctxflags & CTXMOD))
	return;
    ctxflags &= ~CTXMOD;

    if ((action = m_chkids ()) > OK)
	return;			/* child did it for us */

    hstat = signal (SIGHUP, SIG_IGN);
    istat = signal (SIGINT, SIG_IGN);
    qstat = signal (SIGQUIT, SIG_IGN);
    tstat = signal (SIGTERM, SIG_IGN);

    if ((out = fopen (ctxpath, "w")) == NULL)
	adios (ctxpath, MSGSTR(NOWRITE, "unable to write %s"), ctxpath); /*MSG*/
    for (np = m_defs; np; np = np -> n_next)
	if (np -> n_context)
	    fprintf (out, "%s: %s\n", np -> n_name, np -> n_field);
    (void) fclose (out);

    (void) signal (SIGHUP, (void(*)(int)) hstat);
    (void) signal (SIGINT, (void(*)(int)) istat);
    (void) signal (SIGQUIT, (void(*)(int)) qstat);
    (void) signal (SIGTERM, (void(*)(int)) tstat);
    if (action == OK)
	_exit (0);		/* we are child, time to die */
}

/*  */

/* This hack brought to you so we can handle set[ug]id MH programs.  If we
   return NOTOK, then no fork is made, we update .mh_profile normally, and
   return to the caller normally.  If we return 0, then the child is
   executing, .mh_profile is modified after we set our [ug]ids to the norm.
   If we return > 0, then the parent is executed and .mh_profile has
   already be modified.  We can just return to the caller immediately. */


static int  m_chkids () {
    int     i,
            child_id;

    if (getuid () == geteuid ())
	return (NOTOK);

    for (i = 0; (child_id = fork ()) == -1 && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK:
	    break;

	case OK:
	    (void) setgid (getgid ());
	    (void) setuid (getuid ());
	    break;

	default:
	    (void) pidwait (child_id, NOTOK);
	    break;
    }

    return child_id;
}
