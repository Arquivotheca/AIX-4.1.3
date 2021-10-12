static char sccsid[] = "@(#)91	1.3  src/bos/usr/bin/mh/sbr/m_tmpfil.c, cmdmh, bos411, 9428A410j 6/15/90 22:14:21";
/* 
 * COMPONENT_NAME: CMDMH m_tmpfil.c
 * 
 * FUNCTIONS: m_tmpfil 
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


/* m_tmpfil.c - construct a temporary file */

#include "mh.h"
#include <stdio.h>


char   *m_tmpfil (template)
register char  *template;
{
    static char tmpfil[BUFSIZ];

    (void) sprintf (tmpfil, "/tmp/%sXXXXXX", template);
    (void) unlink (mktemp (tmpfil));

    return tmpfil;
}
