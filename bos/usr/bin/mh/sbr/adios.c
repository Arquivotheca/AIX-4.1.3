static char sccsid[] = "@(#)34	1.3  src/bos/usr/bin/mh/sbr/adios.c, cmdmh, bos411, 9428A410j 6/15/90 22:10:56";
/* 
 * COMPONENT_NAME: CMDMH adios.c
 * 
 * FUNCTIONS: adios 
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

/* adios.c - print out error message and exit */

#include "mh.h"


/* VARARGS2 */

void adios (what, fmt, a, b, c, d, e, f)
char   *what,
       *fmt,
       *a,
       *b,
       *c,
       *d,
       *e,
       *f;
{
    advise (what, fmt, a, b, c, d, e, f);
    done (1);
}
