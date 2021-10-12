static char sccsid[] = "@(#)81	1.8  src/bos/usr/bin/mh/uip/rmm.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:30";
/* 
 * COMPONENT_NAME: CMDMH rmm.c
 * 
 * FUNCTIONS: MSGSTR, Mrmm 
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
/* static char sccsid[] = "rmm.c	7.1 87/10/13 17:36:33"; */

/* rmm.c - remove a message */

#include "mh.h"
#include <stdio.h>
#include <locale.h>

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

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     msgp = 0,
            msgnum,
            vecp;
    char   *cp,
           *dp,
           *maildir,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
          **vec,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
    struct msgs *mp;

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

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, switches)) {
		case AMBIGSW: 
		    ambigsw (cp, switches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(MHELPSW, "%s [+folder] [msgs] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);
	    }
	if (*cp == '+') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, TFOLDER);
	}
	else
	    msgs[msgp++] = cp;
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!msgp)
	msgs[msgp++] = "cur";
    if (!folder)
	folder = m_getfolder ();
    maildir = m_maildir (folder);

    if (chdir (maildir) == NOTOK)
	adios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/
    if (mp -> hghmsg == 0)
	adios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), folder); /*MSG*/

    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    done (1);
    m_setseq (mp);

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
#ifdef	notdef
	    mp -> msgstats[msgnum] |= DELETED;
#endif	notdef
	    mp -> msgstats[msgnum] &= ~EXISTS;
	}
    mp -> msgflags |= SEQMOD;

    m_replace (pfolder, folder);
    m_sync (mp);
    m_update ();

    if (rmmproc) {
	if (mp -> numsel > MAXARGS - 2)
	    adios (NULLCP, MSGSTR(MANYEMSGS, "more than %d messages for %s exec"), MAXARGS - 2, rmmproc); /*MSG*/
	vec = (char **) calloc ((unsigned) (mp -> numsel + 2), sizeof *vec);
	if (vec == NULL)
	    adios (NULLCP, MSGSTR(NOAVECTOR, "unable to allocate exec vector")); /*MSG*/
	vecp = 1;
	for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED)
		vec[vecp++] = getcpy (m_name (msgnum));
	vec[vecp] = NULL;

	(void) fflush (stdout);
	vec[0] = r1bindex (rmmproc, '/');
	execvp (rmmproc, vec);
	adios (rmmproc, MSGSTR(UNEXEC, "unable to exec %s"), rmmproc); /*MSG*/
    }

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    (void) strcpy (buf, m_backup (dp = m_name (msgnum)));
	    if (rename (dp, buf) == NOTOK)
		admonish (buf, MSGSTR(NORENAME, "unable to rename %s to %s"), dp, buf); /*MSG*/
	}

    done (0);
}
