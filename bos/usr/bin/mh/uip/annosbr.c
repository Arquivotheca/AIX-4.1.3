static char sccsid[] = "@(#)41        1.6  src/bos/usr/bin/mh/uip/annosbr.c, cmdmh, bos411, 9428A410j 3/28/91 14:48:40";
/* 
 * COMPONENT_NAME: CMDMH annosbr.c
 * 
 * FUNCTIONS: MSGSTR, annosbr, annotate 
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
/* static char sccsid[] = "annosbr.c	7.1 87/10/13 17:23:16"; */

/* annosbr.c - prepend annotation to messages */

#include "mh.h"
#include "tws.h"
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

static	annosbr (int, register char *, register char *, 
		 register char *, int );
extern int  errno;

long lseek ();

/*  */

annotate (file, comp, text, inplace)
register char   *file,
		*comp,
		*text;
int     inplace;
{
    int     i,
            fd;

    if ((fd = lkopen (file, 2)) == NOTOK) {
	switch (errno) {
	    case ENOENT: 
		break;

	    default: 
		admonish (file, MSGSTR(NOLOCKOPN, "unable to lock and open %s"), file); /*MSG*/
		break;
	}
	return 1;
    }

    i = annosbr (fd, file, comp, text, inplace);

    (void) lkclose (fd, file);

    return i;
}

/*  */

static	annosbr (int src, 
		 register char *file, 
		 register char *comp, 
		 register char *text, 
		 int   	inplace )
{
    int     mode,
            fd;
    register char  *cp,
                   *sp;
    char    buffer[BUFSIZ],
            tmpfil[BUFSIZ];
    struct stat st;
    register    FILE *tmp;

    mode = fstat (src, &st) != NOTOK ? (st.st_mode & 0777) : m_gmprot ();

    (void) strcpy (tmpfil, m_scratch (file, "annotate"));

    if ((tmp = fopen (tmpfil, "w")) == NULL) {
	admonish (tmpfil, MSGSTR(NOCREATE, "unable to create %s"), tmpfil); /*MSG*/
	return 1;
    }
    (void) chmod (tmpfil, mode);

    fprintf (tmp, "%s: %s\n", comp, dtimenow ());
    if (cp = text) {
	do {
	    while (*cp == ' ' || *cp == '\t')
		cp++;
	    sp = cp;
	    while (*cp && *cp++ != '\n')
		continue;
	    if (cp - sp)
		fprintf (tmp, "%s: %*.*s", comp, cp - sp, cp - sp, sp);
	} while (*cp);
	if (cp[-1] != '\n' && cp != text)
	    (void) putc ('\n', tmp);
    }
    (void) fflush (tmp);
    cpydata (src, fileno (tmp), file, tmpfil);
    (void) fclose (tmp);

    if (inplace) {
	if ((fd = open (tmpfil, 0)) == NOTOK)
	    adios (tmpfil, MSGSTR(NOOPENRR, "unable to open for re-reading %s"), tmpfil); /*MSG*/
	(void) lseek (src, 0L, 0);
	cpydata (fd, src, tmpfil, file);
	(void) close (fd);
	(void) unlink (tmpfil);
    }
    else {
	(void) strcpy (buffer, m_backup (file));
	if (rename (file, buffer) == NOTOK) {
	    admonish (buffer, MSGSTR(NORENAME, "unable to rename %s to %s"), file, buffer); /*MSG*/
	    return 1;
	}
	if (rename (tmpfil, file) == NOTOK) {
	    admonish (file, MSGSTR(NORENAME, "unable to rename %s to %s"), tmpfil, file); /*MSG*/
	    return 1;
	}
    }

    return 0;
}
