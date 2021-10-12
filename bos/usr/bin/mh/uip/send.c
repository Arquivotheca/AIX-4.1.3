static char sccsid[] = "@(#)85	1.15  src/bos/usr/bin/mh/uip/send.c, cmdmh, bos411, 9428A410j 1/3/94 08:06:03";
/* 
 * COMPONENT_NAME: CMDMH send.c
 * 
 * FUNCTIONS: MSGSTR, Msend 
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
/* static char sccsid[] = "send.c	7.1 87/10/13 17:38:07"; */

/* send.c - send a composed message */

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

static struct swit switches[] = {
#define	ALIASW	0
    "alias aliasfile", 0,

#define	DEBUGSW	1
    "debug", -5,

#define	DRAFTSW	2
    "draft", 0,

#define	DFOLDSW	3
    "draftfolder +folder", 6,
#define	DMSGSW	4
    "draftmessage msg", 6,
#define	NDFLDSW	5
    "nodraftfolder", 0,

#define	ENCRSW	6
    "encrypt",
#ifndef	TMA
	-7,
#else	TMA
	0,
#endif	TMA
#define	NENCRSW	7
    "noencrypt",
#ifndef	TMA
	-9,
#else	TMA
	0,
#endif	TMA

#define	FILTSW	8
    "filter filterfile", 0,
#define	NFILTSW	9
    "nofilter", 0,

#define	FRMTSW	10
    "format", 0,
#define	NFRMTSW	11
    "noformat", 0,

#define	FORWSW	12
    "forward", 0,
#define	NFORWSW	13
    "noforward", 0,

#define	MSGDSW	14
    "msgid", 0,
#define	NMSGDSW	15
    "nomsgid", 0,

#define	PUSHSW	16
    "push", 0,
#define	NPUSHSW	17
    "nopush", 0,

#define	UNIQSW	18
    "unique", -6,
#define	NUNIQSW	19
    "nounique", -8,

#define	VERBSW	20
    "verbose", 0,
#define	NVERBSW	21
    "noverbose", 0,

#define	WATCSW	22
    "watch", 0,
#define	NWATCSW	23
    "nowatch", 0,

#define	WIDTHSW	24
    "width columns", -24,

#define	HELPSW	25
    "help", 4,

#define	MAILSW	26
    "mail", -4,
#define	SAMLSW	27
    "saml", -4,
#define	SENDSW	28
    "send", -4,
#define	SOMLSW	29
    "soml", -4,

#define	CLIESW	30
    "client host", -6,
#define	SERVSW	31
    "server host", -6,
#define	SNOOPSW	32
    "snoop", -5,

    NULL, (int)NULL
};

#ifndef _AIX
static struct swit anyl[] = {
#define	NOSW	0
    "no", 0,
#define	YESW	1
    "yes", 0,
#define	LISTDSW	2
    "list", 0,

    NULL, (int)NULL
};
#else
#define NOSW    0
#define YESW    1
#define LISTDSW 2
#define NUMOPTIONS 4
static struct swit anyl[NUMOPTIONS];
#endif _AIX


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

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
    int     msgp = 0,
	    distsw = 0,
            vecp = 1,
            isdf = 0,
            msgnum,
            status;
    char   *cp,
           *dfolder = NULL,
           *maildir,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS],
           *vec[MAXARGS];
    char hlds[NL_TEXTMAX];
    struct msgs *mp;
    struct stat st;
#ifdef	UCI
    FILE   *fp;
#endif	UCI
#ifdef _AIX
    int i;
#endif

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

#ifdef _AIX	/* build option list */
    for (i=0; i<NUMOPTIONS; i++) {
	anyl[i].minchars = (int)NULL;
    }
    anyl[YESW].sw = MSGSTR(YES, "yes");
    anyl[NOSW].sw = MSGSTR(NO, "no");
    anyl[LISTDSW].sw = MSGSTR(LIST2, "list");
    anyl[NUMOPTIONS-1].sw = (char *)NULL;
#endif

    invo_name = r1bindex (argv[0], '/');
    if ((cp = m_find (invo_name)) != NULL) {
	ap = brkstring (cp = getcpy (cp), " ", "\n");
	ap = copyip (ap, arguments);
    }
    else
	ap = arguments;
    (void) copyip (argv + 1, ap);
    argp = arguments;

    vec[vecp++] = "-library";
    vec[vecp++] = getcpy (m_maildir (""));

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
		    (void) sprintf (buf, MSGSTR(HELPSW11, "%s [file] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);	/* thanks, phyl */

		case DRAFTSW: 
		    msgs[msgp++] = draft;
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
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    msgs[msgp++] = cp;
		    continue;
		case NDFLDSW: 
		    dfolder = NULL;
		    isdf = NOTOK;
		    continue;

		case PUSHSW: 
		    pushsw++;
		    continue;
		case NPUSHSW: 
		    pushsw = 0;
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
		case SENDSW: 
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
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    vec[vecp++] = cp;
		    continue;
	    }
	else
	    msgs[msgp++] = cp;
    }

/*  */

    if (dfolder == NULL) {
	if (msgp == 0) {
#ifdef	WHATNOW
	    if ((cp = getenv ("mhdraft")) && *cp) {
		msgs[msgp++] = cp;
		goto go_to_it;
	    }
#endif	WHATNOW
	    msgs[msgp++] = getcpy (m_draft (NULLCP, NULLCP, 1, &isdf));
	    if (stat (msgs[0], &st) == NOTOK)
		adios (msgs[0], MSGSTR(NOSTAT2, "unable to stat draft file %s"), msgs[0]); /*MSG*/
	    sprintf(hlds, MSGSTR(USE, "Use \"%s\"? "), msgs[0]); /*MSG*/
	    for (status = LISTDSW; status != YESW;) {
		if (!(argp = getans (hlds, anyl)))
		    done (1);
		switch (status = smatch (*argp, anyl)) {
		    case NOSW: 
			done (0);
		    case YESW: 
			break;
		    case LISTDSW: 
			(void) showfile (++argp, msgs[0]);
			break;
		    default:
			advise (NULLCP, MSGSTR(WHAT, "say what?")); /*MSG*/
			break;
		}
	    }
	}
	else
	    for (msgnum = 0; msgnum < msgp; msgnum++)
		msgs[msgnum] = getcpy (m_maildir (msgs[msgnum]));
    }
    else {
	if (!m_find ("path"))
	    free (path ("./", TFOLDER));

	if (!msgp)
	    msgs[msgp++] = "cur";
	maildir = m_maildir (dfolder);

	if (chdir (maildir) == NOTOK)
	    adios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
	if (!(mp = m_gmsg (dfolder)))
	    adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), dfolder); /*MSG*/
	if (mp -> hghmsg == 0)
	    adios (NULLCP, MSGSTR(NOMESF, "no messages in %s"), dfolder); /*MSG*/

	for (msgnum = 0; msgnum < msgp; msgnum++)
	    if (!m_convert (mp, msgs[msgnum]))
		done (1);
	m_setseq (mp);

	for (msgp = 0, msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	    if (mp -> msgstats[msgnum] & SELECTED) {
		msgs[msgp++] = getcpy (m_name (msgnum));
#ifdef	notdef
		mp -> msgstats[msgnum] |= DELETED;
#endif	notdef
		mp -> msgstats[msgnum] &= ~EXISTS;
	    }
	mp -> msgflags |= SEQMOD;

	m_sync (mp);
    }
#ifdef	WHATNOW
go_to_it: ;
#endif	WHATNOW

/*  */

#ifdef	TMA
    if ((cp = getenv ("KDS")) == NULL || *cp == (char)NULL)
	if ((cp = m_find ("kdsproc")) && *cp)
	    (void) putenv ("KDS", cp);
    if ((cp = getenv ("TMADB")) == NULL || *cp == (char)NULL)
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

    for (msgnum = 0; msgnum < msgp; msgnum++)
	if (stat (msgs[msgnum], &st) == NOTOK)
	    adios (msgs[msgnum], MSGSTR(NOSTAT2, "unable to stat draft file %s"), msgs[msgnum]); /*MSG*/

    if ((annotext = getenv ("mhannotate")) == NULL || *annotext == (char)NULL)
	annotext = NULL;
    if (annotext
	    && ((altmsg = getenv ("mhaltmsg")) == NULL || *altmsg == (char)NULL))
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
    if (pushsw)
	push ();

    status = 0;
    vec[0] = r1bindex (postproc, '/');
    closefds (3);

    for (msgnum = 0; msgnum < msgp; msgnum++)
	switch (sendsbr (vec, vecp, msgs[msgnum], &st)) {
	    case DONE: 
		done (++status);

	    case NOTOK: 
		status++;	/* fall */
	    case OK:
		break;
	}

    m_update ();

    done (status);
}
