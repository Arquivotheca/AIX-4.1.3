static char sccsid[] = "@(#)46	1.8  src/bos/usr/bin/mh/uip/comp.c, cmdmh, bos411, 9428A410j 11/9/93 09:40:45";
/* 
 * COMPONENT_NAME: CMDMH comp.c
 * 
 * FUNCTIONS: MSGSTR, Mcomp 
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
/* static char sccsid[] = "comp.c	7.1 87/10/13 17:24:34"; */

/* comp.c - compose a message */

#include "mh.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	DFOLDSW	0
    "draftfolder +folder", 0,
#define	DMSGSW	1
    "draftmessage msg", 0,
#define	NDFLDSW	2
    "nodraftfolder", 0,

#define	EDITRSW	3
    "editor editor", 0,
#define	NEDITSW	4
    "noedit", 0,

#define	FILESW	5
    "file file", 0,
#define	FORMSW	6
    "form formfile", 0,

#define	USESW	7
    "use", 0,
#define	NUSESW	8
    "nouse", 0,

#define	WHATSW	9
    "whatnowproc program", 0,
#define	NWHATSW	10
    "nowhatnowproc", 0,

#define	HELPSW	11
    "help", 4,


    NULL, (int)NULL
};

/*  */

static struct swit aqrunl[] = {
#define	NOSW	0
    "quit", 0,
#define	YESW	1
    "replace", 0,
#define	USELSW	2
    "use", 0,
#define	LISTDSW	3
    "list", 0,
#define	REFILSW	4
    "refile +folder", 0,
#define NEWSW	5
    "new", 0,

    NULL, (int)NULL
};


static struct swit aqrul[] = {
    "quit", 0,
    "replace", 0,
    "use", 0,
    "list", 0,
    "refile", 0,

    NULL, (int)NULL
};

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     use = NOUSE,
            nedit = 0,
	    nwhat = 0,
            i,
            in,
	    isdf = 0,
            out;
    char   *cp,
           *cwd,
           *maildir,
           *dfolder = NULL,
           *ed = NULL,
           *file = NULL,
           *form = NULL,
           *folder = NULL,
           *msg = NULL,
            buf[BUFSIZ],
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

		case FORMSW: 
		    if (!(form = *argp++) || *form == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case USESW: 
		    use++;
		    continue;
		case NUSESW: 
		    use = NOUSE;
		    continue;

		case FILESW: 	/* compatibility */
		    if (file)
			adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
		    if (!(file = *argp++) || *file == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
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
		    if (file)
			adios (NULLCP, MSGSTR(ONEDMSG, "only one draft message at a time!")); /*MSG*/
		    if (!(file = *argp++) || *file == '-')
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

    if ((dfolder || m_find ("Draft-Folder")) && !folder && msg && !file)
	file = msg, msg = NULL;
    if (form && (folder || msg))
	    adios (NULLCP, MSGSTR(NOMIX, "can't mix forms and folders/msgs")); /*MSG*/

    if (folder || msg) {
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

	if ((in = open (form = getcpy (m_name (mp -> lowsel)), 0)) == NOTOK)
	    adios (form, MSGSTR(NOOPENM, "unable to open message %s"), form); /*MSG*/ 
    }
    else
	if (form) {
	    if ((in = open (libpath (form), 0)) == NOTOK)
		adios (form, MSGSTR(NOOPENFL, "unable to open form file %s"), form); /*MSG*/
	}
	else {
	    if ((in = open (libpath (components), 0)) == NOTOK)
		adios (components, MSGSTR(NOOPENDEF, "unable to open default components file %s"), components); /*MSG*/
	    form = components;
	}

/*  */

try_it_again: ;
    (void) strcpy (drft, m_draft (dfolder, file, use, &isdf));
    if ((out = open (drft, 0)) != NOTOK) {
	i = fdcompare (in, out);
	(void) close (out);
	if (use || i)
	    goto edit_it;

	if (stat (drft, &st) == NOTOK)
	    adios (drft, MSGSTR(NOSTAT, "unable to stat %s"), drft); /*MSG*/
	printf (MSGSTR(DRAFT, "Draft \"%s\" exists (%ld bytes)."), drft, st.st_size); /*MSG*/
	for (i = LISTDSW; i != YESW;) {
	    if (!(argp = getans (MSGSTR(DISPOS,"\nDisposition? "),
				isdf ? aqrunl : aqrul))) /*MSG*/
		done (1);
	    switch (i = smatch (*argp, isdf ? aqrunl : aqrul)) {
		case NOSW: 
		    done (0);
		case NEWSW: 
		    file = NULL;
		    use = NOUSE;
		    goto try_it_again;
		case YESW: 
		    break;
		case USELSW:
		    use++;
		    goto edit_it;
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
    else
	if (use)
	    adios (drft, MSGSTR(NOOPEN, "unable to open %s"), drft); /*MSG*/

    if ((out = creat (drft, m_gmprot ())) == NOTOK)
	adios (drft, MSGSTR(NOCREATE, "unable to create %s"), drft); /*MSG*/
    cpydata (in, out, form, drft);
    (void) close (in);
    (void) close (out);

edit_it: ;
    m_update ();

    if (nwhat)
	done (0);
    (void) m_whatnow (ed, nedit, use, drft, NULLCP, 0, NULLMP, NULLCP, 0, cwd);
    done (1);
}
