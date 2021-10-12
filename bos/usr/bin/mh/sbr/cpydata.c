static char sccsid[] = "@(#)46	1.5  src/bos/usr/bin/mh/sbr/cpydata.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:07";
/* 
 * COMPONENT_NAME: CMDMH cpydata.c
 * 
 * FUNCTIONS: MSGSTR, cpydata 
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
/* static char sccsid[] = "cpydata.c	7.1 87/10/13 17:04:16"; */

/* cpydata.c - copy from one fd to another */

#include "mh.h"
#include <stdio.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 

void cpydata (in, out, ifile, ofile)
register int    in,
                out;
register char  *ifile,
               *ofile;
{
    register int    i;
    char    buffer[BUFSIZ];

    while ((i = read (in, buffer, sizeof buffer)) > 0)
	if (write (out, buffer, i) != i)
	    adios (ofile, MSGSTR(WERR, "error writing %s"), ofile); /*MSG*/

    if (i == NOTOK)
	adios (ifile, MSGSTR(RERR, "error reading %s"), ifile); /*MSG*/
}
