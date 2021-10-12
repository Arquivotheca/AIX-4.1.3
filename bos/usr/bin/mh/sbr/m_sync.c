static char sccsid[] = "@(#)90	1.6  src/bos/usr/bin/mh/sbr/m_sync.c, cmdmh, bos411, 9428A410j 3/27/91 17:50:56";
/* 
 * COMPONENT_NAME: CMDMH m_sync.c
 * 
 * FUNCTIONS: MSGSTR, m_sync 
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
/* static char sccsid[] = "m_sync.c	7.1 87/10/13 17:13:21"; */

/* m_sync.c - synchronize message sequences */

#include "mh.h"
#include <stdio.h>
#include <signal.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


/* decision logic
    1.  public and folder readonly: make it private
    2a. public: add it to the sequences file
    2b. private: add it to the profile
 */


void m_sync (mp)
register struct msgs *mp;
{
    int     bits;
    register int    i;
    register char  *cp;
    char    flags,
	    attr[BUFSIZ],
	    seq[BUFSIZ];
    register FILE  *fp;
#ifdef BSD42
    int	    oldsig;
#else not BSD42
    int     (*hstat) (), (*istat) (), (*qstat) (), (*tstat) ();
#endif BSD42

    if (!(mp -> msgflags & SEQMOD))
	return;
    mp -> msgflags &= ~SEQMOD;

    m_getdefs ();
    (void) sprintf (seq, "%s/%s", mp -> foldpath, mh_seq);
    bits = FFATTRSLOT;
    fp = NULL;

    flags = mp -> msgflags;
    if (mh_seq == NULL || *mh_seq == (int)NULL)
	mp -> msgflags |= READONLY;

    for (i = 0; mp -> msgattrs[i]; i++) {
	(void) sprintf (attr, "atr-%s-%s", mp -> msgattrs[i], mp -> foldpath);
	if (mp -> msgflags & READONLY
		|| mp -> attrstats & (1 << (bits + i))) {
    private: ;
	    if (cp = m_seq (mp, mp -> msgattrs[i]))
		m_replace (attr, cp);
	    else
		(void) m_delete (attr);
	}
	else {
	    (void) m_delete (attr);
	    if ((cp = m_seq (mp, mp -> msgattrs[i])) == NULL)
		continue;
	    if (fp == NULL) {
		if ((fp = fopen (seq, "w")) == NULL
			&& unlink (seq) != NOTOK 
			&& (fp = fopen (seq, "w")) == NULL) {
		    admonish (attr, MSGSTR(NOWRITE, "unable to write %s"), attr); /*MSG*/
		    goto private;
		}
#ifdef BSD42
		oldsig = sigblock (sigmask(SIGHUP) | sigmask(SIGINT) |
				   sigmask(SIGQUIT) | sigmask(SIGTERM));
#else not BSD42
		hstat = signal (SIGHUP, SIG_IGN);
		istat = signal (SIGINT, SIG_IGN);
		qstat = signal (SIGQUIT, SIG_IGN);
		tstat = signal (SIGTERM, SIG_IGN);
#endif BSD42
	    }
	    fprintf (fp, "%s: %s\n", mp -> msgattrs[i], cp);
	}
    }

    if (fp) {
	(void) fclose (fp);
#ifdef BSD42
	(void) sigsetmask (oldsig);
#else not BSD42
	(void) signal (SIGHUP, hstat);
	(void) signal (SIGINT, istat);
	(void) signal (SIGQUIT, qstat);
	(void) signal (SIGTERM, tstat);
#endif BSD42
    }
    else
	if (!(mp -> msgflags & READONLY))
	    (void) unlink (seq);

    mp -> msgflags = flags;
}
