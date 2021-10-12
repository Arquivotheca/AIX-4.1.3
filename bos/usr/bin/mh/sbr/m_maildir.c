static char sccsid[] = "@(#)75  1.4  src/bos/usr/bin/mh/sbr/m_maildir.c, cmdmh, bos411, 9428A410j 10/10/90 11:32:24";
/* 
 * COMPONENT_NAME: CMDMH m_maildir.c
 * 
 * FUNCTIONS: exmaildir, m_maildir, m_mailpath 
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


/* m_maildir.c - get the path for the mail directory */

#include "mh.h"
#include <stdio.h>

#define	CWD	"./"
#define	NCWD	(sizeof CWD - 1)
#define	DOT	"."
#define	DOTDOT	".."
#define	PWD	"../"
#define	NPWD	(sizeof PWD - 1)


static char mailfold[BUFSIZ];

static char   *exmaildir ();


char   *m_maildir (folder)
register char   *folder;
{
    register char  *cp,
                   *ep;

    if ((cp = exmaildir (folder))
	    && (ep = cp + strlen (cp) - 1) > cp
	    && *ep == '/')
	*ep = (int)NULL;

    return cp;
}

/*  */

char   *m_mailpath (folder)
register char   *folder;
{
    register char  *cp;
    char    maildir[BUFSIZ];

    if (*folder == '/'
	    || strncmp (folder, CWD, NCWD) == 0
	    || strcmp (folder, DOT) == 0
	    || strcmp (folder, DOTDOT) == 0
	    || strncmp (folder, PWD, NPWD) == 0)
	cp = path (folder, TFOLDER);
    else {
	(void) strcpy (maildir, mailfold);/* preserve... */
	cp = getcpy (m_maildir (folder));
	(void) strcpy (mailfold, maildir);
    }

    return cp;
}

/*  */

static char *exmaildir (folder)
register char   *folder;
{
    register char  *cp,
                   *pp;

    if (folder == NULL)
	folder = m_getfolder ();
    if (*folder == '/'
	    || strncmp (folder, CWD, NCWD) == 0
	    || strcmp (folder, DOT) == 0
	    || strcmp (folder, DOTDOT) == 0
	    || strncmp (folder, PWD, NPWD) == 0) {
	(void) strcpy (mailfold, folder);
	return mailfold;
    }

    cp = mailfold;
    if ((pp = m_find ("path")) != NULL && *pp) {
	if (*pp != '/') {
	    (void) sprintf (cp, "%s/", mypath);
	    cp += strlen (cp);
	}
	cp = copy (pp, cp);
    }
    else
	cp = copy (path ("./", TFOLDER), cp);
    if (cp[-1] != '/')
	*cp++ = '/';
    (void) strcpy (cp, folder);

    return mailfold;
}
