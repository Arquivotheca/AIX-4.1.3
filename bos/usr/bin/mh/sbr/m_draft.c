static char sccsid[] = "@(#)66	1.7  src/bos/usr/bin/mh/sbr/m_draft.c, cmdmh, bos411, 9428A410j 2/1/93 16:48:21";
/* 
 * COMPONENT_NAME: CMDMH m_draft.c
 * 
 * FUNCTIONS: MSGSTR, m_draft 
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
/* static char sccsid[] = "m_draft.c	7.1 87/10/13 17:07:48"; */

/* m_draft.c - construct the draft name */

#include "mh.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


extern int errno;


char   *m_draft (folder, msg, use, isdf)
register char  *folder,
               *msg;
register int    use,
               *isdf;
{
    register char  *cp;
    register struct msgs   *mp;
    struct stat st;
    static char buffer[BUFSIZ];
    char hlds[NL_TEXTMAX];

    if (*isdf == NOTOK || folder == NULL || *folder == (int)NULL) {
	if (*isdf == NOTOK || (cp = m_find ("Draft-Folder")) == NULL) {
	    *isdf = 0;
	    return m_maildir (msg && *msg ? msg : draft);
	}
	else
	    folder = path (*cp == '+' || *cp == '@' ? cp + 1 : cp,
		    *cp != '@' ? TFOLDER : TSUBCWF);
    }
    *isdf = 1;
    
    (void) chdir (m_maildir (""));
    (void) strcpy (buffer, m_maildir (folder));
    if (stat (buffer, &st) == NOTOK) {
	if (errno != ENOENT)
	    adios (buffer, MSGSTR(ERRF, "error on folder %s"), buffer); /*MSG*/
	sprintf(hlds, MSGSTR(CREFOL, "Create folder \"%s\" <yes or no>? "), buffer); /*MSG*/
	if (!getanswer (hlds))
	    done (0);
	if (!makedir (buffer))
	    adios (NULLCP, MSGSTR(NOCREAT, "unable to create folder %s"), buffer); /*MSG*/
    }

    if (chdir (buffer) == NOTOK)
	adios (buffer, MSGSTR(NOCHANGE, "unable to change directory to %s"), buffer); /*MSG*/
    if (!(mp = m_gmsg (folder)))
	adios (NULLCP, MSGSTR(NOREADF, "unable to read folder %s"), folder); /*MSG*/

    if ((mp = m_remsg (mp, 0, MAXFOLDER)) == NULL)
	adios (NULLCP, MSGSTR(NOAFSTOR, "unable to allocate folder storage")); /*MSG*/
    mp -> msgflags |= MHPATH;

    if (!m_convert (mp, msg && *msg ? msg : use ? "cur" : "new"))
	done (1);
    m_setseq (mp);
    if (mp -> numsel > 1)
	adios (NULLCP, MSGSTR(ONE, "only one message draft at a time!")); /*MSG*/

    (void) sprintf (buffer, "%s/%s", mp -> foldpath, m_name (mp -> lowsel));
    cp = buffer;

    m_setcur (mp, mp -> lowsel);
    m_sync (mp);
    m_fmsg (mp);

    return cp;
}
