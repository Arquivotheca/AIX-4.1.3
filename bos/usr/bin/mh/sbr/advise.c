static char sccsid[] = "@(#)37	1.3  src/bos/usr/bin/mh/sbr/advise.c, cmdmh, bos411, 9428A410j 6/15/90 22:11:08";
/* 
 * COMPONENT_NAME: CMDMH advise.c
 * 
 * FUNCTIONS: advise 
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

/* advise.c - print out error message */

#include "mh.h"


/* VARARGS2 */

void advise (what, fmt, a, b, c, d, e, f)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    advertise (what, NULLCP, fmt, a, b, c, d, e, f);
}
