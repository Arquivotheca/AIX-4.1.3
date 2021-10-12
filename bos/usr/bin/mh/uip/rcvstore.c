static char sccsid[] = "@(#)74	1.8  src/bos/usr/bin/mh/uip/rcvstore.c, cmdmh, bos411, 9428A410j 11/9/93 09:43:51";
/* 
 * COMPONENT_NAME: CMDMH rcvstore.c
 * 
 * FUNCTIONS: MSGSTR, Mrcvstore 
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
/* static char sccsid[] = "rcvstore.c	7.1 87/10/13 17:34:00"; */

/* rcvstore.c - incorporate new mail asynchronously
		originally from Julian Onions */

#include "mh.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define CRETSW	0
    "create",	0,
#define NCRETSW	1
    "nocreate", 0,

#define PUBSW	2
    "public",	0,
#define NPUBSW	3
    "nopublic",  0,

#define SEQSW	4
    "sequence name", 0,

#define ZEROSW  5
    "zero",	0,
#define NZEROSW 6
    "nozero",	0,

#define HELPSW  7
    "help", 4,

    NULL, (int)NULL
};

/*  */

extern int  errno;

/*  */

/* ARGSUSED */

main (argc, argv)
int	argc;
char   *argv[];
{
    int     publicsw = -1,
            zerosw = 0,
            msgnum,
            create = 1,
            fd,
            seqp = 0;
    char   *cp,
           *maildir,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *seqs[NATTRS];
    struct msgs *mp;
    struct stat st;

    setlocale(LC_ALL,"");
    catd = catopen(MF_MH, NL_CAT_LOCALE);

    invo_name = r1bindex (argv[0], '/');
    mts_init (invo_name);
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
		    (void) sprintf (buf, MSGSTR(UMESG, "%s [+folder] [switches]"), invo_name); /*MSG*/
		    help (buf, switches);
		    done (1);

		case SEQSW: 
		    if (!(cp = *argp++) || *cp == '-')
			adios (NULLCP, MSGSTR(NONAMEARG, "missing argument name to %s"), argp[-2]); /*MSG*/
		    if (seqp < NATTRS)
			seqs[seqp++] = cp;
		    else
			adios (NULLCP, MSGSTR(ONLYSEQS, "only %d sequences allowed!"), NATTRS); /*MSG*/
		    continue;
		case PUBSW: 
		    publicsw = 1;
		    continue;
		case NPUBSW: 
		    publicsw = 0;
		    continue;
		case ZEROSW: 
		    zerosw++;
		    continue;
		case NZEROSW: 
		    zerosw = 0;
		    continue;

		case CRETSW: 
		    create++;
		    continue;
		case NCRETSW: 
		    create = 0;
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    else
		folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    adios (NULLCP, MSGSTR(USAGEM, "usage: %s [+folder] [switches]"), invo_name); /*MSG*/
    }

/*  */

    if (!m_find ("path"))
	free (path ("./", TFOLDER));
    if (!folder)
	folder = defalt;
    maildir = m_maildir (folder);

    if (stat (maildir, &st) == NOTOK) {
	if (errno != ENOENT)
	    adios (maildir, MSGSTR(ERRF, "error on folder %s"), maildir); /*MSG*/
	if (!create)
	    adios (NULLCP, MSGSTR(NOEXIST2, "folder %s doesn't exist"), maildir); /*MSG*/
	if (!makedir (maildir))
	    adios (NULLCP, MSGSTR(NOCREAT, "unable to create folder %s"), maildir); /*MSG*/
    }

    if (chdir (maildir) == NOTOK)
	adios (maildir, MSGSTR(NOCHANGE, "unable to change directory to %s"), maildir); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/

    (void) signal (SIGHUP, SIG_IGN);
    (void) signal (SIGINT, SIG_IGN);
    (void) signal (SIGQUIT, SIG_IGN);
    (void) signal (SIGTERM, SIG_IGN);

/*  */

    if ((fd = creat (cp = m_scratch ("", invo_name), m_gmprot ())) == NOTOK)
	adios (cp, MSGSTR(NOCREATE, "unable to create %s"), cp); /*MSG*/

    cpydata (fileno (stdin), fd, "standard input", cp);

    if (fstat (fd, &st) == NOTOK) {
	(void) unlink (cp);
	adios (cp, MSGSTR(NOFSTAT, "unable to fstat %s"), cp); /*MSG*/
    }
    (void) close (fd);
    if (st.st_size == 0) {
	(void) unlink (cp);
	advise (NULLCP, MSGSTR(EMPTY5, "empty file")); /*MSG*/
	done (0);
    }

    msgnum = mp -> hghmsg;
    do {
	msgnum++, mp -> hghmsg++;
	if (msgnum > mp -> hghoff)
	    if ((mp = m_remsg (mp, 0, mp -> hghoff + MAXFOLDER)) == NULL)
		adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/

	mp -> msgstats[msgnum] |= EXISTS | UNSEEN;
	errno = 0;
    } while (link (cp, m_name (msgnum)) == NOTOK && errno == EEXIST);

    (void) unlink (cp);
    if (errno != 0)
	adios (NULLCP, MSGSTR(NOFILEMSG, "can't file message %d"), msgnum); /*MSG*/

    if (mp -> lowmsg == 0)
	mp -> lowmsg = msgnum;
    mp -> msgflags |= SEQMOD;

    seqs[seqp] = NULL;
    for (seqp = 0; seqs[seqp]; seqp++) {
	if (zerosw && !m_seqnew (mp, seqs[seqp], publicsw))
	    done (1);
	if (!m_seqadd (mp, seqs[seqp], msgnum, publicsw))
	    done (1);
    }

    m_setvis (mp, 0);
    m_sync (mp);
    m_update ();

    done (0);
}
