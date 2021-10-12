static char sccsid[] = "@(#)47	1.6  src/bos/usr/bin/mh/sbr/cpydgst.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:09";
/* 
 * COMPONENT_NAME: CMDMH cpydgst.c
 * 
 * FUNCTIONS: MSGSTR, cpydgst, flush, output 
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
/* static char sccsid[] = "cpydgst.c	7.1 87/10/13 17:04:25"; */

/* cpydgst.c - copy from one fd to another in encapsulating mode */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


/* All we want to do is to perform the substitution

	\n(-.*)\n	-->	\n- \1\n

    we could use

	sed -e 's%^-%- -%' < ifile > ofile

    to do this, but the routine below is faster than the pipe, fork, and
    exec.
 */

#define	S1	0
#define	S2	1

#define	output(c)   if (bp >= dp) {flush(); *bp++ = c;} else *bp++ = c
#define	flush()	    if ((j = bp - outbuf) && write (out, outbuf, j) != j) \
			adios (ofile, MSGSTR(WERR, "error writing %s"), ofile); \
		    else \
			bp = outbuf /*MSG*/

/*  */

void cpydgst (in, out, ifile, ofile)
register int    in,
                out;
register char  *ifile,
               *ofile;
{
    register int    i,
                    state;
    register char  *cp,
                   *ep;
    char    buffer[BUFSIZ];
    register int    j;
    register char  *bp,
                   *dp;
    char    outbuf[BUFSIZ];

    dp = (bp = outbuf) + sizeof outbuf;
    for (state = S1; (i = read (in, buffer, sizeof buffer)) > 0;)
	for (ep = (cp = buffer) + i; cp < ep; cp++) {
	    if (*cp == (char)NULL)
		continue;
	    switch (state) {
		case S1: 
		    if (*cp == '-') {
			output ('-');
			output (' ');
		    }
		    state = S2;	/* fall */

		case S2: 
		    output (*cp);
		    if (*cp == '\n')
			state = S1;
		    break;
	    }
	}

    if (i == NOTOK)
	adios (ifile, MSGSTR(RERR, "error reading %s"), ifile); /*MSG*/
    flush ();
}
