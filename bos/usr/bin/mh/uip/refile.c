static char sccsid[] = "@(#)77	1.10  src/bos/usr/bin/mh/uip/refile.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:09";
/* 
 * COMPONENT_NAME: CMDMH refile.c
 * 
 * FUNCTIONS: MSGSTR, Mrefile, clsfolds, m_file, mhremove, opnfolds 
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
/* static char sccsid[] = "refile.c	7.1 87/10/13 17:35:02"; */

/* refile.c - file messages away */

#include "mh.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	DRAFTSW	0
    "draft", 0,

#define	LINKSW	1
    "link", 0,
#define	NLINKSW	2
    "nolink", 0,

#define	PRESSW	3
    "preserve", 0,
#define	NPRESSW	4
    "nopreserve", 0,

#define	SRCSW	5
    "src +folder", 0,

#define	FILESW	6
    "file file", 0,

#define	HELPSW	7
    "help", 4,

    NULL, (int)NULL
};

/*  */

extern int  errno;


static char maildir[BUFSIZ];

struct st_fold {
    char   *f_name;
    struct msgs *f_mp;
};

static	opnfolds (register struct st_fold *, int);
static	clsfolds (register struct st_fold *, int);
static	mhremove (register struct msgs *, register int, register char **);

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int	    linkf = 0,
            prsrvf = 0,
	    filep = 0,
            foldp = 0,
            msgp = 0,
	    isdf = 0,
	    i,
            msgnum;
    char   *cp,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
	   *files[NFOLDERS + 1],
           *msgs[MAXARGS];
    struct st_fold   folders[NFOLDERS + 1];
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
		    (void) sprintf (buf, MSGSTR(HELPSW9, "%s [msgs] [switches] +folder ..."), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case LINKSW: 
		    linkf++;
		    continue;
		case NLINKSW: 
		    linkf = 0;
		    continue;

		case PRESSW: 
		    prsrvf++;
		    continue;
		case NPRESSW: 
		    prsrvf = 0;
		    continue;

		case SRCSW: 
		    if (folder)
			adios (NULLCP, MSGSTR(ONEFOLDER2, "only one source folder at a time!")); /*MSG*/
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    folder = path (*cp == '+' || *cp == '@' ? cp + 1 : cp,
				   *cp != '@' ? TFOLDER : TSUBCWF);
		    continue;
		case DRAFTSW:
		    if (filep > NFOLDERS)
			adios (NULLCP, MSGSTR(MAXFILES, "only %d files allowed!"), NFOLDERS); /*MSG*/
		    isdf = 0;
		    files[filep++] = getcpy (m_draft (NULLCP, NULLCP, 1, &isdf));
		    continue;
		case FILESW: 
		    if (filep > NFOLDERS)
			adios (NULLCP, MSGSTR(MAXFILES, "only %d files allowed!"), NFOLDERS); /*MSG*/
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    files[filep++] = path (cp, TFILE);
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (foldp > NFOLDERS)
		adios (NULLCP, MSGSTR(MAXFOLDERS, "only %d folders allowed!"), NFOLDERS); /*MSG*/
	    folders[foldp++].f_name =
		    path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    msgs[msgp++] = cp;
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (foldp == 0)
	adios (NULLCP, MSGSTR(NOFOLDER, "no folder specified")); /*MSG*/

#ifdef	WHATNOW
    if (!msgp && !filep && (cp = getenv ("mhdraft")) && *cp)
	files[filep++] = cp;
#endif	WHATNOW

    if (filep > 0) {
	if (folder || msgp)
	    adios (NULLCP, MSGSTR(CHOICE, "use -file or some messages, not both")); /*MSG*/
	opnfolds (folders, foldp);
	for (i = 0; i < filep; i++)
	    if (m_file (files[i], folders, foldp, prsrvf))
		done (1);
	if (!linkf)
	    mhremove (NULLMP, filep, files);
	done (0);
    }

    if (!msgp)
	msgs[msgp++] = "cur";
    if (!folder)
	folder = m_getfolder ();
    (void) strcpy (maildir, m_maildir (folder));

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

    opnfolds (folders, foldp);
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    cp = getcpy (m_name (msgnum));
	    if (m_file (cp, folders, foldp, prsrvf))
		done (1);
	    free (cp);
	    if (!linkf) {
#ifdef	notdef
		mp -> msgstats[msgnum] |= DELETED;
#endif	notdef
		mp -> msgstats[msgnum] &= ~EXISTS;
	    }
	}
    if (!linkf)
	mp -> msgflags |= SEQMOD;
    clsfolds (folders, foldp);

    m_replace (pfolder, folder);
    if (mp -> hghsel != mp -> curmsg
	    && (mp -> numsel != mp -> nummsg || linkf))
	m_setcur (mp, mp -> hghsel);
    m_sync (mp);
    m_update ();

    if (!linkf)
	mhremove (mp, filep, files);

    done (0);
}

/*  */

static	opnfolds (register struct st_fold *folders, 
		  int nfolders)
{
    register char  *cp;
    char    nmaildir[BUFSIZ];
    char hlds[NL_TEXTMAX];
    register struct st_fold *fp,
                            *ep;
    register struct msgs   *mp;
    struct stat st;

    for (ep = (fp = folders) + nfolders; fp < ep; fp++) {
	(void) chdir (m_maildir (""));
	(void) strcpy (nmaildir, m_maildir (fp -> f_name));

	if (stat (nmaildir, &st) == NOTOK) {
	    if (errno != ENOENT)
		adios (nmaildir, MSGSTR(ERRF, "error on folder %s"), nmaildir); /*MSG*/
	    sprintf(hlds, MSGSTR(CREFOL, "Create folder \"%s\" <yes or no>? "), nmaildir); /*MSG*/
	    if (!getanswer (hlds))
		done (1);
	    if (!makedir (nmaildir))
		adios (NULLCP, MSGSTR(NOCREAT, "unable to create folder %s"), nmaildir); /*MSG*/
	}

	if (chdir (nmaildir) == NOTOK)
	    adios (nmaildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), nmaildir); /*MSG*/
	if (!(mp = m_gmsg (fp -> f_name)))
	    adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), fp -> f_name); /*MSG*/
	mp -> curmsg = 0;

	fp -> f_mp = mp;

	(void) chdir (maildir);
    }
}

/*  */

static	clsfolds (register struct st_fold *folders, 
		  int nfolders)
{
    register struct st_fold *fp,
                           *ep;
    register struct msgs   *mp;

    for (ep = (fp = folders) + nfolders; fp < ep; fp++) {
	mp = fp -> f_mp;
	m_setseq (mp);
	m_sync (mp);
    }
}

/*  */

static	mhremove (
		  register struct msgs *mp,
		  register int filep,
		  register char **files)
{
    register int    i,
                    vecp;
    register char  *cp,
                  **vec;

    if (rmmproc) {
	if (filep > 0)
	    vec = files;
	else {
	    if (mp -> numsel > MAXARGS - 2)
		adios (NULLCP, MSGSTR(MANYEMSGS, "more than %d messages for %s exec"), MAXARGS - 2, rmmproc); /*MSG*/
	    vec = (char **) calloc ((unsigned) (mp -> numsel + 2), sizeof *vec);
	    if (vec == NULL)
		adios (NULLCP, MSGSTR(NOAVECTOR, "unable to allocate exec vector")); /*MSG*/
	    vecp = 1;
	    for (i = mp -> lowsel; i <= mp -> hghsel; i++)
		if (mp -> msgstats[i] & SELECTED)
		    vec[vecp++] = getcpy (m_name (i));
	    vec[vecp] = NULL;
	}

	(void) fflush (stdout);
	vec[0] = r1bindex (rmmproc, '/');
	execvp (rmmproc, vec);
	adios (rmmproc, MSGSTR(UNEXEC, "unable to exec %s"), rmmproc); /*MSG*/
    }

    if (filep > 0) {
	for (i = 0; i < filep; i++)
	    if (unlink (files[i]) == NOTOK)
		admonish (files[i], MSGSTR(NOUNLINK, "unable to unlink %s"), files[i]); /*MSG*/
    }
    else
	for (i = mp -> lowsel; i <= mp -> hghsel; i++)
	    if (mp -> msgstats[i] & SELECTED)
		if (unlink (cp = m_name (i)) == NOTOK)
		    admonish (cp, MSGSTR(NOUNLINK, "unable to unlink %s"), cp); /*MSG*/
}

/*  */

m_file (msg, folders, nfolders, prsrvf)
register char  *msg;
struct st_fold  *folders;
int	nfolders,
	prsrvf;
{
    int     in,
            out,
            linkerr,
            msgnum;
    register char  *nmsg;
    char    newmsg[BUFSIZ];
    register struct st_fold *fp,
			    *ep;
    register struct msgs *mp;
    struct stat st,
                s1;

    for (ep = (fp = folders) + nfolders; fp < ep; fp++) {
	mp = fp -> f_mp;
	if (prsrvf && (msgnum = m_atoi (nmsg = msg)) > 0) {
	    if (msgnum >= mp -> hghoff)
		if (mp = m_remsg (mp, 0, msgnum + MAXFOLDER))
		    fp -> f_mp = mp;
		else
		    adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/
	    if (!(mp -> msgstats[msgnum] & EXISTS)) {
		mp -> msgstats[msgnum] |= EXISTS;
#ifdef	notdef
		mp -> msgstats[msgnum] &= ~DELETED;
#endif	notdef
		mp -> nummsg++;
	    }
	    mp -> msgstats[msgnum] |= SELECTED;		    
	    if (msgnum > mp -> hghmsg)
		mp -> hghmsg = msgnum;
	}
	else {
	    if (mp -> hghmsg >= mp -> hghoff)
		if (mp = m_remsg (mp, 0, mp -> hghoff + MAXFOLDER))
		    fp -> f_mp = mp;
		else
		    adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

	    nmsg = m_name (msgnum = ++mp -> hghmsg);
	    mp -> nummsg++;
	    mp -> msgstats[msgnum] |= EXISTS | SELECTED;
	}
	if (mp -> lowmsg == 0)
	    mp -> lowmsg = msgnum;
	if (mp -> lowsel == 0 || msgnum < mp -> lowsel)
	    mp -> lowsel = msgnum;
	if (msgnum > mp -> hghsel)
	    mp -> hghsel = msgnum;

/*  */

	(void) sprintf (newmsg, "%s/%s", mp -> foldpath, nmsg);
	if (link (msg, newmsg) == NOTOK) {
	    linkerr = errno;
	    if (linkerr == EEXIST
		    || (linkerr == EXDEV && stat (newmsg, &st) != NOTOK)) {
		if (linkerr != EEXIST
			|| stat (msg, &s1) == NOTOK
			|| stat (newmsg, &st) == NOTOK
			|| s1.st_ino != st.st_ino) {
		    advise (NULLCP, MSGSTR(MEXISTS, "message %s:%s already exists"), fp -> f_name, newmsg); /*MSG*/
		    return 1;
		}
		continue;
	    }
	    if (linkerr == EXDEV) {
		if ((in = open (msg, 0)) == NOTOK) {
		    advise (msg, MSGSTR(NOOPENM, "unable to open message %s"), msg); /*MSG*/
		    return 1;
		}
		(void) fstat (in, &st);
		if ((out = creat (newmsg, (int) st.st_mode & 0777))
			== NOTOK) {
		    advise (newmsg, MSGSTR(NOCREATE, "unable to create %s"), newmsg); /*MSG*/
		    (void) close (in);
		    return 1;
		}
		cpydata (in, out, msg, newmsg);
		(void) close (in);
		(void) close (out);
	    }
	    else {
		advise (newmsg, MSGSTR(LINKINGERR, "error linking %s to %s"), msg, newmsg); /*MSG*/
		return 1;
	    }
	}
    }

    return 0;
}
