static char sccsid[] = "@(#)58	1.4  src/bos/usr/bin/mh/sbr/getanswer.c, cmdmh, bos411, 9428A410j 2/1/93 16:47:33";
/* 
 * COMPONENT_NAME: CMDMH getanswer.c
 * 
 * FUNCTIONS: getanswer 
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


/* getanswer.c - get a yes/no answer from the user */

#include "mh.h"
#include <stdio.h>


int     getanswer (prompt)
register char   *prompt;
{
    static int  interactive = NOTOK;

    if (interactive < OK)
	interactive = isatty (fileno (stdin)) ? DONE : OK;

#ifndef _AIX
    return (interactive ? gans (prompt, anoyes) : DONE);
#else
    return (interactive ? confirm (prompt) : DONE);
#endif

}
