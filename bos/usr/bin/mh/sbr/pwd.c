static char sccsid[] = "@(#)03        1.6  src/bos/usr/bin/mh/sbr/pwd.c, cmdmh, bos411, 9428A410j 3/27/91 17:52:44";
/* 
 * COMPONENT_NAME: CMDMH pwd.c
 * 
 * FUNCTIONS: MSGSTR, getwd, pwd 
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
/* static char sccsid[] = "pwd.c	7.1 87/10/13 17:16:40"; */

/* pwd.c - return the current working directory */

#include "mh.h"
#include <stdio.h>
#ifndef	BSD42
#include <sys/types.h>
#include <sys/fullstat.h>
#ifndef SYS5
#include <ndir.h>
#else	SYS5
#include <sys/dir.h>
#endif	SYS5
#endif	BSD42

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

#define	MAXPATHLEN	1024

static char curwd[MAXPATHLEN];



char   *pwd () {
    register char  *cp;

#ifndef	BSD42
    if (getwd (curwd) == NOTOK) {
	admonish (NULL, MSGSTR(NOWDIR, "unable to determine working directory")); /*MSG*/
#else	BSD42
    if (getwd (curwd) == (int)NULL) {
	admonish (NULLCP, MSGSTR(NOWDIR2, "unable to determine working directory: %s"), curwd); /*MSG*/
#endif	BSD42
	if (mypath == NULL
		|| *mypath == (int)NULL
		|| ((void) strcpy (curwd, mypath), chdir (curwd)) == NOTOK) {
	    (void) strcpy (curwd, "/");
	    (void) chdir (curwd);
	}
	return curwd;
    }

    if ((cp = curwd + strlen (curwd) - 1) > curwd && *cp == '/')
	*cp = (int)NULL;

    return curwd;
}

/*  */

#ifndef	BSD42
/* getwd() - get the current working directory */

/* Algorithm from several sources, -ljobs, pwd.c, etc., etc. */

getwd (cwd)
register char   *cwd;
{
    int     found;
    char    tmp1[BUFSIZ],
            tmp2[BUFSIZ];
    struct fullstat st1,
                st2,
                root;
    register struct direct *dp;
    register    DIR * dd;

    (void) strcpy (cwd, "/");
    (void) fullstat ("/", FL_STAT_REV, &root);

    for (;;) {
	if ((dd = opendir ("..")) == NULL)
	    return NOTOK;
	if (fullstat (".", FL_STAT_REV, &st2) == NOTOK || 
	    fullstat ("..", FL_STAT_REV, &st1) == NOTOK)
	    goto out;
	if (st2.st_ino == root.st_ino && st2.st_dev == root.st_dev) {
	    closedir (dd);
	    return chdir (cwd);
	}

	if (st2.st_ino == st1.st_ino && st2.st_dev == st1.st_dev) {
	    closedir (dd);
	    (void) chdir ("/");
	    if ((dd = opendir (".")) == NULL)
		return NOTOK;
	    if (fullstat (".", FL_STAT_REV, &st1) < 0)
		goto out;
	    if (st2.st_dev != st1.st_dev)
		while (dp = readdir (dd)) {
		    if (fullstat (dp -> d_name, FL_STAT_REV, &st1) == NOTOK)
			goto out;
		    if (st2.st_dev == st1.st_dev) {
			(void) sprintf (tmp1, "%s%s", dp -> d_name, cwd);
			(void) strcpy (cwd + 1, tmp1);
			closedir (dd);
			return (chdir (cwd));
		    }
		}
	    else {
		closedir (dd);
		return (chdir (cwd));
	    }
	}

	found = 0;
	while (dp = readdir (dd)) {
	    (void) sprintf (tmp2, "../%s", dp -> d_name);
	    if (fullstat (tmp2, FL_STAT_REV, &st1) != NOTOK
		    && st1.st_ino == st2.st_ino
		    && st1.st_dev == st2.st_dev) {
		closedir (dd);
		found++;
		(void) chdir ("..");
		(void) sprintf (tmp1, "%s%s", dp -> d_name, cwd);
		(void) strcpy (cwd + 1, tmp1);
		break;
	    }
	}
	if (!found)
	    goto out;
    }

out: ;
    closedir (dd);
    return NOTOK;
}
#endif	not BSD42
