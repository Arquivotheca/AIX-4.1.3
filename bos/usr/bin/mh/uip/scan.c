static char sccsid[] = "@(#)83	1.8  src/bos/usr/bin/mh/uip/scan.c, cmdmh, bos411, 9428A410j 11/9/93 09:44:34";
/* 
 * COMPONENT_NAME: CMDMH scan.c
 * 
 * FUNCTIONS: MSGSTR, Mscan 
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
/* static char sccsid[] = "scan.c	7.1 87/10/13 17:37:20"; */

/* scan.c - display a one-line "scan" listing */

#include "mh.h"
#include "formatsbr.h"
#include "scansbr.h"
#include "tws.h"
#include <errno.h>
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	CLRSW	0
    "clear", 0,
#define	NCLRSW	1
    "noclear", 0,

#define	FORMSW	2
    "form formatfile", 0,
#define	FMTSW	3
    "format string", 5,

#define	HEADSW	4
    "header", 0,
#define	NHEADSW	5
    "noheader", 0,

#define	WIDSW	6
    "width columns", 0,

#ifdef	BERK
#define	REVSW	7
    "reverse", 0,
#define	NREVSW	8
    "noreverse", 0,

#define	HELPSW	9
#else	not BERK
#define	HELPSW	7
#endif	not BERK
    "help", 4,

    NULL, (int)NULL
};

/*  */

extern int errno;


void	clear_screen ();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char   *argv[];
{
     int    clearflag = 0,
	    hdrflag = 0,
	    width = 0,
            msgp = 0,
	    ontty,
	    state,
            msgnum;
#ifdef  BERK
    register int    revflag = 0,
		    firstlim,
		    lastlim,
		    incr;
#endif  BERK
    char   *cp,
           *maildir,
           *folder = NULL,
	   *form = NULL,
	   *format = NULL,
            buf[100],
          **ap,
          **argp,
           *nfs,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
    struct msgs *mp;
    FILE * in;

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

		case CLRSW: 
		    clearflag++;
		    continue;
		case NCLRSW: 
		    clearflag = 0;
		    continue;

		case FORMSW: 
		    if (!(form = *argp++) || *form == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    format = NULL;
		    continue;
		case FMTSW: 
		    if (!(format = *argp++) || *format == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    form = NULL;
		    continue;

		case HEADSW: 
		    hdrflag++;
		    continue;
		case NHEADSW: 
		    hdrflag = 0;
		    continue;

		case WIDSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    width = atoi (cp);
		    continue;
#ifdef  BERK
		case REVSW:
		    revflag++;
		    continue;
		case NREVSW:
		    revflag = 0;
		    continue;
#endif  BERK
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
    if (!msgp)
	msgs[msgp++] = "all";
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
	    done(1);
    m_setseq (mp);

    m_replace (pfolder, folder);
    m_sync (mp);
    m_update ();

    nfs = new_fs (form, format, FORMAT);

    ontty = isatty (fileno (stdout));

/*  */

#ifdef  BERK
    if (revflag) {
	firstlim = mp -> hghsel;
	lastlim = mp -> lowsel;
	incr = -1;
    }
    else {
	firstlim = mp -> lowsel;
	lastlim = mp -> hghsel;
	incr = 1;
    }

    for (msgnum = firstlim;
	    (revflag ? msgnum >= lastlim : msgnum <= lastlim);
	    msgnum += incr)
#else	not BERK
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
#endif  not BERK
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if ((in = fopen (cp = m_name (msgnum), "r")) == NULL) {
		if (errno != EACCES)
		    admonish (cp, MSGSTR(NOOPENM, "unable to open message %s"), cp); /*MSG*/
		else
		    printf (MSGSTR(UNREADABLE2, "%*d  unreadable\n"), DMAXFOLDER, msgnum); /*MSG*/
		free (cp);
		continue;
	    }

	    if (hdrflag)
		printf (MSGSTR(FOLDER, "Folder %-32s%s\n\n"), folder, dtimenow ()); /*MSG*/
	    switch (state = scan (in, msgnum, 0, nfs, width,
			msgnum == mp -> curmsg,
			hdrflag, 0L, 1)) {
		case SCNMSG: 
		case SCNERR: 
		    break;

		default: 
		    adios (NULLCP, MSGSTR(SCAN1, "scan() botch (%d)"), state); /*MSG*/

		case SCNEOF: 
		    printf (MSGSTR(EMPTY4, "%*d  empty\n"), DMAXFOLDER, msgnum); /*MSG*/
		    break;
	    }
	    hdrflag = 0;
	    (void) fclose (in);
	    if (ontty)
		(void) fflush (stdout);
	}

/*  */

    if (clearflag)
	clear_screen ();

    done (0);
}
