static char sccsid[] = "@(#)62	1.10.1.1  src/bos/usr/bin/mh/uip/mhmail.c, cmdmh, bos411, 9428A410j 11/9/93 09:42:22";
/* 
 * COMPONENT_NAME: CMDMH mhmail.c
 * 
 * FUNCTIONS: MSGSTR, Mmhmail, intrser 
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
/* static char sccsid[] = "mhmail.c	7.1 87/10/13 17:29:21"; */

/* mhmail.c - simple mail program */

#include "mh.h"
#include <stdio.h>
#include <signal.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	BODYSW	0
    "body text", 0,

#define	CCSW	1
    "cc addrs ...", 0,

#define	FROMSW	2
    "from addr", 0,

#define	SUBJSW	3
    "subject subject", 0,

#define	HELPSW	4
    "help", 4,

    NULL, (int)NULL
};

/*  */

static void	intrser (int);


static char tmpfil[BUFSIZ];

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     child_id,
	    status,
            i,
            iscc = 0,
            somebody;
    char   *cp,
           *tolist = NULL,
           *cclist = NULL,
           *subject = NULL,
	   *from = NULL,
           *body = NULL,
          **argp = argv + 1,
            buf[100];
    FILE * out;

        setlocale(LC_ALL,"");
	catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    m_foil (NULLCP);

    if (argc == 1) {
	execlp (incproc, r1bindex (incproc, '/'), NULLCP);
	adios (incproc, MSGSTR(UNEXEC, "unable to exec %s"), incproc); /*MSG*/
    }

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);

		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/

		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(ADDRS, "%s [addrs ... [switches]]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case FROMSW: 
		    if (!(from = *argp++) || *from == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case BODYSW: 
		    if (!(body = *argp++) || *body == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case CCSW: 
		    iscc++;
		    continue;

		case SUBJSW: 
		    if (!(subject = *argp++) || *subject == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
	    }
	if (iscc)
	    cclist = cclist ? add (cp, add (", ", cclist)) : getcpy (cp);
	else
	    tolist = tolist ? add (cp, add (", ", tolist)) : getcpy (cp);
    }

/*  */

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    if ((out = fopen (tmpfil, "w")) == NULL)
	adios (tmpfil, MSGSTR(NOWRITE, "unable to write %s"), tmpfil); /*MSG*/
    (void) chmod (tmpfil, 0600);

    setsig (SIGINT, (void(*)(int))intrser);

    fprintf (out, "To: %s\n", tolist);
    if (cclist)
	fprintf (out, "cc: %s\n", cclist);
    if (subject)
	fprintf (out, "Subject: %s\n", subject);
    if (from)
	fprintf (out, "From: %s\n", from);
    fprintf (out, "\n");

    if (body)
	fprintf (out, "%s\n", body);
    else {
	for (somebody = 0;
		(i = read (fileno (stdin), buf, sizeof buf)) > 0;
		somebody++)
	    if (fwrite (buf, sizeof *buf, i, out) != i)
		adios (tmpfil, MSGSTR(WERR, "error writing %s"), tmpfil); /*MSG*/
	if (!somebody) {
	    (void) unlink (tmpfil);
	    done (1);
	}
    }
    (void) fclose (out);

/*  */

    for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 		/* report failure and then send it */
	    admonish (NULLCP, MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	case OK: 
	    execlp (postproc, r1bindex (postproc, '/'), tmpfil, NULLCP);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (postproc);
	    _exit (-1);

	default: 
	    if (status = pidXwait (child_id, postproc)) {
		fprintf (stderr, MSGSTR(LTRSAVED, "Letter saved in dead.letter\n")); /*MSG*/
		execl ("/bin/mv", "mv", tmpfil, "dead.letter", NULLCP);
		execl ("/usr/bin/mv", "mv", tmpfil, "dead.letter", NULLCP);
		perror ("mv");
		_exit (-1);
	    }

	    (void) unlink (tmpfil);
	    done (status ? 1 : 0);
    }
}

/*  */

/* ARGSUSED */

static void  intrser (int i)
{
#ifndef	BSD42
    if (i)
	(void) signal (i, SIG_IGN);
#endif	BSD42

    (void) unlink (tmpfil);
    done (i != 0 ? 1 : 0);
}
