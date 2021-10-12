static char sccsid[] = "@(#)17	1.7  src/bos/usr/bin/mh/sbr/vfgets.c, cmdmh, bos411, 9428A410j 3/27/91 17:52:56";
/* 
 * COMPONENT_NAME: CMDMH vfgets.c
 * 
 * FUNCTIONS: MSGSTR, vfgets 
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
/* static char sccsid[] = "vfgets.c	7.1 87/10/13 17:20:11"; */

/* vfgets.c - virtual fgets */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


#define	QUOTE	'\\'


int	vfgets (in, bp)
register FILE *in;
register char  **bp;
{
    register int    toggle;
    register char  *cp,
                   *dp,
                   *ep,
		   *fp;
    static int  len = 0;
    static char *pp = NULL;

    if (pp == NULL)
	if ((pp = (char *)malloc ((unsigned) (len = BUFSIZ))) == NULL)
	    adios (NULLCP, MSGSTR(NOSTOR, "unable to allocate string storage")); /*MSG*/

    for (ep = (cp = pp) + len - 1;;) {
	if (fgets (cp, ep - cp + 1, in) == NULL) {
	    if (cp != pp) {
		*bp = pp;
		return OK;
	    }
	    return (ferror (in) ? NOTOK : DONE);
	}

	if ((dp = cp + strlen (cp) - 2) < cp || *dp != QUOTE) {
wrong_guess: ;
	    if (cp > ++dp)
		adios (NULLCP, MSGSTR(BIGLOSE2, "vfgets() botch -- you lose big")); /*MSG*/
	    if (*dp == '\n') {
		*bp = pp;
		return OK;
	    }
	    else
		cp = ++dp;
	}
	else {
	    for (fp = dp - 1, toggle = 0; fp >= cp; fp--)
		if (*fp != QUOTE)
		    break;
		else
		    toggle = !toggle;
	    if (toggle)
		goto wrong_guess;
	    if (*++dp == '\n')
		*--dp = (char)NULL, cp = dp;
	    else
		cp = ++dp;
	}

	if (cp >= ep) {
	    register int curlen = cp - pp;

	    if ((dp = (char *)realloc (pp, (unsigned) (len += BUFSIZ))) == NULL)
		adios (NULLCP, MSGSTR(NOSTOR, "unable to allocate string storage")); /*MSG*/
	    else
		cp = dp + curlen, ep = (pp = dp) + len - 1;
	}
    }
}
