static char sccsid[] = "@(#)98	1.13  src/bos/usr/bin/mh/uip/whatnowsbr.c, cmdmh, bos411, 9428A410j 2/16/94 10:51:01";
/* 
 * COMPONENT_NAME: CMDMH whatnowsbr.c
 * 
 * FUNCTIONS: MSGSTR, WhatNow, copyf, editfile, sendfile, sendit, 
 *            whomfile 
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
/* static char sccsid[] = "whatnowsbr.c	7.1 87/10/13 17:43:44"; */

/* whatnowsbr.c - the WhatNow shell */

#include "mh.h"
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

char *getenv();

/*  */

static struct swit whatnowswitches[] = {
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

#define	PRMPTSW	5
    "prompt string", 4,

#define	HELPSW	6
    "help", 4,

    NULL, (int)NULL
};

/*  */

static struct swit aleqs[] = {
#define	DISPSW	0
    "display [<switches>]", 0,
#define	EDITSW	1
    "edit [<editor> <switches>]", 0,
#define	LISTSW	2
    "list [<switches>]", 0,
#define	PUSHSW	3
    "push [<switches>]", 0,
#define	QUITSW	4
    "quit [-delete]", 0,
#define	REFILEOPT 5
    "refile [<switches>] +folder", 0,
#define	SENDSW	6
    "send [<switches>]", 0,
#define	WHOMSW	7
    "whom [<switches>]", 0,

    NULL, (int)NULL
};

/*  */

static	int editfile (register char **, register char **, 
		      register char *, register int, 
		      register struct msgs *, register char *, 
		      register char *);
#ifdef	BSD42
static int  copyf (register char *, register char *);
#endif  BSD42
static  sendfile (register char **, register char *, int);
static  sendit (register char  *, register char  **,
		register char  *, int     pushed);
static	int whomfile (register char **, register char *);

/*  */

/* ARGSUSED */

int	WhatNow (argc, argv)
int	argc;
char  **argv;
{
    int     isdf = 0,
	    nedit = 0,
	    use = 0;
    char   *cp,
           *dfolder = NULL,
           *dmsg = NULL,
           *ed = NULL,
           *drft = NULL,
           *msgnam = NULL,
            buf[100],
	    prompt[BUFSIZ],
          **ap,
          **argp,
           *arguments[MAXARGS],
	   promptbuf[BUFSIZ],
	   *myprompt;
    struct stat st;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    strncpy(promptbuf, MSGSTR(WHATNOW1, "\nWhat now? "), BUFSIZ);  /*MSG*/
    myprompt = promptbuf;

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
	    switch (smatch (++cp, whatnowswitches)) {
		case AMBIGSW: 
		    ambigsw (cp, whatnowswitches);
		    done (1);
		case UNKWNSW: 
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(HELPSW6, "%s [switches] [file]"), invo_name); /*MSG*/
		    help (buf, whatnowswitches);
		    done (1);

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

		case EDITRSW: 
		    if (!(ed = *argp++) || *ed == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    nedit = 0;
		    continue;
		case NEDITSW: 
		    nedit++;
		    continue;

		case PRMPTSW:
		    if (!(myprompt = *argp++) || *myprompt == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
	    }
	if (drft)
	    adios (NULLCP, MSGSTR(ONEDRAFT, "only one draft at a time!")); /*MSG*/
	else
	    drft = cp;
    }

/*  */

    if (drft == NULL && (drft = getenv ("mhdraft")) == NULL || *drft == (char)NULL)
	drft = getcpy (m_draft (dfolder, dmsg, 1, &isdf));
    msgnam = (cp = getenv ("mhaltmsg")) && *cp ? getcpy (cp) : NULLCP;
    if (ed == NULL && ((ed = getenv ("mheditor")) == NULL || *ed == (char)NULL))
	ed = NULL, nedit++;
    if ((cp = getenv ("mhuse")) && *cp)
	use = atoi (cp);
    if (!nedit
	    && editfile (&ed, NULLVP, drft, use, NULLMP, msgnam, NULLCP) < 0)
	done (1);

/*  */

    (void) sprintf (prompt, myprompt, invo_name);
    for (;;) {
	if (!(argp = getans (prompt, aleqs))) {
	    (void) unlink (LINK);
	    done (1);
	}
	switch (smatch (*argp, aleqs)) {
	    case DISPSW: 
		if (msgnam)
		    (void) showfile (++argp, msgnam);
		else
		    advise (NULLCP, MSGSTR(NOALT, "no alternate message to display")); /*MSG*/
		break;

	    case EDITSW: 
		if (*++argp)
		    ed = *argp++;
		if (editfile (&ed, argp, drft, NOUSE, NULLMP, msgnam, NULLCP)
			== NOTOK)
		    done (1);
		break;

	    case LISTSW: 
		(void) showfile (++argp, drft);
		break;

	    case WHOMSW: 
		(void) whomfile (++argp, drft);
		break;

	    case QUITSW: 
		if (*++argp && (*argp[0] == 'd' ||
			    ((*argp)[0] == '-' && (*argp)[1] == 'd'))) {
		    if (unlink (drft) == NOTOK)
			adios (drft, MSGSTR(NOUNLINK, "unable to unlink %s"), drft); /*MSG*/
		}
		else
		    if (stat (drft, &st) != NOTOK)
			advise (NULLCP, MSGSTR(DLEFT, "draft left on %s"), drft); /*MSG*/
		done (1);

	    case PUSHSW: 
		sendfile (++argp, drft, 1);
		done (1);

	    case SENDSW: 
		sendfile (++argp, drft, 0);
		break;

	    case REFILEOPT: 
		if (refile (++argp, drft) == 0)
		    done (0);
		break;

	    default: 
		advise (NULLCP, MSGSTR(WHAT, "say what?")); /*MSG*/
		break;
	}
    }
}

/*    EDIT */

static int  reedit = 0;
static char *edsave = NULL;


/* ARGSUSED */

static	int editfile (register char **ed, 
		      register char **arg, 
		      register char *file, 
		      register int  use, 
		      register struct msgs *mp,
		      register char *altmsg, 
		      register char *cwd)
{
    int     pid,
            status;
    register int    vecp;
    register char  *cp;
    char    altpath[BUFSIZ],
            linkpath[BUFSIZ],
           *vec[MAXARGS];
    struct stat st;
#ifdef BSD42
    int	    slinked;
#endif BSD42

    if (!reedit) {		/* set initial editor */
	if (*ed == NULL && (*ed = m_find ("editor")) == NULL)
	    *ed = sysed;
    }
    else
	if (!*ed) {		/* no explicit editor */
	    *ed = edsave;
	    if ((cp = r1bindex (*ed, '/')) == NULL)
		cp = *ed;
	    cp = concat (cp, "-next", NULLCP);
	    if ((cp = m_find (cp)) != NULL)
		*ed = cp;
	}

    if (altmsg) {
	if (mp == NULL || *altmsg == '/' || cwd == NULL)
	    (void) strcpy (altpath, altmsg);
	else
	    (void) sprintf (altpath, "%s/%s", mp -> foldpath, altmsg);
	if (cwd == NULL)
	    (void) strcpy (linkpath, LINK);
	else
	    (void) sprintf (linkpath, "%s/%s", cwd, LINK);
    }

    if (altmsg) {
	(void) unlink (linkpath);
#ifdef BSD42
	if (link (altpath, linkpath) == NOTOK) {
	    (void) symlink (altpath, linkpath);
	    slinked = 1;
	}
	else
	    slinked = 0;
#else  not BSD42
	(void) link (altpath, linkpath);
#endif not BSD42
    }

    m_update ();
    (void) fflush (stdout);

    switch (pid = vfork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    status = NOTOK;
	    break;

	case OK: 
	    if (cwd)
		(void) chdir (cwd);
	    if (altmsg) {
		if (mp)
		    (void) putenv ("mhfolder", mp -> foldpath);
		(void) putenv ("editalt", altpath);
	    }

	    vecp = 0;
	    vec[vecp++] = r1bindex (*ed, '/');
	    if (arg)
		while (*arg)
		    vec[vecp++] = *arg++;
	    vec[vecp++] = file;
	    vec[vecp] = NULL;

	    execvp (*ed, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (*ed);
	    _exit (-1);

	default: 
	    if (status = pidwait (pid, NOTOK)) {
		if (((status & 0xff00) != 0xff00)
			&& (!reedit || (status & 0x00ff)))
		    if (!use && (status & 0xff00)) {
			(void) unlink (file);
			advise (NULLCP, MSGSTR(PROBS, "problems with edit--%s deleted"), file); /*MSG*/
		    }
		    else
			advise (NULLCP, MSGSTR(PROBS2, "problems with edit--%s preserved"), file); /*MSG*/
		status = -2;
		break;
	    }

	    reedit++;
#ifdef BSD42
	    if (altmsg
		    && mp
		    && (!mp -> msgflags & READONLY)
		    && (slinked
		           ? lstat (linkpath, &st) != NOTOK
				&& (st.st_mode & S_IFMT) == S_IFREG
				&& copyf (linkpath, altpath) == NOTOK
		           : stat (linkpath, &st) != NOTOK
				&& st.st_nlink == 1
				&& (unlink (altpath) == NOTOK
					|| link (linkpath, altpath) == NOTOK)))
		advise (linkpath, MSGSTR(NOUPDATE, "unable to update %s from %s"), altmsg, linkpath); /*MSG*/
#else	not BSD42
	    if (altmsg
		    && mp
		    && (!mp -> msgflags & READONLY)
		    && stat (linkpath, &st) != NOTOK
		    && st.st_nlink == 1
		    && (unlink (altpath) == NOTOK
			|| link (linkpath, altpath) == NOTOK))
		advise (linkpath, MSGSTR(NOUPDATE, "unable to update %s from %s"), altmsg, linkpath); /*MSG*/
#endif	not BSD42
    }

    edsave = getcpy (*ed);
    *ed = NULL;
    if (altmsg)
	(void) unlink (linkpath);

    return status;
}

/*  */

#ifdef	BSD42
static int  copyf (register char   *ifile,
		   register char   *ofile )
{
    register int    i;
    int     in,
            out;
    char    buffer[BUFSIZ];

    if ((in = open (ifile, 0)) == NOTOK)
	return NOTOK;
    if ((out = open (ofile, 1)) == NOTOK || ftruncate (out, 0) == NOTOK) {
	if (out != NOTOK) {
	    admonish (ofile, MSGSTR(NOTRUNC, "unable to truncate %s"), ofile); /*MSG*/
	    (void) close (out);
	}
	(void) close (in);
	return NOTOK;
    }

    while ((i = read (in, buffer, sizeof buffer)) > OK)
	if (write (out, buffer, i) != i) {
	    advise (ofile, MSGSTR(DAMAGE, "may have damaged %s"), ofile); /*MSG*/
	    i = NOTOK;
	    break;
	}

    (void) close (in);
    (void) close (out);

    return i;
}
#endif	BSD42

/*    SEND */

static  sendfile (register char **arg,
		  register char *file,
		  int     pushsw     )
{
    register int    child_id,
                    i,
                    vecp;
    char *cp,
	 *sp,
	 *vec[MAXARGS];

    if (strcmp (sp = r1bindex (sendproc, '/'), "send") == 0) {
	cp = invo_name;
	sendit (invo_name = sp, arg, file, pushsw);
	invo_name = cp;
	return;
    }

    m_update ();
    (void) fflush (stdout);

    for (i = 0; (child_id = vfork ()) == NOTOK && i < 5; i++)
	sleep (5);
    switch (child_id) {
	case NOTOK: 
	    advise (NULLCP, MSGSTR(DIRECT, "unable to fork, so sending directly...")); /*MSG*/
	case OK: 
	    vecp = 0;
	    vec[vecp++] = invo_name;
	    if (pushsw)
		vec[vecp++] = "-push";
	    if (arg)
		while (*arg)
		    vec[vecp++] = *arg++;
	    vec[vecp++] = file;
	    vec[vecp] = NULL;

	    execvp (sendproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (sendproc);
	    _exit (-1);

	default: 
	    if (pidwait (child_id, OK) == 0)
		done (0);
	    return;
    }
}

/*  */

static struct swit  sendswitches[] = {
#define	ALIASW	0
    "alias aliasfile", 0,

#define	DEBUGSW	1
    "debug", -5,

#define	ENCRSW	2
    "encrypt",
#ifndef	TMA
    -7,
#else	TMA
    0,
#endif	TMA
#define	NENCRSW	3
    "noencrypt",
#ifndef	TMA
    -9,
#else	TMA
    0,
#endif	TMA

#define	FILTSW	4
    "filter filterfile", 0,
#define	NFILTSW	5
    "nofilter", 0,

#define	FRMTSW	6
    "format", 0,
#define	NFRMTSW	7
    "noformat", 0,

#define	FORWSW	8
    "forward", 0,
#define	NFORWSW	9
    "noforward", 0,

#define	MSGDSW	10
    "msgid", 0,
#define	NMSGDSW	11
    "nomsgid", 0,

#define	SPSHSW	12
    "push", 0,
#define	NSPSHSW	13
    "nopush", 0,

#define	UNIQSW	14
    "unique", -6,
#define	NUNIQSW	15
    "nounique", -8,

#define	VERBSW	16
    "verbose", 0,
#define	NVERBSW	17
    "noverbose", 0,

#define	WATCSW	18
    "watch", 0,
#define	NWATCSW	19
    "nowatch", 0,

#define	WIDTHSW	20
    "width columns", 0,

#define	SHELPSW	21
    "help", 4,

#define	MAILSW	22
    "mail", -4,
#define	SAMLSW	23
    "saml", -4,
#define	SSNDSW	24
    "send", -4,
#define	SOMLSW	25
    "soml", -4,

#define	CLIESW	26
    "client host", -6,
#define	SERVSW	27
    "server host", -6,
#define	SNOOPSW	28
    "snoop", -5,

#define SDRFSW 29
    "draftfolder +folder", -6,
#define SDRMSW 30
    "draftmessage msg", -6,
#define SNDRFSW 31
    "nodraftfolder", -3,

    NULL, (int)NULL
};

/*  */

extern int debugsw;		/* from sendsbr.c */
extern int forwsw;
extern int inplace;
extern int pushsw;
extern int unique;

extern char *altmsg;		/*  .. */
extern char *annotext;
extern char *distfile;

/*  */

static  sendit (register char  *sp,
		register char  **arg,
		register char  *file,
		int     pushed)
{
    int     distsw = 0,
	    vecp = 1;
    char   *cp,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *vec[MAXARGS];
    struct stat st;
#ifdef	UCI
    FILE   *fp;
#endif	UCI

    if (arg)
	(void) copyip (arg, vec);
    if ((cp = m_find (sp)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    if (arg)
	(void) copyip (vec, ap);
    argp = arguments;

    debugsw = 0, forwsw = 1, inplace = 0, unique = 0;
    altmsg = annotext = distfile = NULL;
    vec[vecp++] = "-library";
    vec[vecp++] = getcpy (m_maildir (""));

/*  */

    while (cp = *argp++) {
	if (*cp == '-')
	    switch (smatch (++cp, sendswitches)) {
		case AMBIGSW: 
		    ambigsw (cp, sendswitches);
		    return;
		case UNKWNSW: 
		    advise (NULLCP, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		    return;
		case SHELPSW: 
		    (void) sprintf (buf, MSGSTR(SHELPSW1, "%s [switches]"), sp); /*MSG*/
		    help (buf, sendswitches);
		    return;

		case SPSHSW: 
		    pushed++;
		    continue;
		case NSPSHSW: 
		    pushed = 0;
		    continue;

		case UNIQSW: 
		    unique++;
		    continue;
		case NUNIQSW: 
		    unique = 0;
		    continue;
		case FORWSW: 
		    forwsw++;
		    continue;
		case NFORWSW: 
		    forwsw = 0;
		    continue;

		case DEBUGSW: 
		    debugsw++;	/* fall */
		case NFILTSW: 
		case FRMTSW: 
		case NFRMTSW: 
		case MSGDSW: 
		case NMSGDSW: 
		case VERBSW: 
		case NVERBSW: 
		case WATCSW: 
		case NWATCSW: 
		case MAILSW: 
		case SAMLSW: 
		case SSNDSW: 
		case SOMLSW: 
		case ENCRSW: 
		case NENCRSW: 
		case SNOOPSW: 
		    vec[vecp++] = --cp;
		    continue;

		case ALIASW: 
		case FILTSW: 
		case WIDTHSW: 
		case CLIESW: 
		case SERVSW: 
		    vec[vecp++] = --cp;
		    if (!(cp = *argp++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
			return;
		    }
		    vec[vecp++] = cp;
		    continue;

		case SDRFSW: 
		case SDRMSW: 
		    if (!(cp = *argp++) || *cp == '-') {
			advise (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
			return;
		    }
		case SNDRFSW: 
		    continue;
	    }
	advise (NULLCP, MSGSTR(USAGE4, "usage: %s [switches]"), sp); /*MSG*/
	return;
    }

/*  */

#ifdef	TMA
    if ((cp = getenv ("KDS")) == NULL || *cp == NULL)
	if ((cp = m_find ("kdsproc")) && *cp)
	    (void) putenv ("KDS", cp);
    if ((cp = getenv ("TMADB")) == NULL || *cp == NULL)
	if ((cp = m_find ("tmadb")) && *cp)
	    (void) putenv ("TMADB", m_maildir (cp));
#endif	TMA

    if ((cp = getenv ("SIGNATURE")) == NULL || *cp == (char)NULL)
	if ((cp = m_find ("signature")) && *cp) {
	}
#ifdef	UCI
	else {
	    (void) sprintf (buf, "%s/.signature", mypath);
	    if ((fp = fopen (buf, "r")) != NULL
		&& fgets (buf, sizeof buf, fp) != NULL) {
		    (void) fclose (fp);
		    if (cp = index (buf, '\n'))
			*cp = NULL;
/*		    (void) putenv ("SIGNATURE", buf); */
	    }
	}
#endif	UCI
    /*
     * Translate "\n" to a "real" \n, plus 5 spaces to line it up.
     * This allows multiple line signatures.
     */
    
    if (cp != NULL) {
      char *pcp, *pncp, *ncp;

      pcp = cp;			/* Start at the beginning */

      ncp = (char *) malloc (strlen(cp)*4 + 1);	/* Space for new string */
      pncp = ncp;

      while (*pcp != (char)NULL) {
	if (*pcp == '\\' && *(pcp+1) == 'n') { /* If "\n", replace with */
	  *pncp++ = '\n';	               /* <cr> and 6 spaces.    */
	  strcpy(pncp, "      ");
	  pncp += 6;
	  pcp += 2;
	}
	else
	  *pncp++ = *pcp++;	/* else, just copy the character. */
      }
      *pncp = (char)NULL;
      
      (void) putenv ("SIGNATURE", ncp);
      free(ncp);		/* free allocated space. */
    }

    if ((annotext = getenv ("mhannotate")) == NULL || *annotext == (char)NULL)
	annotext = NULL;
    if ((altmsg = getenv ("mhaltmsg")) == NULL || *altmsg == (char)NULL)
	altmsg = NULL;
    if (annotext && ((cp = getenv ("mhinplace")) != NULL && *cp != (char)NULL))
	inplace = atoi (cp);

    if ((cp = getenv ("mhdist"))
	    && *cp
	    && (distsw = atoi (cp))
	    && altmsg) {
	vec[vecp++] = "-dist";
	distfile = getcpy (m_scratch (altmsg, invo_name));
	if (link (altmsg, distfile) == NOTOK)
	    adios (distfile, MSGSTR(NOLINK, "unable to link %s to %s"), altmsg, distfile); /*MSG*/
    }
    else
	distfile = NULL;

    if (altmsg == NULL || stat (altmsg, &st) == NOTOK)
	st.st_mtime = 0, st.st_dev = 0, st.st_ino = 0;
    if (pushsw = pushed)
	push ();

    vec[0] = r1bindex (postproc, '/');
    closefds (3);

    if (sendsbr (vec, vecp, file, &st) == OK)
	done (0);
}

/*    WHOM */

static	int whomfile (register char **arg,
		      register char *file )
{
    int     pid;
    register int    vecp;
    char   *vec[MAXARGS];

    m_update ();
    (void) fflush (stdout);

    switch (pid = vfork ()) {
	case NOTOK: 
	    advise ("fork", MSGSTR(NOFORK, "unable to fork")); /*MSG*/
	    return 1;

	case OK: 
	    vecp = 0;
	    vec[vecp++] = r1bindex (whomproc, '/');
	    vec[vecp++] = file;
	    if (arg)
		while (*arg)
		    vec[vecp++] = *arg++;
	    vec[vecp] = NULL;

	    execvp (whomproc, vec);
	    fprintf (stderr, MSGSTR(NOEXEC, "unable to exec ")); /*MSG*/
	    perror (whomproc);
	    _exit (-1);		/* NOTREACHED */

	default: 
	    return (pidwait (pid, NOTOK) & 0377 ? 1 : 0);
    }
}
