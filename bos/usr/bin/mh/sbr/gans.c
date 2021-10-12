static char sccsid[] = "@(#)56	1.6  src/bos/usr/bin/mh/sbr/gans.c, cmdmh, bos411, 9428A410j 2/1/93 16:46:27";
/* 
 * COMPONENT_NAME: CMDMH gans.c
 * 
 * FUNCTIONS: MSGSTR, gans, confirm 
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
/* static char sccsid[] = "gans.c	7.1 87/10/13 17:06:04"; */

/* gans.c - get an answer from the user */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


gans (prompt, ansp)
register char *prompt;
register struct swit *ansp;
{
    register int    i;
    register char  *cp;
    register struct swit   *ap;
    char    ansbuf[BUFSIZ];
    char hlds[NL_TEXTMAX];

    strcpy(hlds, prompt);	/* save it in case we have to call catgets */
    for (;;) {
	printf ("%s", hlds);
	(void) fflush (stdout);
	cp = ansbuf;
	while ((i = getchar ()) != '\n') {
	    if (i == EOF)
		return 0;
	    if (cp < &ansbuf[sizeof ansbuf - 1]) {
		if (i >= 'A' && i <= 'Z')
		    i += 'a' - 'A';
		*cp++ = i;
	    }
	}
	*cp = 0;
	if (ansbuf[0] == '?' || cp == ansbuf) {
	    printf (MSGSTR(OPTIONS, "Options are:\n")); /*MSG*/
	    for (ap = ansp; ap -> sw; ap++)
		printf ("  %s\n", ap -> sw);
	    continue;
	}
	if ((i = smatch (ansbuf, ansp)) < 0) {
	    printf ("%s: %s.\n", ansbuf, i == -1 ? MSGSTR(UNKNOWN, "unknown") : MSGSTR(AMBIGUOUS, "ambiguous")); /*MSG*/
	    continue;
	}
	return i;
    }
}

int
confirm (prompt)
	char *prompt;
{
	char line[BUFSIZ];
	int i;

	for (;;) {
		printf("%s",prompt);
		(void) fflush(stdout);
		(void) gets(line);
		if ((i = rpmatch(line)) != (-1))
			return (i);
	}
}
