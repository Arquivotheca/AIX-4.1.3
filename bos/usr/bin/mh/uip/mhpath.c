static char sccsid[] = "@(#)63	1.8  src/bos/usr/bin/mh/uip/mhpath.c, cmdmh, bos411, 9428A410j 11/9/93 09:42:32";
/* 
 * COMPONENT_NAME: CMDMH mhpath.c
 * 
 * FUNCTIONS: MSGSTR, Mmhpath 
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
/* static char sccsid[] = "mhpath.c	7.1 87/10/13 17:29:43"; */

/* mhpath.c - print full pathnames */

#include "mh.h"
#include <stdio.h>

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

main(argc, argv)
	int argc;
	char *argv[];
{
    int     msgp = 0,
            msgnum;
    char   *cp,
           *maildir,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
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
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(MHELPSW, "%s [+folder] [msgs] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    msgs[msgp++] = cp;
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));

    if (!folder)
	folder = m_getfolder ();
    maildir = m_maildir (folder);
    if (!msgp) {
	printf ("%s\n", maildir);
	done (0);
    }

    if (chdir (maildir) == NOTOK)
	adios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/

    if ((mp = m_remsg (mp, 0, MAXFOLDER)) == NULL)
	adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/
    mp -> msgflags |= MHPATH;

    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (!m_convert (mp, msgs[msgnum]))
	    done (1);
    m_setseq (mp);

/*  */

    if (mp -> numsel > MAXARGS - 2)
	adios (NULLCP, MSGSTR(MANYMSGS, "more than %d messages"), MAXARGS - 2); /*MSG*/

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    printf ("%s/%s\n", mp -> foldpath, m_name (msgnum));

    m_sync (mp);
    m_update ();

    done (0);
}
