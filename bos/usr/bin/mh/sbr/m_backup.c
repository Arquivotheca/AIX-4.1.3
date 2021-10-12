static char sccsid[] = "@(#)62	1.3  src/bos/usr/bin/mh/sbr/m_backup.c, cmdmh, bos411, 9428A410j 6/15/90 22:12:38";
/* 
 * COMPONENT_NAME: CMDMH m_backup.c
 * 
 * FUNCTIONS: m_backup 
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


/* m_backup.c - construct a backup file */

#include "mh.h"
#include <stdio.h>


char   *m_backup (file)
register char   *file;
{
    register char  *cp;
    static char buffer[BUFSIZ];

    if ((cp = r1bindex (file, '/')) == file)
	(void) sprintf (buffer, "%s%s", SBACKUP, cp);
    else
	(void) sprintf (buffer, "%.*s%s%s", cp - file, file, SBACKUP, cp);
    (void) (unlink (buffer));

    return buffer;
}
