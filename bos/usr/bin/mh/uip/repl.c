static char sccsid[] = "@(#)78	1.10  src/bos/usr/bin/mh/uip/repl.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:18";
/* 
 * COMPONENT_NAME: CMDMH repl.c
 * 
 * FUNCTIONS: MSGSTR, Mrepl, docc 
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
/* static char sccsid[] = "repl.c	7.1 87/10/13 17:35:24"; */

/* repl.c - reply to a message */

#include "mh.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define DEFAULTFILTER	"/usr/lib/mh/repl.filter"

/*  */


static struct swit switches[] = {
#define	ANNOSW	0
    "annotate", 0,
#define	NANNOSW	1
    "noannotate", 0,

#define	CCSW	2
    "cc all/to/cc/me", 0,
#define	NCCSW	3
    "nocc all/to/cc/me", 0,

#define	DFOLDSW	4
    "draftfolder +folder", 0,
#define	DMSGSW	5
    "draftmessage msg", 0,
#define	NDFLDSW	6
    "nodraftfolder", 0,

#define	EDITRSW	7
    "editor editor", 0,
#define	NEDITSW	8
    "noedit", 0,

#define	FCCSW	9
    "fcc +folder", 0,

#define	FILTSW	10
    "filter filterfile", 0,
#define	FORMSW	11
    "form formfile", 0,

#define	FRMTSW	12
    "format", 5,
#define	NFRMTSW	13
    "noformat", 7,

#define	INPLSW	14
    "inplace", 0,
#define	NINPLSW	15
    "noinplace", 0,

#define	QURYSW	16
    "query", 0,
#define	NQURYSW	17
    "noquery", 0,

#define	WHATSW	18
    "whatnowproc program", 0,
#define	NWHATSW	19
    "nowhatnowproc", 0,

#define	WIDTHSW	20
    "width columns", 0,

#define	HELPSW	21
    "help", 4,

#define	FILESW	22
    "file file", -4,		/* interface from msh */

#ifdef	MHE
#define	BILDSW	23
    "build", -5,		/* interface from mhe */
#endif	MHE

    NULL, (int)NULL
};


static struct swit ccswitches[] = {
#define	CTOSW	0
    "to", 0,
#define	CCCSW	1
    "cc", 0,
#define	CMESW	2
    "me", 0,
#define	CALSW	3
    "all", 0,

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

#ifndef	ATHENA
#define	CCDFLT	1
#else	ATHENA
#define	CCDFLT	0
#endif	ATHENA

short   ccto = CCDFLT;		/* global for replsbr */
short   cccc = CCDFLT;
short   ccme = CCDFLT;
short   format = 1;
short   outputlinelen = OUTPUTLINELEN;
short   querysw = 0;

char   *fcc = NULL;		/* global for replsbr */
char   *filter = NULL;
char   *form = NULL;

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int	    i,
	    isdf = 0,
	    anot = 0,
	    inplace = 0,
#ifdef	MHE
	    buildsw = 0,
#endif	MHE
	    nedit = 0,
	    nwhat = 0;
    char   *cp,
	   *cwd,
	   *dp,
           *maildir,
           *file = NULL,
           *folder = NULL,
           *msg = NULL,
	   *dfolder = NULL,
	   *dmsg = NULL,
	   *ed = NULL,
	    drft[BUFSIZ],
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS];
    struct msgs *mp = NULL;
    struct stat st;
    FILE        *in;

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
		    (void) sprintf (buf, MSGSTR(HELPSW10, "%s: [+folder] [msg] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (0);

		case ANNOSW: 
		    anot++;
		    continue;
		case NANNOSW: 
		    anot = 0;
		    continue;

		case CCSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    docc (cp, 1);
		    continue;
		case NCCSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    docc (cp, 0);
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

		case FCCSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    dp = NULL;
		    if (*cp == '@')
			cp = dp = path (cp + 1, TSUBCWF);
		    if (fcc)
			fcc = add (", ", fcc);
		    fcc = add (cp, fcc);
		    if (dp)
			free (dp);
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
 		        filter = getcpy (m_maildir (DEFAULTFILTER));
 		    else
 		        filter = getcpy (m_maildir (cp));
		    continue;
		case FORMSW: 
		    if (!(form = *argp++) || *form == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case FRMTSW: 
		    format++;
		    continue;
		case NFRMTSW: 
		    format = 0;
		    continue;

		case INPLSW: 
		    inplace++;
		    continue;
		case NINPLSW: 
		    inplace = 0;
		    continue;

		case QURYSW: 
		    querysw++;
		    continue;
		case NQURYSW: 
		    querysw = 0;
		    continue;

		case WIDTHSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if ((outputlinelen = atoi (cp)) < 10)
			adios (NULLCP, MSGSTR(BADWIDTH, "impossible width %d"), outputlinelen); /*MSG*/
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
	    if (msg)
		adios (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/
	    else
		msg = cp;
    }

/*  */

    cwd = getcpy (pwd ());

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (file && (msg || folder))
	adios (NULLCP, MSGSTR(NOFFS, "can't mix files and folders/msgs")); /*MSG*/

try_it_again: ;
#ifndef MHE
    (void) strcpy (drft, m_draft (dfolder, dmsg, NOUSE, &isdf));
    if (stat (drft, &st) != NOTOK) {
#else	MHE
    (void) strcpy (drft, buildsw ? m_maildir ("reply")
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

    if (!msg)
	msg = "cur";
    if (!folder)
	folder = m_getfolder ();
    maildir = m_maildir (folder);

    if (chdir (maildir) == NOTOK)
	adios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/
    if (mp -> hghmsg == 0)
	adios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), folder); /*MSG*/

    if (!m_convert (mp, msg))
	done (1);
    m_setseq (mp);

    if (mp -> numsel > 1)
	adios (NULLCP, MSGSTR(ONEMSG, "only one message at a time!")); /*MSG*/

    m_replace (pfolder, folder);
    if (mp -> lowsel != mp -> curmsg)
	m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_update ();

go_to_it: ;
    msg = file ? file : getcpy (m_name (mp -> lowsel));

    if ((in = fopen (msg, "r")) == NULL)
	adios (msg, MSGSTR(NOOPEN, "unable to open %s"), msg); /*MSG*/

    replout (in, msg, drft);
    (void) fclose (in);

    if (nwhat)
	done (0);
    (void) m_whatnow (ed, nedit, NOUSE, drft, msg, 0, mp,
	    anot ? "Replied" : NULLCP, inplace, cwd);
    done (1);
}

/*  */

docc (cp, ccflag)
register char   *cp;
int     ccflag;
{
    switch (smatch (cp, ccswitches)) {
	case AMBIGSW: 
	    ambigsw (cp, ccswitches);
	    done (1);
	case UNKWNSW: 
	    adios (NULLCP, "-%scc %s unknown", ccflag ? "" : "no", cp);

	case CTOSW: 
	    ccto = ccflag;
	    break;

	case CCCSW: 
	    cccc = ccflag;
	    break;

	case CMESW: 
	    ccme = ccflag;
	    break;

	case CALSW: 
	    ccto = cccc = ccme = ccflag;
	    break;
    }
}
