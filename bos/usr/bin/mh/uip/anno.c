static char sccsid[] = "@(#)40	1.8  src/bos/usr/bin/mh/uip/anno.c, cmdmh, bos411, 9428A410j 11/9/93 09:40:15";
/* 
 * COMPONENT_NAME: CMDMH anno.c
 * 
 * FUNCTIONS: MSGSTR, Manno, make_comp 
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
/*static char sccsid[] = "anno.c	7.1 87/10/13 17:22:56"; */

/* anno.c - annotate messages */

#include "mh.h"
#include <ctype.h>
#include <stdio.h>

#include "mh_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

/*  */

static struct swit switches[] = {
#define	COMPSW	0
    "component field", 0,

#define	INPLSW	1
    "inplace", 0,
#define	NINPLSW	2
    "noinplace", 0,

#define	TEXTSW	3
    "text body", 0,

#define	HELPSW	4
    "help", 4,

    NULL, (int)NULL
};
static make_comp (register char **);

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     inplace = 0,
            msgp = 0,
            msgnum;
    char   *cp,
           *maildir,
           *comp = NULL,
           *text = NULL,
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

		case COMPSW: 
		    if (comp)
			adios (NULLCP, MSGSTR(OONECOMP, "only one component at a time!")); /*MSG*/
		    if (!(comp = *argp++) || *comp == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
		    continue;

		case INPLSW: 
		    inplace++;
		    continue;
		case NINPLSW: 
		    inplace = 0;
		    continue;

		case TEXTSW: 
		    if (text)
			adios (NULLCP, MSGSTR(ONEBODY, "only one body at a time!")); /*MSG*/
		    if (!(text = *argp++) || *text == '-')
			adios (NULLCP, MSGSTR(NOARG, "missing argument to %s"), argp[-2]); /*MSG*/
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
	msgs[msgp++] = "cur";
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

    make_comp (&comp);

    for (msgnum = mp -> lowsel; msgnum <= mp -> hghsel; msgnum++)
	if (mp -> msgstats[msgnum] & SELECTED)
	    (void) annotate (m_name (msgnum), comp, text, inplace);

    m_replace (pfolder, folder);
    if (mp -> lowsel != mp -> curmsg)
	m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_update ();

    done (0);
}

/*  */

static make_comp (register char **ap)
{
    register char  *cp;
    char    buffer[BUFSIZ];

    if (*ap == NULL) {
	printf (MSGSTR(COMPNM, "Enter component name: ")); /*MSG*/
	(void) fflush (stdout);

	if (fgets (buffer, sizeof buffer, stdin) == NULL)
	    done (1);
	*ap = trimcpy (buffer);
    }

    if ((cp = *ap + strlen (*ap) - 1) > *ap && *cp == ':')
	*cp = (char)NULL;
    if (strlen (*ap) == 0)
	adios (NULLCP, MSGSTR(NULLCOMPNM, "null component name")); /*MSG*/
    if (**ap == '-')
	adios (NULLCP, MSGSTR(INVCOMPNM, "invalid component name %s"), *ap); /*MSG*/
    if (strlen (*ap) >= NAMESZ)
	adios (NULLCP, MSGSTR(BIGCOMPNM, "too large component name %s"), *ap); /*MSG*/

    for (cp = *ap; *cp; cp++)
	if (!isalnum (*cp) && *cp != '-')
	    adios (NULLCP, MSGSTR(INVCOMPNM, "invalid component name %s"), *ap); /*MSG*/
}
