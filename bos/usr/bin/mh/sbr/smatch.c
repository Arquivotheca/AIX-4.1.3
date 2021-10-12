static char sccsid[] = "@(#)08	1.3  src/bos/usr/bin/mh/sbr/smatch.c, cmdmh, bos411, 9428A410j 6/15/90 22:15:23";
/* 
 * COMPONENT_NAME: CMDMH smatch.c
 * 
 * FUNCTIONS: abs, smatch 
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


/* smatch.c - match a switch */

#include "mh.h"


#define abs(i) (i < 0 ? -i : i)


smatch(string, swp)
register char *string;
register struct swit *swp;
{
    register char  *sp,
                   *tcp;
    struct swit *tp;
    int     firstone,
            stringlen;

    firstone = UNKWNSW;

    for (stringlen = strlen (string), tp = swp; tcp = tp -> sw; tp++) {
	if (stringlen < abs (tp -> minchars))
	    continue;		/* no match */
	for (sp = string; *sp == *tcp++;) {
	    if (*sp++ == 0)
		return (tp - swp);/* exact match */
	}
	if (*sp != 0) {
	    if (*sp != ' ')
		continue;	/* no match */
	    if (*--tcp == 0)
		return (tp - swp);/* exact match */
	}
	if (firstone == UNKWNSW)
	    firstone = tp - swp;
	else
	    firstone = AMBIGSW;
    }

    return (firstone);
}
