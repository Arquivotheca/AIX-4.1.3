static char sccsid[] = "@(#)87  1.8  src/bos/usr/bin/mh/uip/sendsbr.c, cmdmh, bos411, 9428A410j 3/28/91 16:09:22";
/* 
 * COMPONENT_NAME: CMDMH sendsbr.c
 * 
 * FUNCTIONS: MSGSTR, alert, anno, annoaux, done, sendaux, sendsbr, 
 *            tmp_fd 
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
/* static char sccsid[] = "sendsbr.c	8.1 88/06/10 11:35:37"; */

/* sendsbr.c - routines to help WhatNow/Send along */

#include "mh.h"
#include <setjmp.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

int     debugsw = 0;		/* global */
int     forwsw = 1;
int     inplace = 0;
int     pushsw = 0;
int     unique = 0;

char   *altmsg = NULL;		/*  .. */
char   *annotext = NULL;
char   *distfile = NULL;


static int armed = 0;
static jmp_buf env;


char   *getusr ();
long	lseek ();

char *strcpy();
char *getenv();
static  alert (register char *, int);
static  int tmp_fd();
static anno (int, register struct stat *);
static	annoaux (int);

/*  */

int	sendsbr (vec, vecp, drft, st)
register char **vec,
	       *drft;
int     vecp;
register struct stat *st;
{
    int     status;

    armed++;
    switch (setjmp (env)) {
	case OK: 
	    status = sendaux (vec, vecp, drft, st) ? NOTOK : OK;
	    break;

	default: 
	    status = DONE;
	    break;
    }
    armed = 0;
    if (distfile)
	(void) unlink (distfile);

    return status;
}

/*  */

int	sendaux (vec, vecp, drft, st)
register char **vec,
	       *drft;
int     vecp;
register struct stat *st;
{
    int     child_id,
            i,
	    status,
            fd,
            fd2;
    char    backup[BUFSIZ],
            buf[BUFSIZ],
            file[BUFSIZ];

    fd = pushsw ? tmp_fd () : NOTOK;
    fd2 = NOTOK;

    if (pushsw && unique) {
	if (rename (drft, strcpy (file, m_scratch (drft, invo_name)))
		== NOTOK)
	    adios (file, MSGSTR(NORENAME, "unable to rename %s to %s"), drft, file); /*MSG*/
	drft = file;
    }
    vec[vecp++] = drft;
#if defined(X400)
    if (pushsw)
        vec[vecp++] = "-xpush";
#endif
    if (annotext)
	if ((fd2 = tmp_fd ()) != NOTOK) {
	    vec[vecp++] = "-idanno";
	    (void) sprintf (buf, "%d", fd2);
	    vec[vecp++] = buf;
	}
	else
	    admonish (NULLCP, MSGSTR(NOLISTFILE, "unable to create file for annotation list")); /*MSG*/
    if (distfile && distout (drft, distfile, backup) == NOTOK)
	done (1);
    vec[vecp] = NULL;

    for (i = 0; (child_id = vfork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 		/* oops */
	    adios ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	case OK: 		/* send it */
	    if (fd != NOTOK) {
		(void) dup2 (fd, fileno (stdout));
		(void) dup2 (fd, fileno (stderr));
		(void) close (fd);
	    }
	    execvp (postproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (postproc);
	    _exit (-1);

	default: 		/* wait for it */
	    if ((status = pidwait (child_id, NOTOK)) == 0) {
		if (annotext && fd2 != NOTOK)
		    anno (fd2, st);
		if (rename (drft, strcpy (buf, m_backup (drft))) == NOTOK)
		    advise (buf, MSGSTR(NORENAME, "unable to rename %s to %s"), drft, buf); /*MSG*/
	    }
	    else {
		if (fd != NOTOK) {
		    alert (drft, fd);
		    (void) close (fd);
		}
		else
		    advise (NULLCP, MSGSTR(NOTDELIVER, "message not delivered to everyone")); /*MSG*/
		if (fd2 != NOTOK)
		    (void) close (fd2);
		if (distfile) {
		    (void) unlink (drft);
		    if (rename (backup, drft) == NOTOK)
			advise (drft, MSGSTR(NORENAME, "unable to rename %s to %s"), backup, drft); /*MSG*/
		}
	    }
	    break;
    }

    return status;
}

/*  */

static  alert (register char   *file,
	       int     out)
{
    int     child_id,
            i,
            in;
    char    buf[BUFSIZ];

    for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 		/* oops */
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	case OK: 		/* send it */
	    (void) signal (SIGHUP, SIG_IGN);
	    (void) signal (SIGINT, SIG_IGN);
	    (void) signal (SIGQUIT, SIG_IGN);
	    (void) signal (SIGTERM, SIG_IGN);
	    if (forwsw)
		if ((in = open (file, 0)) == NOTOK)
		    admonish (file, MSGSTR(NOROPEN, "unable to re-open %s"), file); /*MSG*/
		else {
		    (void) lseek (out, 0L, 2);
		    (void) strcpy (buf, MSGSTR(MNOTDEL, "\nMessage not delivered to everyone.\n")); /*MSG*/
		    (void) write (out, buf, strlen (buf));
		    (void) strcpy (buf, MSGSTR(UNSENTDRFT, "\n------- Unsent Draft\n\n")); /*MSG*/
		    (void) write (out, buf, strlen (buf));
		    cpydgst (in, out, file, MSGSTR(TMPFILE, "temporary file")); /*MSG*/
		    (void) close (in);
		    (void) strcpy (buf, MSGSTR(DRFTEND, "\n------- End of Unsent Draft\n")); /*MSG*/
		    (void) write (out, buf, strlen (buf));
		    if (rename (file, strcpy (buf, m_backup (file))) == NOTOK)
			admonish (buf, MSGSTR(NORENAME, "unable to rename %s to %s"), file, buf); /*MSG*/
		}
	    (void) lseek (out, 0L, 0);
	    (void) dup2 (out, fileno (stdin));
	    (void) close (out);
	    if (forwsw)		
	        (void) sprintf (buf, MSGSTR(NOSENDDRFT, "send failed on enclosed draft")); /*MSG*/
	    else
	        (void) sprintf (buf, MSGSTR(NOSEND, "send failed on %s"), file); /*MSG*/

	    execlp (mailproc, r1bindex (mailproc, '/'), getusr (),
		    "-subject", buf, NULLCP);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (mailproc);
	    _exit (-1);

	default: 		/* no waiting... */
	    break;
    }
}

/*  */

static int  tmp_fd () {
    int     fd;
    char    tmpfil[BUFSIZ];

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    if ((fd = creat (tmpfil, 0600)) == NOTOK)
	return NOTOK;
    (void) close (fd);

    if ((fd = open (tmpfil, 2)) == NOTOK)
	return NOTOK;
    if (debugsw)
	advise (NULLCP, "temporary file %s selected", tmpfil);
    else
	if (unlink (tmpfil) == NOTOK)
	    advise (tmpfil, MSGSTR(NOREMOVE, "unable to remove %s"), tmpfil); /*MSG*/

    return fd;
}

/*  */

static anno (int fd,
	     register struct stat *st)
{
    int     child_id;
    void     (*hstat) (int), (*istat) (int), (*qstat) (int), (*tstat) (int);
    static char *cwd = NULL;
    struct stat st2;

    if (altmsg &&
	    (stat (altmsg, &st2) == NOTOK
		|| st -> st_mtime != st2.st_mtime
		|| st -> st_dev != st2.st_dev
		|| st -> st_ino != st2.st_ino)) {
	if (debugsw)
	    admonish (NULLCP, "$mhaltmsg mismatch");
	return;
    }

    child_id = debugsw ? NOTOK : fork ();
    switch (child_id) {
	case NOTOK: 		/* oops */
	    if (!debugsw)
		advise (NULLCP,
			    MSGSTR(BYHAND, "unable to fork, so doing annotations by hand...")); /*MSG*/
	    if (cwd == NULL)
		cwd = getcpy (pwd ());

	case OK: 
	    hstat = signal (SIGHUP, SIG_IGN);
	    istat = signal (SIGINT, SIG_IGN);
	    qstat = signal (SIGQUIT, SIG_IGN);
	    tstat = signal (SIGTERM, SIG_IGN);
	    annoaux (fd);
	    if (child_id == OK)
		_exit (0);
	    (void) signal (SIGHUP, (void(*)(int))hstat);
	    (void) signal (SIGINT, (void(*)(int))istat);
	    (void) signal (SIGQUIT, (void(*)(int))qstat);
	    (void) signal (SIGTERM, (void(*)(int))tstat);
	    (void) chdir (cwd);
	    break;

	default: 		/* no waiting... */
	    (void) close (fd);
	    break;
    }
}

/*  */

static	annoaux (int	fd)
{
    int	    fd2,
	    fd3,
	    msgnum;
    char   *cp,
	   *folder,
	   *maildir,
            buffer[BUFSIZ],
          **ap;
    FILE   *fp;
    struct msgs *mp;

    if ((folder = getenv ("mhfolder")) == NULL || *folder == (char)NULL) {
	if (debugsw)
	    admonish (NULLCP, "$mhfolder not set");
	return;
    }
    maildir = m_maildir (folder);
    if (chdir (maildir) == NOTOK) {
	if (debugsw)
	    admonish (maildir, "unable to change directory to");
	return;
    }
    if (!(mp = m_gmsg (folder))) {
	if (debugsw)
	    admonish (NULLCP, "unable to read folder %s");
	return;
    }
    if (mp -> hghmsg == 0) {
	if (debugsw)
	    admonish (NULLCP, "no messages in %s", folder);
	goto oops;
    }

    if ((cp = getenv ("mhmessages")) == NULL || *cp == (char)NULL) {
	if (debugsw)
	    admonish (NULLCP, "$mhmessages not set");
	goto oops;
    }
    if (!debugsw			/* MOBY HACK... */
	    && pushsw
	    && (fd3 = open ("/dev/null", 2)) != NOTOK
	    && (fd2 = dup (fileno (stderr))) != NOTOK) {
	(void) dup2 (fd3, fileno (stderr));
	(void) close (fd3);
    }
    else
	fd2 = NOTOK;
    for (ap = brkstring (cp = getcpy (cp), " ", NULLCP); *ap; ap++)
	(void) m_convert (mp, *ap);
    free (cp);
    if (fd2 != NOTOK)
	(void) dup2 (fd2, fileno (stderr));
    if (mp -> numsel == 0) {
	if (debugsw)
	    admonish (NULLCP, "no messages to annotate");
	goto oops;
    }

    (void) lseek (fd, 0L, 0);
    if ((fp = fdopen (fd, "r")) == NULL) {
	if (debugsw)
	    admonish (NULLCP, "unable to fdopen annotation list");
	goto oops;
    }
    cp = NULL;
    while (fgets (buffer, sizeof buffer, fp) != NULL)
	cp = add (buffer, cp);
    (void) fclose (fp);

    if (debugsw)
	advise (NULLCP, "annotate%s with %s: \"%s\"",
		inplace ? " inplace" : "", annotext, cp);
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if (debugsw)
		advise (NULLCP, "annotate message %d", msgnum);
	    (void) annotate (m_name (msgnum), annotext, cp, inplace);
	}

    free (cp);

oops: ;
    m_fmsg (mp);
}

/*  */

void done (status)
int	status;
{
    if (armed)
	longjmp (env, status ? status : NOTOK);

    exit (status);
}
