static char sccsid[] = "@(#)42	1.6  src/bos/usr/bin/mh/sbr/concat.c, cmdmh, bos411, 9428A410j 3/27/91 17:44:04";
/* 
 * COMPONENT_NAME: CMDMH concat.c
 * 
 * FUNCTIONS: MSGSTR, concat 
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
/* static char sccsid[] = "concat.c	7.1 87/10/13 17:03:50"; */

/* concat.c - concatenate a bunch of strings in managed memory */

#include "mh.h"
#include <stdio.h>
#include <varargs.h>

#include "mh_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MH,n,s) 


/* VARARGS */

char   *concat (va_alist)
va_dcl
{
    register char  *cp,
                   *dp,
                   *sp;
    register unsigned   len;
    register    va_list list;

    len = 1;
    va_start (list); 
    while (cp = va_arg (list, char *))
	len += strlen (cp);
    va_end (list);

    dp = sp = (char *)malloc (len);
    if (dp == NULL)
	adios (NULLCP, MSGSTR(NOSTOR, "unable to allocate string storage")); /*MSG*/

    va_start (list); 
    while (cp = va_arg (list, char *))
	sp = copy (cp, sp);
    va_end (list);

    return dp;
}
