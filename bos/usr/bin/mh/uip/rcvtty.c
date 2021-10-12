static char sccsid[] = "@(#)76  1.9  src/bos/usr/bin/mh/uip/rcvtty.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:00";
/* 
 * COMPONENT_NAME: CMDMH rcvtty.c
 * 
 * FUNCTIONS: MSGSTR, Mrcvtty, alert, alrmser, header_fd, message_fd 
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
/* static char sccsid[] = "rcvtty.c	7.1 87/10/13 17:34:38"; */

/* rcvtty.c - a rcvmail program (a lot like rcvalert) handling IPC ttys */

#include "mh.h"
#include "rcvmail.h"
#include "scansbr.h"
#include "tws.h"
#include <signal.h>
#include <sys/stat.h>
#ifndef	TTYD
#include <utmp.h>
#endif	not TTYD

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	HELPSW	0
    "help", 4,

    NULL, (int)NULL
};

/*  */

static  jmp_buf myctx;
static  int alrmser(int), header_fd();
long	lseek ();
char   *getusr ();
static int  message_fd (char **);
#ifndef	TTYD
static  alert (register char *, int);
#endif TTYD


/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   **argv;
{
    int     md,
	    vecp = 0;
    char   *cp,
           *user,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
	   *vec[MAXARGS];
#ifndef	TTYD
    char    tty[BUFSIZ];
    struct utmp ut;
    register FILE  *uf;
#endif	not TTYD

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    vec[vecp++] = --cp;
		    continue;
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(HELPSW8, "%s [command ...]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);
	    }
	vec[vecp++] = cp;
    }
    vec[vecp] = 0;

/*  */

    if ((md = vecp ? message_fd (vec) : header_fd ()) == NOTOK)
	exit (RCV_MBX);

    user = getusr ();
#ifndef	TTYD
    if ((uf = fopen ("/etc/utmp", "r")) == NULL)
	exit (RCV_MBX);
    while (fread ((char *) &ut, sizeof ut, 1, uf) == 1)
	if (ut.ut_name[0] != (char)NULL
		&& strncmp (user, ut.ut_name, sizeof ut.ut_name) == 0) {
	    (void) strncpy (tty, ut.ut_line, sizeof ut.ut_line);
	    alert (tty, md);
	}
    (void) fclose (uf);
#else	TTYD
    alert (user, md);
#endif	TTYD

    exit (RCV_MOK);
}

/*  */

/* ARGSUSED */

static	int alrmser (int i)
{
    longjmp (myctx, DONE);
}


static int  message_fd (char *vec[])
{
    int     bytes,
	    child_id,
            fd;
    char    tmpfil[BUFSIZ];
    struct stat st;

    (void) unlink (mktemp (strcpy (tmpfil, "/tmp/rcvttyXXXXX")));
    if ((fd = creat (tmpfil, 0600)) == NOTOK)
	return header_fd ();
    (void) close (fd);

    if ((fd = open (tmpfil, 2)) == NOTOK)
	return header_fd ();
    (void) unlink (tmpfil);

/*  */

    switch (child_id = vfork ()) {
	case NOTOK: 
	    (void) close (fd);
	    return header_fd ();

	case OK: 
	    rewind (stdin);
	    if (dup2 (fd, 1) == NOTOK || dup2 (fd, 2) == NOTOK)
		_exit (-1);
	    closefds (3);
#ifdef	BSD42
	    (void) setpgrp (0, getpid ());
#endif	BSD42
	    execvp (vec[0], vec);
	    _exit (-1);

	default: 
	    switch (setjmp (myctx)) {
		case OK: 
		    (void) signal (SIGALRM, (void(*)(int))alrmser);
		    bytes = fstat (fileno (stdin), &st) != NOTOK
			? (int) st.st_size : 100;
		    if (bytes <= 0)
			bytes = 100;
		    (void) alarm ((unsigned) (bytes * 60 + 300));

		    (void) pidwait (child_id, OK);

		    (void) alarm (0);
		    if (fstat (fd, &st) != NOTOK && st.st_size > 0L)
			return fd;
		    (void) close (fd);
		    return header_fd ();

		default: 
#ifndef	BSD42
		    (void) kill (child_id, SIGKILL);
#else	BSD42
		    (void) killpg (child_id, SIGKILL);
#endif	BSD42
		    (void) close (fd);
		    return header_fd ();
	    }
    }
}

/*  */

static int  header_fd () {
    int     fd;
    char    tmpfil[BUFSIZ];

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    if ((fd = creat (tmpfil, 0600)) == NOTOK)
	return NOTOK;
    (void) close (fd);

    if ((fd = open (tmpfil, 2)) == NOTOK)
	return NOTOK;
    (void) unlink (tmpfil);

    rewind (stdin);
    (void) scan (stdin, 0, 0, NULLVP, 0, 0, 0, 0L, 0);
    (void) write (fd, scanl, strlen (scanl));

    return fd;
}

/*  */

#ifndef	TTYD
static  alert (register char *tty, int md)
{
    int     i,
            td;
    char    buffer[BUFSIZ],
            ttyspec[BUFSIZ];
    struct stat st;

    (void) sprintf (ttyspec, "/dev/%s", tty);
    if (stat (ttyspec, &st) == NOTOK || (st.st_mode & 02) == 0)
	return;

    switch (setjmp (myctx)) {
	case OK: 
	    (void) signal (SIGALRM, (void(*)(int))alrmser);
	    (void) alarm (2);
	    td = open (ttyspec, 1);
	    (void) alarm (0);
	    if (td == NOTOK)
		return;
	    break;

	default: 
	    (void) alarm (0);
	    return;
    }

    (void) lseek (md, 0L, 0);

    while ((i = read (md, buffer, sizeof buffer)) > 0)
	if (write (td, buffer, i) != i)
	    break;

    (void) close (td);
}
#else	TTYD

/*  */

static  alert (user, md)
register char   *user;
int     md;
{
    int     i,
            td;
    char    buffer[BUFSIZ];

    if ((td = ttyw ("notify", NULLCP, NULLCP, user)) == NOTOK)
	return;
    (void) signal (SIGPIPE, SIG_IGN);

    (void) lseek (md, 0L, 0);
    while ((i = read (md, buffer, sizeof buffer)) > 0)
	if (write (td, buffer, i) != i)
	    break;

    (void) close (td);
}
#endif	TTYD
