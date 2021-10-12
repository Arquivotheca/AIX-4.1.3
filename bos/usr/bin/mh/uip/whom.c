static char sccsid[] = "@(#)99	1.12  src/bos/usr/bin/mh/uip/whom.c, cmdmh, bos411, 9428A410j 11/9/93 09:45:40";
/* 
 * COMPONENT_NAME: CMDMH whom.c
 * 
 * FUNCTIONS: MSGSTR, Mwhom 
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
/* static char sccsid[] = "whom.c	7.1 87/10/13 17:44:11"; */

/* whom.c - report who a message would go to */

#include "mh.h"
#include <stdio.h>
#include <signal.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 
/*  */

char *getenv();

static struct swit switches[] = {
#define	ALIASW	0
    "alias aliasfile", 0,

#define	CHKSW	1
    "check", 0,
#define	NOCHKSW	2
    "nocheck", 0,

#define	DRAFTSW	3
    "draft", 0,

#define	DFOLDSW	4
    "draftfolder +folder", 6,
#define	DMSGSW	5
    "draftmessage msg", 6,
#define	NDFLDSW	6
    "nodraftfolder", 0,

#define	HELPSW	7
    "help", 4,

#define	CLIESW	8
    "client host", -6,
#define	SERVSW	9
    "server host", -6,
#define	SNOOPSW	10
    "snoop", -5,

    NULL, (int)NULL
};

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     child_id,
	    i,
	    status,
	    isdf = 0,
	    distsw = 0,
            vecp = 1;
    char   *cp,
	   *dfolder = NULL,
	   *dmsg = NULL,
           *msg = NULL,
          **ap,
          **argp,
	    backup[BUFSIZ],
            buf[100],
           *arguments[MAXARGS],
           *vec[MAXARGS];

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

    vec[0] = r1bindex (postproc, '/');
    vec[vecp++] = "-whom";
    vec[vecp++] = "-library";
    vec[vecp++] = getcpy (m_maildir (""));

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
		    (void) sprintf (buf, MSGSTR(HELPSW6, "%s [switches] [file]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case CHKSW: 
		case NOCHKSW: 
		case SNOOPSW:
		    vec[vecp++] = --cp;
		    continue;

		case DRAFTSW:
		    msg = draft;
		    continue;

		case DFOLDSW: 
		    if (dfolder)
			adios (NULLCP, MSGSTR(ONEDFOLD, "only one draft folder at a time!")); /*MSG*/
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    dfolder = path (*cp == '+' || *cp == '@' ? cp + 1 : cp,
			    *cp != '@' ? TFOLDER : TSUBCWF);
		    continue;
		case DMSGSW: 
		    if (dmsg)
			adios (NULLCP, MSGSTR(ONEDMSG, "only one draft message at a time!")); /*MSG*/
		    if (!(dmsg = *argp++) || *dmsg == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case NDFLDSW: 
		    dfolder = NULL;
		    isdf = NOTOK;
		    continue;

		case ALIASW: 
		case CLIESW: 
		case SERVSW: 
		    vec[vecp++] = --cp;
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    vec[vecp++] = cp;
		    continue;
	    }
	if (msg)
	    adios (NULLCP, MSGSTR(ONEDRAFT, "only one draft at a time!")); /*MSG*/
	else
	    vec[vecp++] = msg = cp;
    }

/*  */

    if (msg == NULL) {
#ifdef	WHATNOW
	if ((cp = getenv ("mhdraft")) == NULL || *cp == NULL)
#endif	WHATNOW
	    cp  = getcpy (m_draft (dfolder, dmsg, 1, &isdf));
	msg = vec[vecp++] = cp;
    }
    if ((cp = getenv ("mhdist"))
	    && *cp
	    && (distsw = atoi (cp))
	    && (cp = getenv ("mhaltmsg"))
	    && *cp) {
	if (distout (msg, cp, backup) == NOTOK)
	    done (1);
	vec[vecp++] = "-dist";
	distsw++;
    }
    vec[vecp] = NULL;

    closefds (3);

    if (distsw)
	for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	    sleep (5);
    switch (distsw ? child_id : OK) {
	case NOTOK:
    	    advise (NULLCP, MSGSTR(CHECKING, "unable to fork, so checking directly...")); /*MSG*/
	case OK:
	    execvp (postproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (postproc);
	    _exit (-1);

	default:
	    (void) signal (SIGHUP, SIG_IGN);
	    (void) signal (SIGINT, SIG_IGN);
	    (void) signal (SIGQUIT, SIG_IGN);
	    (void) signal (SIGTERM, SIG_IGN);

	    status = pidwait (child_id, OK);

	    (void) unlink (msg);
	    if (rename (backup, msg) == NOTOK)
		adios (msg, MSGSTR(NORENAME, "unable to rename %s to %s"), backup, msg); /*MSG*/
	    done (status);
    }
}
