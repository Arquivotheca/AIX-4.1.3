static char sccsid[] = "@(#)59	1.8  src/bos/usr/bin/mh/uip/mark.c, cmdmh, bos411, 9428A410j 11/9/93 09:42:00";
/* 
 * COMPONENT_NAME: CMDMH mark.c
 * 
 * FUNCTIONS: MSGSTR, Mmark 
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
/* static char sccsid[] = "mark.c	7.1 87/10/13 17:28:16"; */

/* mark.c - mark messages */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	ADDSW	0
    "add", 0,
#define	DELSW	1
    "delete", 0,
#define	LSTSW	2
    "list", 0,

#define	SEQSW	3
    "sequence name", 0,

#define	PUBLSW	4
    "public", 0,
#define	NPUBLSW	5
    "nopublic", 0,

#define	ZEROSW	6
    "zero", 0,
#define	NZEROSW	7
    "nozero", 0,

#define	HELPSW	8
    "help", 4,

#define	DEBUGSW	9
    "debug", -5,

    NULL, (int)NULL
};

/*  */

/* ARGSUSED */

main (argc, argv)
int	argc;
char  **argv;
{
    int     addsw = 0,
            deletesw = 0,
            debugsw = 0,
            listsw = 0,
	    publicsw = -1,
            zerosw = 0,
            seqp = 0,
            msgp = 0,
            i,
            msgnum;
    char   *cp,
           *maildir,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *seqs[NATTRS + 1],
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
		    adios (NULLCP, MSGSTR(NOTKNOWN, "-%s unknown\n"), cp); /*MSG*/
		case HELPSW: 
		    (void) sprintf (buf, MSGSTR(MHELPSW, "%s [+folder] [msgs] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case ADDSW: 
		    addsw++;
		    deletesw = listsw = 0;
		    continue;
		case DELSW: 
		    deletesw++;
		    addsw = listsw = 0;
		    continue;
		case LSTSW: 
		    listsw++;
		    addsw = deletesw = 0;
		    continue;

		case SEQSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    if (seqp < NATTRS)
			seqs[seqp++] = cp;
		    else
			adios (NULLCP, MSGSTR(ONLYSEQS, "only %d sequences allowed!"), NATTRS); /*MSG*/
		    continue;

		case PUBLSW: 
		    publicsw = 1;
		    continue;
		case NPUBLSW: 
		    publicsw = 0;
		    continue;

		case DEBUGSW: 
		    debugsw++;
		    continue;

		case ZEROSW: 
		    zerosw++;
		    continue;
		case NZEROSW: 
		    zerosw = 0;
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

    if (!addsw && !deletesw && !listsw)
	if (seqp)
	    addsw++;
	else
	    listsw++;

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!msgp)
	msgs[msgp++] = listsw ? "all" :"cur";
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

    if (publicsw == -1)
	publicsw = mp -> msgflags & READONLY ? 0 : 1;
    if (publicsw && (mp -> msgflags & READONLY))
	adios (NULLCP, MSGSTR(READONLY1, "folder %s is read-only, so -public not allowed"), folder); /*MSG*/

/*  */

    if (debugsw) {
	printf ("invo_name=%s mypath=%s defpath=%s\n",
		invo_name, mypath, defpath);
	printf ("ctxpath=%s context flags=%s\n",
		ctxpath, sprintb (buf, (unsigned) ctxflags, DBITS));
	printf ("foldpath=%s flags=%s\n",
		mp -> foldpath,
		sprintb (buf, (unsigned) mp -> msgflags, FBITS));
	printf ("lowmsg=%d hghmsg=%d nummsg=%d curmsg=%d\n",
		mp -> lowmsg, mp -> hghmsg, mp -> nummsg, mp -> curmsg);
	printf ("lowsel=%d hghsel=%d numsel=%d\n",
		mp -> lowsel, mp -> hghsel, mp -> numsel);
#ifndef	MTR
	printf ("lowoff=%d hghoff=%d\n",
		mp -> lowoff, mp -> hghoff);
#else	MTR
	printf ("lowoff=%d hghoff=%d msgbase=0x%x msgstats=0x%x\n",
		mp -> lowoff, mp -> hghoff, mp -> msgbase, mp -> msgstats);
#endif	MTR
    }

    if (seqp == 0 && (addsw || deletesw))
	adios (NULLCP, MSGSTR(NEEDONE, "-%s requires at least one -sequence argument"), addsw ? "add" : "delete"); /*MSG*/
    seqs[seqp] = NULL;

    if (addsw)
	for (seqp = 0; seqs[seqp]; seqp++) {
	    if (zerosw && !m_seqnew (mp, seqs[seqp], publicsw))
		done (1);
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    if (!m_seqadd (mp, seqs[seqp], msgnum, publicsw))
			done (1);
	}

    if (deletesw)
	for (seqp = 0; seqs[seqp]; seqp++) {
	    if (zerosw)
		for (msgnum = mp -> lowmsg; msgnum <= mp -> hghmsg; msgnum++)
		    if (mp -> msgstats[msgnum] & EXISTS)
			if (!m_seqadd (mp, seqs[seqp], msgnum, publicsw))
			    done (1);
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    if (!m_seqdel (mp, seqs[seqp], msgnum))
			done (1);
	}

    if (listsw) {
	int bits = FFATTRSLOT;

	if (seqp == 0)
	    for (i = 0; mp -> msgattrs[i]; i++)
		printf ("%s%s: %s\n", mp -> msgattrs[i],
			mp -> attrstats & (1 << (bits + i))
			    ? " (private)" : "",
			m_seq (mp, mp -> msgattrs[i]));
	else
	    for (seqp = 0; seqs[seqp]; seqp++)
		printf ("%s: %s\n", seqs[seqp], m_seq (mp, seqs[seqp]));

	if (debugsw)
	    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
		if (mp -> msgstats[msgnum] & SELECTED)
		    printf ("%*d: %s\n", DMAXFOLDER, msgnum,
			sprintb (buf, (unsigned) mp -> msgstats[msgnum],
			    m_seqbits (mp)));
    }

    m_replace (pfolder, folder);
    m_sync (mp);
    m_update ();

    done (0);
}
