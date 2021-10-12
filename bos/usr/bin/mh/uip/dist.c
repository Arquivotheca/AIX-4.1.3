static char sccsid[] = "@(#)49	1.8  src/bos/usr/bin/mh/uip/dist.c, cmdmh, bos411, 9428A410j 11/9/93 09:41:06";
/* 
 * COMPONENT_NAME: CMDMH dist.c
 * 
 * FUNCTIONS: MSGSTR, Mdist 
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
/* static char sccsid[] = "dist.c	7.1 87/10/13 17:25:33"; */

/* dist.c - re-distribute a message */

#include "mh.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

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

#define	FORMSW	7
    "form formfile", 0,

#define	INPLSW	8
    "inplace", 0,
#define	NINPLSW	9
    "noinplace", 0,

#define	WHATSW	10
    "whatnowproc program", 0,
#define	NWHATSW	11
    "nowhatnowproc", 0,

#define	HELPSW	12
    "help", 4,

#define	FILESW	13
    "file file", -4,		/* interface from msh */

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

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     anot = 0,
            inplace = 0,
	    nedit = 0,
	    nwhat = 0,
            i,
            in,
	    isdf = 0,
            out;
    char   *cp,
	   *cwd,
           *maildir,
	   *msgnam,
           *dfolder = NULL,
           *dmsg = NULL,
           *ed = NULL,
           *file = NULL,
           *folder = NULL,
           *form = NULL,
           *msg = NULL,
            buf[100],
	    drft[BUFSIZ],
          **ap,
          **argp,
           *arguments[MAXARGS];
    struct msgs *mp = NULL;
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
		    (void) sprintf (buf, MSGSTR(UMSG, "%s [+folder] [msg] [switches]"), invo_name); /*MSG*/
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
		case FORMSW: 
		    if (!(form = *argp++) || *form == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case INPLSW: 
		    inplace++;
		    continue;
		case NINPLSW: 
		    inplace = 0;
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

    if (form) {
	if ((in = open (libpath (form), 0)) == NOTOK)
	    adios (form, MSGSTR(NOOPENFL, "unable to open form file %s"), form); /*MSG*/
    }
    else {
	if ((in = open (libpath (distcomps), 0)) == NOTOK)
	    adios (distcomps, MSGSTR(NOOPENDEF, "unable to open default components file %s"), distcomps); /*MSG*/
	form = distcomps;
    }

try_it_again: ;
    (void) strcpy (drft, m_draft (dfolder, dmsg, NOUSE, &isdf));
    if (stat (drft, &st) != NOTOK) {
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
    if ((out = creat (drft, m_gmprot ())) == NOTOK)
	adios (drft, MSGSTR(NOCREATE, "unable to create %s"), drft); /*MSG*/

    cpydata (in, out, form, drft);
    (void) close (in);
    (void) close (out);

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

go_to_it: ;
    if ((in = open (msgnam = file ? file : getcpy (m_name (mp -> lowsel)), 0))
	    == NOTOK)
	adios (msgnam, MSGSTR(NOOPENM, "unable to open message %s"), msgnam); /*MSG*/

    if (!file) {
	m_replace (pfolder, folder);
	if (mp -> lowsel != mp -> curmsg)
	    m_setcur (mp, mp -> lowsel);
	m_sync (mp);
	m_update ();
    }

    if (nwhat)
	done (0);
    (void) m_whatnow (ed, nedit, NOUSE, drft, msgnam, 1, mp,
	anot ? "Resent" : NULLCP, inplace, cwd);
    done (1);
}
