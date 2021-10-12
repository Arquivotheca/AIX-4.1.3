static char sccsid[] = "@(#)88	1.9  src/bos/usr/bin/mh/uip/show.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:47";
/* 
 * COMPONENT_NAME: CMDMH show.c
 * 
 * FUNCTIONS: MSGSTR, Mshow, OfficialName 
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
/* static char sccsid[] = "show.c	7.1 87/10/13 17:38:55"; */

/* show.c - list messages */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 
/*  */

static struct swit switches[] = {
#define	DRFTSW	0
    "draft", -5,

#define	FORMSW	1
    "form formfile", -4,
#define	PROGSW	2
    "moreproc program", -4,
#define	NPROGSW	3
    "nomoreproc", -3,
#define	LENSW	4
    "length lines", 4,
#define	WIDSW	5
    "width columns", 4,

#define	SHOWSW	6
    "showproc program", 4,
#define	NSHOWSW	7
    "noshowproc", 3,

#define	HEADSW	8
    "header", 4,
#define	NHEADSW	9
    "noheader", 3,

#define	FILESW	10
    "file file", -4,		/* interface from showfile */

#define	HELPSW	11
    "help", 4,

    NULL, (int)NULL
};


#define	SHOW	0
#define	NEXT	1
#define	PREV	2

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     draftsw = 0,
            headersw = 1,
            nshow = 0,
            msgp = 0,
            vecp = 1,
	    procp = 1,
	    isdf = 0,
	    mode = SHOW,
            msgnum;
    char   *cp,
           *maildir,
           *file = NULL,
           *folder = NULL,
           *proc,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS],
           *vec[MAXARGS];
    struct msgs *mp;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    if (uleq (invo_name, "next"))
	mode = NEXT;
    else
	if (uleq (invo_name, "prev"))
	    mode = PREV;
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
		case NPROGSW:
		    vec[vecp++] = --cp;
		    continue;
		case HELPSW: 
		    if (mode == SHOW)
		    (void) sprintf (buf, MSGSTR(HELPSW2, "%s [+folder] [msgs] [switches] [switches for showproc]"), invo_name); /*MSG*/
		    else
		    (void) sprintf (buf, MSGSTR(HELPSW3, "%s [+folder] [switches] [switches for showproc]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case DRFTSW: 
		    if (file)
			adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
		    draftsw++;
		    if (mode == SHOW)
			continue;
	    usage:  ;
		    adios (NULLCP, MSGSTR(USAGE2, "usage: %s [+folder] [switches] [switches for showproc]"), invo_name); /*MSG*/
		case FILESW: 
		    if (mode != SHOW)
			goto usage;
		    if (draftsw || file)
			adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    file = path (cp, TFILE);
		    continue;

		case HEADSW: 
		    headersw++;
		    continue;
		case NHEADSW: 
		    headersw = 0;
		    continue;

		case FORMSW:
		case PROGSW:
		case LENSW:
		case WIDSW:
		    vec[vecp++] = --cp;
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    vec[vecp++] = cp;
		    continue;

		case SHOWSW: 
		    if (!(showproc = *argp++) || *showproc == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    nshow = 0;
		    continue;
		case NSHOWSW: 
		    nshow++;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    if (mode != SHOW)
		goto usage;
	    else
		msgs[msgp++] = cp;
    }
    procp = vecp;

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));

    if (draftsw || file) {
	if (msgp > 1)
	    adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
	vec[vecp++] = draftsw
	    ? getcpy (m_draft (folder, msgp ? msgs[0] : NULL, 1, &isdf))
	    : file;
	goto go_to_it;
    }

#ifdef	WHATNOW
    if (!msgp && mode == SHOW && (cp = getenv ("mhdraft")) && *cp) {
	vec[vecp++] = cp;
	goto go_to_it;
    }
#endif	WHATNOW

    if (!msgp)
	msgs[msgp++] = mode == NEXT ? "next" : mode == PREV ? "prev" : "cur";
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
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    mp -> msgstats[msgnum] |= UNSEEN;
    m_setseq (mp);
    m_setvis (mp, 1);

    if (mp -> numsel > MAXARGS - 2)
	adios (NULLCP, MSGSTR(MANYMSGS2, "more than %d messages for show exec"), MAXARGS - 2); /*MSG*/
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    vec[vecp++] = getcpy (m_name (msgnum));

    m_replace (pfolder, folder);
    if (mp -> hghsel != mp -> curmsg)
	m_setcur (mp, mp -> hghsel);
    m_sync (mp);
    m_update ();

    if (vecp == 2 && headersw)
	printf (MSGSTR(MSG1, "(Message %s:%s)\n"), folder, vec[1]); /*MSG*/

/*  */

go_to_it: ;
    (void) fflush (stdout);

    if (nshow)
	proc = "/bin/cat";
    else {
	(void) putenv ("mhfolder", folder);
	if (strcmp (r1bindex (showproc, '/'), "mhl") == 0) {
	    vec[0] = "mhl";
	    (void) mhl (vecp, vec);
	    done (0);
	}
	proc = showproc;
    }

    if (!draftsw
	    && chdir (maildir = concat (m_maildir (""), "/", NULLCP))
	    != NOTOK) {
	mp -> foldpath = concat (mp -> foldpath, "/", NULLCP);
	cp = ssequal (maildir, mp -> foldpath)
	    ? mp -> foldpath + strlen (maildir)
	    : mp -> foldpath;
	for (msgnum = procp; msgnum < vecp; msgnum++)
	    vec[msgnum] = concat (cp, vec[msgnum], NULLCP);
    }

    vec[0] = r1bindex (proc, '/');
    execvp (proc, vec);
    adios (proc, MSGSTR(UNEXEC, "unable to exec %s"), proc); /*MSG*/
}

/*  */

/* Cheat:  we are loaded with adrparse, which wants a routine called
   OfficialName().  We call adrparse:getm() with the correct arguments
   to prevent OfficialName() from being called.  Hence, the following
   is to keep the loader happy.
 */

char   *OfficialName (name)
register char  *name;
{
    return name;
}
