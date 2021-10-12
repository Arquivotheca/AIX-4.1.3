static char sccsid[] = "@(#)81	1.3  src/bos/usr/bin/mh/sbr/m_scratch.c, cmdmh, bos411, 9428A410j 6/15/90 22:13:46";
/* 
 * COMPONENT_NAME: CMDMH m_scratch.c
 * 
 * FUNCTIONS: m_scratch 
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


/* m_scratch.c - construct a scratch file */

#include "mh.h"
#include <stdio.h>


char   *m_scratch (file, template)
register char  *file,
               *template;
{
    register char  *cp;
    static char buffer[BUFSIZ],
		tmpfil[BUFSIZ];

    (void) sprintf (tmpfil, "%sXXXXXX", template);
    (void) mktemp (tmpfil);
    if ((cp = r1bindex (file, '/')) == file)
	(void) strcpy (buffer, tmpfil);
    else
	(void) sprintf (buffer, "%.*s%s", cp - file, file, tmpfil);
    (void) unlink (buffer);

    return buffer;
}
