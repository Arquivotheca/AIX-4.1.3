static char sccsid[] = "@(#)90	1.8  src/bos/usr/bin/mh/uip/sortm.c, cmdmh, bos411, 9428A410j 11/9/93 09:45:09";
/* 
 * COMPONENT_NAME: CMDMH sortm.c
 * 
 * FUNCTIONS: MSGSTR, Msortm, file_dates, gettws, msgsort, read_dates 
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
/* static char sccsid[] = "sortm.c	7.1 87/10/13 17:39:43"; */

/* sortm.c - sort messages in a folder by date/time */

#include "mh.h"
#include "tws.h"
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	DATESW	0
    "datefield field", 0,

#define	VERBSW	1
    "verbose", 0,
#define NVERBSW	2
    "noverbose", 0,

#define	HELPSW	3
    "help", 4,

    NULL, (int)NULL
};

/*  */

struct smsg {
    int     s_msg;
    struct tws  s_tws;
};

static struct smsg *smsgs;


static int     read_dates(), msgsort ();
static file_dates();
static struct tws *gettws ();


long	time ();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     verbosw = 0,
            msgp = 0,
	    i,
            msgnum;
    char   *cp,
           *maildir,
           *datesw = NULL,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
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
		    adios (NULLCP, MSGSTR(UNKWNSW1, "-%s unknown"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(MHELPSW, "%s [+folder] [msgs] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case DATESW: 
		    if (datesw)
			adios (NULLCP, MSGSTR(ONEDFLD, "only one date field at a time!")); /*MSG*/
		    if (!(datesw = *argp++) || *datesw == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case VERBSW: 
		    verbosw++;
		    continue;
		case NVERBSW: 
		    verbosw = 0;
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

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!msgp)
	msgs[msgp++] = "all";
    if (!datesw)
	datesw = "Date";
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

    if ((i = read_dates (mp, datesw)) <= 0)
	adios (NULLCP, MSGSTR(NOSORT, "no messages to sort")); /*MSG*/
    qsort ((char *) smsgs, i, sizeof *smsgs, msgsort);
    file_dates (mp, verbosw);

    m_replace (pfolder, folder);
    m_sync (mp);
    m_update ();

    done (0);
}

/*  */

static int  read_dates (mp, datesw)
register struct  msgs *mp;
register char   *datesw;
{
    int     msgnum;
    struct tws  tb;
    register struct smsg *s;
    register struct tws *tw;

    twscopy (&tb, dtwstime ());

    smsgs = (struct smsg   *)
		calloc ((unsigned) (mp -> hghsel - mp -> lowsel + 2),
			sizeof *smsgs);
    if (smsgs == NULL)
	adios (NULLCP, MSGSTR(NOASSTOR, "unable to allocate sort storage")); /*MSG*/

    s = smsgs;
    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++) {
	tw = NULL;
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if ((tw = gettws (datesw, msgnum)) == NULL)
		tw = msgnum != mp -> lowsel ? &((s - 1) -> s_tws) : &tb;
	}
	else
	    if (mp -> msgstats[msgnum] & EXISTS)
		tw = &tb;

	if (tw) {
	    s -> s_msg = msgnum;
	    twscopy (&s -> s_tws, tw);
	    s++;
	}
    }

    s -> s_msg = 0;
    return (s - smsgs);
}

/*  */

static struct tws  *gettws (datesw, msg)
register char   *datesw;
int     msg;
{
    int     compnum,
            state;
    register char  *hp,
                   *msgnam;
    char    buf[BUFSIZ],
            nam[NAMESZ];
    register struct tws *tw;
    register    FILE *in;

    if ((in = fopen (msgnam = m_name (msg), "r")) == NULL) {
	admonish (msgnam, MSGSTR(NORMSG, "unable to read message %s"), msgnam); /*MSG*/
	return NULL;
    }

/*  */

    for (compnum = 1, state = FLD, hp = NULL;;) {
	switch (state = m_getfld (state, nam, buf, sizeof buf, in)) {
	    case FLD: 
	    case FLDEOF: 
	    case FLDPLUS: 
		compnum++;
		if (hp != NULL)
		    free (hp), hp = NULL;
		hp = add (buf, NULLCP);
		while (state == FLDPLUS) {
		    state = m_getfld (state, nam, buf, sizeof buf, in);
		    hp = add (buf, hp);
		}
		if (uleq (nam, datesw))
		    break;
		if (state != FLDEOF)
		    continue;

	    case BODY: 
	    case BODYEOF: 
	    case FILEEOF: 
		admonish (NULLCP, MSGSTR(NOFIELD, "no %s field in message %d"), datesw, msg); /*MSG*/

	    case LENERR: 
	    case FMTERR: 
		if (state == LENERR || state == FMTERR)
		    admonish (NULLCP,
			    MSGSTR(FRMTERR2, "format error in message %d(header #%d)"), msg, compnum); /*MSG*/
		if (hp != NULL)
		    free (hp);
		(void) fclose (in);
		return NULL;

	    default: 
		adios (NULLCP, MSGSTR(INTERR2, "internal error -- you lose")); /*MSG*/
	}
	break;
    }

    if ((tw = dparsetime (hp)) == NULL)
	admonish (NULLCP, MSGSTR(NOPARSE, "unable to parse %s field in message %d"), datesw, msg); /*MSG*/

    if (hp != NULL)
	free (hp);
    (void) fclose (in);
    return tw;
}

/*  */

static int  msgsort (a, b)
register struct smsg *a,
		     *b;
{
    return twsort (&a -> s_tws, &b -> s_tws);
}

/*  */

static  file_dates (mp, verbosw)
register struct  msgs *mp;
int     verbosw;
{
    register int    i,
                    j,
                    k;
    short   stats;
    char    f1[BUFSIZ],
            f2[BUFSIZ],
            tmpfil[BUFSIZ];

    (void) strcpy (tmpfil, m_scratch ("", invo_name));

    for (i = 0; j = smsgs[i++].s_msg;)
	if (i != j) {
	    (void) strcpy (f1, m_name (i));
	    (void) strcpy (f2, m_name (j));
	    if (mp -> msgstats[i] & EXISTS) {
		if (verbosw)
		    printf (MSGSTR(SWAP, "swap messages %s and %s\n"), f2, f1); /*MSG*/

		if (rename (f1, tmpfil) == NOTOK) {
		    admonish (tmpfil, MSGSTR(NORENAME2, "unable to rename %s to %s "), f1, tmpfil); /*MSG*/
		    continue;
		}

		if (rename (f2, f1) == NOTOK) {
		    admonish (f1, MSGSTR(NORENAME, "unable to rename %s to %s"), f2, f1); /*MSG*/
		    continue;
		}

		if (rename (tmpfil, f2) == NOTOK) {
		    admonish (f2, MSGSTR(NORENAME, "unable to rename %s to %s"), tmpfil, f2); /*MSG*/
		    continue;
		}

		for (k = i; smsgs[k].s_msg; k++)
		    if (smsgs[k].s_msg == i) {
			smsgs[k].s_msg = j;
			break;
		    }
	    }
	    else {
		if (verbosw)
		    printf (MSGSTR(CHANGES, "message %s becomes message %s\n"), f2, f1); /*MSG*/

		if (rename (f2, f1) == NOTOK) {
		    admonish (f1, MSGSTR(NORENAME2, "unable to rename %s to %s "), f2, f1); /*MSG*/
		    continue;
		}
	    }

	    smsgs[i - 1].s_msg = i;
	    stats = mp -> msgstats[i];
	    mp -> msgstats[i] = mp -> msgstats[j];
	    mp -> msgstats[j] = stats;
	    if (mp -> curmsg == j)
		m_setcur (mp, i);
	    mp -> msgflags |= SEQMOD;
	}
}
