static char sccsid[] = "@(#)02	1.5  src/bos/usr/bin/mh/sbr/putenv.c, cmdmh, bos411, 9428A410j 10/10/90 16:18:12";
/* 
 * COMPONENT_NAME: CMDMH putenv.c
 * 
 * FUNCTIONS: nvmatch, putenv, unputenv 
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


/* putenv.c - (un) set an envariable */

#include "mh.h"
#include <stdio.h>


extern  char **environ;
static nvmatch (register char *, register char *);


int     putenv (name, value)
register char  *name,
               *value;
{
    register int    i;
    register char **ep,
                  **nep,
                   *cp;

    if ((cp = (char *)malloc ((unsigned) (strlen (name) + strlen (value) + 2)))
	    == NULL)
	return 1;
    (void) sprintf (cp, "%s=%s", name, value);

    for (ep = environ, i = 0; *ep; ep++, i++)
	if (nvmatch (name, *ep)) {
	    *ep = cp;
	    return 0;
	}

    if ((nep = (char **) malloc ((unsigned) ((i + 2) * sizeof *nep))) == NULL)
	return 1;
    for (ep = environ, i = 0; *ep; nep[i++] = *ep++)
	continue;
    nep[i++] = cp;
    nep[i] = NULL;
    environ = nep;
    return 0;
}


int	unputenv (name)
char   *name;
{
    char  **ep,
          **nep;

    for (ep = environ; *ep; ep++)
	if (nvmatch (name, *ep))
	    break;
    if (*ep == NULL)
	return 1;

    for (nep = ep + 1; *nep; nep++)
	continue;
    *ep = *--nep;
    *nep = NULL;
    return 0;
}

/*  */

static nvmatch (register char *s1, register char *s2)
{
    while (*s1 == *s2++)
	if (*s1++ == '=')
	    return 1;

    return (*s1 == '\0' && *--s2 == '=');
}
