static char sccsid[] = "@(#)68	1.9  src/bos/usr/bin/mh/uip/packf.c, cmdmh, bos411, 9428A410j 11/9/93 09:43:00";
/* 
 * COMPONENT_NAME: CMDMH packf.c
 * 
 * FUNCTIONS: MSGSTR, Mpackf, done 
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
/* static char sccsid[] = "packf.c	7.1 87/10/13 17:31:23"; */

/* packf.c - pack a folder (used to be called "pack") */

#include "mh.h"
#include "dropsbr.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define FILESW	0
    "file name", 0,

#define	HELPSW	1
    "help", 4,

    NULL, (int)NULL
};

/*  */

extern int errno;


static int  md = NOTOK;

char   *file = NULL;

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     msgp = 0,
            fd,
            msgnum;
    char   *cp,
           *maildir,
           *msgnam,
           *folder = NULL,
            buf[100],
          **ap,
          **argp,
           *arguments[MAXARGS],
           *msgs[MAXARGS];
    char hlds[NL_TEXTMAX];
    struct msgs *mp;
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

		case FILESW: 
		    if (file)
			adios (NULLCP, MSGSTR(ONEFILE, "only one file at a time!")); /*MSG*/
		    if (!(file = *argp++) || *file == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;
	    }
	if (*cp == '+' || *cp == '@') {
	    if (folder)
		adios (NULLCP, MSGSTR(ONEFOLDER, "only one folder at a time!")); /*MSG*/
	    folder = path (cp + 1, *cp == '+' ? TFOLDER : TSUBCWF);
	}
	else
	    msgs[msgp++] = cp;
    }

/*  */

    if (!file)
	file = "./msgbox";
    file = path (file, TFILE);
    if (stat (file, &st) == NOTOK) {
	if (errno != ENOENT)
	    adios (file, MSGSTR(FILEERR, "error on file %s"), file); /*MSG*/
	sprintf(hlds, MSGSTR(CREFIL, "Create file \"%s\" <yes or no>? "), file); /*MSG*/
	if (!getanswer (hlds))
	    done (1);
    }

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
	    done (1);
    m_setseq (mp);

    if ((md = mbx_open (file, getuid (), getgid (), m_gmprot ())) == NOTOK)
	adios (file, MSGSTR(NOOPEN, "unable to open %s"), file); /*MSG*/

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED) {
	    if ((fd = open (msgnam = m_name (msgnum), 0)) == NOTOK) {
		admonish (msgnam, MSGSTR(NORMSG, "unable to read message %s"), msgnam); /*MSG*/
		break;
	    }

	    if (mbx_copy (file, md, fd, 1, NULLCP, 1) == NOTOK)
		adios (file, MSGSTR(WERROR3, "error writing to file %s"), file); /*MSG*/

	    (void) close (fd);
	}
    (void) mbx_close (file, md);

    m_replace (pfolder, folder);
    if (mp -> hghsel != mp -> curmsg)
	m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_update ();

    done (0);
}

/*  */

void done (status)
int status;
{
    (void) mbx_close (file, md);

    exit (status);
}
