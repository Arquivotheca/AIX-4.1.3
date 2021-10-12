static char sccsid[] = "@(#)56  1.11  src/bos/usr/bin/mh/uip/forw.c, cmdmh, bos411, 9428A410j 11/9/93 09:41:30";
/* 
 * COMPONENT_NAME: CMDMH forw.c
 * 
 * FUNCTIONS: MSGSTR, Mforw, build_form, copy_draft, mhl_draft 
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
/* static char sccsid[] = "forw.c	7.1 87/10/13 17:27:14"; */

/* forw.c - forward messages */

#include "mh.h"
#include "formatsbr.h"
#include "tws.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#undef IFMT
#define	IFMT	"digest-issue-%s"
#define	VFMT	"digest-volume-%s"

/*  */

static struct swit switches[] = {
#define	ANNOSW	0
    "annotate", 0,
#define	NANNOSW	1
    "noannotate", 0,

#define	DFOLDSW	2
    "draftfolder +folder", 0,
#define	DMSGSW	3
    "draftmessage msg", 0,
#define	NDFLDSW	4
    "nodraftfolder", 0,

#define	EDITRSW	5
    "editor editor", 0,
#define	NEDITSW	6
    "noedit", 0,

#define	FILTSW	7
    "filter filterfile", 0,
#define	FORMSW	8
    "form formfile", 0,

#define	FRMTSW	9
    "format", 5,
#define	NFRMTSW	10
    "noformat", 7,

#define	INPLSW	11
    "inplace", 0,
#define	NINPLSW	12
    "noinplace", 0,

#define	DGSTSW	13
    "digest list", 0,
#define	ISSUESW	14
    "issue number", 0,
#define	VOLUMSW	15
    "volume number", 0,

#define	WHATSW	16
    "whatnowproc program", 0,
#define	NWHATSW	17
    "nowhatnowproc", 0,

#define	HELPSW	18
    "help", 4,

#define	FILESW	19
    "file file", -4,		/* interface from msh */

#ifdef	MHE
#define	BILDSW	20
    "build", -5,		/* interface from mhe */
#endif	MHE

    NULL, (int)NULL
};

/*  */

static struct swit aqrnl[] = {
#define	NOSW	0
    "quit", 0,
#define	YESW	1
    "replace", 0,
#define	LISTDSW	2
    "list", 0,
#define	REFILSW	3
    "refile +folder", 0,
#define NEWSW	4
    "new", 0,

    NULL, (int)NULL
};

static struct swit aqrl[] = {
    "quit", 0,
    "replace", 0,
    "list", 0,
    "refile +folder", 0,

    NULL, (int)NULL
};

/*  */

static char drft[BUFSIZ];

static char delim3[] =
    "\n------------------------------------------------------------\n\n";
static char delim4[] = "\n------------------------------\n\n";


static struct msgs *mp = NULL;		/* used a lot */

static	copy_draft (int,register char *,
		    register char *),
	mhl_draft  (int, register char *, 
		    register char *, register char *);

static int  build_form (register char *,register char *,
			int  volume, int issue );

long	lseek (), time ();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     msgp = 0,
            anot = 0,
            inplace = 0,
	    issue = 0,
	    volume = 0,
#ifdef	MHE
	    buildsw = 0,
#endif	MHE
	    nedit = 0,
	    nwhat = 0,
	    i,
            in,
            out,
	    isdf = 0,
            msgnum;
    char   *cp,
	   *cwd,
           *maildir,
	   *dfolder = NULL,
	   *dmsg = NULL,
	   *digest = NULL,
           *ed = NULL,
           *file = NULL,
	   *filter = NULL,
           *folder = NULL,
           *form = NULL,
            buf[100],
	    value[10],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
    struct stat st;

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

		case ANNOSW: 
		    anot++;
		    continue;
		case NANNOSW: 
		    anot = 0;
		    continue;

		case EDITRSW: 
		    if (!(ed = *argp++) || *ed == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    nedit = 0;
		    continue;
		case NEDITSW:
		    nedit++;
		    continue;

		case WHATSW: 
		    if (!(whatnowproc = *argp++) || *whatnowproc == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    nwhat = 0;
		    continue;
#ifdef	MHE
		case BILDSW:
		    buildsw++;	/* fall... */
#endif	MHE
		case NWHATSW: 
		    nwhat++;
		    continue;

		case FILESW: 
		    if (file)
			adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    file = path (cp, TFILE);
		    continue;
		case FILTSW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    filter = getcpy (libpath (cp));
		    continue;
		case FORMSW: 
		    if (!(form = *argp++) || *form == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case FRMTSW:
		    filter = getcpy (libpath (mhlforward));
		    continue;
		case NFRMTSW:
		    filter = NULL;
		    continue;

		case INPLSW: 
		    inplace++;
		    continue;
		case NINPLSW: 
		    inplace = 0;
		    continue;

		case DGSTSW: 
		    if (!(digest = *argp++) || *digest == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
		case ISSUESW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((issue = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
		    continue;
		case VOLUMSW:
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((volume = atoi (cp)) < 1)
			adios (NULLCP, MSGSTR(BADARG, "bad argument %s %s"), argp[-2], cp); /*MSG*/
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

    cwd = getcpy (pwd ());

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (file && (msgp || folder))
	adios (NULLCP, MSGSTR(NOFFS, "can't mix files and folders/msgs")); /*MSG*/

try_it_again: ;
#ifndef MHE
    (void) strcpy (drft, m_draft (dfolder, dmsg, NOUSE, &isdf));
    if (stat (drft, &st) != NOTOK) {
#else	MHE
    (void) strcpy (drft, buildsw ? m_maildir ("draft")
			  : m_draft (dfolder, NULLCP, NOUSE, &isdf));
    if (!buildsw && stat (drft, &st) != NOTOK) {
#endif	MHE
	printf (MSGSTR(DRAFT, "Draft \"%s\" exists (%ld bytes)."), drft, st.st_size); /*MSG*/
	for (i = LISTDSW; i != YESW;) {
	    if (!(argp = getans (MSGSTR(DISPOS,"\nDisposition? "),
				isdf ? aqrnl : aqrl))) /*MSG*/
		done (1);
	    switch (i = smatch (*argp, isdf ? aqrnl : aqrl)) {
		case NOSW: 
		    done (0);
		case NEWSW: 
		    dmsg = NULL;
		    goto try_it_again;
		case YESW: 
		    break;
		case LISTDSW: 
		    (void) showfile (++argp, drft);
		    break;
		case REFILSW: 
		    if (refile (++argp, drft) == 0)
			i = YESW;
		    break;
		default: 
		    advise (NULLCP, MSGSTR(WHAT, "say what?")); /*MSG*/
		    break;
	    }
	}
    }

/*  */

    if (file) {
	anot = 0;
	goto go_to_it;
    }

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

/*  */

go_to_it: ;
    if (filter && access (filter, 04) == NOTOK)
	adios (filter, MSGSTR(NOREAD, "unable to read %s"), filter); /*MSG*/

    if (digest) {
	if (issue == 0) {
	    (void) sprintf (buf, IFMT, digest);
	    if (volume == 0
		    && (cp = m_find (buf))
		    && ((issue = atoi (cp)) < 0))
		issue = 0;
	    issue++;
	}
	if (volume == 0) {
	    (void) sprintf (buf, VFMT, digest);
	    if ((cp = m_find (buf)) == NULL || (volume = atoi (cp)) <= 0)
		volume = 1;
        }
	if (!form)
	    form = digestcomps;
	in = build_form (form, digest, volume, issue);
    }
    else
	if (form) {
	    if ((in = open (libpath (form), 0)) == NOTOK)
		adios (form, MSGSTR(NOOPENFL, "unable to open form file %s"), form); /*MSG*/
	}
	else {
	    if ((in = open (libpath (forwcomps), 0)) == NOTOK)
		adios (forwcomps, MSGSTR(NOOPENDEF, "unable to open default components file %s"), forwcomps); /*MSG*/
	    form = forwcomps;
	}

    if ((out = creat (drft, m_gmprot ())) == NOTOK)
	adios (drft, MSGSTR(NOCREATE, "unable to create %s"), drft); /*MSG*/

    cpydata (in, out, form, drft);
    (void) close (in);

/*  */

    if (file) {
	if ((in = open (file, 0)) == NOTOK)
	    adios (file, MSGSTR(NOOPEN, "unable to open %s"), file); /*MSG*/
	cpydata (in, out, file, drft);
	(void) close (in);
	(void) close (out);
	goto edit_it;
    }

    if (filter)
	mhl_draft (out, digest, drft, filter);
    else
	copy_draft (out, digest, drft);
    (void) close (out);

    if (digest) {
	(void) sprintf (buf, IFMT, digest);
	(void) sprintf (value, "%d", issue);
	m_replace (buf, getcpy (value));
	(void) sprintf (buf, VFMT, digest);
	(void) sprintf (value, "%d", volume);
	m_replace (buf, getcpy (value));
    }

    m_replace (pfolder, folder);
    if (mp -> lowsel != mp -> curmsg)
	m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_update ();

edit_it: ;
    if (nwhat)
	done (0);
    (void) m_whatnow (ed, nedit, NOUSE, drft, NULLCP, 0, mp,
	anot ? "Forwarded" : NULLCP, inplace, cwd);
    done (1);
}

/*  */

static	mhl_draft  (int out, register char   *digest,
		    register char *file, register char *filter)
{
    int     i,
            child_id,
	    msgnum,
            pd[2];
    char   *vec[MAXARGS];

    if (pipe (pd) == NOTOK)
	adios ("pipe", MSGSTR(NOPIPE, "unable to pipe")); /*MSG*/

    vec[0] = r1bindex (mhlproc, '/');

    for (i = 0; (child_id = fork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 
	    adios ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/

	case OK: 
	    (void) close (pd[0]);
	    (void) dup2 (pd[1], 1);
	    (void) close (pd[1]);

	    i = 1;
	    vec[i++] = "-forwall";
	    vec[i++] = "-form";
	    vec[i++] = filter;
	    if (digest) {
		vec[i++] = "-digest";
		vec[i++] = digest;
	    }
	    if (mp -> numsel >= MAXARGS - i)
		adios (NULLCP, MSGSTR(MANYEMSGS, "more than %d messages for %s exec"), vec[0], MAXARGS - i); /*MSG*/
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    vec[i++] = getcpy (m_name (msgnum));
	    vec[i] = NULL;

	    execvp (mhlproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (mhlproc);
	    _exit (-1);

	default: 
	    (void) close (pd[1]);
	    cpydata (pd[0], out, vec[0], file);
	    (void) close (pd[0]);
	    (void) pidXwait (child_id, mhlproc);
	    break;
    }
}

/*  */

static	copy_draft (int out, 
		    register char *digest,
		    register char *file)
{
    int     fd,i,
            msgcnt,
            msgnum;
    register char  *bp,
                   *msgnam;
    char    buffer[BUFSIZ];

    msgcnt = 1;
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if (digest)
		(void) strcpy (buffer,
			msgnum == mp -> lowsel ? delim3 : delim4);
	    else {
		(void) strcpy (bp = buffer, "\n-------"), bp += strlen (bp);
		if (msgnum == mp -> lowsel) {
		    if (mp -> numsel > 1)
		        (void) sprintf (bp, MSGSTR(FORMSGS, " Forwarded Messages")); /*MSG*/
		    else
		        (void) sprintf (bp, MSGSTR(FORWMSG, " Forwarded Message")); /*MSG*/
		}
		else
		    (void) sprintf (bp, MSGSTR(MESG, " Message %d"), msgcnt); /*MSG*/
		bp += strlen (bp);
		(void) strcpy (bp, "\n\n");
	    }
	    (void) write (out, buffer, strlen (buffer));

	    if ((fd = open (msgnam = m_name (msgnum), 0)) == NOTOK) {
		admonish (msgnam, MSGSTR(NORMSG, "unable to read message %s"), msgnam); /*MSG*/
		continue;
	    }
	    cpydgst (fd, out, msgnam, file);
	    (void) close (fd);

	    msgcnt++;
	}

    if (digest)
	(void) strcpy (buffer, delim4);
    else {
	if (mp -> numsel > 1)
	    (void) sprintf (buffer, MSGSTR(ENDFORW, "\n------- End of Forwarded Messages\n\n")); /*MSG*/
	else
	    (void) sprintf (buffer, MSGSTR(ENDFORW2, "\n------- End of Forwarded Message\n\n")); /*MSG*/
    }
    (void) write (out, buffer, strlen (buffer));

    if (digest) {
	(void) sprintf (buffer, MSGSTR(DEND, "End of %s Digest\n"), digest); /*MSG*/
	i = strlen (buffer);
	for (bp = buffer + i; i > 1; i--)
	    *bp++ = '*';
	*bp++ = '\n';
	*bp = (char)NULL;
	(void) write (out, buffer, strlen (buffer));
    }
}

/*  */

static int  build_form (register char *form,
			register char *digest,
			int    volume,
			int    issue )
{
    int	    in;
    int     fmtsize;
    register char *nfs;
    char   *line,
            tmpfil[BUFSIZ];
    register    FILE *tmp;
    register struct comp *cptr;
    struct format *fmt;
    int     dat[4];

    nfs = new_fs (form, NULLCP, NULLCP);
    fmtsize = strlen (nfs) + 256;
    (void) fmt_compile (nfs, &fmt);

    FINDCOMP (cptr, "digest");
    if (cptr)
	cptr->c_text = digest;
    FINDCOMP (cptr, "date");
    if (cptr)
	cptr->c_text = getcpy(dtimenow ());

    dat[0] = volume;
    dat[1] = issue;
    dat[2] = 0;
    dat[3] = fmtsize;

    (void) strcpy (tmpfil, m_tmpfil (invo_name));
    if ((tmp = fopen (tmpfil, "w+")) == NULL)
	adios (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
    (void) unlink (tmpfil);
    if ((in = dup (fileno (tmp))) == NOTOK)
	adios ("dup", MSGSTR(NODUP, "unable to dup")); /*MSG*/

    if ((line = (char *)malloc ((unsigned) fmtsize)) == NULLCP)
	adios (NULLCP, MSGSTR(NOAFLSTOR, "unable to allocate format line storage")); /*MSG*/
    (void) fmtscan (fmt, line, fmtsize, dat);
    (void) fputs (line, tmp);
    (void) free (line);
    if (fclose (tmp))
	adios (tmpfil, MSGSTR(WERR, "error writing %s"), tmpfil); /*MSG*/

    (void) lseek (in, 0L, 0);
    return in;
}
