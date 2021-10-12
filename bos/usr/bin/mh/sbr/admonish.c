static char sccsid[] = "@(#)35	1.5  src/bos/usr/bin/mh/sbr/admonish.c, cmdmh, bos411, 9428A410j 3/27/91 17:43:49";
/* 
 * COMPONENT_NAME: CMDMH admonish.c
 * 
 * FUNCTIONS: MSGSTR, admonish 
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
/* static char sccsid[] = "admonish.c	7.1 87/10/13 17:02:47"; */

/* admonish.c - admonish the user */

#include "mh.h"

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


/* VARARGS2 */

void admonish (what, fmt, a, b, c, d, e, f)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    char fmt2[NL_TEXTMAX];

    strcpy (fmt2, fmt);
    advertise (what, MSGSTR(CONT, "continuing..."), fmt2, a, b, c, d, e, f); /*MSG*/
}
