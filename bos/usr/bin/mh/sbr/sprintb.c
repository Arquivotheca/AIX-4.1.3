static char sccsid[] = "@(#)10	1.4  src/bos/usr/bin/mh/sbr/sprintb.c, cmdmh, bos411, 9428A410j 10/10/90 16:18:40";
/* 
 * COMPONENT_NAME: CMDMH sprintb.c
 * 
 * FUNCTIONS: sprintb 
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


/* sprintb.c - sprintf a %b string */

#include "mh.h"
#include <stdio.h>


char   *sprintb (buffer, v, bits)
register char  *buffer,
               *bits;
register unsigned   v;
{
    register int    i,
                    j;
    register char   c,
                   *bp;

    (void) sprintf (buffer, bits && *bits == 010 ? "0%o" : "0x%x", v);
    bp = buffer + strlen (buffer);

    if (bits && *++bits) {
	j = 0;
	*bp++ = '<';
	while (i = *bits++)
	    if (v & (1 << (i - 1))) {
		if (j++)
		    *bp++ = ',';
		for (; (c = *bits) > 32; bits++)
		    *bp++ = c;
	    }
	    else
		for (; *bits > 32; bits++)
		    continue;
	*bp++ = '>';
	*bp = (char)NULL;
    }

    return buffer;
}
