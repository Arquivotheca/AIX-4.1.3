static char sccsid[] = "@(#)80	1.9  src/bos/usr/bin/mh/uip/rmf.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:24";
/* 
 * COMPONENT_NAME: CMDMH rmf.c
 * 
 * FUNCTIONS: MSGSTR, Mrmf, rma, rmf 
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
/* static char sccsid[] = "rmf.c	8.1 88/04/15 15:50:39"; */

/* rmf.c - remove a folder */

#include "mh.h"
#include <stdio.h>
#include <sys/types.h>
#ifndef	BSD42
#ifndef	SYS5
#include <ndir.h>
#else	SYS5
#include <sys/dir.h>
#endif	SYS5
#else	BSD42
#include <sys/dir.h>
#endif	BSD42

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	INTRSW	0
    "interactive", 0,
#define	NINTRSW	1
    "nointeractive", 0,

#define	HELPSW	2
    "help", 4,

    NULL, (int)NULL
};

static int rmf();
static rma();

/*  */

/* ARGSUSED */

main (argc, argv)
int argc;
char *argv[];
{
    int     defolder = 0,
            interactive = -1;
    char   *cp,
           *folder = NULL,
            newfolder[BUFSIZ],
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS];
    char    hlds[NL_TEXTMAX];

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
		    (void) sprintf (buf, MSGSTR(UMESG, "%s [+folder] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case INTRSW: 
		    interactive = 1;
		    continue;
		case NINTRSW: 
		    interactive = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    adios (NULLCP, MSGSTR(USAGEM, "usage: %s [+folder] [switches]"), invo_name); /*MSG*/
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!folder) {
	folder = m_getfolder ();
	defolder++;
    }
    if (strcmp (m_mailpath (folder), pwd ()) == 0)
	adios (NULLCP, MSGSTR(NOREMOVE2, "sorry, you can't remove the current working directory")); /*MSG*/

    if (interactive == -1)
	interactive = defolder;

    if (index (folder, '/') && (*folder != '/') && (*folder != '.')) {
	for (cp = copy (folder, newfolder); cp > newfolder && *cp != '/'; cp--)
	    continue;
	if (cp > newfolder)
	    *cp = (char)NULL;
	else
	    (void) strcpy (newfolder, defalt);
    }
    else
	(void) strcpy (newfolder, defalt);

    if (interactive) {
	sprintf(hlds, MSGSTR(RMFOLD, "Remove folder \"%s\" <yes or no>? "), folder); /*MSG*/
	if (!getanswer (hlds))
	    done (0);
    }

    if (rmf (folder) == OK && strcmp (m_find (pfolder), newfolder)) {
	printf (MSGSTR(CURRENT, "[+%s now current]\n"), newfolder); /*MSG*/
	m_replace (pfolder, newfolder);
    }
    m_update ();

    done (0);
}

/*  */

static int  rmf (folder)
register char *folder;
{
    int     i,
            j,
            others;
    register char  *maildir;
    char    cur[BUFSIZ];
    char    hlds[NL_TEXTMAX];
    register struct direct *dp;
    register    DIR * dd;

#ifdef	COMPAT
    (void) m_delete (concat (current, "-", m_mailpath (folder), NULLCP));
#endif	COMPAT
    switch (i = chdir (maildir = m_maildir (folder))) {
	case OK: 
	    if (access (".", 2) != NOTOK && access ("..", 2) != NOTOK)
		break;		/* fall otherwise */

	case NOTOK: 
	    (void) sprintf (cur, "atr-%s-%s", current, m_mailpath (folder));
	    if (!m_delete (cur)) {
		printf (MSGSTR(DEREF, "[+%s de-referenced]\n"), folder); /*MSG*/
		return OK;
	    }
	    strcpy (hlds, MSGSTR(NOPROFILE, "you have no profile entry for the %s folder +%s")); /*MSG*/
	    advise (NULLCP, hlds, i == NOTOK ? MSGSTR(UNREADABLE, "unreadable") : MSGSTR(RO, "read-only"), folder); /*MSG*/
	    return NOTOK;
    }

    if ((dd = opendir (".")) == NULL)
	adios (NULLCP, MSGSTR(NOREADF2, "unable to read folder +%s"), folder); /*MSG*/
    others = 0;

    j = strlen (SBACKUP);
    while (dp = readdir (dd)) {
	switch (dp -> d_name[0]) {
	    case '.': 
		if (strcmp (dp -> d_name, ".") == 0
			|| strcmp (dp -> d_name, "..") == 0)
		    continue;	/* else fall */

	    case ',': 
#ifdef	MHE
	    case '+': 
#endif	MHE
#ifdef	UCI
	    case '_': 
	    case '#': 
#endif	UCI
		break;

	    default: 
		if (m_atoi (dp -> d_name))
		    break;
#ifdef	COMPAT
		if (strcmp (dp -> d_name, current) == 0)
		    break;
#endif	COMPAT
		if (strcmp (dp -> d_name, LINK) == 0
			|| strncmp (dp -> d_name, SBACKUP, j) == 0)
		    break;

		admonish (NULLCP, MSGSTR(NODELETE, "file \"%s/%s\" not deleted"), folder, dp -> d_name); /*MSG*/
		others++;
		continue;
	}
	if (unlink (dp -> d_name) == NOTOK) {
	    admonish (dp -> d_name, MSGSTR(NOUNLINK2, "unable to unlink %s:%s"), folder, dp -> d_name); /*MSG*/
	    others++;
	}
    }

    closedir (dd);

    rma (folder);

    (void) chdir ("..");
    if (others == 0 && remdir (maildir))
	return OK;

    advise (NULLCP, MSGSTR(NOTGONE, "folder +%s not removed"), folder); /*MSG*/
    return NOTOK;
}

/*  */

static  rma (folder)
register char   *folder;
{
    register int    alen,
                    j,
                    plen;
    register char  *cp;
    register struct node   *np,
                           *pp;

    alen = strlen ("atr-");
    plen = strlen (cp = m_mailpath (folder)) + 1;

    m_getdefs ();
    for (np = m_defs, pp = NULL; np; np = np -> n_next)
	if (ssequal ("atr-", np -> n_name)
		&& (j = strlen (np -> n_name) - plen) > alen
		&& *(np -> n_name + j) == '-'
		&& strcmp (cp, np -> n_name + j + 1) == 0) {
	    if (!np -> n_context)
		admonish (NULLCP, MSGSTR(BUG, "bug: m_delete(key=\"%s\")"), np -> n_name); /*MSG*/
	    if (pp) {
		pp -> n_next = np -> n_next;
		np = pp;
	    }
	    else
		m_defs = np -> n_next;
	    ctxflags |= CTXMOD;
	}
	else
	    pp = np;
}
